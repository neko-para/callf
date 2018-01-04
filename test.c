#include "callf.h"
#include <stdio.h>

void func() {
	int ret = printf("Hello");
}

int main() {
	int ret;
	const char* in = "Hello World\n";
	const void* param[1] = {(const void*)&in};
	callfAuto(printf, &ret, "ip", param);
	printf("%d\n", ret);
	return 0;
}
