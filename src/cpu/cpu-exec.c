#include <cpu.h>
#include <basics.h>

#define MAX_INST_TO_PRINT 25

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

static char iringbuf[CONFIG_IRINGBUF_SIZE][128];
int irb_begin, irb_end;

void device_update();
void check_wp();

void iringbuf_init() {
  irb_begin = irb_end = 0;
  return ;
}
void iringbuf_display() {
  if (irb_begin == irb_end) {
    Warn("Iringbuf is empty!");
    return ;
  }
  Plain_log("Iringbuf Status:\n");
  if (irb_begin < irb_end)
    for (int i = irb_begin; i < irb_end; i++)
      Plain_log("%s\n", iringbuf[i]);
  else{
    for (int i = irb_begin; i < CONFIG_IRINGBUF_SIZE; i++)
      Plain_log("%s\n", iringbuf[i]);
    for (int i = 0; i < irb_end; i++)
      Plain_log("%s\n", iringbuf[i]);
  }
  Plain_log("End iringbuf status\n");
  return ;
}
static void iringbuf_write(char *s) {
  strncpy(iringbuf[irb_end], s, 128);
  (++irb_end >= CONFIG_IRINGBUF_SIZE) ? (irb_end = 0) : (0);
  if (irb_begin == irb_end || (irb_end - irb_begin) == CONFIG_IRINGBUF_SIZE)
    ((++irb_begin) >= CONFIG_IRINGBUF_SIZE) ? (irb_begin = 0) : (0);
}

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
  log_write("%s\n", _this->logbuf);
  iringbuf_write(_this->logbuf);
  if (g_print_step) puts(_this->logbuf);
  check_wp();
}

static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  isa_exec_once(s);
  cpu.pc = s->dnpc;

  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  uint8_t *inst = (uint8_t *)&s->isa.inst.val;
  for (int i = 3; i >= 0; i--) 
    p += snprintf(p, 4, " %02x", inst[i]);
  
  // memset(p, ' ', 2); p += 2;
  // void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  // disassemble(p, s->logbuf + sizeof(s->logbuf) - p, s->pc, (uint8_t *)&s->isa.inst.val, ilen);
}

static void execute(uint64_t n) {
  Decode s;
  for (;n > 0; n --) {
    exec_once(&s, cpu.pc);
    g_nr_guest_inst ++;
  #ifdef CONFIG_DEBUG
    trace_and_difftest(&s, cpu.pc);
  #endif
    if (nemu_state.state != NEMU_RUNNING) break;
    device_update();
    word_t intr = isa_query_intr();
    if (intr != INTR_EMPTY) cpu.pc = isa_raise_intr(intr, cpu.pc);
  }
}

static void statistic() {
#define NUMBERIC_FMT "%'" PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
  isa_reg_display();
  iringbuf_display();
  statistic();
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();
  execute(n);
  uint64_t timer_end = get_time();

  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING:
      nemu_state.state = NEMU_STOP; break;

    case NEMU_STOP:
      Log("nemu: %s at pc = " FMT_WORD, ANSI_FMT("HIT BRKPOINT", ANSI_FG_MAGENTA), cpu.pc);
      break;
    
    case NEMU_END: case NEMU_ABORT:
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT: statistic();
  }
}
