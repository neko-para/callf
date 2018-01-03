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

static unsigned parse_size(const char** ptr) {
	switch(**ptr) {
		case 'b':
		case 'B':
		case 'w':
		case 'W':
		case 'i':
		case 'I':
		case 'l':
		case 'L':
		case 's':
			return 4;
		case 'q':
		case 'Q':
		case 'd':
			return 8;
		case 't':
			return 12;
		case 'm': {
			unsigned n = 0;
			while (((unsigned)((*ptr)[1] - '0')) < 10) {
				n = ((n + (n << 2)) << 1) + (*++(*ptr) ^ '0');
			}
			return n + ((n & 3) ? 4 : 0);
		}
	}
	return 0;
}

static unsigned count(const char* ptr) {
	unsigned size = 0;
	while (*ptr) {
		size += parse_size(&ptr);
		++ptr;
	}
	return size;
}

void callfAuto(void* func_ptr, void* return_block, const char* desc, const void *const* data) {
	char return_type = *desc;
	unsigned return_size = parse_size(&desc);
	unsigned return_buffer[return_size];
	unsigned size = count(desc);
	unsigned char buffer[size];
	unsigned char* ptr = buffer;
	while (*desc) {
		switch(*desc) {
		case 'b':
			*((int*)ptr) = *(char*)*data;
			ptr += 4;
			break;
		case 'B':
			*((unsigned*)ptr) = *(unsigned char*)*data;
			ptr += 4;
			break;
		case 'w':
			*((int*)ptr) = *(short*)*data;
			ptr += 4;
			break;
		case 'W':
			*((unsigned*)ptr) = *(unsigned short*)*data;
			ptr += 4;
			break;
		case 'i':
		case 'l':
			*((int*)ptr) = *(int*)*data;
			ptr += 4;
			break;
		case 'I':
		case 'L':
		case 's':
			*((unsigned*)ptr) = *(unsigned*)*data;
			ptr += 4;
			break;
		case 'q':
			*((long long*)ptr) = *(long long*)*data;
			ptr += 8;
			break;
		case 'Q':
		case 'd':
			*((unsigned long long*)ptr) = *(unsigned long long*)*data;
			ptr += 8;
			break;
		case 't':
			((unsigned*)ptr)[0] = ((unsigned*)*data)[0];
			((unsigned*)ptr)[1] = ((unsigned*)*data)[1];
			((unsigned*)ptr)[2] = ((unsigned*)*data)[2];
			ptr += 12;
			break;
		case 'm': {
			unsigned n = 0, i;
			while (((unsigned)(desc[1] - '0')) < 10) {
				n = ((n + (n << 2)) << 1) + (*++desc ^ '0');
			}
			n = ((n & 3) ? 1 : 0) + (n >> 2);
			for (i = 0; i < n; ++i) {
				((unsigned*)ptr)[i] = ((unsigned*)*data)[i];
			}
			ptr += n << 2;
			break;
		}
		}
		++desc;
	}
	callfPure(func_ptr, return_size, size, return_buffer, buffer);
	switch(return_type) {
		case 'b':
			*(char*)return_block = *(int*)buffer;
			break;
		case 'B':
			*(unsigned char*)return_block = *(unsigned*)buffer;
			break;
		case 'w':
			*(short*)return_block = *(int*)buffer;
			break;
		case 'W':
			*(unsigned short*)return_block = *(unsigned*)buffer;
			break;
		case 'i':
		case 'l':
			*(int*)return_block = *(int*)buffer;
			break;
		case 'I':
		case 'L':
		case 's':
			*(unsigned*)return_block = *(unsigned*)buffer;
			break;
		case 'q':
			*(long long*)return_block = *(long long*)buffer;
			break;
		case 'Q':
		case 'd':
			*(unsigned long long*)return_block = *(unsigned long long*)buffer;
			break;
		case 't':
			((unsigned*)return_block)[0] = ((unsigned*)buffer)[0];
			((unsigned*)return_block)[1] = ((unsigned*)buffer)[1];
			((unsigned*)return_block)[2] = ((unsigned*)buffer)[2];
			break;
		case 'm': {
			unsigned i;
			unsigned n = return_size >> 2, r = return_size & 3;
			for (i = 0; i < n; ++i) {
				((unsigned*)return_block)[i] = ((unsigned*)buffer)[i];
			}
			n <<= 2;
			for (i = 0; i < r; ++i) {
				((unsigned char*)return_block)[n + i] = ((unsigned char*)buffer)[n + i];
			}
			break;
		}
	}
}
