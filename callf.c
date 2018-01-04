#include "callf.h"

void callfPure(void* func_ptr, unsigned return_size, unsigned push_size, void* return_block, const void* push_block, int return_float) {
	__asm__ volatile (
		"movl %1, %%ecx;"
		"subl %%ecx, %%esp;"
		"movl %%esp, %%edi;"
		"movl %4, %%esi;"
		"shrl $2, %%ecx;"
		"cld;"
		"rep movsd;"
		"movl %2, %%eax;"
		"cmpl $8, %%eax;"
		"jle .return_less_than_8;"
		"pushl %3;"
		".return_less_than_8:;"
		"call *%0;"
		"addl %1, %%esp;"
		"movl %%eax, %%ecx;"
		"movl %5, %%eax;"
		"cmpl $0, %%eax;"
		"jne .return_float_type;"
		"movl %2, %%eax;"
		"cmpl $8, %%eax;"
		"jg .end;"
		"movl %3, %%eax;"
		"movl %%ecx, (%%eax);"
		"movl %%edx, 4(%%eax);"
		"jmp .end;"
		".return_float_type:"
		"movl %3, %%ecx;"
		"movl %2, %%eax;"
		"cmpl $4, %%eax;"
		"je .return_float;"
		"cmpl $8, %%eax;"
		"je .return_double;"
		"cmpl $12, %%eax;"
		"je .return_long_double;"
		"jmp .end;"
		".return_float:"
		"fstps (%%ecx);"
		"jmp .end;"
		".return_double:"
		"fstpl (%%ecx);"
		"jmp .end;"
		".return_long_double:"
		"fstpt (%%ecx);"
		".end:;"
		:
		: "m" (func_ptr), "m" (push_size), "m" (return_size), "m" (return_block), "m" (push_block), "m" (return_float)
	);
}

static unsigned parse_size(const char** ptr) {
	switch(*((*ptr)++)) {
		case 'v':
			return 0;
		case 'b':
		case 'B':
		case 'w':
		case 'W':
		case 'i':
		case 'I':
		case 'l':
		case 'L':
		case 'f':
		case 'p':
			return 4;
		case 'q':
		case 'Q':
		case 'd':
			return 8;
		case 't':
			return 12;
		case 's': {
			unsigned n = 0;
			const char* p = (*ptr) + 1;
			while (((unsigned)(*p - '0')) < 10) {
				n = ((n + (n << 2)) << 1) + (*p++ ^ '0');
			}
			*ptr = p;
			return n + ((n & 3) ? 4 : 0);
		}
	}
	return 0;
}

static unsigned count(const char* ptr) {
	unsigned size = 0;
	while (*ptr) {
		size += parse_size(&ptr);
	}
	return size;
}

void callfAuto(void* func_ptr, void* return_block, const char* desc, const void *const* data) {
	char return_type = *desc;
	unsigned return_size = parse_size(&desc);
	unsigned char return_buffer[return_size];
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
		case 'p':
		case 'f':
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
		case 's': {
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
	callfPure(func_ptr, return_size, size, return_buffer, buffer, return_type == 'f' || return_type == 'd' || return_type == 't');
	switch(return_type) {
		case 'b':
			*(char*)return_block = *(int*)return_buffer;
			break;
		case 'B':
			*(unsigned char*)return_block = *(unsigned*)return_buffer;
			break;
		case 'w':
			*(short*)return_block = *(int*)return_buffer;
			break;
		case 'W':
			*(unsigned short*)return_block = *(unsigned*)return_buffer;
			break;
		case 'i':
		case 'l':
			*(int*)return_block = *(int*)return_buffer;
			break;
		case 'I':
		case 'L':
		case 'f':
			*(unsigned*)return_block = *(unsigned*)return_buffer;
			break;
		case 'q':
			*(long long*)return_block = *(long long*)return_buffer;
			break;
		case 'Q':
		case 'd':
			*(unsigned long long*)return_block = *(unsigned long long*)return_buffer;
			break;
		case 't':
			((unsigned*)return_block)[0] = ((unsigned*)return_buffer)[0];
			((unsigned*)return_block)[1] = ((unsigned*)return_buffer)[1];
			((unsigned*)return_block)[2] = ((unsigned*)return_buffer)[2];
			break;
		case 's': {
			unsigned i;
			unsigned n = return_size >> 2, r = return_size & 3;
			for (i = 0; i < n; ++i) {
				((unsigned*)return_block)[i] = ((unsigned*)return_buffer)[i];
			}
			n <<= 2;
			for (i = 0; i < r; ++i) {
				((unsigned char*)return_block)[n + i] = ((unsigned char*)return_buffer)[n + i];
			}
			break;
		}
	}
}
