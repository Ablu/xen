/*
 *  i386 helpers (without register variable usage)
 * 
 *  Copyright (c) 2003 Fabrice Bellard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * Main cpu loop for handling I/O requests coming from a virtual machine
 * Copyright � 2004, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307 USA.
 */
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <signal.h>
#include <assert.h>

#include <limits.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <xenctrl.h>
#include <xen/io/ioreq.h>
#include <xen/linux/evtchn.h>

#include "cpu.h"
#include "exec-all.h"
#include "vl.h"

extern int domid;

void *shared_vram;

shared_iopage_t *shared_page = NULL;
extern int reset_requested;

CPUX86State *cpu_86_init(void)
{
    CPUX86State *env;
    static int inited;

    cpu_exec_init();

    env = malloc(sizeof(CPUX86State));
    if (!env)
        return NULL;
    memset(env, 0, sizeof(CPUX86State));
    /* init various static tables */
    if (!inited) {
        inited = 1;
    }
    cpu_single_env = env;
    cpu_reset(env);
    return env;
}

/* NOTE: must be called outside the CPU execute loop */
void cpu_reset(CPUX86State *env)
{
}

void cpu_x86_close(CPUX86State *env)
{
    free(env);
}


void cpu_dump_state(CPUState *env, FILE *f, 
                    int (*cpu_fprintf)(FILE *f, const char *fmt, ...),
                    int flags)
{
}

/***********************************************************/
/* x86 mmu */
/* XXX: add PGE support */

void cpu_x86_set_a20(CPUX86State *env, int a20_state)
{
    a20_state = (a20_state != 0);
    if (a20_state != ((env->a20_mask >> 20) & 1)) {
#if defined(DEBUG_MMU)
        printf("A20 update: a20=%d\n", a20_state);
#endif
        env->a20_mask = 0xffefffff | (a20_state << 20);
    }
}

target_ulong cpu_get_phys_page_debug(CPUState *env, target_ulong addr)
{
        return addr;
}

//the evtchn fd for polling
int evtchn_fd = -1;
//the evtchn port for polling the notification, should be inputed as bochs's parameter
u16 ioreq_remote_port, ioreq_local_port;

//some functions to handle the io req packet
void
sp_info()
{
	ioreq_t *req;

	req = &(shared_page->vcpu_iodata[0].vp_ioreq);
        term_printf("event port: %d\n", shared_page->sp_global.eport);
        term_printf("req state: %x, pvalid: %x, addr: %llx, data: %llx, count: %llx, size: %llx\n", req->state, req->pdata_valid, req->addr, req->u.data, req->count, req->size);
}

//get the ioreq packets from share mem
ioreq_t* __cpu_get_ioreq(void)
{
	ioreq_t *req;

	req = &(shared_page->vcpu_iodata[0].vp_ioreq);
	if (req->state == STATE_IOREQ_READY) {
		req->state = STATE_IOREQ_INPROCESS;
	} else {
		fprintf(logfile, "False I/O request ... in-service already: %x, pvalid: %x,port: %llx, data: %llx, count: %llx, size: %llx\n", req->state, req->pdata_valid, req->addr, req->u.data, req->count, req->size);
		req = NULL;
	}

	return req;
}

//use poll to get the port notification
//ioreq_vec--out,the 
//retval--the number of ioreq packet
ioreq_t* cpu_get_ioreq(void)
{
	int rc;
	u16 buf[2];
	rc = read(evtchn_fd, buf, 2);
	if (rc == 2 && buf[0] == ioreq_local_port){//got only one matched 16bit port index
		// unmask the wanted port again
		write(evtchn_fd, &ioreq_local_port, 2);

		//get the io packet from shared memory
		return __cpu_get_ioreq();
	}

	//read error or read nothing
	return NULL;
}

unsigned long
do_inp(CPUState *env, unsigned long addr, unsigned long size)
{
	switch(size) {
	case 1:
		return cpu_inb(env, addr);
	case 2:
		return cpu_inw(env, addr);
	case 4:
		return cpu_inl(env, addr);
	default:
		fprintf(logfile, "inp: bad size: %lx %lx\n", addr, size);
		exit(-1);
	}
}

void
do_outp(CPUState *env, unsigned long addr, unsigned long size, 
        unsigned long val)
{
	switch(size) {
	case 1:
		return cpu_outb(env, addr, val);
	case 2:
		return cpu_outw(env, addr, val);
	case 4:
		return cpu_outl(env, addr, val);
	default:
		fprintf(logfile, "outp: bad size: %lx %lx\n", addr, size);
		exit(-1);
	}
}

extern void cpu_physical_memory_rw(target_phys_addr_t addr, uint8_t *buf, 
                                   int len, int is_write);

static inline void
read_physical(u64 addr, unsigned long size, void *val)
{
        return cpu_physical_memory_rw((target_phys_addr_t)addr, val, size, 0);
}

static inline void
write_physical(u64 addr, unsigned long size, void *val)
{
        return cpu_physical_memory_rw((target_phys_addr_t)addr, val, size, 1);
}

void
cpu_ioreq_pio(CPUState *env, ioreq_t *req)
{
	int i, sign;

	sign = req->df ? -1 : 1;

	if (req->dir == IOREQ_READ) {
		if (!req->pdata_valid) {
			req->u.data = do_inp(env, req->addr, req->size);
		} else {
			unsigned long tmp; 

			for (i = 0; i < req->count; i++) {
				tmp = do_inp(env, req->addr, req->size);
				write_physical((target_phys_addr_t) req->u.pdata
						+ (sign * i * req->size), 
					req->size, &tmp);
			}
		}
	} else if (req->dir == IOREQ_WRITE) {
		if (!req->pdata_valid) {
			do_outp(env, req->addr, req->size, req->u.data);
		} else {
			for (i = 0; i < req->count; i++) {
				unsigned long tmp;

				read_physical((target_phys_addr_t) req->u.pdata
						+ (sign * i * req->size),
					req->size, &tmp);
				do_outp(env, req->addr, req->size, tmp);
			}
		}
	}
}

void
cpu_ioreq_move(CPUState *env, ioreq_t *req)
{
	int i, sign;

	sign = req->df ? -1 : 1;

	if (!req->pdata_valid) {
		if (req->dir == IOREQ_READ) {
			for (i = 0; i < req->count; i++) {
				read_physical(req->addr
						+ (sign * i * req->size),
					req->size, &req->u.data);
			}
		} else if (req->dir == IOREQ_WRITE) {
			for (i = 0; i < req->count; i++) {
				write_physical(req->addr
						+ (sign * i * req->size),
					req->size, &req->u.data);
			}
		}
	} else {
		unsigned long tmp;

		if (req->dir == IOREQ_READ) {
			for (i = 0; i < req->count; i++) {
				read_physical(req->addr
						+ (sign * i * req->size),
					req->size, &tmp);
				write_physical((target_phys_addr_t )req->u.pdata
						+ (sign * i * req->size),
					req->size, &tmp);
			}
		} else if (req->dir == IOREQ_WRITE) {
			for (i = 0; i < req->count; i++) {
				read_physical((target_phys_addr_t) req->u.pdata
						+ (sign * i * req->size),
					req->size, &tmp);
				write_physical(req->addr
						+ (sign * i * req->size),
					req->size, &tmp);
			}
		}
	}
}

void
cpu_ioreq_and(CPUState *env, ioreq_t *req)
{
	unsigned long tmp1, tmp2;

	if (req->pdata_valid != 0)
		hw_error("expected scalar value");

	read_physical(req->addr, req->size, &tmp1);
	if (req->dir == IOREQ_WRITE) {
		tmp2 = tmp1 & (unsigned long) req->u.data;
		write_physical(req->addr, req->size, &tmp2);
	}
	req->u.data = tmp1;
}

void
cpu_ioreq_or(CPUState *env, ioreq_t *req)
{
	unsigned long tmp1, tmp2;

	if (req->pdata_valid != 0)
		hw_error("expected scalar value");

	read_physical(req->addr, req->size, &tmp1);
	if (req->dir == IOREQ_WRITE) {
		tmp2 = tmp1 | (unsigned long) req->u.data;
		write_physical(req->addr, req->size, &tmp2);
	}
	req->u.data = tmp1;
}

void
cpu_ioreq_xor(CPUState *env, ioreq_t *req)
{
	unsigned long tmp1, tmp2;

	if (req->pdata_valid != 0)
		hw_error("expected scalar value");

	read_physical(req->addr, req->size, &tmp1);
	if (req->dir == IOREQ_WRITE) {
		tmp2 = tmp1 ^ (unsigned long) req->u.data;
		write_physical(req->addr, req->size, &tmp2);
	}
	req->u.data = tmp1;
}

void
cpu_handle_ioreq(CPUState *env)
{
	ioreq_t *req = cpu_get_ioreq();

	if (req) {
		if ((!req->pdata_valid) && (req->dir == IOREQ_WRITE)) {
			if (req->size != 4)
				req->u.data &= (1UL << (8 * req->size))-1;
		}

		switch (req->type) {
		case IOREQ_TYPE_PIO:
			cpu_ioreq_pio(env, req);
			break;
		case IOREQ_TYPE_COPY:
			cpu_ioreq_move(env, req);
			break;
		case IOREQ_TYPE_AND:
			cpu_ioreq_and(env, req);
			break;
		case IOREQ_TYPE_OR:
			cpu_ioreq_or(env, req);
			break;
		case IOREQ_TYPE_XOR:
			cpu_ioreq_xor(env, req);
			break;
		default:
			hw_error("Invalid ioreq type 0x%x", req->type);
		}

		/* No state change if state = STATE_IORESP_HOOK */
		if (req->state == STATE_IOREQ_INPROCESS)
			req->state = STATE_IORESP_READY;
		env->send_event = 1;
	}
}

void
cpu_timer_handler(CPUState *env)
{
	cpu_handle_ioreq(env);
}

int xc_handle;

static __inline__ void atomic_set_bit(long nr, volatile void *addr)
{
        __asm__ __volatile__(
                "lock ; bts %1,%0"
                :"=m" (*(volatile long *)addr)
                :"dIr" (nr));
}

void
do_interrupt(CPUState *env, int vector)
{
	unsigned long *intr;

	// Send a message on the event channel. Add the vector to the shared mem
	// page.
	intr = (unsigned long *) &(shared_page->sp_global.pic_intr[0]);
	atomic_set_bit(vector, intr);
        if (loglevel & CPU_LOG_INT)
                fprintf(logfile, "injecting vector: %x\n", vector);
	env->send_event = 1;
}

void
destroy_vmx_domain(void)
{
    extern FILE* logfile;
    char destroy_cmd[20];
    sprintf(destroy_cmd, "xm destroy %d", domid);
    if (system(destroy_cmd) == -1)
        fprintf(logfile, "%s failed.!\n", destroy_cmd);
}

int main_loop(void)
{
	int vector;
 	fd_set rfds;
	struct timeval tv;
	extern CPUState *global_env;
        extern int vm_running;
        extern int shutdown_requested;
	CPUState *env = global_env;
	int retval;
        extern void main_loop_wait(int);

 	/* Watch stdin (fd 0) to see when it has input. */
	FD_ZERO(&rfds);

	while (1) {
                if (vm_running) {
                    if (shutdown_requested) {
                        break;
                    }
                    if (reset_requested){
                        qemu_system_reset();
                        reset_requested = 0;
                    }
                }

		/* Wait up to one seconds. */
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		FD_SET(evtchn_fd, &rfds);

		env->send_event = 0;
		retval = select(evtchn_fd+1, &rfds, NULL, NULL, &tv);
		if (retval == -1) {
			perror("select");
			return 0;
		}

#if __WORDSIZE == 32
#define ULONGLONG_MAX   0xffffffffffffffffULL
#else
#define ULONGLONG_MAX   ULONG_MAX
#endif

		main_loop_wait(0);
#ifdef APIC_SUPPORT
		ioapic_update_EOI();
#endif
		cpu_timer_handler(env);
		if (env->interrupt_request & CPU_INTERRUPT_HARD) {
                        env->interrupt_request &= ~CPU_INTERRUPT_HARD;
			vector = cpu_get_pic_interrupt(env); 
			do_interrupt(env, vector);
		}
#ifdef APIC_SUPPORT
		if (ioapic_has_intr())
                    do_ioapic();
#endif
		if (env->send_event) {
			struct ioctl_evtchn_notify notify;
			notify.port = ioreq_local_port;
			(void)ioctl(evtchn_fd, IOCTL_EVTCHN_NOTIFY, &notify);
		}
	}
        destroy_vmx_domain();
	return 0;
}

static void
qemu_vmx_reset(void *unused)
{
    char cmd[255];

    /* pause domain first, to avoid repeated reboot request*/ 
    xc_domain_pause (xc_handle, domid);

    sprintf(cmd,"xm shutdown -R %d", domid);
    system (cmd);
}

CPUState *
cpu_init()
{
	CPUX86State *env;
	struct ioctl_evtchn_bind_interdomain bind;
	int rc;
      
        cpu_exec_init();
        qemu_register_reset(qemu_vmx_reset, NULL);
	env = malloc(sizeof(CPUX86State));
	if (!env)
		return NULL;
	memset(env, 0, sizeof(CPUX86State));

	cpu_single_env = env;

	if (evtchn_fd != -1)//the evtchn has been opened by another cpu object
		return NULL;

	//use nonblock reading not polling, may change in future.
	evtchn_fd = open("/dev/xen/evtchn", O_RDWR|O_NONBLOCK); 
	if (evtchn_fd == -1) {
		perror("open");
		return NULL;
	}

	bind.remote_domain = domid;
	bind.remote_port   = ioreq_remote_port;
	rc = ioctl(evtchn_fd, IOCTL_EVTCHN_BIND_INTERDOMAIN, &bind);
	if (rc == -1) {
		perror("ioctl");
		return NULL;
	}
	ioreq_local_port = rc;

	return env;
}

