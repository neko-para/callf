#ifndef _CALLF_H_
#define _CALLF_H_

#ifdef __cplusplus
extern "C" {
#endif

void callfPure(void* func_ptr, unsigned return_size, unsigned push_size, void* return_block, const void* push_block, int return_float);
void callfAuto(void* func_ptr, void* return_block, const char* desc, const void *const* data);

#ifdef __cplusplus
}
#endif

#endif
