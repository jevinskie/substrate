int fact_sub(int n) {
	int res = 1;
	while (n > 1)
		res *= n--;
	return res;
}

int fact(int n) {
	return fact_sub(n);
}
