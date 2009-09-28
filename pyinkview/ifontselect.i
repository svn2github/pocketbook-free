//iv_fontselecthandler:
// typedef void (*iv_fontselecthandler)(char *fontr, char *fontb, char *fonti, char *fontbi);
// void OpenFontSelector(char *title, char *font, int with_size, iv_fontselecthandler hproc);

callbackTypemapIn(FontSelectHandler, iv_fontselecthandler);
void OpenFontSelector(char *title, char *font, int with_size, iv_fontselecthandler hproc);
