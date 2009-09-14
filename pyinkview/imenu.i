//iv_menuhandler:
// void OpenMenu(imenu *menu, int pos, int x, int y, iv_menuhandler hproc);

%{
// list<imenu> <-> carray<imenu> utils

static int sequence_to_imenu(PyObject *item, imenu *menu);
static void delete_imenu_array(imenu* array);

static imenu* 
imenu_sequence_to_array(PyObject* list)
{
	imenu* array = NULL;
	if (PyList_Check(list)) {
		int i;
		int size = PyList_Size(list);
		array = (imenu*) malloc((size+1) * sizeof(imenu));
		memset(array, 0, (size+1) * sizeof(imenu));
		for (i = 0; i < size; ++i) {
			PyObject* item = PyList_GetItem(list, i);
  			if (sequence_to_imenu(item, &array[i])) {
				delete_imenu_array(array);
				return NULL;
			}
		}		
		array[i].type = 0;
	}
	else {
		PyErr_SetString(PyExc_TypeError,"not a menuitem list");
		return NULL;
	}
	return array;
}

static int
sequence_to_imenu(PyObject *item, imenu *menu)
{
	if (PyList_Check(item) || PyList_Size(item) != 4) {
		PyObject* type = PyList_GetItem(item, 0);
		PyObject* index = PyList_GetItem(item, 1);
		PyObject* text = PyList_GetItem(item, 2);
		PyObject* submenu = PyList_GetItem(item, 3);
		menu->type = PyLong_AsLong(type);
		if (PyErr_Occurred()) {
			return 1;	// Fail
		}
		menu->index = PyLong_AsLong(index);
		if (PyErr_Occurred()) {
			return 1;	// Fail
		}
#if PY_VERSION_HEX>=0x03000000
  		if (PyUnicode_Check(text)) 
#else  
  		if (PyString_Check(text))
#endif
			menu->text = SWIG_Python_str_AsChar(text);
		else 
			menu->text = NULL;	//TODO: Add TypeError exception
		if (PyErr_Occurred()) {
			return 1;	// Fail
		}
		menu->submenu = NULL;
		if (submenu != Py_None) {
			menu->submenu = imenu_sequence_to_array(submenu);
			if (menu->submenu == NULL) {
				menu->type = 0;
				return 1; 	// Fail
			}
		}
	}
	else {
		PyErr_SetString(PyExc_TypeError,"menuitem should be a list of 4 elements");
		return 1;
	}
	return 0;
}

static void 
delete_imenu_array(imenu* array)
{
	imenu* p = array;
	if (p == NULL)
		return ;
	while (p->type != 0) {
		if (p->submenu != NULL) {
			delete_imenu_array(p->submenu);
		}
		++p;
	}
	free(array);
}


static PyObject *PyOpenMenu_pyfunc_ptr = NULL;
static imenu *PyOpenMenu_menu = NULL;

static void PyOpenMenu_callback(int index)
{
	PyObject *arglist;
	PyObject *result;

	if (!PyOpenMenu_pyfunc_ptr)
		return;

	arglist = Py_BuildValue("(i)", index );             
	result = PyEval_CallObject(PyOpenMenu_pyfunc_ptr, arglist);     
	Py_DECREF(arglist);                           
	if (result) {                                 
		Py_XDECREF(result);
	}
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}

	Py_DECREF(PyOpenMenu_pyfunc_ptr);
	PyOpenMenu_pyfunc_ptr = NULL;   
	delete_imenu_array( PyOpenMenu_menu );
	PyOpenMenu_menu = NULL;

	return;
}

void PyOpenMenu(PyObject *menu, int pos, int x, int y, PyObject *pyfunc)
{
	if (PyOpenMenu_pyfunc_ptr) {
		Py_DECREF(PyOpenMenu_pyfunc_ptr);
		PyOpenMenu_pyfunc_ptr = NULL;
		delete_imenu_array( PyOpenMenu_menu );
		PyOpenMenu_menu = NULL;
	}
	PyOpenMenu_menu = imenu_sequence_to_array(menu);
	if (PyOpenMenu_menu != NULL) {
		PyOpenMenu_pyfunc_ptr = pyfunc;
		Py_INCREF(pyfunc);
		OpenMenu(PyOpenMenu_menu, pos, x, y, PyOpenMenu_callback);
	}
}

%}

%rename(OpenMenu) PyOpenMenu;
extern void PyOpenMenu(PyObject *menu, int pos, int x, int y, PyObject *pyfunc);
