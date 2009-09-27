#ifndef PYINKVIEW_CALLBACK_H__
#define PYINKVIEW_CALLBACK_H__

#ifdef __cplusplus
extern "C"
{
#endif

void generic_callback(ffi_cif* cif, void* resp, void** args, void* userdata);

#ifdef __cplusplus
}
#endif

#endif //PYINKVIEW_CALLBACK_H__
