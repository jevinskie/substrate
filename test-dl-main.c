#include <stdio.h>

extern int fact(int n);

int main(void) {
	printf("fact = %p\n", fact);
	printf("fact(5) = %d\n", fact(5));
	return 0;
}
