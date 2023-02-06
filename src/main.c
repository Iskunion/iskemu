#include <basics.h>

int is_exit_status_bad();

int main(int argc, char *argv[]) {

  init_monitor(argc, argv);

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
