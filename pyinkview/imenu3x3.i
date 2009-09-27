//iv_menuhandler:
// typedef void (*iv_menuhandler)(int index);
// void OpenMenu3x3(const ibitmap *mbitmap, const char *strings[9], iv_menuhandler hproc);

%{
static char** 
strings_to_stringarray(PyObject* list)
{
	char** array = NULL;
	if (PyList_Check(list)) {
		int i;
		int size = PyList_Size(list);
		array = (char**) malloc(size * sizeof(char*));
		for (i = 0; i < size; ++i) {
			PyObject* item = PyList_GetItem(list, i);
  			if (item != Py_None) {
				array[i] = SWIG_Python_str_AsChar(item);
			}
			else {
				array[i] = NULL;
			}
		}		
	}
	else {
		PyErr_SetString(PyExc_TypeError,"not a menuitem list");
		return NULL;
	}
	return array;
}
%}

%{
static char** PyOpenMenu3x3_menu = NULL;
%}

%typemap(in) const char *strings[9] {
	if (PyOpenMenu3x3_menu != NULL) {
		free(PyOpenMenu3x3_menu);
		PyOpenMenu3x3_menu = NULL;
	}
	PyOpenMenu3x3_menu = strings_to_stringarray($input);
	//TODO: Check == NULL
	$1 = PyOpenMenu3x3_menu;
}

callbackTypemapIn(MenuHandler, iv_menuhandler);

void OpenMenu3x3(const ibitmap *mbitmap, const char *strings[9], iv_menuhandler hproc);

%clear const char *strings[9];
%clear iv_menuhandler;

//TODO: free(PyOpenMenu3x3_menu) when the module is unloaded
