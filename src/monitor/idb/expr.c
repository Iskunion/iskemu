
#include "include/idb.h"
#include <regex.h>

enum {
  TK_NOTYPE = 256,
  TK_EQ,
  TK_AND,
  TK_OR,
  TK_LOGICAND,
  TK_LOGICOR,
  TK_XOR,
  TK_LOGICNOT,
  TK_NOT,
  TK_QUESTION,
  TK_COLON,
  TK_LEFTPARENTHESES,
  TK_RIGHTPARENTHESES,
  TK_DEC,
  TK_HEX,
  TK_PLUS,
  TK_MINUS,
  TK_TIMES,
  TK_DIVIDE,
  TK_MOD,
  TK_DOLLAR,
  TK_NAME,
  TK_NEQ,
  TK_COMMA
  /* TODO: Add more token types */
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_PLUS},         // plus
  {"==", TK_EQ},        // equal
  {"&&", TK_LOGICAND},
  {"\\|\\|", TK_LOGICOR},
  {"!=", TK_NEQ},
  {"-", TK_MINUS},
  {"\\*", TK_TIMES},
  {"/", TK_DIVIDE},
  {"\\(", TK_LEFTPARENTHESES},
  {"\\)", TK_RIGHTPARENTHESES},
  {"\\$", TK_DOLLAR},
  {"\\b[a-zA-Z_]+\\w*\\b", TK_NAME},
  {"\\b0x[0-9a-fA-F]+\\b", TK_HEX},
  {"\\b[0-9]+\\b", TK_DEC},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }

}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[256] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

enum {
  EX_ERR_TOKEN_INVAILD = -128,
  EX_ERR_TOKEN_INFLOW,
  EX_ERR_INVALID_REG,
  EX_ERR_INVALID_EXP,
  EX_ERR_INVALID_DIV0,
  EX_ERR_INVALID_NOTBNF
};

static int make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

//        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
//            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          case TK_NAME:
          case TK_HEX:
          case TK_DEC:
          case TK_PLUS:
          case TK_MINUS:
          case TK_DIVIDE:
          case TK_TIMES:
          case TK_DOLLAR:
          case TK_LOGICAND:
          case TK_LOGICOR:
          case TK_LEFTPARENTHESES:
          case TK_RIGHTPARENTHESES:
          case TK_EQ:
          case TK_NEQ:
            if(substr_len > 30)
              return EX_ERR_TOKEN_INFLOW;
            memset(tokens[nr_token].str, '\0', sizeof tokens[nr_token].str);
            memcpy(tokens[nr_token].str, substr_start, substr_len * sizeof(char));
            tokens[nr_token++].type=rules[i].token_type;
            break;
          default: 
            return EX_ERR_TOKEN_INVAILD;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return EX_ERR_TOKEN_INVAILD;
    }
  }

  return 0;
}

static word_t dec10(const char *num);
static word_t hex16(const char *num);

static void print_tokens() {
  // Log("Get Tokens:");
  // for (int i = 0; i < nr_token; i++) {
  //   Log("Tokens %d: %s", i, tokens[i].str);
  //   if (tokens[i].type == TK_DEC || tokens[i].type == TK_HEX)
  //       Log("Number Parsed: %u", tokens[i].type == TK_DEC ? dec10(tokens[i].str) : hex16(tokens[i].str));
  // }
  return ;
}

enum {
  PRIORITY_UNDEF,
  PRIORITY_PARENT,
  PRIORITY_LOGICOR,
  PRIORITY_LOGICAND,
  PRIORITY_EQNEQ,
  PRIORITY_ADDSUB,
  PRIORITY_DIVMUL,
  PRIORITY_SINGAL
};

inline int Priority(int type) {
  switch (type)
  {
    case TK_MINUS:
    case TK_PLUS:
      return PRIORITY_ADDSUB;
    case TK_TIMES:
    case TK_DIVIDE:
      return PRIORITY_DIVMUL;
    case TK_NEQ:
    case TK_EQ:
      return PRIORITY_EQNEQ;
    case TK_LOGICAND:
      return PRIORITY_LOGICAND;
    case TK_LOGICOR:
      return PRIORITY_LOGICOR;
    default:
      return PRIORITY_UNDEF;
  }
  return PRIORITY_UNDEF;
}

word_t hex16(const char *num){
  word_t res;
  sscanf(num, "%x", &res);
  return res;
}

word_t dec10(const char *num){
  word_t res;
  sscanf(num, "%u", &res);
  return res;
}

extern const char *regs[32];

word_t parse_reg(const char *name, bool *success) {
  int i = 0;
  *success = true;
  if (strcmp("pc", name) == 0) {
    return cpu.pc;
  }
  for (; i < 32; i++) {
    if (strcmp(regs[i], name) == 0){
      return cpu.gpr[i];
    }
  }
  if (i == 32)
    *success = false;
  return 0;
}

word_t parse_mem(paddr_t address) {
  return paddr_read(address, 4);
}

#define CHK_SUCCESS do{if (*success < 0) return 0;}while(0)
#define CHK_NOTEND do{ if (begin + 1 == nr_token) {*success = EX_ERR_INVALID_EXP; return 0; } }while(0)

word_t parse_exp(int begin, int priority, int *success, int *endpos);

word_t get_leftblock(int begin, int *success, int *endpos) {
  word_t res;
  *success = 0;
  if (begin == nr_token){
    *success = EX_ERR_INVALID_EXP;
    return 0;
  }
  switch (tokens[begin].type) {
    case TK_MINUS:
      CHK_NOTEND;
      res = get_leftblock(begin + 1, success, endpos);
      CHK_SUCCESS;
      return -res;
    case TK_TIMES:
      CHK_NOTEND;
      res = get_leftblock(begin + 1, success, endpos);
      CHK_SUCCESS;
      return parse_mem(res);
    case TK_DOLLAR:
      CHK_NOTEND;
      if (tokens[begin + 1].type == TK_DOLLAR) {
        ++begin;
        CHK_NOTEND;
        if (tokens[begin + 1].type == TK_DEC && !dec10(tokens[begin + 1].str) && strlen(tokens[begin + 1].str) == 1) {
          res = cpu.gpr[0];
          *endpos = begin + 2;
          return res;
        }
        else {
          *success = EX_ERR_INVALID_REG;
          return 0;
        }
      }
      if (tokens[begin + 1].type == TK_DEC && !dec10(tokens[begin + 1].str) && strlen(tokens[begin + 1].str) == 1) {
        res = cpu.gpr[0];
        *endpos = begin + 2;
        return res;
      }
      if (tokens[begin + 1].type != TK_NAME) {
        *success = EX_ERR_INVALID_REG;
        return 0;
      }
      bool isreg;
      res = parse_reg(tokens[begin + 1].str, &isreg);
      if (!isreg) {
        *success = EX_ERR_INVALID_REG;
        return 0;
      }
      *endpos = begin + 2;
      return res;
    case TK_HEX:
      res = hex16(tokens[begin].str);
      *endpos = begin + 1;
      return res;
    case TK_DEC:
      res = dec10(tokens[begin].str);
      *endpos = begin + 1;
      return res;
    case TK_LEFTPARENTHESES:
      CHK_NOTEND;
      res = parse_exp(begin + 1, PRIORITY_PARENT, success, endpos);
      CHK_SUCCESS;
      begin = *endpos - 1;
      CHK_NOTEND;
      if (tokens[begin + 1].type != TK_RIGHTPARENTHESES) {
        *success = EX_ERR_INVALID_NOTBNF;
        return 0;
      }
      *endpos = begin + 2;
      return res;
    default:
      break;
  }
  *success = EX_ERR_INVALID_EXP;
  return 0;
}

word_t parse_exp(int begin, int priority, int *success, int *endpos)
{
  word_t res = get_leftblock(begin, success, endpos);
  CHK_SUCCESS;
  while (*endpos < nr_token && Priority(tokens[*endpos].type) > priority) {
      int type = tokens[*endpos].type;
      word_t tmp = parse_exp(*endpos+1, Priority(type), success, endpos);
      CHK_SUCCESS;
      switch (type)
      {
      case TK_PLUS:
        res += tmp;
        break;
      case TK_MINUS:
        res -= tmp;
        break;
      case TK_DIVIDE:
        if (!tmp) {
          *success = EX_ERR_INVALID_DIV0;
          return 0;
        }
        res /= tmp;
        break;
      case TK_TIMES:
        res *= tmp;
        break;
      case TK_NEQ:
        res = (res != tmp);
        break;
      case TK_EQ:
        res = (res == tmp);
        break;
      case TK_LOGICAND:
        res = (res && tmp);
        break;
      case TK_LOGICOR:
        res = (res || tmp);
        break;
      default:
        if (!tmp) {
          *success = EX_ERR_INVALID_EXP;
          return 0;
        }
        break;
      }
  }
  return res;
}

word_t expr(char *e, int *success) {
  int res = make_token(e);
  if (res < 0)
    { *success = res; return 0; }
  print_tokens();
  /* TODO: Insert codes to evaluate the expression. */
  
  int endpos = 0;
  word_t result = parse_exp(0, PRIORITY_PARENT, success, &endpos);

  if (*success < 0)
    return 0;

  if (endpos < nr_token) {
    *success = EX_ERR_INVALID_NOTBNF;
    return 0;
  }
     
  return result;
}
