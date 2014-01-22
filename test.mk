.PHONY: clean test test-dl test-dl-substrate

all: test-hooker.so test-main test-dl-main test-dl-hooker.so libtest-dl-lib.so test-dl-substrate-main

CC=clang -g -Wall -Wextra -std=c11
CXX=clang++ -g -Wall -Wextra -std=c++11

test-main: test-main.c
	$(CC) -o test-main -rdynamic -Wall -Wextra test-main.c -fPIE -pie

test-dl-main: test-dl-main.c libtest-dl-lib.so
	$(CC) -o test-dl-main -rdynamic -Wall -Wextra test-dl-main.c -fPIE -pie -L. -ltest-dl-lib

test-dl-substrate-main: test-dl-substrate-main.cpp libtest-dl-lib.so
	$(CXX) -o test-dl-substrate-main -rdynamic -Wall -Wextra test-dl-substrate-main.cpp -fPIE -pie -L. -ltest-dl-lib -ldl -lelf

test-hooker.so: test-hooker.cpp
	$(CXX) -o test-hooker.so -shared -Wall -Wextra test-hooker.cpp -ldl libsubstrate.so -fPIC

libtest-dl-lib.so: libtest-dl-lib.c
	$(CC) -o libtest-dl-lib.so -shared -Wall -Wextra libtest-dl-lib.c -fPIC

test-dl-hooker.so: test-dl-hooker.cpp
	$(CXX) -o test-dl-hooker.so -shared -Wall -Wextra test-dl-hooker.cpp -ldl libsubstrate.so -fPIC

test: test-hooker.so test-main
	LD_LIBRARY_PATH=$(PWD) LD_PRELOAD=$(PWD)/test-hooker.so ./test-main

test-dl: test-dl-hooker.so test-dl-main libtest-dl-lib.so
	LD_LIBRARY_PATH=$(PWD) LD_PRELOAD=$(PWD)/test-dl-hooker.so ./test-dl-main

test-dl-substrate: test-dl-substrate-main libtest-dl-lib.so
	LD_LIBRARY_PATH=$(PWD) ./test-dl-substrate-main

test-dl-substrate-dbg: test-dl-substrate-main libtest-dl-lib.so
	LD_LIBRARY_PATH=$(PWD) lldb-3.5 ./test-dl-substrate-main

clean:
	rm -f test-hooker.so test-main test-dl-main test-dl-hooker.so libtest-dl-lib.so test-dl-substrate-main
