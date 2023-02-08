
#include "include/idb.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

static WP* new_wp() {
  if (free_ == NULL) {
    Warn("Warning: There is no free watchpoint!");
    return NULL;
  }
  WP *ret = free_;
  free_ = free_->next;
  ret->next = head;
  head = ret;
  return ret;
}

static void free_wp(WP *wp) {
  if (wp == NULL) {
    Warn("A Null WP detected!");
    return ;
  }
  WP *pre = head;
  for (; pre != NULL; pre = pre->next) {
    if (pre->next == wp) break;
  }
  if (pre == NULL && wp != head) {
    Warn("No such watchpoint when freeing!");
    return ;
  }
  WP *nxt = wp->next;
  
  //delete wp in head
  //head can not be null here which is guaranteed
  if (wp == head)
    head = nxt;
  else pre->next = nxt;

  //insert wp in free_
  wp->next = free_;
  free_ = wp;
  
  return ;
}

void watchpoint_display(){
  int wp_cnt = 0;
  if (head == NULL) {
    puts("No watchpoints now!");
    return ;
  }
  for (WP *now = head; now != NULL; now = now->next) {
    ++wp_cnt;
    printf("Watchpoint %3d with No. %3d:\n", wp_cnt, now->NO);
    printf("  Expression %s\n", now->expr);
    int success = 0;
    word_t ret = expr(now->expr, &success);
    if (success < 0)
      printf("  Now value: " ANSI_FMT("failed to evaluate\n", ANSI_FG_RED));
    else
      printf("  Now value: %d\n", ret);
  }
  return ;
}

word_t watchpoint_evaluate(WP *wp, int *succ, bool *changed){
  word_t ret;
  int dummysucc = 0;
  if (succ != NULL) {
    ret = expr(wp->expr, succ);
    if (*succ < 0)
      return 0;
  }
  else ret = expr(wp->expr, &dummysucc);
  
  if (ret != wp->last && changed != NULL)
    *changed = true;
  wp->last = ret;
  return ret;
}

void watchpoint_add(char *e) {
  WP *newwp = new_wp();
  memset(newwp->expr, 0, sizeof (newwp->expr));
  strcpy(newwp->expr, e);
  watchpoint_evaluate(newwp, NULL, NULL);
  return ;
}

void watchpoint_erase_byNo(int No) {
  int wp_cnt = 0;
  if (head == NULL) {
    puts("No watchpoints now!");
    return ;
  }
  WP *now = head;
  for (; now != NULL; now = now->next) {
    ++wp_cnt;
    if (now->NO == No) {
      printf("Watchpoint %3d with No. %3d: %s erased.\n", wp_cnt, now->NO, now->expr);
      free_wp(now);
      return ;
    }
  }
  if (now == NULL)
    Warn("Invaild watchpoint specified.");
  return ;
}

void watchpoint_erase_byList(int cnt) {
  int wp_cnt = 0;
  if (head == NULL) {
    puts("No watchpoints now!");
    return ;
  }
  WP *now = head;
  for (; now != NULL; now = now->next) {
    ++wp_cnt;
    if (wp_cnt == cnt){
      printf("Watchpoint %3d with No. %3d: %s erased.\n", wp_cnt, now->NO, now->expr);
      free_wp(now);
      return ;
    }
  }
  if (now == NULL)
    Warn("Invaild watchpoint specified.");
  return ;
}

void check_wp(){
  WP *now = head;
  int cnt = 0;
  for (; now != NULL; now = now->next) {
    bool changed = false;
    ++cnt;
    word_t last = now->last;
    word_t now_value = watchpoint_evaluate(now, NULL, &changed);
    if (changed){
      nemu_state.state = NEMU_STOP;
      printf(ANSI_FMT("The watchpoint %d with No: %d changed from the original value %d to %d, stop the program.\n", ANSI_FG_GREEN), cnt, now->NO, last, now_value);
    }
  }
  return ;
}