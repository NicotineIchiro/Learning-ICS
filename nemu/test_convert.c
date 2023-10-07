#include <stdio.h>
#include <stdlib.h>
int main() {
	printf("%lu\n", strtoul("0xFF", NULL, 16));

	return 0;
}
