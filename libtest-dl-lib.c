#include <stdio.h>

int a_number = 243;
static int a_static_number = 42;

int fact_sub(int n) {
	int res = 1;
	while (n > 1)
		res *= n--;
	return res;
}

static int fact_static_sub(int n) {
	int res = 1;
	while (n > 1)
		res *= n--;
	return res-1;
}

int fact(int n) {
	printf("libtest a_number_p %p\n", &a_number);
	printf("libtest a_number %d\n", a_number);
	printf("libtest fact_sub %p\n", fact_sub);
	return fact_sub(n);
}

int fact_static(int n) {
	printf("libtest a_static_number_p %p\n", &a_static_number);
	printf("libtest a_static_number %d\n", a_static_number);
	printf("libtest fact_static_sub %p\n", fact_static_sub);
	return fact_static_sub(n);
}
