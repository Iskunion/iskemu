
#ifndef __IDB_H__
#define __IDB_H__

#include <basics.h>
#include <cpu.h>
#include <mainmem.h>

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr[256];
  word_t last;
  /* TODO: Add more members if necessary */
} WP;

word_t expr(char *e, int *success);

void isa_reg_display();
void watchpoint_display();
void watchpoint_add(char *e);
void watchpoint_erase_byNo(int No);
void watchpoint_erase_byList(int cnt);
word_t watchpoint_evaluate(WP *wp, int *succ, bool *changed);

#endif
