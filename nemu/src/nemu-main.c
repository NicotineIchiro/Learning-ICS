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
#include <common.h>
extern word_t expr(char *e, bool *success);
void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();
char expr_buf[2048];
char line_buf[2048];
void test_expr() {
	FILE * fp = fopen("/tmp/output10000.txt", "r");
	assert(fp != NULL);
	uint64_t num, eval_result;
	bool flag;
	for (int i = 0; i < 10000; ++i) {
		fgets(line_buf, 2048, fp);
		sscanf(line_buf, "%lu %s", &num, expr_buf);
		if ((eval_result = expr(expr_buf, &flag)) != num)
			printf("Case %d: %s\n"
						 "Parse:%lu, Result:%lu\n", \
						 i, expr_buf, \
						 eval_result, num);
	}
	fclose(fp);
	exit(0);
}

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

	test_expr();

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
