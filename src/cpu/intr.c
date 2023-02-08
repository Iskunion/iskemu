#include <cpu.h>

extern int is_batch_mode;

void set_nemu_state(int state, vaddr_t pc, int halt_ret);
word_t isa_raise_intr(word_t NO, vaddr_t epc) {

  Log("Received interrupt %d at 0x%08x", NO, epc);

  csrs(MEPC) = (NO == IRQ_TIMER) ? (epc) : (epc + 4);
  csrs(MCAUSE) = NO;
  (csrs(MSTATUS) & MIE) ? (csrs(MSTATUS) |= MPIE) : (csrs(MSTATUS) &= ~(MPIE));
  csrs(MSTATUS) &= ~(MIE);

  // if (!is_batch_mode) {
  //   if (NO == -1)
  //     set_nemu_state(NEMU_STOP, epc, 0);
  // }

  return csrs(MTVEC);
}

word_t isa_query_intr() {
  if (cpu.intr && (csrs(MSTATUS) & MIE)) {
    cpu.intr = false;
    return IRQ_TIMER;
  }
  return INTR_EMPTY;
}