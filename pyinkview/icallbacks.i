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

%define Callback(CallbackClass, callback_func, handler, rtype, argnum, ...)
%{
static ffi_type *handler##_cl_arg_types[] = __VA_ARGS__;
static ffi_cif handler##_cif;

%}

%inline %{
void _initialize_##handler##_cif() {
	if (ffi_prep_cif(&handler##_cif, FFI_DEFAULT_ABI, argnum, rtype, handler##_cl_arg_types) != FFI_OK ) { 
		PyErr_SetString(PyExc_TypeError, "Callback type initialization error");
		return ;
	}
}
%}

%pythoncode %{
_initialize_##handler##_cif()
%}

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
		if (ffi_prep_closure(cb->pcl, &handler##_cif, callback_func, (void *)pyfunc ) != FFI_OK) {
			PyErr_SetString(PyExc_TypeError,"closure creating error");
			return NULL;
		}
		return cb;
	}

	~CallbackClass() {
		Py_DECREF((PyObject*)($self->pcl->user_data));
		//free($self-<pcl);
		//free($self);
	}
};

%enddef

// Closures information

// typedef int (*iv_handler)(int type, int par1, int par2);
Callback(MainHandler, generic_callback, iv_handler, &ffi_type_sint, 3, {&ffi_type_sint, &ffi_type_sint, &ffi_type_sint, NULL});

// typedef void (*iv_timerproc)();
Callback(TimerProc, generic_callback, iv_timerproc, &ffi_type_void, 0, {&ffi_type_void, NULL});

// typedef void (*iv_menuhandler)(int index);
Callback(MenuHandler, generic_callback, iv_menuhandler, &ffi_type_void, 1, {&ffi_type_sint, NULL});

// typedef void (*iv_keyboardhandler)(char *text);
Callback(KeyboardHandler, generic_callback, iv_keyboardhandler, &ffi_type_void, 1, {&ffi_type_pointer, NULL});

// typedef void (*iv_dialoghandler)(int button);
Callback(DialogHandler, generic_callback, iv_dialoghandler, &ffi_type_void, 1, {&ffi_type_sint, NULL});

// typedef void (*iv_timeedithandler)(long newtime);
Callback(TimeEditHandler, generic_callback, iv_timeedithandler, &ffi_type_void, 1, {&ffi_type_slong, NULL});

// typedef void (*iv_fontselecthandler)(char *fontr, char *fontb, char *fonti, char *fontbi);
Callback(FontSelectHandler, generic_callback, iv_fontselecthandler, &ffi_type_void, 4, {&ffi_type_pointer, &ffi_type_pointer, &ffi_type_pointer, &ffi_type_pointer, NULL});

// typedef void (*iv_dirselecthandler)(char *path);
Callback(DirSelectHandler, generic_callback, iv_dirselecthandler, &ffi_type_void, 1, {&ffi_type_pointer, NULL});

// typedef void (*iv_confighandler)();
Callback(ConfigHandler, generic_callback, iv_confighandler, &ffi_type_void, 0, {&ffi_type_void, NULL});

// typedef void (*iv_itemchangehandler)(char *name);
Callback(ItemChangeHandler, generic_callback, iv_itemchangehandler, &ffi_type_void, 1, {&ffi_type_pointer, NULL});

// typedef void (*iv_pageselecthandler)(int page);
Callback(PageSelectHandler, generic_callback, iv_pageselecthandler, &ffi_type_void, 1, {&ffi_type_sint, NULL});

// typedef void (*iv_bmkhandler)(int action, int page, long long position);
Callback(BmkHandler, generic_callback, iv_bmkhandler, &ffi_type_void, 3, {&ffi_type_sint, &ffi_type_sint, &ffi_type_slonglong, NULL});

// typedef void (*iv_tochandler)(long long position);
Callback(TocHandler, generic_callback, iv_tochandler, &ffi_type_void, 1, {&ffi_type_slonglong, NULL})

// typedef void (*iv_listhandler)(int action, int x, int y, int idx, int state);
Callback(ListHandler, generic_callback, iv_listhandler, &ffi_type_void, 5, {&ffi_type_sint, &ffi_type_sint, &ffi_type_sint, &ffi_type_sint, &ffi_type_sint, NULL});

// typedef void (*iv_rotatehandler)(int direction);
Callback(RotateHandler, generic_callback, iv_rotatehandler, &ffi_type_void, 1, {&ffi_type_sint, NULL});

//TODO:
// Contain void* - generic callback canntot be used
// typedef int (*iv_hashenumproc)(char *name, void *value, void *userdata);
// typedef int (*iv_hashcmpproc)(char *name1, void *value1, char *name2, void *value2);
// typedef void * (*iv_hashaddproc)(void *data);
// typedef void (*iv_hashdelproc)(void *data);
