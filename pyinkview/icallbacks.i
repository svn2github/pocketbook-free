
%{
#include <Python.h>
#include <stdlib.h>
#include <ffi.h>
#include <sys/mman.h>
//#include <unistd.h>
%}



%{

// General callback

static void
internal_callback(ffi_cif* cif, void* resp, void** args, void* userdata)
{
	PyObject* pyfunc = userdata;
	PyObject *arglist;
	PyObject *result;
	int i;
	int ires = 0;

	if (cif->nargs < 0) {
		printf("nargs error\n");
	}

	arglist = PyTuple_New(cif->nargs);
	for (i = 0; i < cif->nargs; ++i) {
		PyObject* arg = NULL;
		if (cif->arg_types[i] == &ffi_type_sint) {
			arg = PyLong_FromLong(*((int*)args[i]));
		}
		else {
			printf("unsupported argument type\n");
		}
		PyTuple_SetItem(arglist, i, arg );
	}

	result = PyEval_CallObject(pyfunc, arglist);     
	Py_DECREF(arglist);                           
	if (result) {                                 
		Py_XDECREF(result);
	}
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}
	return;
}

%}


%define Callback(ClassName, cif)

%inline %{
typedef struct tag##ClassName {
	ffi_closure *pcl;
} ClassName;
%}

%extend ClassName {

	ClassName(PyObject *pyfunc) {
		ClassName* cb = (ClassName*) malloc(sizeof(ClassName));
		printf("##ClassName##()\n");
		cb->pcl = mmap(NULL, sizeof(ffi_closure), 
	 	                PROT_READ|PROT_WRITE|PROT_EXEC, 
	 	                MAP_PRIVATE | MAP_ANON, -1, 0); 

		if (ffi_prep_closure(cb->pcl, &cif, internal_callback,
			 (void *)pyfunc ) != FFI_OK) {
			// TODO: PyError_Set()
			printf("creating closure error");
		}
		return cb;
	}

	~ClassName() {
		printf("~##ClassName##()\n");
	}
};

%enddef

// Closures information

%{
// I cannot pass {... } to the macro :-(
// typedef int (*iv_handler)(int type, int par1, int par2);
static ffi_type *iv_handler_cl_arg_types[] = {&ffi_type_sint, &ffi_type_sint, &ffi_type_sint, NULL};
static ffi_cif iv_handler_cif;
%}
Callback(HandlerCallback, iv_handler_cif)

%{
// typedef void (*iv_timerproc)();
static ffi_type *iv_timeproc_cl_arg_types[] = {&ffi_type_void, NULL};
static ffi_cif iv_timeproc_cif;
%}
Callback(TimeProcCallback, iv_timeproc_cif)

%{
// typedef void (*iv_menuhandler)(int index);
static ffi_type *iv_menuhandler_cl_arg_types[] = {&ffi_type_void, &ffi_type_sint, NULL};
static ffi_cif iv_menuhandler_cif;
%}
Callback(MenuHandlerCallback, iv_menuhandler_cif)

%{
// typedef void (*iv_keyboardhandler)(char *text);
static ffi_type *iv_keyboardhandler_cl_arg_types[] = {&ffi_type_void, &ffi_type_pointer, NULL};
static ffi_cif iv_keyboardhandler_cif;
%}
Callback(KeyboardHandlerCallback, iv_keyboardhandler_cif)

%{
// typedef void (*iv_dialoghandler)(int button);
static ffi_type *iv_dialoghandler_cl_arg_types[] = {&ffi_type_void, NULL};
static ffi_cif iv_dialoghandler_cif;
%}
//Callback(, iv_dialoghandler_cif)

%{
// typedef void (*iv_timeedithandler)(long newtime);
static ffi_type *iv_timeedithandler_cl_arg_types[] = {&ffi_type_void, NULL};
static ffi_cif iv_timeedithandler_cif;
%}
//Callback(, iv_timeedithandler_cif)

%{
// typedef void (*iv_fontselecthandler)(char *fontr, char *fontb, char *fonti, char *fontbi);
static ffi_type *iv_fontselecthandler_cl_arg_types[] = {&ffi_type_void, NULL};
static ffi_cif iv_fontselecthandler_cif;
%}
//Callback(, iv_fontselecthandler_cif)

%{
// typedef void (*iv_dirselecthandler)(char *path);
static ffi_type *iv_dirselecthandler_cl_arg_types[] = {&ffi_type_void, NULL};
static ffi_cif iv_dirselecthandler_cif;
%}
//Callback(, iv_dirselecthandler_cif)

%{
// typedef void (*iv_confighandler)();
static ffi_type *iv_confighandler_cl_arg_types[] = {&ffi_type_void, NULL};
static ffi_cif iv_confighandler_cif;
%}
//Callback(, iv_confighandler_cif)

%{
// typedef void (*iv_itemchangehandler)(char *name);
static ffi_type *iv_itemchangehandler_cl_arg_types[] = {&ffi_type_void, NULL};
static ffi_cif iv_itemchangehandler_cif;
%}
//Callback(, iv_itemchangehandler_cif)

%{
// typedef void (*iv_pageselecthandler)(int page);
static ffi_type *iv_pageselecthandler_cl_arg_types[] = {&ffi_type_void, NULL};
static ffi_cif iv_pageselecthandler_cif;
%}
//Callback(, iv_pageselecthandler_cif)

%{
// typedef void (*iv_bmkhandler)(int action, int page, long long position);
static ffi_type *iv_bmkhandler_cl_arg_types[] = {&ffi_type_void, NULL};
static ffi_cif iv_bmkhandler_cif;
%}
//Callback(, iv_bmkhandler_cif)

%{
// typedef void (*iv_tochandler)(long long position);
static ffi_type *iv_tochandler_cl_arg_types[] = {&ffi_type_void, NULL};
static ffi_cif iv_tochandler_cif;
%}
//Callback(, iv_tochandler_cif)

%{
//!: Not used
//typedef void (*iv_itempaint)(int x, int y, int index, int selected);
//static ffi_type *iv_itempaint_cl_arg_types[] = {&ffi_type_void, NULL};
//static ffi_cif iv_itempaint_cif;
%}
//Callback(, iv_itempaint_cif)

%{
// typedef void (*iv_listhandler)(int action, int x, int y, int idx, int state);
static ffi_type *iv_listhandler_cl_arg_types[] = {&ffi_type_void, NULL};
static ffi_cif iv_listhandler_cif;
%}
//Callback(, iv_listhandler_cif)

%{
// typedef void (*iv_rotatehandler)(int direction);
static ffi_type *iv_rotatehandler_cl_arg_types[] = {&ffi_type_void, NULL};
static ffi_cif iv_rotatehandler_cif;
%}
//Callback(, iv_rotatehandler_cif)

%{
//!: Not used
//typedef int (*iv_turnproc)(int direction);
//static ffi_type *iv_turnproc_cl_arg_types[] = {&ffi_type_void, NULL};
//static ffi_cif iv_turnproc_cif;
%}
//Callback(, iv_turnproc_cif)

%{
//!: Not used?
//typedef int (*iv_recurser)(char *path, int type, void *data);
//static ffi_type *iv_recurser_cl_arg_types[] = {&ffi_type_void, NULL};
//static ffi_cif iv_recurser_cif;
%}
//Callback(, iv_recurser_cif)

%{
// typedef int (*iv_hashenumproc)(char *name, void *value, void *userdata);
static ffi_type *iv_hashenumproc_cl_arg_types[] = {&ffi_type_void, NULL};
static ffi_cif iv_hashenumproc_cif;
%}
//Callback(, iv_hashenumproc_cif)

%{
// typedef int (*iv_hashcmpproc)(char *name1, void *value1, char *name2, void *value2);
static ffi_type *iv_hashcmpproc_cl_arg_types[] = {&ffi_type_void, NULL};
static ffi_cif iv_hashcmpproc_cif;
%}
//Callback(, iv_hashcmpproc_cif)

%{
// typedef void * (*iv_hashaddproc)(void *data);
static ffi_type *iv_hashaddproc_cl_arg_types[] = {&ffi_type_void, NULL};
static ffi_cif iv_hashaddproc_cif;
%}
//Callback(, iv_hashaddproc_cif)

%{
// typedef void (*iv_hashdelproc)(void *data);
static ffi_type *iv_hashdelproc_cl_arg_types[] = {&ffi_type_void, NULL};
static ffi_cif iv_hashdelproc_cif;
%}
//Callback(, iv_hashdelproc_cif)


%inline %{
void _initialize_ffi_types()
{
	if (ffi_prep_cif(&iv_handler_cif, FFI_DEFAULT_ABI, 1, &ffi_type_void, iv_handler_cl_arg_types) != FFI_OK ) {
		goto fail;		
	}

	return ;
fail:
	printf("Callback type initialization error");
//	TODO: SetError
}
%}

// FFI types initialization
%pythoncode %{
_initialize_ffi_types();
%}
