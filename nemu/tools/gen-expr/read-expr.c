#include <stdio.h>
#include <stdint.h>
#include <assert.h>
char line_buf[2048];
char expr_buf[2048];
int main() {
	FILE * fp = fopen("./output.txt", "r");
	assert(fp != NULL);
	int N = 2048;
	uint64_t num;
	for (int i = 0; i < 1000; ++i) {
		fgets(line_buf, 2048, fp);
		sscanf(line_buf, "%lu %s", &num, expr_buf);
		printf("Val: %lu\n"
					 "Expr: %s\n", num, expr_buf);
	}


	fclose(fp);

	return 0;
}
