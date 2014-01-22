#include <stdio.h>

extern int fact(int n);
extern int fact_static(int n);

int main(void) {
	printf("fact = %p\n", fact);
	printf("fact(5) = %d\n", fact(5));
	printf("fact_static = %p\n", fact_static);
	printf("fact_static(5) = %d\n", fact_static(5));
	return 0;
}
