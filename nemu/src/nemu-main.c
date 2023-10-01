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

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif
//TEST GEN_EXPR
//	char exprbuf[1024];
//	FILE *fp;
//	fp = fopen("$NEMU_HOME/tools/gen-expr/output", "r");
//	assert(fp != NULL);
//	uint32_t result;
//	for (int i = 0; fscanf(fp, "%d %s", &result, exprbuf) != EOF;++i) {
//		bool flag;
//		flag = true;
//		uint32_t parse_result;
//
//		if ((parse_result = expr(exprbuf, &flag)) != result)
//			printf("Case %d: %s\nParse:%u, Result:%u\n");
//	}

//TEST GEN_EXPR

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
