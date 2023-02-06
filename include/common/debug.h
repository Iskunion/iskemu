
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "common.h"
#include "utils.h"
#include <stdio.h>


#define Log(format, ...) \
    _Log(ANSI_FMT("[%s:%d %s] " format, ANSI_FG_BLUE) "\n", \
        __FILE__, __LINE__, __func__, ## __VA_ARGS__)

#define Plain_log(format, ...) \
    _Log(format, ## __VA_ARGS__)

#define Warn(format, ...) \
    _Log(ANSI_FMT("WARNING: [%s] " format, ANSI_FG_YELLOW) "\n", \
        __func__, ## __VA_ARGS__)

#define Turnascii(number) \
    ((number > 32 && number < 127) ? number : ' ')

#define Assert(cond, format, ...) \
  do { \
    if (!(cond)) { \
      fflush(stdout); \
      fprintf(stderr, ANSI_FMT(format, ANSI_FG_RED) "\n", ##  __VA_ARGS__);\
      extern FILE* log_fp; \
      fflush(log_fp); \
      extern void assert_fail_msg(); \
      assert_fail_msg(); \
      assert(cond); \
    } \
  } while (0)

#define panic(format, ...) Assert(0, format, ## __VA_ARGS__)

#endif
