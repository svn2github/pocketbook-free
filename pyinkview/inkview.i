// Python wrapper for inkview
%module inkview

%{
#include "inkview.h"
%}

//TODO: Not needed
// Callback function
%typemap(in) PyObject* pyfunc {
	if (!PyCallable_Check($input)) {
		PyErr_SetString(PyExc_TypeError, "Need a callable object!");
		return NULL;
	}
	$1 = $input;
}


//TODO: Use macro
// void Stretch(const unsigned char *src, int format, int sw, int sh, int scanline, int dx, int dy, int dw, int dh, int rotate);
%typemap(in) (const unsigned char* src) {
	/* Check if is a list */
	if (PyList_Check($input)) {
		int i;
		int size = PyList_Size($input);
		$1 = (unsigned char *) malloc(size * sizeof(char));
		for (i = 0; i < size; ++i) {
			PyObject *o = PyList_GetItem($input,i);
			if (PyLong_Check(o)) {
				/* Chack? */
				$1[i] = (unsigned char) PyLong_AsLong(PyList_GetItem($input,i));
			} else {
				PyErr_SetString(PyExc_TypeError,"list must contain unsigned chars");
				free($1);
				return NULL;
			}
		}
	} else {
		PyErr_SetString(PyExc_TypeError,"not a list");
		return NULL;
	}
}

%typemap(freearg) (const unsigned char* src) {
	free((unsigned char *) $1);
}


// char **EnumFonts();
// char **EnumKeyboards();
// char ** EnumLanguages();
// char ** EnumThemes();
// char **GetLastOpen();
// char **EnumNotepads();
// char **EnumDictionaries();
%typemap(out) char** {
	char** p = $1;
	$result = PyList_New(0);
	while (*p != 0) {
	    PyObject* s = SWIG_Python_str_FromChar(*p);
	    PyList_Append($result, s);
	    ++p;
	}
}

%include "icallbacks.i"
%include "imain.i"
%include "idialog.i"
%include "itimer.i"
%include "imenu.i"
%include "imenu3x3.i"
%include "ikeyboard.i"
%include "idirselect.i"
%include "ilist.i"

//icontent
%ignore OpenContents;
//ipageselect
%ignore OpenPageSelector;
//ibookmark
%ignore OpenBookmarks;
%ignore SwitchBookmark;
//irotatebox
%ignore OpenRotateBox;
//itimeedit
%ignore OpenTimeEdit;
//ifontselect
%ignore OpenFontSelector;


%include "inkview.h"	//TODO: This file should contain only constants! Rename it with indview_const.h or something else..

%inline %{
extern ibitmap background, books, m3x3;	
extern ibitmap item1, item2;
%}


%include "icontent.i"
%include "ipageselect.i"
%include "ibookmark.i"	//TODO: in progress. modify bmklist and other
%include "irotatebox.i"
%include "itimeedit.i"
%include "ifontselect.i"


//Handlers in progress:
%include "iconfigedit.i"

//???
%include "ihash.i"
//TODO: icanvas - can be edited manually!

