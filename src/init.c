#include <cpu.h>
#include <mainmem.h>

// this is not consistent with uint8_t
// but it is ok since we do not access the array directly
static const uint32_t img [] = {
  0x800002b7,  // lui t0,0x80000
  0x0002a023,  // sw  zero,0(t0)
  0x0002a503,  // lw  a0,0(t0)
  0x00100073,  // ebreak (used as nemu_trap)
  0x45264433,
  0x637A7A79,
  0x5375694A,
  0x65746174,
  0x20202049,
  0x45564F4C,
  0x71797920
};

static void restart() {
  cpu.pc = RESET_VECTOR;
  cpu.gpr[0] = 0;
  cpu.csrs[MSTATUS] = 0x1800;
}

void init_isa() {
  memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));
  restart();
}

void idb_mainloop();

void engine_start() {
  idb_mainloop();
}