#include "CydiaSubstrate.h"
#include <stdio.h>
#include <dlfcn.h>

typedef int (*fact_t)(int);

fact_t orig_fact_sub;
fact_t orig_fact_static_sub;

int my_fact_sub(int n) {
	printf("calling fact_sub(%d)\n", n);
	int res = orig_fact_sub(n);
	printf("res = %d\n", res);
	res *= 2;
	printf("real res = %d\n", res);
	return res;
}

int my_fact_static_sub(int n) {
	printf("calling fact_static_sub(%d)\n", n);
	int res = orig_fact_static_sub(n);
	printf("res = %d\n", res);
	res *= 2;
	printf("real res = %d\n", res);
	return res;
}

__attribute__ ((constructor))
void init_hooking(void) {
	void *fact_sub = MSFindSymbol(NULL, "fact_sub");
	printf("hooker fact_sub = %p\n", fact_sub);
	void *fact = MSFindSymbol(NULL, "fact");
	printf("hooker fact = %p\n", fact);
	void *a_number_p = MSFindSymbol(NULL, "a_number");
	printf("hooker a_number_p: %p\n", a_number_p);
	if (a_number_p)
		*(int *)a_number_p = 1337;
	MSHookFunction(fact_sub, (void *)my_fact_sub, (void **)&orig_fact_sub);

	void *a_static_number_p = MSFindSymbol(NULL, "a_static_number");
	printf("hooker a_static_number_p: %p\n", a_static_number_p);
	void *fact_static_sub = MSFindSymbol(NULL, "fact_static_sub");
	printf("hooker fact_static_sub = %p\n", fact_static_sub);
	if (a_static_number_p)
		*(int *)a_static_number_p = 7;
	MSHookFunction(fact_static_sub, (void *)my_fact_static_sub, (void **)&orig_fact_static_sub);
}
