#include "callf.h"
#include <stdio.h>
#include <math.h>

double rs(double x) {
	printf("%lf\n", x);
	return sqrt(x);
}

int main() {
	double ret;
	double in = 2.0;
	const void* param[1] = {(const void*)&in};
	callfAuto(rs, &ret, "dd", param);
	printf("%lf\n", ret);
	return 0;
}
