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


%define Callback(CallbackClass, callback_func, handler, rtype, argnum)
%{
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
// Note: I cannot pass {... } to the macro :-(

// typedef int (*iv_handler)(int type, int par1, int par2);
%{
static ffi_type *iv_handler_cl_arg_types[] = {&ffi_type_sint, &ffi_type_sint, &ffi_type_sint, NULL};
%}
Callback(MainHandler, generic_callback, iv_handler, &ffi_type_sint, 3);

// typedef void (*iv_timerproc)();
%{
static ffi_type *iv_timerproc_cl_arg_types[] = {&ffi_type_void, NULL};
%}
Callback(TimerProc, generic_callback, iv_timerproc, &ffi_type_void, 0);

// typedef void (*iv_menuhandler)(int index);
%{
static ffi_type *iv_menuhandler_cl_arg_types[] = {&ffi_type_sint, NULL};
%}
Callback(MenuHandler, generic_callback, iv_menuhandler, &ffi_type_void, 1);

// typedef void (*iv_keyboardhandler)(char *text);
%{
static ffi_type *iv_keyboardhandler_cl_arg_types[] = {&ffi_type_pointer, NULL};
%}
Callback(KeyboardHandler, generic_callback, iv_keyboardhandler, &ffi_type_void, 1);

// typedef void (*iv_dialoghandler)(int button);
%{
static ffi_type *iv_dialoghandler_cl_arg_types[] = {&ffi_type_sint, NULL};
%}
Callback(DialogHandler, generic_callback, iv_dialoghandler, &ffi_type_void, 1);

// typedef void (*iv_timeedithandler)(long newtime);
%{
static ffi_type *iv_timeedithandler_cl_arg_types[] = {&ffi_type_slong, NULL};
%}
Callback(TimeEditHandler, generic_callback, iv_timeedithandler, &ffi_type_void, 1);

// typedef void (*iv_fontselecthandler)(char *fontr, char *fontb, char *fonti, char *fontbi);
%{
static ffi_type *iv_fontselecthandler_cl_arg_types[] = {&ffi_type_pointer, &ffi_type_pointer, &ffi_type_pointer, &ffi_type_pointer, NULL};
%}
Callback(FontSelectHandler, generic_callback, iv_fontselecthandler, &ffi_type_void, 4);

// typedef void (*iv_dirselecthandler)(char *path);
%{
static ffi_type *iv_dirselecthandler_cl_arg_types[] = {&ffi_type_pointer, NULL};
%}
Callback(DirSelectHandler, generic_callback, iv_dirselecthandler, &ffi_type_void, 1);

// typedef void (*iv_confighandler)();
%{
static ffi_type *iv_confighandler_cl_arg_types[] = {&ffi_type_void, NULL};
%}
Callback(ConfigHandler, generic_callback, iv_confighandler, &ffi_type_void, 0);

// typedef void (*iv_itemchangehandler)(char *name);
%{
static ffi_type *iv_itemchangehandler_cl_arg_types[] = {&ffi_type_pointer, NULL};
%}
Callback(ItemChangeHandler, generic_callback, iv_itemchangehandler, &ffi_type_void, 1);

// typedef void (*iv_pageselecthandler)(int page);
%{
static ffi_type *iv_pageselecthandler_cl_arg_types[] = {&ffi_type_sint, NULL};
%}
Callback(PageSelectHandler, generic_callback, iv_pageselecthandler, &ffi_type_void, 1);

// typedef void (*iv_bmkhandler)(int action, int page, long long position);
%{
static ffi_type *iv_bmkhandler_cl_arg_types[] = {&ffi_type_sint, &ffi_type_sint, &ffi_type_slonglong, NULL};
%}
Callback(BmkHandler, generic_callback, iv_bmkhandler, &ffi_type_void, 3);

// typedef void (*iv_tochandler)(long long position);
%{
static ffi_type *iv_tochandler_cl_arg_types[] = {&ffi_type_slonglong, NULL};
%}
Callback(TocHandler, generic_callback, iv_tochandler, &ffi_type_void, 1)

// typedef void (*iv_listhandler)(int action, int x, int y, int idx, int state);
%{
static ffi_type *iv_listhandler_cl_arg_types[] = {&ffi_type_sint, &ffi_type_sint, &ffi_type_sint, &ffi_type_sint, &ffi_type_sint, NULL};
%}
Callback(ListHandler, generic_callback, iv_listhandler, &ffi_type_void, 5);

// typedef void (*iv_rotatehandler)(int direction);
%{
static ffi_type *iv_rotatehandler_cl_arg_types[] = {&ffi_type_sint, NULL};
%}
Callback(RotateHandler, generic_callback, iv_rotatehandler, &ffi_type_void, 1);

//TODO:
// Contain void* - generic callback canntot be used
// typedef int (*iv_hashenumproc)(char *name, void *value, void *userdata);
// typedef int (*iv_hashcmpproc)(char *name1, void *value1, char *name2, void *value2);
// typedef void * (*iv_hashaddproc)(void *data);
// typedef void (*iv_hashdelproc)(void *data);
