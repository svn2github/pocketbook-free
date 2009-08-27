#include "main.h"

#define scalev(x) ((scale * (x)) / 100)

extern const ibitmap item;

using namespace std;

//страны
vector<structcountry> countries;
//города
vector<structcity> cities;
//все станции в городе (вершины графа)
vector<station> sts;
//все переезды (ребра графа)
vector<edge> edgs;
//все переходы
vector<transfer> tredgs;
//линии
vector<line> lines;

//указатель на текущее содержание (при каждом выборе содержания память надо освобождать)
tocentry * contcities;

//Для поиска станций
char searchst[STATIONNAME_SIZE] = "";

//текущая выбранная станция
int curst = -1;
//текущая выбранный город
int curcity = -1;

//масштаб (в процентах)
int scale = 100;

//масштаб при котором карта будет вписана в экран
int fscale = 100;
//максимальные x и y (для вычисления масштаба) с учетом offset
point pmax = {0, 0};
//текущее положение экрана (левый верхний угол)
point cur = {0, 0};
//шаг сдвига экрана
point step = {0, 0};

//ориентация экрана
int orient = 0;

//диаметр станции (по умолчанию 12)
int diameter = 12;
//ширина линии (по умолчанию 8)
int lineswidth = 8;

//Меню
imenu map_menu[20];

iconfig * cfg = NULL;
static iconfigedit confedit[] = {
	{ CFG_TEXT, &item, "Path with disk", "Example of text edit control", "path", DEFAULT_DIR, NULL, NULL },
	{ NULL, NULL, 0, NULL, NULL}
};

bool dirchoosen = false;
char def_dir[1024];

//Создание меню в зависимости от того, что у нас происходит на экране
void create_menu() {
	memset(map_menu, 0, sizeof(map_menu));
	int n = 0;
	map_menu[n].type = ITEM_HEADER;
	map_menu[n].index = 0;
	map_menu[n].text = "Menu";
	map_menu[n].submenu = NULL;
	n++;
	
	if (curst != -1) {
		map_menu[n].type = ITEM_ACTIVE;
		map_menu[n].index = 107;
		map_menu[n].text = "Clear search";
		map_menu[n].submenu = NULL;
		n++;
	}
	
	map_menu[n].type = ITEM_ACTIVE;
	map_menu[n].index = 102;
	map_menu[n].text = "Find station";
	map_menu[n].submenu = NULL;
	n++;

	map_menu[n].type = ITEM_SEPARATOR;
	map_menu[n].index = 0;
	map_menu[n].text = NULL;
	map_menu[n].submenu = NULL;
	n++;

	if (scale < 150) {
		map_menu[n].type = ITEM_ACTIVE;
		map_menu[n].index = 105;
		map_menu[n].text = "Zoom +";
		map_menu[n].submenu = NULL;
		n++;
	}

	if (scale != fscale && fscale < 100) {
		map_menu[n].type = ITEM_ACTIVE;
		map_menu[n].index = 101;
		map_menu[n].text = "Zoom -";
		map_menu[n].submenu = NULL;
		n++;
	}
	map_menu[n].type = ITEM_ACTIVE;
	map_menu[n].index = 103;
	map_menu[n].text = "Rotate";
	map_menu[n].submenu = NULL;
	n++;
	map_menu[n].type = ITEM_SEPARATOR;
	map_menu[n].index = 0;
	map_menu[n].text = NULL;
	map_menu[n].submenu = NULL;
	n++;
	
	if (cities.size() > 1) {
		map_menu[n].type = ITEM_ACTIVE;
		map_menu[n].index = 104;
		map_menu[n].text = "Other city";
		map_menu[n].submenu = NULL;
		n++;

		map_menu[n].type = ITEM_SEPARATOR;
		map_menu[n].index = 0;
		map_menu[n].text = NULL;
		map_menu[n].submenu = NULL;
		n++;
	}
	
	map_menu[n].type = ITEM_ACTIVE;
	map_menu[n].index = 108;
	map_menu[n].text = "Preference";
	map_menu[n].submenu = NULL;
	n++;

	map_menu[n].type = ITEM_SEPARATOR;
	map_menu[n].index = 0;
	map_menu[n].text = NULL;
	map_menu[n].submenu = NULL;
	n++;
	
	map_menu[n].type = ITEM_ACTIVE;
	map_menu[n].index = 106;
	map_menu[n].text = "Exit";
	map_menu[n].submenu = NULL;
	n++;

	map_menu[n].type = 0;
	map_menu[n].index = 0;
	map_menu[n].text = NULL;
	map_menu[n].submenu = NULL;
}

//При выборе города инициализируем его карту и переходим на нее
void city_selected(long long int page) {
	curcity = page;
	//Инициализируем карту
	string curmap = cities[page-1].path + string("Metro.map");
	parse_map(curmap.data());
	delete[] contcities;

	//Переходим на экран с картой
	SetEventHandler(map_paint_handler);
}

//При выборе станции заносим номер станции в глобальную переменную и перерисовываем карту
void station_selected(long long int page) {
	delete[] contcities;
	curst = page;
	abs_go(sts[curst].x, sts[curst].y);
}

//Инициализация карт
//Смотрим в каких папках есть карты и файл .cty, берем оттуда названия городов
void read_city(char * cdir) {
	//Просматриваем директорию
	DIR * dir = opendir(cdir);
	//
	countries.clear();
	cities.clear();
	if (dir) {
		struct dirent * ep;
		structcity city;
		char * country;
		const char * cityname;
		vector<structcountry>::iterator countryit, countryend;
		int fc = 0;
		int i = 0;
		structcountry curcountry;
		while ((ep = readdir(dir)) != NULL) {
			if (ep->d_name[0] == '.' && (ep->d_name[1] == 0 || (ep->d_name[1] == '.' && ep->d_name[2] == 0))) {
				continue;
			}

			memset(&city, 0, sizeof(city));
			string citypath;
			citypath = cdir + string("/") + ep->d_name + string("/") + ep->d_name + string(".cty");
			CSimpleIni citycty(true);

			SI_Error rc = load_cp1251_ini_file(&citycty,  const_cast<char*> (citypath.data()));
			if (rc < 0) {
				continue;
			}
			country =  const_cast<char*> (citycty.GetValue("Options", "Country", NULL));
			if (!country) {continue;}

			strcpy(city.path, string(cdir + string("/") + ep->d_name + string("/")).c_str());

			cityname = citycty.GetValue("Options", "RusName", NULL);
			if (!cityname) {
				cityname = citycty.GetValue("Options", "Cityname", NULL);
			}
			strcpy(city.name, cityname);
			cities.push_back(city);

			countryit = countries.begin();
			countryend = countries.end();
			fc = 0;
			for (countryit = countries.begin(); countryit != countryend; countryit++) {
				if (utfcasecmp(countryit->name, country) == 0) {
					for(i = 0; i < CITIES_IN_COUNTRY && countryit->cities[i] != 0; i++) {}
					countryit->cities[i] = cities.size();
					fc = 1;
				}
			}

			if (fc == 0) {
				memset(&curcountry, 0 , sizeof (curcountry));
				strcpy(curcountry.name, country);
				curcountry.cities[0] = cities.size();
				countries.push_back(curcountry);
			}
			citycty.Reset();
		}
		sort( countries.begin(), countries.end(), utfcompare );
		closedir(dir);
	}
}
void dir_selected(char *path) {
	if (path != NULL) {
		if(!cfg) cfg = OpenConfig(CONFIG_FILE, confedit);
		WriteString(cfg, "path", path);
		SaveConfig(cfg);
		read_city(path);
	}
	dirchoosen = true;
	SendEvent(city_choose_handler, EVT_SHOW, 0, 0);
}

void city_init() {
	dirchoosen = false;
	if(!cfg) cfg = OpenConfig(CONFIG_FILE, confedit);
	sprintf(def_dir, "%s", ReadString(cfg, "path", DEFAULT_DIR));
	read_city(def_dir);
	if (cities.size() == 0) {
		OpenDirectorySelector("Open directory", def_dir, 1024, dir_selected);
	} else {
		dirchoosen = true;
	}
}

//Показываем доступные карты
int city_choose_handler(int type, int par1, int par2) {
	if (type == EVT_INIT) {
		city_init();
	}
	if (type == EVT_SHOW && dirchoosen == true) {
		//Инициализируем список городов
		ClearScreen();
		int toc_count = countries.size() + cities.size();
		int countr_count = countries.size();
		contcities = new tocentry [toc_count];
		memset (contcities, 0, sizeof(tocentry) * toc_count);
		int i = 0, n = 0, j = 0;

		for (i = 0, n = 0; i < countr_count; i++) {
			contcities[n].level = 1;
			contcities[n].page = n+1;
			contcities[n].position = n+1;
			contcities[n].text = countries[i].name;
			n++;

			for (j = 0; countries[i].cities[j] != 0; j++) {
				contcities[n].level = 2;
				contcities[n].page = n+1;
				contcities[n].position = countries[i].cities[j];
				contcities[n].text = cities[countries[i].cities[j] - 1].name;
				n++;
			}
		}
		if (n == 0) {
			Message(ICON_ERROR, "Error", "Maps not founded", 4000);
			CloseApp();
		} else if (cities.size() == 1) {
			city_selected(1);
		} else {
			OpenContents(contcities, toc_count, 0, city_selected);
		}
	}
	return 0;
}

int parse_trp(const char * path) {
	//грузим ини файл и перекодируем в утф
	CSimpleIni trp(true);
	SI_Error rc = load_cp1251_ini_file(&trp, path);
	if (rc < 0) {return -1;}
	//
	char curlinenum[8];
	int linenum = 1;
	char *linestations_c = NULL, *linestations = NULL, *linename = NULL, *driving = NULL, *temp = NULL;
	int	len = 0, drlen = 0, curio = 0, curiz =0, curins = 0, curstq = 0, curlinest = 0, curi = 0, curlen = 0, firstst = 0;
	station curstation;
	edge curedge;
	line curline;
	int ltype = 0;
	while (linenum > 0) {
		sprintf(curlinenum, "Line%d", linenum);
		memset(&curline, 0, sizeof(curline));
		linename = const_cast<char*> (trp.GetValue(curlinenum, "Name", NULL));
		linestations_c = const_cast<char*> (trp.GetValue(curlinenum, "Stations", NULL));
		if (!linestations_c || !linename) {break;}
		driving = const_cast<char*> (trp.GetValue(curlinenum, "Driving", NULL));
		linenum++;
		strcpy(curline.name, linename);
		curline.color = DGRAY;
		curline.beginst = sts.size();
		curlen = 0; 
		linestations = linestations_c;
		
		while (linestations != NULL) {
			memset(&curstation, 0, sizeof(curstation));
			temp = get_station_from_string(&linestations, &len);
			if (len < 0) {
				return len;
			}
			strncpy(curstation.name, linestations, len);
			linestations = temp;
			curstation.colortype = ltype; //color[linenum%2];
			curstation.solid = false;
			curstation.x = curstation.y = 0;
			sts.push_back(curstation);
		}
		curline.endst = sts.size();

		if (!driving) {continue;}
		curstq = curline.beginst;
		curlinest = curline.beginst;
		linestations = linestations_c;
		
		firstst = 0;

		while (strlen(driving) != 0) {
			drlen = strcspn(driving, ",");
			curio = strcspn(driving, "(");
			curiz = strcspn(driving, ")");
			
			memset(&curedge, 0, sizeof(curedge));
			curedge.st[0] = curedge.st[1] = -1;
			curedge.time[0] = curedge.time[1] = 0;
			
			curedge.colortype = ltype;
			if (curins == 0) {
				curedge.st[0] = curstq;
				if (curio != 0) {
					curedge.st[1] = curstq + 1;
					curedge.time[0] = curedge.time[1] = atoi(driving);
					if (curedge.time[0] != 0 || curedge.time[1] != 0) {
						sts[curedge.st[0]].solid = true;
						sts[curedge.st[1]].solid = true;
					}
					edgs.push_back(curedge);
				} else {
					firstst = curstq;
					
					for(; curlinest < curstq; curlinest++) {
						linestations = get_station_from_string(&linestations, &len);
					}
					
					get_station_from_string(&linestations, &len);
					temp = linestations + len;
					if (temp[0] == '(') {
						linestations = linestations + len + 1;
						curins = 2;
					} else {
						curins = 3;
					}
				}
				curstq++;
			}

			if (curins == 4) {
				curedge.st[0] = firstst;
				curedge.st[1] = firstst - 1;
				//посмотреть
				curedge.time[1] = atoi(driving);
				if (curedge.time[0] != 0 || curedge.time[1] != 0) {
					sts[curedge.st[0]].solid = true;
					sts[curedge.st[1]].solid = true;
				}
				edgs.push_back(curedge);
				curins = 0;
			}
			
			if (curins == 3) {
				curedge.st[0] = firstst;
				curedge.st[1] = firstst + 1;
				//посмотреть
				curedge.time[0] = atoi(driving + 1);
				if (curedge.time[0] != 0 || curedge.time[1] != 0) {
					sts[curedge.st[0]].solid = true;
					sts[curedge.st[1]].solid = true;
				}
				edgs.push_back(curedge);
				curins = 4;
			}
			
			if (curins == 1 || curins == 2) {
				curedge.st[0] = firstst;
				temp = get_station_from_string(&linestations, &len, true);
				
				if (len < 0) {
					return len;
				}
				if (len != 0) {
					for (curi = curline.beginst; curi < curline.endst; curi++) {
						if (!utfncasecmp(linestations, sts[curi].name, len)) {
							curedge.st[1] = curi;
							break;
						}
					}
										
					curlen = edgs.size();
					for (curi =0; curi < curlen; curi ++) {
						if ((edgs[curi].st[0] == curedge.st[0] && edgs[curi].st[1] == curedge.st[1]) || (edgs[curi].st[0] == curedge.st[1] && edgs[curi].st[1] == curedge.st[0])) {
							break;
						}
					}
					
					if (curi == curlen) {
						if (curins == 2) {
							curedge.time[0] = atoi(driving + 1);
						} else {
							curedge.time[0] = atoi(driving);
						}
						if (curedge.time[0] != 0 || curedge.time[1] != 0) {
							sts[curedge.st[0]].solid = true;
							sts[curedge.st[1]].solid = true;
						}
						edgs.push_back(curedge);
					}
				}
				if (curins == 2) {
					curins = 1;
				}
				linestations = temp;
				if (curiz <= drlen) {
					curins = 0;
				}
			}
			driving = driving + drlen;
			if (strlen(driving) != 0) {
				driving = driving + 1;
			}
		}
		
		lines.push_back(curline);
		ltype++;
	}
	
	
	//разбираем пересадки
	CSimpleIni::TNamesDepend transferkeys;
	trp.GetAllKeys("Transfers", transferkeys);
	CSimpleIni::TNamesDepend::const_iterator transfersi;
	char * pszValue;
	int err = 0;
	int n = 0, cline = -1, time = 0;
	transfer curtredg;
	
	for (transfersi = transferkeys.begin(); transfersi != transferkeys.end(); ++transfersi) {
		memset(&curtredg, 0, sizeof(curtredg));
		curtredg.invisible = false;
		curtredg.st[0] = curtredg.st[1] = -1;
		
		pszValue = const_cast<char*> (trp.GetValue("Transfers", const_cast<char*> (transfersi->pItem), NULL));
		cline = -1;
		time = n = 0;
		while (pszValue != NULL) {
			temp = get_station_from_string(&pszValue, &len);
			if (len < 0) {break;}
			
			switch(n) {
				case 0:
				case 2:
					err = 1;
					curlen = lines.size();
					for (curi = 0; curi < curlen; curi ++) {
						if(strlen(lines[curi].name) == len && !utfncasecmp(lines[curi].name, pszValue, len)) {
							cline = curi;
							err = 0;
						}
					}
					break;
				case 1:
				case 3:
					if (err != 1) {
						for (curi = lines[cline].beginst; curi < lines[cline].endst; curi ++) {
							if(strlen(sts[curi].name) == len && !utfncasecmp(sts[curi].name, pszValue, len)) {
								curtredg.st[(n-1)/2] = curi;
							}
						}
					}
					break;
				case 4:
					curtredg.time[1] = curtredg.time[0] = atoi(pszValue);
					break;
				case 5:
				case 6:
					if (!utfcasecmp(pszValue, "invisible")) {
						curtredg.invisible = true;
					} else {
						curtredg.time[1] = atoi(pszValue);
					}
					break;
			}
			n++;
			pszValue = temp;
		}
		
		if (curtredg.st[0] != -1 && curtredg.st[1] != -1) {
			tredgs.push_back(curtredg);
		}
	}
	trp.Reset();
	return 1;
}

void parse_map(const char * path) {
	CSimpleIni map(true);
	SI_Error rc = load_cp1251_ini_file(&map, path);
	if (rc < 0) {return;}
	//очищаем переменные
	lines.clear();
	sts.clear();
	edgs.clear();
	tredgs.clear();
	curst = -1;
	//
	char * trmaps = const_cast<char*> (map.GetValue("Options", "Transports", "Metro.trp", NULL));
	
	char * temp; int len; 
	
	string curtrp;
	while (trmaps != NULL) {
		temp = get_station_from_string(&trmaps, &len);
		curtrp = cities[curcity-1].path + string(trmaps, len);
		parse_trp(curtrp.data());
		trmaps = temp;
	}
	if (lines.size() == 0) {return;}
	//
	diameter = (int) map.GetLongValue("Options", "StationDiameter", 12, NULL);
	lineswidth = (int) map.GetLongValue("Options", "LinesWidth", 12, NULL);
	//
	//Разбираем дополнительные точки
	CSimpleIni::TNamesDepend adpointkeys;
	
	map.GetAllKeys("AdditionalNodes", adpointkeys);
	CSimpleIni::TNamesDepend::const_iterator adkeysi;
	char * pszValue;
	int n = 0, curlen = 0, curi = 0, prevst = -1, nextst = -1, cline = -1, count = 0;
	point add[ADDITIONAL_POINTS];
	
	for (adkeysi = adpointkeys.begin(); adkeysi != adpointkeys.end(); ++adkeysi) {
		memset(add, 0, sizeof(add));
		
		pszValue = const_cast<char*> (map.GetValue("AdditionalNodes", const_cast<char*> (adkeysi->pItem), NULL));
		
		prevst = nextst = cline = -1;
		count = n = 0;
		
		while (pszValue != NULL) {
			
			temp = get_station_from_string(&pszValue, &len);
			if (len < 0) {break;}
			
			switch(n) {
				case 0:
					curlen = lines.size();
					for (curi = 0; curi < curlen; curi ++) {
						if(!utfncasecmp(lines[curi].name, pszValue, len)) {
							cline = curi;
						}
					}
					break;
				case 1:
					if (cline != -1) {
						for (curi = lines[cline].beginst; curi < lines[cline].endst; curi ++) {
							if(!utfncasecmp(sts[curi].name, pszValue, len)) {
								prevst = curi;
							}
						}
					}
					break;
				case 2:
					if (cline != -1) {
						for (curi = lines[cline].beginst; curi < lines[cline].endst; curi ++) {
							if(!utfncasecmp(sts[curi].name, pszValue, len)) {
								nextst = curi;
							}
						}
					}
					break;
				default:
					if (n - 3 < ADDITIONAL_POINTS * 2) {
						if ((n%2) == 1) {
							if (utfncasecmp(pszValue, " spline", len) && utfncasecmp(pszValue, "spline", len) ) {
								add[(n-3)/2].x = atoi(pszValue);
								count = (n-1)/2;
							}
						} else {
							add[(n-4)/2].y = atoi(pszValue);
						}
					}
					break;
			}
			n++;
			pszValue = temp;
		}
		
		if (prevst != -1 && nextst != -1) {
			curlen = edgs.size();
			for (curi = 0; curi < curlen; curi ++) {
				if (edgs[curi].st[0] == prevst && edgs[curi].st[1] == nextst) {
					edgs[curi].addcount = count;
					memcpy(edgs[curi].addit, add, sizeof(add));
					break;
				} else if (edgs[curi].st[0] == nextst && edgs[curi].st[1] == prevst) {
					edgs[curi].addcount = count;
					for (int i = 0; i < count; i++) {
						edgs[curi].addit[i] = add[count - i - 1];
					}
				}
			}
		}
	}
	//
	vector<line>::iterator lineit = lines.begin(), lineend = lines.end();
	char * stationsxy, * stationsrect;
	pmax.x = pmax.y = cur.x = cur.y = step.x = step.y = 0;
	for (; lineit < lineend; lineit++ ) {
		stationsxy = const_cast<char*> (map.GetValue(lineit->name, "Coordinates", NULL));
		stationsrect = const_cast<char*> (map.GetValue(lineit->name, "Rects", NULL));

		if (!stationsxy && !stationsrect) {continue;}
		for (curi = lineit->beginst; curi < lineit->endst; curi ++) {
			if (strlen(stationsxy) != 0) {
				//Находим x
				sts[curi].x = get_int_from_string(&stationsxy);
				//Находим y
				sts[curi].y = get_int_from_string(&stationsxy);
				
				if (sts[curi].x + (diameter / 2) + IDENT > pmax.x) {pmax.x = sts[curi].x + (diameter / 2) + IDENT;}
				if (sts[curi].y + (diameter / 2) + IDENT > pmax.y) {pmax.y = sts[curi].y + (diameter / 2) + IDENT;}
			}
			
			if (stationsrect != NULL) {
				sts[curi].rectx = get_int_from_string(&stationsrect);
				sts[curi].recty = get_int_from_string(&stationsrect);
				sts[curi].rectw = get_int_from_string(&stationsrect);
				sts[curi].recth = get_int_from_string(&stationsrect);
				if (sts[curi].rectx + sts[curi].rectw + IDENT > pmax.x) {pmax.x = sts[curi].rectx + sts[curi].rectw + IDENT;}
				if (sts[curi].recty + sts[curi].recth + IDENT > pmax.y) {pmax.y = sts[curi].recty + sts[curi].recth + IDENT;}
			}
		}
	}
	
	calc_fscale_step();
	scale = fscale;
	
	map.Reset();
}

//Поиск станций. Если станции найдена спрашиваем какую искали. Если найдена одна отображаем ее
void search_station(char *s) {
	int i = 0, n = 0, end = sts.size();
	int *ni = new int [end];

	for (i = 0; i < end; i++) {
		if (sts[i].x == 0 && sts[i].y == 0) {
			continue;
		}		
		if (utfcasestr(sts[i].name, s)) {
			ni[n] = i;
			n++;
		}
	}
	if (n > 1) {
		contcities = new tocentry [n];
		memset (contcities, 0, sizeof(tocentry) * n);

		for (i = 0; i < n; i++) {
			contcities[i].level = 1;
			contcities[i].page = i+1;
			contcities[i].position = ni[i];
			contcities[i].text = sts[ni[i]].name;
		}
	}
	if (n == 1) {
		curst = ni[0];
	}

	delete [] ni;

	if (n > 1) {
		OpenContents(contcities, n, 0, station_selected);
	} else if (n == 1) {
		abs_go(sts[curst].x, sts[curst].y);
	} else {
		Message(ICON_INFORMATION, "Sorry", "Station not found", 3000);
	}
}

void zoom_min() {
	if (scale == 150 && ((fscale < 150 && fscale > 100) || fscale <= 100)) {
		calc_fscale_step();
		cur.x = (cur.x + (ScreenWidth() / 3)) - (ScreenWidth() / 2);
		cur.y = (cur.y + (ScreenHeight() / 3)) - (ScreenHeight() / 2);
		if (fscale <= 100) {
			scale = 100;
		} else {
			scale = fscale;
		}
		//Для убирания различных дурацких эффектов типо вылезания за пределы экрана
		if (!go(0, 0)) {
			draw_map();
		}
	} else if (scale == 100 && fscale < 100) {
		scale = fscale;
		draw_map();
	}
}

void zoom_pl() {
	if (scale < 100) {
		scale = 100;
		draw_map();
	} else if (scale < 150) {
		cur.x = (cur.x + (ScreenWidth() / 2)) - (ScreenWidth() / 3);
		cur.y = (cur.y + (ScreenHeight() / 2)) - (ScreenHeight() / 3);
		calc_fscale_step(150);
		scale = 150;
		draw_map();
	}
}

void draw_cursor() {
	if (scale >= 100) {
		return;
	}
	DrawRect(0, 0, ScreenWidth(), ScreenHeight(), BLACK);
	DrawRect(scalev(cur.x), scalev(cur.y), scalev(ScreenWidth()), scalev(ScreenHeight()), BLACK);
}

void config_ok() {
	SaveConfig(cfg);
}

void map_paint_menu_handler(int pos) {
	if (pos == 101) {
		zoom_min();
	}

	if (pos == 105) {
		zoom_pl();
	}

	if (pos == 103) {
		orient++; if (orient > 2) orient = 0;
		SetOrientation(orient);
		
		calc_fscale_step();
		
		if(scale != 100) {
			scale = fscale;
		}
		draw_map();
	}
	if (pos == 102) {
		curst = -1;
		OpenKeyboard("Station name", searchst, STATIONNAME_SIZE - 1, 0, search_station);
	}

	if (pos == 104) {
		city_init();
		SetEventHandler(city_choose_handler);
	}

	if (pos == 107) {
		curst = -1;
		draw_map();
	}
	
	if (pos == 108) {
		if(!cfg) cfg = OpenConfig(CONFIG_FILE, confedit);
		OpenConfigEditor("Configuration", cfg, confedit, config_ok, NULL);
	}

	if (pos == 106) {
		CloseApp();
	}
}

void draw_map() {
	ClearScreen();
	int fsize = scalev(10);
	ifont *arial = NULL;
	if (fsize >= 8) {arial = OpenFont("DroidSans", fsize, 1);}

	vector<station>::iterator stit;
	vector<edge>::iterator edgit;
	vector<transfer>::iterator tredgit;
	int cura = VALIGN_TOP;
	int color = BLACK, i = 0;
	
	point ofs = {0, 0};
	
	if (scale >= 100) {
		ofs.x += cur.x;
		ofs.y += cur.y;
	}
	
	if (curst != -1) {color = LGRAY;}
	
	point p1, p2, p3[ADDITIONAL_POINTS];
	bool solid = false;
	int hh = 0;
	bool go = false;
	for (edgit=edgs.begin(); edgit != edgs.end(); edgit++) {
		go = false;
		if ((sts[edgit->st[0]].x == 0 && sts[edgit->st[0]].y == 0) || (sts[edgit->st[1]].x == 0 && sts[edgit->st[1]].y == 0)) {
			continue;
		}
		p1.x = scalev(sts[edgit->st[0]].x - ofs.x);
		p1.y = scalev(sts[edgit->st[0]].y - ofs.y);
		p2.x = scalev(sts[edgit->st[1]].x - ofs.x);
		p2.y = scalev(sts[edgit->st[1]].y - ofs.y);
		for (i = 0; i < edgit->addcount; i++) {
			p3[i].x = scalev(edgit->addit[i].x - ofs.x);
			p3[i].y = scalev(edgit->addit[i].y - ofs.y);
		}
		
		if (edgit->time[0] == 0 && edgit->time[1] == 0) {
			solid = false;
		} else {
			solid = true;
		}
		
		DrawEdge(p1, p2, scalev(lineswidth), color, edgit->colortype, p3, edgit->addcount, solid);
		hh++;
	}
	for (tredgit=tredgs.begin(); tredgit != tredgs.end(); tredgit++) {
		if ((sts[tredgit->st[0]].x == 0 && sts[tredgit->st[0]].y == 0) || (sts[tredgit->st[1]].x == 0 && sts[tredgit->st[1]].y == 0)) {
			continue;
		}
		if (!tredgit->invisible) {
			p1.x = scalev(sts[tredgit->st[0]].x - ofs.x);
			p1.y = scalev(sts[tredgit->st[0]].y - ofs.y);
			p2.x = scalev(sts[tredgit->st[1]].x - ofs.x);
			p2.y = scalev(sts[tredgit->st[1]].y - ofs.y);
			
			DrawChange(p1, p2, scalev(lineswidth + 4), color);
			DrawStation(p1.x, p1.y, scalev(diameter+8), color, 1, false);
			DrawStation(p2.x, p2.y, scalev(diameter+8), color, 1, false);
		}
	}
	
	for (stit=sts.begin(), i = 0; stit != sts.end(); stit++, i++) {
		if (stit->x == 0 && stit->y == 0) {
			continue;
		}
		color = BLACK;
		if (curst != -1 && curst != i) {color = LGRAY;}
		if (fsize >= 8 && stit->rectw > 0 && stit->recth > 0) {
			SetFont(arial, color);
			cura = 0;
			if (stit->recth > stit->rectw) {
				cura = ROTATE;
				if (stit->recty - stit->y < 0) {
					cura = cura | ALIGN_LEFT;
				} else {
					cura = cura | ALIGN_RIGHT;
				}
			} else {
				if (stit->rectx - stit->x > 0) {
					cura = ALIGN_LEFT;
				} else {
					cura = ALIGN_RIGHT;
				}
			}
			DrawTextRect(scalev(stit->rectx - ofs.x), scalev(stit->recty - ofs.y), scalev(stit->rectw), scalev(stit->recth), stit->name, cura);
		}
		DrawStation(scalev(stit->x - ofs.x), scalev(stit->y - ofs.y), scalev(diameter), color, stit->colortype, stit->solid);
	}
	if (fsize >= 8) {CloseFont(arial);}
	draw_cursor();
	
	FullUpdate();
}

void abs_go(int x, int y) {
	int scalew = ScreenWidth();
	int scaleh = ScreenHeight();
	if (scale == 150) {
		scalew = (ScreenWidth() * 2) / 3;
		scaleh = (ScreenHeight() * 2) / 3;
	}
	
	cur.x = x - (scalew / 2);
	cur.y = y - (scaleh / 2);
	if (!go(0, 0)) {
		draw_map();
	}
}

bool go(int x, int y) {
	int scalew = ScreenWidth();
	int scaleh = ScreenHeight();
	if (scale == 150) {
		scalew = (ScreenWidth() * 2) / 3;
		scaleh = (ScreenHeight() * 2) / 3;
	}
	
	point pr = cur;
	if (cur.x + x * step.x < 5) {
		cur.x = 0;
	} else if (pmax.x - (cur.x + x * step.x + scalew)  < 5) {
		cur.x = pmax.x - scalew;
	} else {
		cur.x += x * step.x;
	}
	if (cur.y + y * step.y < 5) {
		cur.y = 0;
	} else if (pmax.y - (cur.y + y * step.y + scaleh) < 5) {
		cur.y = pmax.y - scaleh;
	} else {
		cur.y += y * step.y;
	}
	if (cur.x != pr.x || cur.y != pr.y) {
		draw_map();
		return true;
	}
	return false;
}

//При выборе карты
int map_paint_handler(int type, int par1, int par2) {
	if (type == EVT_SHOW) {
		draw_map();
	}

	if (type == EVT_KEYPRESS) {
		switch (par1) {
			case KEY_OK:
			case KEY_MENU:
				create_menu();
				OpenMenu(map_menu, 0, 20, 20, map_paint_menu_handler);
				break;
			case KEY_MINUS:
				zoom_min();
				break;

			case KEY_PLUS:
				zoom_pl();
				break;

			case KEY_BACK:
				CloseApp();
				break;

			case KEY_RIGHT:
				go(1, 0);
				break;
				
			case KEY_LEFT:
				go(-1, 0);
				break;
				
			case KEY_UP:
				go(0, -1);
				break;
				
			case KEY_DOWN:
				go(0, 1);
				break;
		}
	}
	return 1;
}

int main(int argc, char **argv) {
	InkViewMain(city_choose_handler);
	return 0;
}

bool utfcompare (structcountry i, structcountry j) {
	int cmp = utfcasecmp(i.name, j.name);
	return (cmp < 0);
}


void calc_fscale_step(int sc) {
	step.x = step.y = 0;
	int scalew = (ScreenWidth() * 100) / sc;
	int scaleh = (ScreenHeight() * 100) / sc;
	int sx = 0, sy = 0;
	sx = (scalew * 100) / pmax.x;
	sy = (scaleh * 100) / pmax.y;
	if (sc == 100) { fscale = min(sx, sy); }
	
	if (sx < 100) {
		step.x = (pmax.x - scalew) / (pmax.x / scalew);
	}
	
	if (sy < 100) {
		step.y = (pmax.y - scaleh) / (pmax.y / scaleh);
	}
}
