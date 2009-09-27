//iv_handler:
//
// typedef int (*iv_handler)(int type, int par1, int par2);
//
// void InkViewMain(iv_handler h);
// iv_handler SetEventHandler(iv_handler hproc);
// iv_handler GetEventHandler();
//
// void SendEvent(iv_handler hproc, int type, int par1, int par2);

callbackTypemapIn(MainHandler, iv_handler);

%typemap(out) iv_handler {
	Py_INCREF(Py_None);
	$result = Py_None;
}

void InkViewMain(iv_handler h);
iv_handler SetEventHandler(iv_handler hproc);
iv_handler GetEventHandler();	//TODO:
%ignore GetEventHandler;
void SendEvent(iv_handler hproc, int type, int par1, int par2);

%clear iv_handler;

