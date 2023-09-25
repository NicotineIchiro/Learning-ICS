/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <string.h>
#include <debug.h>
enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
	TK_NUM,
};
enum {
	BE_ERREXPR = 512,
	IN_PARMATCH,
};
static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},    // spaces  <- Why add an plus? <- one or more space
	{"\\n", TK_NOTYPE},
	{"\\(", '('},
	{"\\)", ')'},
	{"\\*", '*'},
	{"/", '/'},
  {"\\+", '+'},         // plus
	{"\\-", '-'},
  {"==", TK_EQ},        // equal
	{"[[:digit:]]+", TK_NUM},
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

static Token tokens[32] __attribute__((used)) = {};
//static int nr_token = 0;
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
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

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
				Assert(nr_token < 32, "Number of token exceeds capacity!\n");
        switch (rules[i].token_type) {
					case TK_NOTYPE:
						break;
					case '(': case ')': case '*': case '/': case '+': case '-': case TK_EQ:
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token].type = rules[i].token_type;
						nr_token++;
						break;
					case TK_NUM:
						Assert(substr_len < 32, "Single token buffer overflow!\n");
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token].type = rules[i].token_type;
						nr_token++;
						//TODO: buffer overflow treat
						break;
          default: //TODO();
						i = NR_REGEX - 1;
						break;
        }

        break;
      }
    }
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
	}
	//for (int j = 0; j < 32; ++j)
		//printf("%s ", tokens[j].str);

  return true;
}

static bool check_parentheses(uint32_t p, uint32_t q) {
	if (tokens[p].type != '(' || tokens[q].type != ')')
		return false;
	//TODO... How to treat the bad parentheses expr?
	int pth_deep = 0;

		
}
static uint32_t eval(uint32_t p, uint32_t q) {
	//end eval when meet illegal expr.
	if (p > q) {
		return BE_ERREXPR;
		//when bad expression, end prog;
	}
	else if (p == q) {
		//terminology?
		return atoi(tokens[p].str);
	}
	else if (check_parentheses(p, q) == true) {
		return eval(p + 1, q - 1);
	}
	else {
		//TODO...
		//parentheses unmatch || no parenthses but legal
		//get main op;
		int main_opi = -1;
		size_t token_len;
		int pth_deep = 0;
		for (uint32_t i = p; i < q; ++i) {
			switch(tokens[i].type) {
				case '(':
					pth_deep++;
					break;
				case ')':
					pth_deep--;
					break;
				case '*': case '/':
					if (pth_deep != 0) continue;
	
					if (main_opi == -1 || tokens[main_opi].type == '*' || \
						 	tokens[main_opi].type == '/') {
						main_opi = i;
					}
					break;
				case '+': case '-':
					if (pth_deep != 0) continue;

						main_opi = i;
					break;
				default:
					continue;
			}
		}

		uint32_t val1 = eval(p, main_opi - 1);
		uint32_t val2 = eval(main + 1, q);

		switch (tokens[main_opi].type) {
			case '+':	return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': return val1 / val2;
			default: Assert(0, "No legal main operator!\n");
		}
	}
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
	//1:for token in tokens
	//	eval(token, 0, strlen(token.str))
	//
	//2:eval(0, arrlen(tokens));
  TODO();

  return 0;
}
