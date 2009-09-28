//iv_timeedithandler:
// typedef void (*iv_timeedithandler)(long newtime);
// void OpenTimeEdit(char *title, int x, int y, long intime, iv_timeedithandler hproc);

callbackTypemapIn(TimeEditHandler, iv_timeedithandler);
void OpenTimeEdit(char *title, int x, int y, long intime, iv_timeedithandler hproc);
