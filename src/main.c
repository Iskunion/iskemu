#include <basics.h>

void engine_start();
void init_monitor(int argc, char **argv);

NEMUState nemu_state = { .state = NEMU_STOP };

int is_exit_status_bad() {
  int good = (nemu_state.state == NEMU_END && nemu_state.halt_ret == 0) ||
    (nemu_state.state == NEMU_QUIT);
  return !good;
}

int main(int argc, char *argv[]) {

  init_monitor(argc, argv);

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
