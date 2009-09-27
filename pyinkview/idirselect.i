//iv_dirselecthandler:
// typedef void (*iv_dirselecthandler)(char *path);
// void OpenDirectorySelector(char *title, char *buf, int len, iv_dirselecthandler hproc);

%{
#define PYOPENDIRSELECT_BUFFER_SIZE 1024
//Actually, MAX_PATH would be enouth
static char PyOpenDirectorySelector_buffer[PYOPENDIRSELECT_BUFFER_SIZE+1];
%}

%typemap(in) (char *buf, int len) {
	char *s = NULL;
	if (!PyUnicode_Check($input)) {
		PyErr_SetString(PyExc_TypeError, "Expected a string");
		return NULL;
	}
	s = SWIG_Python_str_AsChar($input);
	if (s == NULL) {
		return NULL;
	}
   	strncpy(PyOpenDirectorySelector_buffer, s, PYOPENKEYBOARD_BUFFER_SIZE);
   	free(s);	//TODO: Check that this is really needed
   	PyOpenDirectorySelector_buffer[PYOPENDIRSELECT_BUFFER_SIZE] = '\0';
   	$1 = PyOpenDirectorySelector_buffer;
   	$2 = PYOPENDIRSELECT_BUFFER_SIZE;
}

callbackTypemapIn(DirSelectHandler, iv_dirselecthandler);

void OpenDirectorySelector(char *title, char *buf, int len, iv_dirselecthandler hproc);

%clear (char *buf, int len);
%clear iv_dirselecthandler;
