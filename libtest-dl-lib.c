#include <stdio.h>

int a_number = 243;

int fact_sub(int n) {
	int res = 1;
	while (n > 1)
		res *= n--;
	return res;
}

int fact(int n) {
	printf("a_number_p %p\n", &a_number);
	printf("a_number %d\n", a_number);
	return fact_sub(n);
}
