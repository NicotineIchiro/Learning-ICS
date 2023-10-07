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
#include <ctype.h>
enum {
  TK_NOTYPE = 256, TK_EQ, TK_NEQ, TK_AND,

  /* TODO: Add more token types */
	TK_NUM, TK_HEX, TK_REG, TK_DEREF
};
const uint32_t VAL_ERREXPR = (uint32_t)(-1);
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
	{"!=", TK_NEQ},
	{"&&", TK_AND},
	{"0x[[:xdigit:]]+", TK_HEX},
	{"\\$(\\$0|[[:alnum:]]{2,3})", TK_REG},//BRE or ERE?
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
#define TOKEN_LEN 32
typedef struct token {
  int type;
  char str[TOKEN_LEN];
} Token;
#define TOKENS_NUM 1024
static Token tokens[TOKENS_NUM] __attribute__((used)) = {};
//static int nr_token = 0;
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;
	for (int i = 0; strlen(tokens[i].str) != 0; ++i)
		memset(tokens[i].str, 0, TOKEN_LEN);

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        //Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",i, rules[i].regex, position, substr_len, substr_len, substr_start);
				Assert(substr_len < TOKEN_LEN, "Single token buf overflow!\n");
        position += substr_len;

        /*  Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
				Assert(nr_token < TOKENS_NUM, "Number of token exceeds capacity!\n");

				//TODO: Some new token type.
        switch (rules[i].token_type) {
					case TK_NOTYPE:
						break;
					case '*':	
					case '-':
						if (nr_token == 0 || tokens[nr_token-1].type == '(' || tokens[nr_token-1].type == '/' || tokens[nr_token-1].type == '+' || tokens[nr_token-1].type == '*' || tokens[nr_token-1].type == '-') {
							//TODO: Ignoring minus list.
							strncpy(tokens[nr_token].str, substr_start, substr_len);
							tokens[nr_token].type = rules[i].token_type == '-' ? TK_NUM : TK_DEREF;
							nr_token++;
							break;							
						}
					case '(': case ')':  case '/': case '+': case TK_EQ: case TK_NEQ: case TK_AND:
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token].type = rules[i].token_type;
						nr_token++;
						break;
					case TK_NUM:
						//Assert(substr_len < TOKEN_LEN, "Single token buffer overflow!\n");
						//Single of is treated in in front of switch.
						//if in front of num there is a NUM type '-', cat the num after the '-'
						if (nr_token > 0 && strcmp(tokens[nr_token-1].str, "-") == 0 && tokens[nr_token-1].type == TK_NUM) {
							strncat(tokens[nr_token-1].str, substr_start, substr_len);
							//nr_token++;
							break;		
						}
					case TK_REG:
						//TODO..
					case TK_HEX:
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token].type = rules[i].token_type;
						nr_token++;
						break;
						
						break;
          default:
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
static bool be_flag = false;
static bool check_parentheses(uint32_t p, uint32_t q) {
	if (tokens[p].type != '(' || tokens[q].type != ')')
		return false;
	//TODO... How to treat the bad parentheses expr?
	bool outside_match = true;
	int pth_deep = 0;
	for (uint32_t i = p; i <= q; ++i) {
		switch (tokens[i].type) {
			case '(':
				if (i != p && \
						(tokens[i-1].type == TK_NUM || tokens[i-1].type == ')')) {
					be_flag = true;
					Log("Error: <Num> \'(\' bad expression.");
					return false;
				}
				pth_deep++;
				break;
			case ')':
				pth_deep--;
				if (pth_deep < 0 || \
					 (tokens[i-1].type != TK_NUM && tokens[i-1].type != ')')) {
					be_flag = true;
					Log("Error: Brackets unmatched bad expression.");
					return false;//BAD EXPR
				}
				if (i != q && pth_deep == 0)
					outside_match = false;
				break;
			default:
				continue;	
		}
	}

	return outside_match;
		
} 
static word_t eval(uint32_t p, uint32_t q) {
	//end eval when meet illegal expr.
	if (p > q) {
		be_flag = true;
		Log("Error: Basic bad expression.");
		return VAL_ERREXPR;
	}
	else if (p == q) { 
		//terminology?
		if (tokens[p].type == TK_NUM)
			return atoi(tokens[p].str);
		else
			return (word_t)strtoul(tokens[p].str, NULL, 16);
	}
	else if (check_parentheses(p, q) == true) {
		return eval(p + 1, q - 1);
	}
	else {
		//parentheses unmatch || no parenthses but legal
		//get main op;
		if (be_flag)
		{
			//Log("Error: c_p detect BE, hence do no treat to the expr.\n");
			return VAL_ERREXPR;
		}
		//TODO:Multiple char op?
		int main_opi = -1;
		//size_t token_len;
		//TODO... token buf OF
		int pth_deep = 0;
		
		for (uint32_t i = p; i <= q; ++i) {
			switch(tokens[i].type) {
				case '(':
					pth_deep++;
					break;
				case ')':
					pth_deep--;
					break;	
				case '*': case '/':
					if (pth_deep != 0) 
						continue;
	
					if (main_opi == -1) {
						main_opi = i;
					}
					else {
						switch (tokens[main_opi].type) {
							case '*': case '/':
								main_opi = i;
								break;
							default:
								break;
						}
					}
					break;
				case '+': case '-':
					if (pth_deep != 0) 
						continue;
					if (main_opi == -1) {
						main_opi = i;
						break;
					}

					switch (tokens[main_opi].type) {
						case '*': case '/': case '+': case '-':
							main_opi = i;
							break;
						default:
							break;
					}
					break;
				case TK_AND:
					if (pth_deep != 0)
						continue;
					if (main_opi == -1) {
						main_opi = i;
						break;
					}
					switch (tokens[main_opi].type) {
						case '*': case '/': case '+': case '-': case TK_AND:
							main_opi = i;
							break;
						default:
							break;
					}
					break;
				case TK_EQ: case TK_NEQ:
					if (pth_deep != 0)
						continue;
					main_opi = i;
					break;
				default:
					continue;
			}
		}
		//TODO: reg and deref
		word_t val1 = eval(p, main_opi - 1);
		word_t val2 = eval(main_opi + 1, q);
		//if (be_flag)
		switch (tokens[main_opi].type) {
			case TK_EQ: return val1 == val2;
			case TK_NEQ: return val1 != val2;
			case TK_AND: return val1 && val2;
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
	//2:eval(0, arrlen(tokens));
	be_flag = false;
	word_t result = eval(0, nr_token - 1);
	if (be_flag) {
		*success	= false;
		//Log("Error: Bad expression.");
		return (word_t)VAL_ERREXPR;
	}
	//TODO: How to detect OF?
	//printf("Value == %lu\n", result);
	return result;
		
    //TODO();

  //return 0;
}
