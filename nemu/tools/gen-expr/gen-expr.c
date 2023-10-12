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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  uint64_t result = %s; "
"  printf(\"%%lu\", result); "
"  return 0; "
"}";

static int buf_i = 0;
static int parth_depth = 0;
static int NUM_LEN = 5;
static int BUF_MAX = 50;
static uint32_t choose(uint32_t n) {
	uint32_t result;
	result = rand() % n;
	//while ((result = rand() % n) == 0);
	return result;
}
static int BUF_FULL() { return buf_i >= BUF_MAX; }
static int remain_buf() {
	return BUF_MAX - buf_i - 1;
}
static int buf_enough() {
	//6 -> one op and max NUM_LEN of num 5
	return remain_buf() > parth_depth + 1 + NUM_LEN;
}
static void set_buf_end()
{
	buf_i++;
	buf[buf_i] = '\0';
}
static void gen_rand_op() {
	if (BUF_FULL())	return;

	//printf("gen_op() ");
	int i;
	i = rand() % 4;//TODO: Now ignore "=="
	char op_list[] = {'*', '/', '+', '-'};
	
//	if (i < 4) {
		buf[buf_i] = op_list[i];
		set_buf_end();
//	}
//	else {
//		buf[buf_i] = '=';
	//printf("buf_i: %d\n", buf_i);

	return;
}
//static int emrgency_end = 0;
static void gen(char ch) {
	if (BUF_FULL()) return;
	assert(ch == '(' || ch == ')');

	buf[buf_i] = ch;
//	if (ch == ')' && emrgency_end == 1)
//		return;
	parth_depth += (ch == '(' ? 1 : -1);

	set_buf_end();
	//buf_i++;
	//buf[buf_i] = '\0';

	return;
}
static void gen_num() {
	if (BUF_FULL()) return;
	//printf("gen_num() ");
	//const int UINT64_MAXL = 18;
	int rand_len;
	while ((rand_len = choose(NUM_LEN)) == 0);
	for (int i = 0; i < rand_len; ++i){
		char rand_nc;
		rand_nc = '0' + choose(10);
					
		buf[buf_i] = rand_nc;
		
		buf_i++;
	}
	while (buf[buf_i - rand_len] == '0' && rand_len > 1) {
		buf[buf_i - rand_len] = choose(10) + '0';
	}

	//while (rand_len == 1 && buf[buf_i-1] == '0' && buf[buf_i-2] == '/');
	//printf("buf_i: %d\n", buf_i);
	buf[buf_i]	= '\0';

	return;
}
//static int emergency_return = 0;
static int rec_depth = 0;
//static void rec_indent(int n)
//{
//	for (int i = 1; i <= n; ++i)
//		printf("  ");
//}

static void gen_rand_expr() {
  //buf[0] = '\0';
	//TODO: The end of expr must be NUM or matched right parth
	//			to avoid bad expression.
	int i;
	i = buf_enough() ? choose(3) : 0;	
	rec_depth++;

	// Debug Log
	//char * BNF[] = {"<NUM>", "\'(\'<expr>\')\'", "<expr> op <expr>"};
	//rec_indent(rec_depth);
	//printf("%s in depth %d\n", BNF[i], rec_depth);
	// Debug Log
	
	//TODO: random insert space
	switch (i) {
		case 0:	
			gen_num(); break;
		case 1:
			gen('(');
			gen_rand_expr(); 
			gen(')');
//		while (!buf_enough() && parth_depth > 0) {
//				gen(')');
//				if (parth_depth == 0)
//					emrgency_end = 1;
//			}
			break;
		default: 
			gen_rand_expr();
			if (!buf_enough())
				break;

			gen_rand_op(); 
			gen_rand_expr();
			break;
	}

	rec_depth--;
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();
		buf_i = 0;
		//printf("buf: %s\n", buf);
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);


    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    //int ret = system("gcc /tmp/.code.c -o /tmp/.expr 2> /tmp/.compErr");
    if (ret != 0) continue;


		//Simply throw the expr that 
		//trigger compile time error/warning.
//		FILE * cp;
//		cp = fopen("/tmp/.compErr", "r");
//		assert(cp != NULL);
			
//		char errbuf[1024];
//		int errflag;
//		errflag = 0;
//		if (fgets(errbuf, 1024, cp) != NULL) {
//			errflag = 1;
//			i--;
//			//printf("Error detected, once a more\n");
//		}
//		memset(errbuf, 0, 1024);
//		fclose(cp);
//		if (errflag)	continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    uint64_t result;//Originally int.
    ret = fscanf(fp, "%lu", &result);
    pclose(fp);

		//TODO: Only printf if no warning.
    printf("%lu %s\n", result, buf);
  }
  return 0;
}
