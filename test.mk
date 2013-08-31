all: test-hooker.so test-main

.PHONY: clean

test-main: test-main.c
	gcc -o test-main -rdynamic -Wall -Wextra test-main.c -fPIE -pie

test-hooker.so: test-hooker.cpp
	g++ -o test-hooker.so -shared -Wall -Wextra test-hooker.cpp -ldl libsubstrate.so -fPIC

test: test-hooker.so test-main
	LD_LIBRARY_PATH=$(PWD) LD_PRELOAD=$(PWD)/test-hooker.so ./test-main

clean:
	rm -f test-hooker.so test-main
