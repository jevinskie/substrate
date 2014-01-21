all: test-hooker.so test-main test-dl-main test-dl-hooker.so libtest-dl-lib.so

.PHONY: clean

test-main: test-main.c
	gcc -o test-main -rdynamic -Wall -Wextra test-main.c -fPIE -pie

test-dl-main: test-dl-main.c libtest-dl-lib.so
	gcc -o test-dl-main -rdynamic -Wall -Wextra test-dl-main.c -fPIE -pie -Wl,-L.,-ltest-dl-lib

test-hooker.so: test-hooker.cpp
	g++ -o test-hooker.so -shared -Wall -Wextra test-hooker.cpp -ldl libsubstrate.so -fPIC

libtest-dl-lib.so: libtest-dl-lib.c
	gcc -o libtest-dl-lib.so -shared -Wall -Wextra libtest-dl-lib.c -fPIC

test-dl-hooker.so: test-dl-hooker.cpp
	g++ -o test-dl-hooker.so -shared -Wall -Wextra test-dl-hooker.cpp -ldl libsubstrate.so -fPIC

test: test-hooker.so test-main
	LD_LIBRARY_PATH=$(PWD) LD_PRELOAD=$(PWD)/test-hooker.so ./test-main

test-dl: test-dl-hooker.so test-dl-main
	LD_LIBRARY_PATH=$(PWD) LD_PRELOAD=$(PWD)/test-dl-hooker.so ./test-dl-main

clean:
	rm -f test-hooker.so test-main test-dl-main test-dl-hooker.so libtest-dl-lib.so
