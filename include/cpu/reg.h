
#ifndef __RISCV32_REG_H__
#define __RISCV32_REG_H__

#include <basics.h>

static inline int check_reg_idx(int idx) {
  assert(idx >= 0 && idx < 32);
  return idx;
}

static inline int check_csr_idx(int idx) {
  assert(idx >= 0 && idx < 2048);
  return idx;
}

#define gpr(idx) cpu.gpr[check_reg_idx(idx)]
#define csrs(idx) cpu.csrs[check_csr_idx(idx)]

static inline const char* reg_name(int idx, int width) {
  extern const char* regs[];
  return regs[check_reg_idx(idx)];
}

#endif
