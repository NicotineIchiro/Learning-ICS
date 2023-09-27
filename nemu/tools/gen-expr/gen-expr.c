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
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static int buf_i = 0;
uint32_t choose(uint32_t n) {
	uint32_t result;
	result = rand() % n;
	//while ((result = rand() % n) == 0);
	return result;
}
#define BUF_MAX 50
int BUF_FULL() { return buf_i >= BUF_MAX; }
void set_buf_end()
{
	buf_i++;
	buf[buf_i] = '\0';
}
static void gen_rand_op() {
	if (BUF_FULL())	return;

	printf("gen_op() ");
	int i;
	i = choose(4);//TODO: Now ignore "=="
	char op_list[] = {'*', '/', '+', '-'};
	
//	if (i < 4) {
		buf[buf_i] = op_list[i];
		set_buf_end();
//	}
//	else {
//		buf[buf_i] = '=';
	printf("buf_i: %d\n", buf_i);

	return;
}
static void gen(char ch) {
	if (BUF_FULL()) return;
	printf("gen\'%c\'	", ch);
	assert(ch == '(' || ch == ')');
	buf[buf_i] = ch;
	//printf("buf_i before set: %d\n", buf_i);	
	set_buf_end();
	//buf_i++;
	//buf[buf_i] = '\0';
	printf("buf_i: %d\n", buf_i);

	return;
}
static void gen_num() {
	if (BUF_FULL()) return;
	printf("gen_num() ");
	//const int UINT64_MAXL = 18;
	int length;
	while ((length = choose(5)) == 0);
	//while ((length = choose(BUF_MAX < UINT64_MAXL	? 3 : BUF_MAX / 5)) == 0 );
	for (int i = 0; i < length; ++i){
		char rand_nc;
		rand_nc = '0' + choose(10);
					
		buf[buf_i] = rand_nc;
		
		buf_i++;
	}
	while (buf[buf_i - length] == '0' && length > 1) {
		buf[buf_i - length] = choose(10) + '0';
	}
	printf("buf_i: %d\n", buf_i);
	buf[buf_i]	= '\0';

	return;
}

static void gen_rand_expr() {
  //buf[0] = '\0';
	//TODO: The end of expr must be NUM or matched right parth
	//			to avoid bad expression.	
	switch (choose(3)) {
		case 0: gen_num(); break;
		case 1: gen('('); gen_rand_expr(); gen(')'); break;
		default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
	}
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
		printf("buf: %s\n", buf);
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
