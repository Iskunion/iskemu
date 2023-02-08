
#include <cpu.h>
#include <mainmem.h>
#include <readline/history.h>
#include <readline/readline.h>
#include "include/idb.h"

int is_batch_mode = false;

void init_regex();
void init_wp_pool();

static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(iskemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_si(char *args) {
  char *number = strtok(NULL, " ");

  if (!number) {
    cpu_exec(1);
    return 0;
  }

  uint64_t step = atoll(number);
  if(!step){
    Warn("Invaild times specified, do nothing.");
    return 0;
  }
  cpu_exec(step);
  return 0;
}

void iringbuf_display();
static int cmd_info(char *args) {
  if (args == NULL){
    Warn("No argument specified!");
    return 0;
  }
  char *subcmd = strtok(NULL, " ");
  if (strcmp(subcmd, "r") == 0) {
    isa_reg_display();
  }
  else if(strcmp(subcmd, "w") == 0) {
    watchpoint_display();
  }
  else if(strcmp(subcmd, "i") == 0) {
    iringbuf_display();
  }
  else {
     Warn("Invaild subcmd!");
  }
  return 0;
}


static int cmd_w(char *args) {
  if (args == NULL){
    Warn("No argument specified!");
    return 0;
  }
  char *e = strtok(NULL, "");
  watchpoint_add(e);
  printf("Create the watchpoint: %s.\n", e);
  return 0;
}

static int cmd_d(char *args) {
  if (args == NULL){
    Warn("No argument specified!");
    return 0;
    
  }
  char *number = strtok(NULL, " ");
  if (!number) {
    Warn("No watchpoint specified!");
    return 0;
  }
  uint64_t num = atoll(number);
  if(!num) {
    Warn("Invaild watchpoint specified.");
    return 0;
  }
  watchpoint_erase_byList(num);
  return 0;
}

word_t paddr_read(paddr_t addr, int len);

static int cmd_x(char *args) {
  int success = 0;
  char *subaddr = strtok(NULL, "");

  if (!subaddr) {
    Warn("No Address Specified!");
    return 0;
  }

  char *exp = malloc(sizeof(char) * 200);
  int cnt = 1;
  int scanfres = sscanf(subaddr, "%s%d", exp, &cnt);
  word_t res = expr(exp, &success);

  free(exp);

  if (success < 0) {
    Warn("Paring Address failure!");
    return 0;
  }
  
  if (scanfres < 2) cnt = 1;
  
  puts("   ADDR    \t   BYTE         HEX         DEC      ASCII");

  while(cnt--){
    printf("0x%08x:\t%02x %02x %02x %02x  ", \
    res, paddr_read(res, 1), paddr_read(res + 1, 1), paddr_read(res + 2, 1), paddr_read(res + 3, 1));
    
    printf("%08xH %12d  ", paddr_read(res, 4), paddr_read(res, 4));

    putchar(Turnascii(paddr_read(res, 1)));
    putchar(Turnascii(paddr_read(res+1, 1)));
    putchar(Turnascii(paddr_read(res+2, 1)));
    putchar(Turnascii(paddr_read(res+3, 1)));

    putchar('\n');

    res += 4;
  }

  return 0;
}

static int cmd_p(char *args) {
  if (args == NULL){
    Warn("No argument specified!");
    return 0;
  }
  int success = 0;
  word_t result = expr(args, &success);
  if (success < 0) {
    Warn("Parsing failure!");
  }
  else {
    printf("The result is: %d\n", result);
  }
  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *usage;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "help", "Display information about all supported commands", cmd_help },
  { "c", "c", "Continue the execution of the program", cmd_c },
  { "q", "q", "Exit ISKEMU", cmd_q },
  { "si", "si [TIMES]?", "Execute single instruction TIMES times", cmd_si },
  { "info", "info (r | w | i)", "Print specified state", cmd_info },
  { "x", "x EXPR [N]?", "Scan the N groups of 4 bytes memory started from the value specified by EXPR", cmd_x },
  { "w", "w EXPR", "Watch the expression specified by EXPR", cmd_w },
  { "d", "d N", "Delete the n-th watch point", cmd_d },
  { "p", "p EXPR",  "Print the value of EXPR", cmd_p }
  /* TODO: Add more commands */
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - Usage: %s\n    " ANSI_FMT("%s.\n", ANSI_FG_CYAN), cmd_table[i].name ,cmd_table[i].usage, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - Usage: %s\n    " ANSI_FMT("%s.\n", ANSI_FG_CYAN), cmd_table[i].name ,cmd_table[i].usage, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void idb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_idb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
