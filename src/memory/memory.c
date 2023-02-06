#include <mainmem.h>
#include <cpu.h>

static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};

uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}

static void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD, addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
}

void init_mem() {
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
}

word_t paddr_read(paddr_t addr, int len) {
  if (likely(in_pmem(addr))) return pmem_read(addr, len);
  return mmio_read(addr, len);
}

void paddr_write(paddr_t addr, int len, word_t data) {
  if (likely(in_pmem(addr))) { pmem_write(addr, len, data); return; }
  mmio_write(addr, len, data); return;
}

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type);

word_t vaddr_ifetch(vaddr_t addr, int len) {
  paddr_t raddr = isa_mmu_translate(addr, len, 0); 
  return paddr_read(raddr, len);
}

word_t vaddr_read(vaddr_t addr, int len) {
  paddr_t raddr = isa_mmu_translate(addr, len, 0);
  return paddr_read(raddr, len);
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  paddr_t raddr = isa_mmu_translate(addr, len, 1); 
  paddr_write(raddr, len, data);
}