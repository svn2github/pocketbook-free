#ifndef PYINKVIEW_CALLBACK_H__
#define PYINKVIEW_CALLBACK_H__

#include <ffi.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define ffi_type_slonglong ffi_type_sint64
#define ffi_type_ulonglong ffi_type_uint64

void generic_callback(ffi_cif* cif, void* resp, void** args, void* userdata);

#ifdef __cplusplus
}
#endif

#endif //PYINKVIEW_CALLBACK_H__
