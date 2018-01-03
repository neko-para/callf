#include "callf.h"
#include <stdio.h>

typedef struct T {
	int x, y, z;	
} T;

T test(int x) {
	T t;
	t.x = x;
	return t;
}

int main() {
	T ret;
	int in = 6;
	callfPure(test, sizeof(T), 4, &ret, &in);
	printf("%d\n", ret.x);
}
