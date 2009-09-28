//iv_listhandler:
// typedef void (*iv_listhandler)(int action, int x, int y, int idx, int state);
// void OpenList(char *title, ibitmap *background, int itemw, int itemh, int itemcount, int cpos, iv_listhandler hproc);
// void OpenDummyList(char *title, ibitmap *background, char *text, iv_listhandler hproc);

callbackTypemapIn(ListHandler, iv_listhandler);

void OpenList(char *title, ibitmap *background, int itemw, int itemh, int itemcount, int cpos, iv_listhandler hproc);
void OpenDummyList(char *title, ibitmap *background, char *text, iv_listhandler hproc);
