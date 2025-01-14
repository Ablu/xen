/******************************************************************************
 * arch/x86/mm/shadow/types.h
 *
 * Parts of this code are Copyright (c) 2006 by XenSource Inc.
 * Parts of this code are Copyright (c) 2006 by Michael A Fetterman
 * Parts based on earlier work by Michael A Fetterman, Ian Pratt et al.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _XEN_SHADOW_TYPES_H
#define _XEN_SHADOW_TYPES_H

/* The number of levels in the shadow pagetable is entirely determined
 * by the number of levels in the guest pagetable */
#if GUEST_PAGING_LEVELS != 2
#define SHADOW_PAGING_LEVELS GUEST_PAGING_LEVELS
#else
#define SHADOW_PAGING_LEVELS 3
#endif

/*
 * Define various types for handling pagetabels, based on these options:
 * SHADOW_PAGING_LEVELS : Number of levels of shadow pagetables
 * GUEST_PAGING_LEVELS  : Number of levels of guest pagetables
 */

#if SHADOW_PAGING_LEVELS == 3
#define SHADOW_L1_PAGETABLE_ENTRIES     512
#define SHADOW_L2_PAGETABLE_ENTRIES     512
#define SHADOW_L3_PAGETABLE_ENTRIES       4
#define SHADOW_L1_PAGETABLE_SHIFT        12
#define SHADOW_L2_PAGETABLE_SHIFT        21
#define SHADOW_L3_PAGETABLE_SHIFT        30
#elif SHADOW_PAGING_LEVELS == 4
#define SHADOW_L1_PAGETABLE_ENTRIES     512
#define SHADOW_L2_PAGETABLE_ENTRIES     512
#define SHADOW_L3_PAGETABLE_ENTRIES     512
#define SHADOW_L4_PAGETABLE_ENTRIES     512
#define SHADOW_L1_PAGETABLE_SHIFT        12
#define SHADOW_L2_PAGETABLE_SHIFT        21
#define SHADOW_L3_PAGETABLE_SHIFT        30
#define SHADOW_L4_PAGETABLE_SHIFT        39
#endif

/* Types of the shadow page tables */
typedef l1_pgentry_t shadow_l1e_t;
typedef l2_pgentry_t shadow_l2e_t;
typedef l3_pgentry_t shadow_l3e_t;
typedef l4_pgentry_t shadow_l4e_t;

/* Access functions for them */
static inline paddr_t shadow_l1e_get_paddr(shadow_l1e_t sl1e)
{ return l1e_get_paddr(sl1e); }
static inline paddr_t shadow_l2e_get_paddr(shadow_l2e_t sl2e)
{ return l2e_get_paddr(sl2e); }
static inline paddr_t shadow_l3e_get_paddr(shadow_l3e_t sl3e)
{ return l3e_get_paddr(sl3e); }
static inline paddr_t shadow_l4e_get_paddr(shadow_l4e_t sl4e)
{ return l4e_get_paddr(sl4e); }

static inline mfn_t shadow_l1e_get_mfn(shadow_l1e_t sl1e)
{ return l1e_get_mfn(sl1e); }
static inline mfn_t shadow_l2e_get_mfn(shadow_l2e_t sl2e)
{ return l2e_get_mfn(sl2e); }
static inline mfn_t shadow_l3e_get_mfn(shadow_l3e_t sl3e)
{ return l3e_get_mfn(sl3e); }
static inline mfn_t shadow_l4e_get_mfn(shadow_l4e_t sl4e)
{ return l4e_get_mfn(sl4e); }

static inline u32 shadow_l1e_get_flags(shadow_l1e_t sl1e)
{ return l1e_get_flags(sl1e); }
static inline u32 shadow_l2e_get_flags(shadow_l2e_t sl2e)
{ return l2e_get_flags(sl2e); }
static inline u32 shadow_l3e_get_flags(shadow_l3e_t sl3e)
{ return l3e_get_flags(sl3e); }
static inline u32 shadow_l4e_get_flags(shadow_l4e_t sl4e)
{ return l4e_get_flags(sl4e); }

static inline shadow_l1e_t
shadow_l1e_remove_flags(shadow_l1e_t sl1e, u32 flags)
{ l1e_remove_flags(sl1e, flags); return sl1e; }
static inline shadow_l1e_t
shadow_l1e_flip_flags(shadow_l1e_t sl1e, u32 flags)
{ l1e_flip_flags(sl1e, flags); return sl1e; }

static inline shadow_l1e_t shadow_l1e_empty(void)
{ return l1e_empty(); }
static inline shadow_l2e_t shadow_l2e_empty(void)
{ return l2e_empty(); }
static inline shadow_l3e_t shadow_l3e_empty(void)
{ return l3e_empty(); }
static inline shadow_l4e_t shadow_l4e_empty(void)
{ return l4e_empty(); }

static inline shadow_l1e_t shadow_l1e_from_mfn(mfn_t mfn, u32 flags)
{ return l1e_from_mfn(mfn, flags); }
static inline shadow_l2e_t shadow_l2e_from_mfn(mfn_t mfn, u32 flags)
{ return l2e_from_mfn(mfn, flags); }
static inline shadow_l3e_t shadow_l3e_from_mfn(mfn_t mfn, u32 flags)
{ return l3e_from_mfn(mfn, flags); }
static inline shadow_l4e_t shadow_l4e_from_mfn(mfn_t mfn, u32 flags)
{ return l4e_from_mfn(mfn, flags); }

#define shadow_l1_table_offset(a) l1_table_offset(a)
#define shadow_l2_table_offset(a) l2_table_offset(a)
#define shadow_l3_table_offset(a) l3_table_offset(a)
#define shadow_l4_table_offset(a) l4_table_offset(a)

/**************************************************************************/
/* Access to the linear mapping of shadow page tables. */

/* Offsets into each level of the linear mapping for a virtual address. */
#define shadow_l1_linear_offset(_a)                                           \
        (((_a) & VADDR_MASK) >> SHADOW_L1_PAGETABLE_SHIFT)
#define shadow_l2_linear_offset(_a)                                           \
        (((_a) & VADDR_MASK) >> SHADOW_L2_PAGETABLE_SHIFT)
#define shadow_l3_linear_offset(_a)                                           \
        (((_a) & VADDR_MASK) >> SHADOW_L3_PAGETABLE_SHIFT)
#define shadow_l4_linear_offset(_a)                                           \
        (((_a) & VADDR_MASK) >> SHADOW_L4_PAGETABLE_SHIFT)

/* Where to find each level of the linear mapping.  For PV guests, we use
 * the shadow linear-map self-entry as many times as we need.  For HVM
 * guests, the shadow doesn't have a linear-map self-entry so we must use
 * the monitor-table's linear-map entry N-1 times and then the shadow-map
 * entry once. */
#define __sh_linear_l1_table ((shadow_l1e_t *)(SH_LINEAR_PT_VIRT_START))
#define __sh_linear_l2_table ((shadow_l2e_t *)                               \
    (__sh_linear_l1_table + shadow_l1_linear_offset(SH_LINEAR_PT_VIRT_START)))

// shadow linear L3 and L4 tables only exist in 4 level paging...
#if SHADOW_PAGING_LEVELS == 4
#define __sh_linear_l3_table ((shadow_l3e_t *)                               \
    (__sh_linear_l2_table + shadow_l2_linear_offset(SH_LINEAR_PT_VIRT_START)))
#define __sh_linear_l4_table ((shadow_l4e_t *)                               \
    (__sh_linear_l3_table + shadow_l3_linear_offset(SH_LINEAR_PT_VIRT_START)))
#endif

#define sh_linear_l1_table(v) ({ \
    ASSERT(current == (v)); \
    __sh_linear_l1_table; \
})

// XXX -- these should not be conditional on is_hvm_vcpu(v), but rather on
//        shadow_mode_external(d)...
//
#define sh_linear_l2_table(v) ({ \
    ASSERT(current == (v)); \
    ((shadow_l2e_t *) \
     (is_hvm_vcpu(v) ? __linear_l1_table : __sh_linear_l1_table) + \
     shadow_l1_linear_offset(SH_LINEAR_PT_VIRT_START)); \
})

#if SHADOW_PAGING_LEVELS >= 4
#define sh_linear_l3_table(v) ({ \
    ASSERT(current == (v)); \
    ((shadow_l3e_t *) \
     (is_hvm_vcpu(v) ? __linear_l2_table : __sh_linear_l2_table) + \
      shadow_l2_linear_offset(SH_LINEAR_PT_VIRT_START)); \
})

// we use l4_pgentry_t instead of shadow_l4e_t below because shadow_l4e_t is
// not defined for when xen_levels==4 & shadow_levels==3...
#define sh_linear_l4_table(v) ({ \
    ASSERT(current == (v)); \
    ((l4_pgentry_t *) \
     (is_hvm_vcpu(v) ? __linear_l3_table : __sh_linear_l3_table) + \
      shadow_l3_linear_offset(SH_LINEAR_PT_VIRT_START)); \
})
#endif

 /* Override get_gfn to work with gfn_t */
#undef get_gfn_query
#define get_gfn_query(d, g, t) get_gfn_type((d), gfn_x(g), (t), 0)

/* The shadow types needed for the various levels. */

#if GUEST_PAGING_LEVELS == 2
#define SH_type_l1_shadow  SH_type_l1_32_shadow
#define SH_type_l2_shadow  SH_type_l2_32_shadow
#define SH_type_fl1_shadow SH_type_fl1_32_shadow
#elif GUEST_PAGING_LEVELS == 3
#define SH_type_l1_shadow  SH_type_l1_pae_shadow
#define SH_type_fl1_shadow SH_type_fl1_pae_shadow
#define SH_type_l2_shadow  SH_type_l2_pae_shadow
#elif GUEST_PAGING_LEVELS == 4
#define SH_type_l1_shadow  SH_type_l1_64_shadow
#define SH_type_fl1_shadow SH_type_fl1_64_shadow
#define SH_type_l2_shadow  SH_type_l2_64_shadow
#define SH_type_l2h_shadow SH_type_l2h_64_shadow
#define SH_type_l3_shadow  SH_type_l3_64_shadow
#define SH_type_l4_shadow  SH_type_l4_64_shadow
#endif

#if GUEST_PAGING_LEVELS

/* macros for dealing with the naming of the internal function names of the
 * shadow code's external entry points.
 */
#define INTERNAL_NAME(name) SHADOW_INTERNAL_NAME(name, GUEST_PAGING_LEVELS)

/* macros for renaming the primary entry points, so that they are more
 * easily distinguished from a debugger
 */
#define sh_page_fault              INTERNAL_NAME(sh_page_fault)
#define sh_invlpg                  INTERNAL_NAME(sh_invlpg)
#define sh_gva_to_gfn              INTERNAL_NAME(sh_gva_to_gfn)
#define sh_update_cr3              INTERNAL_NAME(sh_update_cr3)
#define sh_rm_write_access_from_l1 INTERNAL_NAME(sh_rm_write_access_from_l1)
#define sh_rm_mappings_from_l1     INTERNAL_NAME(sh_rm_mappings_from_l1)
#define sh_remove_l1_shadow        INTERNAL_NAME(sh_remove_l1_shadow)
#define sh_remove_l2_shadow        INTERNAL_NAME(sh_remove_l2_shadow)
#define sh_remove_l3_shadow        INTERNAL_NAME(sh_remove_l3_shadow)
#define sh_map_and_validate_gl4e   INTERNAL_NAME(sh_map_and_validate_gl4e)
#define sh_map_and_validate_gl3e   INTERNAL_NAME(sh_map_and_validate_gl3e)
#define sh_map_and_validate_gl2e   INTERNAL_NAME(sh_map_and_validate_gl2e)
#define sh_map_and_validate_gl2he  INTERNAL_NAME(sh_map_and_validate_gl2he)
#define sh_map_and_validate_gl1e   INTERNAL_NAME(sh_map_and_validate_gl1e)
#define sh_destroy_l4_shadow       INTERNAL_NAME(sh_destroy_l4_shadow)
#define sh_destroy_l3_shadow       INTERNAL_NAME(sh_destroy_l3_shadow)
#define sh_destroy_l2_shadow       INTERNAL_NAME(sh_destroy_l2_shadow)
#define sh_destroy_l1_shadow       INTERNAL_NAME(sh_destroy_l1_shadow)
#define sh_unhook_l2_mappings      INTERNAL_NAME(sh_unhook_l2_mappings)
#define sh_unhook_l4_mappings      INTERNAL_NAME(sh_unhook_l4_mappings)
#define sh_paging_mode             INTERNAL_NAME(sh_paging_mode)
#define sh_audit_l1_table          INTERNAL_NAME(sh_audit_l1_table)
#define sh_audit_fl1_table         INTERNAL_NAME(sh_audit_fl1_table)
#define sh_audit_l2_table          INTERNAL_NAME(sh_audit_l2_table)
#define sh_audit_l3_table          INTERNAL_NAME(sh_audit_l3_table)
#define sh_audit_l4_table          INTERNAL_NAME(sh_audit_l4_table)
#define sh_guess_wrmap             INTERNAL_NAME(sh_guess_wrmap)
#define sh_clear_shadow_entry      INTERNAL_NAME(sh_clear_shadow_entry)

#if SHADOW_OPTIMIZATIONS & SHOPT_OUT_OF_SYNC
#define sh_resync_l1               INTERNAL_NAME(sh_resync_l1)
#define sh_safe_not_to_sync        INTERNAL_NAME(sh_safe_not_to_sync)
#define sh_rm_write_access_from_sl1p INTERNAL_NAME(sh_rm_write_access_from_sl1p)
#endif

#endif /* GUEST_PAGING_LEVELS */

#if SHADOW_PAGING_LEVELS == 3
#define MFN_FITS_IN_HVM_CR3(_MFN) !(mfn_x(_MFN) >> 20)
#endif

#define SH_PRI_pte  PRIpte
#define SH_PRI_gpte PRI_gpte
#define SH_PRI_gfn  PRI_gfn

int shadow_set_l1e(struct domain *d, shadow_l1e_t *sl1e,
                   shadow_l1e_t new_sl1e, p2m_type_t new_type,
                   mfn_t sl1mfn);
int shadow_set_l2e(struct domain *d, shadow_l2e_t *sl2e,
                   shadow_l2e_t new_sl2e, mfn_t sl2mfn,
                   unsigned int type_fl1_shadow,
                   mfn_t (*next_page)(mfn_t smfn));
int shadow_set_l3e(struct domain *d, shadow_l3e_t *sl3e,
                   shadow_l3e_t new_sl3e, mfn_t sl3mfn);
int shadow_set_l4e(struct domain *d, shadow_l4e_t *sl4e,
                   shadow_l4e_t new_sl4e, mfn_t sl4mfn);

static void inline
shadow_put_page_from_l1e(shadow_l1e_t sl1e, struct domain *d)
{
    if ( !shadow_mode_refcounts(d) )
        return;

    put_page_from_l1e(sl1e, d);
}

#if (SHADOW_OPTIMIZATIONS & SHOPT_FAST_FAULT_PATH)
/******************************************************************************
 * We implement a "fast path" for two special cases: faults that require
 * MMIO emulation, and faults where the guest PTE is not present.  We
 * record these as shadow l1 entries that have reserved bits set in
 * them, so we can spot them immediately in the fault handler and handle
 * them without needing to hold the paging lock or walk the guest
 * pagetables.
 *
 * This is only feasible for PAE and 64bit Xen: 32-bit non-PAE PTEs don't
 * have reserved bits that we can use for this.  And even there it can only
 * be used if we can be certain the processor doesn't use all 52 address bits.
 *
 * For the MMIO encoding (see below) we need the bottom 4 bits for
 * identifying the kind of entry and a full GFN's worth of bits to encode
 * the originating frame number.  Set all remaining bits to trigger
 * reserved bit faults, if (see above) the hardware permits triggering such.
 */

#ifdef CONFIG_BIGMEM
# define SH_L1E_MAGIC_MASK (0xfffff00000000000UL | _PAGE_PRESENT)
#else
# define SH_L1E_MAGIC_MASK (0xfffffff000000000UL | _PAGE_PRESENT)
#endif

static inline bool sh_have_pte_rsvd_bits(void)
{
    return paddr_bits < PADDR_BITS && !cpu_has_hypervisor;
}

static inline bool sh_l1e_is_magic(shadow_l1e_t sl1e)
{
    BUILD_BUG_ON(!(PADDR_MASK & PAGE_MASK & SH_L1E_MAGIC_MASK));
    return (sl1e.l1 & SH_L1E_MAGIC_MASK) == SH_L1E_MAGIC_MASK;
}

/* Guest not present: a single magic value */
static inline shadow_l1e_t sh_l1e_gnp_raw(void)
{
    return (shadow_l1e_t){ -1ULL };
}

static inline shadow_l1e_t sh_l1e_gnp(void)
{
    /*
     * On systems with no reserved physical address bits we can't engage the
     * fast fault path.
     */
    return sh_have_pte_rsvd_bits() ? sh_l1e_gnp_raw()
                                   : shadow_l1e_empty();
}

static inline bool sh_l1e_is_gnp(shadow_l1e_t sl1e)
{
    return sl1e.l1 == sh_l1e_gnp_raw().l1;
}

/*
 * MMIO: an invalid PTE that contains the GFN of the equivalent guest l1e.
 * We store the GFN in bits 4:35 (BIGMEM: 4:43) of the entry.
 * The present bit is set, and the U/S and R/W bits are taken from the guest.
 * Bit 3 is always 0, to differentiate from gnp above.
 */
#define SH_L1E_MMIO_MAGIC       SH_L1E_MAGIC_MASK
#define SH_L1E_MMIO_MAGIC_BIT   8
#define SH_L1E_MMIO_MAGIC_MASK  (SH_L1E_MMIO_MAGIC | SH_L1E_MMIO_MAGIC_BIT)
#define SH_L1E_MMIO_GFN_MASK    ~(SH_L1E_MMIO_MAGIC_MASK | _PAGE_RW | _PAGE_USER)

static inline shadow_l1e_t sh_l1e_mmio(gfn_t gfn, u32 gflags)
{
    unsigned long gfn_val = MASK_INSR(gfn_x(gfn), SH_L1E_MMIO_GFN_MASK);
    shadow_l1e_t sl1e = { (SH_L1E_MMIO_MAGIC | gfn_val |
                           (gflags & (_PAGE_USER | _PAGE_RW))) };

    BUILD_BUG_ON(SH_L1E_MMIO_MAGIC_BIT <= _PAGE_RW);
    BUILD_BUG_ON(SH_L1E_MMIO_MAGIC_BIT <= _PAGE_USER);

    if ( !sh_have_pte_rsvd_bits() ||
         (cpu_has_bug_l1tf &&
          !is_l1tf_safe_maddr(shadow_l1e_get_paddr(sl1e))) )
        sl1e = shadow_l1e_empty();

    return sl1e;
}

static inline bool sh_l1e_is_mmio(shadow_l1e_t sl1e)
{
    return (sl1e.l1 & SH_L1E_MMIO_MAGIC_MASK) == SH_L1E_MMIO_MAGIC;
}

static inline gfn_t sh_l1e_mmio_get_gfn(shadow_l1e_t sl1e)
{
    return _gfn(MASK_EXTR(sl1e.l1, SH_L1E_MMIO_GFN_MASK));
}

static inline uint32_t sh_l1e_mmio_get_flags(shadow_l1e_t sl1e)
{
    return sl1e.l1 & (_PAGE_USER | _PAGE_RW);
}

#else

#define sh_l1e_gnp() shadow_l1e_empty()
#define sh_l1e_mmio(_gfn, _flags) shadow_l1e_empty()
#define sh_l1e_is_magic(_e) (0)

#endif /* SHOPT_FAST_FAULT_PATH */


#endif /* _XEN_SHADOW_TYPES_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
