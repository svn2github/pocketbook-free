%module a;
%{
#include <Python.h>
#include <stdlib.h>
#include <ffi.h>
#include <sys/mman.h>
//#include <unistd.h>
%}

%{
ffi_cif cif;
ffi_type *cl_arg_types[] = {&ffi_type_sint, NULL};

typedef int (*my_handler)(int a);

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

%inline %{
typedef struct tagCallback {
	ffi_closure *pcl;
} Callback;
%}

%extend Callback {
	Callback(PyObject *pyfunc) {
		Callback* cb = (Callback*) malloc(sizeof(Callback));
		printf("Callback::Callback()\n");
		cb->pcl = mmap(NULL, sizeof(ffi_closure), 
	 	                PROT_READ|PROT_WRITE|PROT_EXEC, 
	 	                MAP_PRIVATE|MAP_ANON, -1, 0); 

		if (ffi_prep_closure(cb->pcl, &cif, internal_callback,
			 (void *)pyfunc ) != FFI_OK) {
			printf("creating closure error");
		}
		return cb;
	}
	~Callback() {
		printf("Callback::~Callback()\n");
	}
};


%{
void initialize_module()
{
	if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1, &ffi_type_void, cl_arg_types) != FFI_OK ) {
		printf("Callback type initialization error");
	}
}

%}

void initialize_module(/*PyObject *self, PyObject *args*/);



%{
my_handler handler;
void set_callback(my_handler callback)
{
	handler = callback;
}

void call_callback()
{
	handler(11);
}

%}

%{
void setCallback(Callback* callback)
{
	set_callback((my_handler)callback->pcl);	
}

void callCallback()
{
	call_callback();
}

%}

extern void setCallback(Callback* callback);
extern void callCallback();


%pythoncode %{
initialize_module()
%}
