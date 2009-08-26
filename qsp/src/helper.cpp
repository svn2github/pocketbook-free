#include "helper.h"

void ShowError()
{
	char message[2048];
	QSP_CHAR *loc;
	long code, where, line;
	QSPGetLastErrorData(&code, &loc, &where, &line);
	const QSP_CHAR *desc = QSPGetErrorDesc(code);
	if (loc)
		sprintf(message, "Location: %s\nArea: %s\nLine: %ld\nCode: %ld\nDesc: %s",
			loc,
			where == QSP_AREA_ONLOCVISIT ? "on visit" : "on action",
			line,
			code,
			desc
		);
	else
		sprintf(message, "Code: %ld\nDesc: %s",
			code,
			desc
		);
	Message(ICON_ERROR, "Error", message, 5000);
}

std::string GetQuestPath()
{
	std::string fullPath (QSPGetQstFullPath());
	
	size_t slash_pos = fullPath.find_last_of("/\\");
	if (slash_pos != std::string::npos)
		return fullPath.substr(0, slash_pos+1);
	
	fullPath.clear();
	return fullPath;
}

std::string GetFileExtension(std::string path)
{
	size_t dot_pos = path.find_last_of(".");
	if (dot_pos != std::string::npos)
		return path.substr(dot_pos+1);
	
	std::string empty;
	return empty;
}

size_t FindSubstr(std::string str, std::string substr)
{
	size_t found = std::string::npos;
	if (str.size() > substr.size())
	{
		for (size_t i = 0; i < str.size() - substr.size(); i++)
		{
			if (str.substr(i, substr.size()) == substr)
			{
				found = i;
				break;
			}
		}
	}
	
	return found;
}

std::string GetHTMLPropValue(std::string str, std::string propName)
{
	size_t prop_pos1 = std::string::npos, prop_pos2 = std::string::npos;
	std::string value;
		
	prop_pos1 = FindSubstr(str, propName);
	if (prop_pos1 == std::string::npos)
		return value;
	
	for (int i = prop_pos1+propName.size(); i < (int)str.size(); i++)
	{
		if (str[i] != ' ' && str[i] != '=')
		{
			if (str[i] == '"')
			{
				prop_pos1 = i+1;
				prop_pos2 = str.find_first_of('"', prop_pos1)-1;
			}
			else
			{
				prop_pos1 = i;
				size_t p_pos2_1 =  str.find_first_of(' ', prop_pos1)-1;
				size_t p_pos2_2 =  str.find_first_of('>', prop_pos1)-1;
				prop_pos2 = p_pos2_1 < p_pos2_2 ? p_pos2_1 : p_pos2_2;
			}
			break;
		}
	}
			
	if (prop_pos2 != std::string::npos)
		value.assign (str.substr(prop_pos1, prop_pos2-prop_pos1+1));
		
	return value;
}

std::string ClearHTMLTags(std::string &text)
{
	size_t pos1, pos2;
	while (1)
	{
		pos1 = text.find_first_of('<', 0);
		if (pos1 == std::string::npos)
			break;
			
		pos2 = text.find_first_of('>', pos1);
		if (pos2 == std::string::npos)
			break;
		
		text.erase(pos1, pos2-pos1+1);
	}
	
	return text;
}

ibitmap *OpenImage(std::string fileName)
{
	std::string ext = GetFileExtension(fileName);
	if (ext == "jpg" || ext == "jpeg" || ext == "JPG" || ext == "JPEG")
		return LoadJPEG((char*)fileName.c_str(), ScreenHeight()/2, ScreenHeight()/2, 100, 100, 1);
	else
		return LoadBitmap((char*)fileName.c_str());
}

void ParseText(const char *src_text, PBListBox &listBox, std::vector<std::pair<std::string, std::string> > &links)
{
	links.clear();
	
	if (src_text == 0)
		return;
	
	if (strlen(src_text) == 0)
		return;
		
	std::string text(src_text);
	size_t pos1 = 0, pos2 = 0, img_pos = 0, par_begin = 0, par_end = 0, aux_symb_pos = 0;
	
	// getting paragraphs
	size_t max_pos = text.size()-1;
	while (par_end < max_pos)
	{
		par_end = text.find_first_of('\n', par_begin);
		if (par_end == std::string::npos)
			par_end = max_pos+1;
		std::string par(text.substr(par_begin, par_end-par_begin));

		// clearing paragraph text from html tags
		while (1)
		{	
			pos1 = par.find_first_of('<', 0);
			if (pos1 == std::string::npos)
				break;
				
			pos2 = par.find_first_of('>', pos1);
			if (pos2 == std::string::npos)
				break;
				
			std::string tag = par.substr(pos1, pos2-pos1+1);	
			std::string link = GetHTMLPropValue(tag, "href");

			if (link.size() > 0)
			{
				std::string link_name;
				link_name += "[";
				link_name += par.substr(pos2+1, par.find_first_of('<', pos2)-pos2-1);
				link_name += "]";
				
				to_utf8((unsigned char *)link_name.c_str(), &link_name, koi8_to_unicode);
				links.push_back(make_pair(link_name, link));
			}
			else
			{
				img_pos = FindSubstr(tag, "img");
				if (img_pos != std::string::npos)
				{
					std::string img_src = GetHTMLPropValue(tag, "src");
					std::string path; // = GetQuestPath();
					path += img_src;
					to_utf8((unsigned char *)path.c_str(), &path, koi8_to_unicode);
					ibitmap *image = OpenImage(path);
					PBListBoxItem *newItem = listBox.AddItem(image);
					newItem->SetCanBeFocused(false);
				}
			}

			par.erase(pos1, pos2-pos1+1);
		}
			
		do
		{
			aux_symb_pos = par.find_first_of("\r");
			if (aux_symb_pos != std::string::npos)
				par.erase(aux_symb_pos, 1);
		}
		while (aux_symb_pos != std::string::npos);
		if (par.size() > 0)
		{
			to_utf8((unsigned char *)par.c_str(), &par, koi8_to_unicode);
			PBListBoxItem *newItem = listBox.AddItem(par);
			newItem->SetCanBeFocused(false);
		}
		
		par_begin = par_end + 1;
	}
}

void SetStringToCharString(char *dest, std::string src, int dest_size)
{
	int size = src.size() > dest_size-1 ? dest_size-1 : src.size();
	strncpy(dest, src.c_str(), size);
	dest[size] = 0;
}
