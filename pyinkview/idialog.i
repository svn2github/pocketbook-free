//iv_dialoghandler:
// typedef void (*iv_dialoghandler)(int button);
// void Dialog(int icon, char *title, char *text, char *button1, char *button2, iv_dialoghandler hproc);

callbackTypemapIn(DialogHandler, iv_dialoghandler);
void Dialog(int icon, char *title, char *text, char *button1, char *button2, iv_dialoghandler hproc);

