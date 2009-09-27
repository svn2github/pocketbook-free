
%{
#include <Python.h>
#include <stdlib.h>
#include <ffi.h>
#include <sys/mman.h>
//#include <unistd.h>
#include "callback.h"

%}
// Replace argument type 
%define callbackTypemapIn(CallbackClass, handler)
%typemap(in) handler {
	CallbackClass *callback = NULL;
	int res2 = SWIG_ConvertPtr($input, (void**)&callback, SWIGTYPE_p_tag##CallbackClass, 0 |  0 );
	if (!SWIG_IsOK(res2)) {
		PyErr_SetString(PyExc_TypeError, #CallbackClass##" argument expected");
		return NULL;
	}
	$1 = (handler)callback->pcl;
}
%enddef 

%define Callback(CallbackClass, cif, callback_func)

%inline %{
typedef struct tag##CallbackClass {
	ffi_closure *pcl;
} CallbackClass;
%}

%extend CallbackClass {
	CallbackClass(PyObject *pyfunc) {
		if (!PyCallable_Check(pyfunc)) {
			PyErr_SetString(PyExc_TypeError,"Need a callable object!");
			return NULL;
		}
		CallbackClass* cb = (CallbackClass*) malloc(sizeof(CallbackClass));
		cb->pcl = mmap(NULL, sizeof(ffi_closure), 
	 	                PROT_READ|PROT_WRITE|PROT_EXEC, 
	 	                MAP_PRIVATE | MAP_ANON, -1, 0); 
		Py_INCREF(pyfunc);
		if (ffi_prep_closure(cb->pcl, &cif, callback_func, (void *)pyfunc ) != FFI_OK) {
			PyErr_SetString(PyExc_TypeError,"closure creating error");
			return NULL;
		}
		return cb;
	}

	~CallbackClass() {
		Py_DECREF((PyObject*)($self->pcl->user_data));
	}
};

%enddef

// Closures information
// Note: I cannot pass {... } to the macro :-(

%{
// typedef int (*iv_handler)(int type, int par1, int par2);
static ffi_type *iv_handler_cl_arg_types[] = {&ffi_type_sint, &ffi_type_sint, &ffi_type_sint, NULL};
static ffi_cif iv_handler_cif;
%}
Callback(MainHandler, iv_handler_cif, generic_callback);

%{
// typedef void (*iv_timerproc)();
static ffi_type *iv_timerproc_cl_arg_types[] = {&ffi_type_void, NULL};
static ffi_cif iv_timerproc_cif;
%}
Callback(TimerProc, iv_timerproc_cif, generic_callback);

%{
// typedef void (*iv_menuhandler)(int index);
static ffi_type *iv_menuhandler_cl_arg_types[] = {&ffi_type_sint, NULL};
static ffi_cif iv_menuhandler_cif;
%}
Callback(MenuHandler, iv_menuhandler_cif, generic_callback);

%{
// typedef void (*iv_keyboardhandler)(char *text);
static ffi_type *iv_keyboardhandler_cl_arg_types[] = {&ffi_type_pointer, NULL};
static ffi_cif iv_keyboardhandler_cif;
%}
Callback(KeyboardHandler, iv_keyboardhandler_cif, generic_callback)

%{
// typedef void (*iv_dialoghandler)(int button);
static ffi_type *iv_dialoghandler_cl_arg_types[] = {&ffi_type_sint, NULL};
static ffi_cif iv_dialoghandler_cif;
%}
Callback(DialogHandler, iv_dialoghandler_cif, generic_callback);

%{
// typedef void (*iv_timeedithandler)(long newtime);
static ffi_type *iv_timeedithandler_cl_arg_types[] = {&ffi_type_slong, NULL};
static ffi_cif iv_timeedithandler_cif;
%}
Callback(TimeEditHandler, iv_timeedithandler_cif, generic_callback)

%{
// typedef void (*iv_fontselecthandler)(char *fontr, char *fontb, char *fonti, char *fontbi);
static ffi_type *iv_fontselecthandler_cl_arg_types[] = {&ffi_type_pointer, &ffi_type_pointer, &ffi_type_pointer, &ffi_type_pointer, NULL};
static ffi_cif iv_fontselecthandler_cif;
%}
Callback(FontSelectHandler, iv_fontselecthandler_cif, generic_callback)

%{
// typedef void (*iv_dirselecthandler)(char *path);
static ffi_type *iv_dirselecthandler_cl_arg_types[] = {&ffi_type_pointer, NULL};
static ffi_cif iv_dirselecthandler_cif;
%}
Callback(DirSelectHandler, iv_dirselecthandler_cif, generic_callback)

//TODO: 

//%{
//// typedef void (*iv_confighandler)();
//static ffi_type *iv_confighandler_cl_arg_types[] = {&ffi_type_void, NULL};
//static ffi_cif iv_confighandler_cif;
//%}
////Callback(, iv_confighandler_cif, generic_callback)
//
//%{
//// typedef void (*iv_itemchangehandler)(char *name);
//static ffi_type *iv_itemchangehandler_cl_arg_types[] = {&ffi_type_void, NULL};
//static ffi_cif iv_itemchangehandler_cif;
//%}
////Callback(, iv_itemchangehandler_cif, generic_callback)
//
//%{
//// typedef void (*iv_pageselecthandler)(int page);
//static ffi_type *iv_pageselecthandler_cl_arg_types[] = {&ffi_type_void, NULL};
//static ffi_cif iv_pageselecthandler_cif;
//%}
////Callback(, iv_pageselecthandler_cif, generic_callback)
//
//%{
//// typedef void (*iv_bmkhandler)(int action, int page, long long position);
//static ffi_type *iv_bmkhandler_cl_arg_types[] = {&ffi_type_void, NULL};
//static ffi_cif iv_bmkhandler_cif;
//%}
////Callback(, iv_bmkhandler_cif, generic_callback)
//
//%{
//// typedef void (*iv_tochandler)(long long position);
//static ffi_type *iv_tochandler_cl_arg_types[] = {&ffi_type_void, NULL};
//static ffi_cif iv_tochandler_cif;
//%}
////Callback(, iv_tochandler_cif, generic_callback)
//
//%{
//// typedef void (*iv_listhandler)(int action, int x, int y, int idx, int state);
//static ffi_type *iv_listhandler_cl_arg_types[] = {&ffi_type_void, NULL};
//static ffi_cif iv_listhandler_cif;
//%}
////Callback(, iv_listhandler_cif, generic_callback)
//
//%{
//// typedef void (*iv_rotatehandler)(int direction);
//static ffi_type *iv_rotatehandler_cl_arg_types[] = {&ffi_type_void, NULL};
//static ffi_cif iv_rotatehandler_cif;
//%}
////Callback(, iv_rotatehandler_cif, generic_callback)
//

// Contain void* - generic callback canntot be used
//%{
//// typedef int (*iv_hashenumproc)(char *name, void *value, void *userdata);
//static ffi_type *iv_hashenumproc_cl_arg_types[] = {&ffi_type_void, NULL};
//static ffi_cif iv_hashenumproc_cif;
//%}
////Callback(, iv_hashenumproc_cif, generic_callback)
//
//%{
//// typedef int (*iv_hashcmpproc)(char *name1, void *value1, char *name2, void *value2);
//static ffi_type *iv_hashcmpproc_cl_arg_types[] = {&ffi_type_void, NULL};
//static ffi_cif iv_hashcmpproc_cif;
//%}
////Callback(, iv_hashcmpproc_cif, generic_callback)
//
//%{
//// typedef void * (*iv_hashaddproc)(void *data);
//static ffi_type *iv_hashaddproc_cl_arg_types[] = {&ffi_type_void, NULL};
//static ffi_cif iv_hashaddproc_cif;
//%}
////Callback(, iv_hashaddproc_cif, generic_callback)
//
//%{
//// typedef void (*iv_hashdelproc)(void *data);
//static ffi_type *iv_hashdelproc_cl_arg_types[] = {&ffi_type_void, NULL};
//static ffi_cif iv_hashdelproc_cif;
//%}
////Callback(, iv_hashdelproc_cif, generic_callback)


%inline %{
void _initialize_ffi_types()
{
#define PREP_CIF(handler, argnum, rtype) do {\
		if (ffi_prep_cif(&handler ## _cif, FFI_DEFAULT_ABI, argnum, rtype, handler ## _cl_arg_types) != FFI_OK ) { \
			goto fail;		\
		} \
	} while(0)
		
	PREP_CIF(iv_handler, 3, &ffi_type_void); //TODO: Should be sint
	PREP_CIF(iv_timerproc, 0, &ffi_type_void);
	PREP_CIF(iv_menuhandler, 1, &ffi_type_void);
	PREP_CIF(iv_keyboardhandler, 1, &ffi_type_void);
	PREP_CIF(iv_dialoghandler, 1, &ffi_type_void);
	PREP_CIF(iv_timeedithandler, 1, &ffi_type_void);	
	PREP_CIF(iv_fontselecthandler, 4, &ffi_type_void);
	PREP_CIF(iv_dirselecthandler, 1, &ffi_type_void);

//	PREP_CIF(iv_confighandler, 0, &ffi_type_void);
//	PREP_CIF(iv_itemchangehandler, 1, &ffi_type_void);
//	PREP_CIF(iv_pageselecthandler, 1, &ffi_type_void);
//	PREP_CIF(iv_bmkhandler, 3, &ffi_type_void);
//	PREP_CIF(iv_tochandler, 1, &ffi_type_void);
//	PREP_CIF(iv_listhandler, 5, &ffi_type_void);
//	PREP_CIF(iv_rotatehandler, 1, &ffi_type_void);
	// Contain void*
//	PREP_CIF(iv_hashenumproc,3,&ffi_type_sint);
//	PREP_CIF(iv_hashcmpproc,4,&ffi_type_sint);
//	PREP_CIF(iv_hashaddproc,1,&ffi_type_void);
//	PREP_CIF(iv_hashdelproc,1,&ffi_type_void);
	
	
#undef PREP_CIF
	
	return ;	/* Ok */
fail:
	PyErr_SetString(PyExc_TypeError,"Callback type initialization error");
}


%}

// FFI types initialization
%pythoncode %{
_initialize_ffi_types();
%}
