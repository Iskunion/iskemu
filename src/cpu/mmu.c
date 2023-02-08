#include <cpu.h>
#include <mainmem.h>

#define exp2(x)         (1u << (x))
#define PGW             (12)
#define PGSIZE          exp2(PGW)
#define PGMASK          BITMASK(PGW)
#define PGPN(addr)      ((addr) >> (PGW))
#define PGOFF(addr)     ((addr) & (PGMASK))
#define PGSTART(pn)     ((paddr_t) ((pn) << (PGW)))

#define T0SZ            (10)
#define T1SZ            (10)

#define VPN_1(vaddr)  BITS(vaddr, (PGW + T0SZ + T1SZ - 1), (PGW + T0SZ))
#define VPN_0(vaddr)  BITS(vaddr, (PGW + T0SZ - 1), PGW)

typedef uint32_t PTE;

#define PTE_V 0x01
#define PTE_R 0x02
#define PTE_W 0x04
#define PTE_X 0x08
#define PTE_U 0x10
#define PTE_A 0x40
#define PTE_D 0x80

#define PTE_PPN(ppn)    ((uint32_t) (ppn) << 10)
#define PPN(pte)        ((uint32_t) (pte) >> 10)

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  //check state
  if (isa_mmu_check(vaddr, len, type) == MMU_DIRECT) return vaddr; 
  // Log("Paging Opened!");

  //L1 page table
  paddr_t p_target_pde = PGSTART(csrs(SATP));
  p_target_pde += sizeof(PTE) * VPN_1(vaddr);

  word_t target_pde = paddr_read(p_target_pde, sizeof(PTE));

  //L2 page table
  paddr_t p_target_pte;
  if (target_pde & PTE_V)
    p_target_pte = PGSTART(PPN(target_pde));
  else {panic("L1 miss: 0x%08x", vaddr);}

  p_target_pte += sizeof(PTE) * VPN_0(vaddr);

  word_t target_pte = paddr_read(p_target_pte, sizeof(PTE));

  //Return physical address
  if (target_pte & PTE_V)
    return PGSTART(PPN(target_pte)) | PGOFF(vaddr);
  else {panic("L2 miss: 0x%08x", vaddr);}
}