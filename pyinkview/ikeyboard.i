//iv_keyboardhandler:
// typedef void (*iv_keyboardhandler)(char *text);
// void OpenKeyboard(char *title, char *buffer, int maxlen, int flags, iv_keyboardhandler hproc);
// void OpenCustomKeyboard(char *filename, char *title, char *buffer, int maxlen, int flags, iv_keyboardhandler hproc);
//
//!: 'buffer' can be updated
//

%{
#define PYOPENKEYBOARD_BUFFER_SIZE 1024
static char PyOpenKeyboard_buffer[PYOPENKEYBOARD_BUFFER_SIZE];
%}

%typemap(in) (char* buffer, int maxlen) {
	char *s = NULL;
	if (!PyUnicode_Check($input)) {
		PyErr_SetString(PyExc_TypeError, "Expected a string");
		return NULL;
	}
	s = SWIG_Python_str_AsChar($input);
	if (s == NULL) {
		return NULL;
	}
	PyOpenKeyboard_buffer[0] = '\0';
   	strncpy(PyOpenKeyboard_buffer, s, PYOPENKEYBOARD_BUFFER_SIZE);
   	free(s);	//TODO: Check that this is really needed
   	PyOpenKeyboard_buffer[PYOPENKEYBOARD_BUFFER_SIZE-1] = '\0';
   	$1 = PyOpenKeyboard_buffer;
   	$2 = PYOPENKEYBOARD_BUFFER_SIZE-1;
}

callbackTypemapIn(KeyboardHandler, iv_keyboardhandler);

void OpenKeyboard(char *title, char *buffer, int maxlen, int flags, iv_keyboardhandler hproc);
void OpenCustomKeyboard(char *filename, char *title, char *buffer, int maxlen, int flags, iv_keyboardhandler hproc);

%clear (char* buffer, int maxlen);
%clear iv_keyboardhandler;
