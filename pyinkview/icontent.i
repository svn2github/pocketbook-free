//iv_tochandler:
// typedef void (*iv_tochandler)(long long position);
// void OpenContents(tocentry *toc, int count, long long position, iv_tochandler hproc);

%{

static tocentry* PyOpenContents_toc = NULL;

static int sequence_to_tocentry(PyObject* item, tocentry* tocitem)
{
	if (PyList_Check(item) || PyList_Size(item) != 4) {
		PyObject* level = PyList_GetItem(item, 0);
		PyObject* page = PyList_GetItem(item, 1);
		PyObject* position = PyList_GetItem(item, 2);
		PyObject* text = PyList_GetItem(item, 3);
		
		tocitem->level = PyLong_AsLong(level);
		if (PyErr_Occurred()) 
			return 1;	// Fail

		tocitem->page = PyLong_AsLong(page);
		if (PyErr_Occurred()) 
			return 1;	// Fail

		tocitem->position = PyLong_AsLong(position);
		if (PyErr_Occurred()) 
			return 1;	// Fail

#if PY_VERSION_HEX>=0x03000000
  		if (PyUnicode_Check(text)) 
#else  
  		if (PyString_Check(text))
#endif
			tocitem->text = SWIG_Python_str_AsChar(text);
		else 
			tocitem->text = NULL;	//TODO: Add TypeError exception

		if (PyErr_Occurred())
			return 1;	// Fail
	}
	else {
		PyErr_SetString(PyExc_TypeError,"tocentry should be a list of 4 elements");
		return 1;
	}
	return 0;

}

static tocentry* convert_list_to_tocentry(PyObject* toc, int* pcount)
{
	if (!PyList_Check(toc)) {
		PyErr_SetString(PyExc_TypeError,"not a content list");
		return NULL;
	}
	tocentry* array = NULL;
	int i;
	int size = PyList_Size(toc);
	array = (tocentry*) malloc(size * sizeof(tocentry));
	for (i = 0; i < size; ++i) {
		PyObject* item = PyList_GetItem(toc, i);
		if (sequence_to_tocentry(item, &array[i])) {
			free(array);	// Fail
			return NULL;
		}
	}
	*pcount = size;
	return array;
}

%}

%typemap(in) (tocentry *toc, int count) {
	int count = 0;
	if (PyOpenContents_toc != NULL) {
		free(PyOpenContents_toc);
		PyOpenContents_toc = NULL;
	}
	PyOpenContents_toc = convert_list_to_tocentry($input, &count);
	if (PyOpenContents_toc == NULL) {
		return NULL;
	}
	$1 = PyOpenContents_toc;
	$2 = count;
}

callbackTypemapIn(TocHandler, iv_tochandler);
void OpenContents(tocentry *toc, int count, long long position, iv_tochandler hproc);
