#ifndef HELPER_H
#define HELPER_H

#include "convert.h"
#include "pbcontrols.h"

#include "qsp/qsp.h"

#define APP_VERSION "5.5.9 / 0.94"

void ShowError();
std::string GetQuestPath();
std::string GetFileExtension(std::string path);
size_t FindSubstr(std::string str, std::string substr);
std::string GetHTMLPropValue(std::string str, std::string propName);
void ParseText(const char *src_text, PBListBox &listBox, std::vector<std::pair<std::string, std::string> > &links);
std::string ClearHTMLTags(std::string &text);

#endif
