#include <stdio.h>

int fact(int n) {
	int res = 1;
	while (n > 1)
		res *= n--;
	return res;
}

int main(void) {
	printf("fact = %p\n", fact);
	printf("fact(5) = %d\n", fact(5));
	return 0;
}
