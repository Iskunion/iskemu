
#include <basics.h>
#include <cpu.h>

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
  printf("NOW PC: "ANSI_FMT("0x%08x\n", ANSI_FG_CYAN), cpu.pc);
  for (int i = 0; i < 4; i++) printf(" REG     HEX         DEC     ");
  putchar('\n');
    for (int i = 0; i < 32; i++){
      printf("%3s: 0x%08x %12d ", regs[i], gpr(i), gpr(i));
      if ((i & 3) == 3)
        putchar('\n');
    }
  printf("CSRS:\n");
  printf("mstatus: \t0x%08x\n", csrs(MSTATUS));
  printf("mtvec:   \t0x%08x\n", csrs(MTVEC));
  printf("mepc:    \t0x%08x\n", csrs(MEPC));
  printf("mcause:  \t0x%08x\n", csrs(MCAUSE));
  printf("satp:    \t0x%08x\n", csrs(SATP));
}