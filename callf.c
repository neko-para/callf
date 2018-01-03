#include "callf.h"

void callfPure(void* func_ptr, unsigned return_size, unsigned push_size, void* return_block, const void* push_block) {
	if (return_size <= 8) {
		__asm__ volatile (
			"movl %1, %%ecx;"
			"subl %%ecx, %%esp;"
			"movl %%esp, %%edi;"
			"movl %3, %%esi;"
			"shrl $2, %%ecx;"
			"cld;"
			"rep movsd;"
			"call *%0;"
			"addl %1, %%esp;"
			"movl %2, %%ecx;"
			"movl %%eax, (%%ecx);"
			"movl %%edx, 4(%%ecx);"
			:
			: "m" (func_ptr), "m" (push_size), "m" (return_block), "m" (push_block)
		);
	} else {
		__asm__ volatile (
			"movl %1, %%ecx;"
			"subl %%ecx, %%esp;"
			"movl %%esp, %%edi;"
			"movl %3, %%esi;"
			"shrl $2, %%ecx;"
			"cld;"
			"rep movsd;"
			"pushl %2;"
			"call *%0;"
			"addl %1, %%esp;"
			:
			: "m" (func_ptr), "m" (push_size), "m" (return_block), "m" (push_block)
		);
	}
}
