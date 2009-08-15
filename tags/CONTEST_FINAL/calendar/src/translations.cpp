/*
 * translations.cpp
 *
 *  Created on: Jun 28, 2009
 *      Author: fax
 */

#include "config.hpp"

// TODO: Write a simple Perl script to generate this file, or use ready translation mechanisims


using namespace std;

const char* TR(const std::string& label) {

	char* tr = GetLangText(const_cast<char*> (label.c_str()));
#ifdef TRACE_TR
	dbg << "Translation for '" << label << "' is " << tr << endl;
#endif
	return tr;
}

#define ADD_RU_TRANSLATION(x) add_translation(x, x##_ru)
#define ADD_RU_TRANSLATIONS(x) add_translations(x, x##_ru)

string LANGUAGES[] = { "by", "ru", "en", "ua", "de" };
string LANGUAGES_ru[] = { "Беларусский (отсуствует)", "Русский", "Английский",
		"Украинский (отсуствует)", "Немецкий (отсуствует)" };

string CFG_ORIENTATION[] = {"Orientation", "Choose device orientation"};
string CFG_ORIENTATION_ru[] = {"Режим отображения", "Выберите режим поворота устройства"};

string ORIENTATIONS[] = { "Portrait", "Landscape", "Reverse Landscape",
		"Reverse Portrait", "Auto" };
string ORIENTATIONS_ru[] = { "Портретный", "Пейзажный (90 градусов)",
		"Пейзажный (270 градусов)", "Перевернутый (180 градусов)",
		"Автоматический" };

string YESNO[] = { "Yes", "No" };
string YESNO_ru[] = { "Да", "Нет" };

std::string month_names[] = { "bad", "January", "February", "March", "April",
		"May", "June", "July", "August", "September", "October", "November",
		"December" };
std::string month_names_ru[] = { "Ошибка", "Январь", "Февраль", "Март",
		"Апрель", "Май", "Июнь", "Июль", "Август", "Сентябрь", "Октябрь",
		"Ноябрь", "Декабрь" };

string month_names_s[] = { "bad", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
string month_names_s_ru[] = { "Ошибка", "Янв", "Фев", "Мар", "Апр", "Май",
		"Июн", "Июл", "Авг", "Сен", "Окт", "Ноя", "Дек" };

string day_names[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
		"Friday", "Saturday" };
string day_names_ru[] = { "Воскресенье", "Понедельник", "Вторник", "Среда",
		"Четверг", "Пятница", "Суббота" };

string days_s[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
string days_s_ru[] = { "Вск", "Пнд", "Втр", "Срд", "Чтв", "Птн", "Сбт" };

string days_ss[] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" };
string days_ss_ru[] = { "Вс", "Пн", "Вт", "Ср", "Чт", "Пт", "Сб" };

string WW = "WW";
string WW_ru = "НД";

string YEAR = "Year";
string YEAR_ru = "Год";

string LOCATION = "Location:";
string LOCATION_ru = "Место:";

string DESCRIPTION = "Details:";
string DESCRIPTION_ru = "Подробности:";

string PARTICIPANTS = "Participants:";
string PARTICIPANTS_ru = "Участники:";

string FULL_DAY = "Whole day";
string FULL_DAY_ru = "Целый день";

// TODO: Minor: make it work correct for all numbers
string evt[] = { "1 event", "2 events", "3 events", "4 events", " events" };
string evt_ru[] = { "1 событие", "2 события", "3 события", "4 события",
		" событий" };

string CFG_CONFIGURE_TITLE = "Configure Calendar";
string CFG_CONFIGURE_TITLE_ru = "Настройка Календаря";

string CFG_ENTRY_WEEKSTART[] = { "Week Starts On",
		"Select the first week day in your country" };
string CFG_ENTRY_WEEKSTART_ru[] = { "Начало недели",
		"Выберите день, с которого начинается неделя" };

string CFG_ENTRY_STVIEW[] = { "Starting View",
		"Select the view which will be active when Calendar starts" };
string CFG_ENTRY_STVIEW_ru[] = { "Начальный вид",
		"Выберите, что будет показываться при старте" };

string CFG_ENTRY_PROFILE_DIR[] = { "Calendar File Directory",
		"Select directory to place calendar files" };
string CFG_ENTRY_PROFILE_DIR_ru[] = { "Директория календарных файлов",
		"Выберите директорию для хранения календарных файлов" };

string CFG_ENTRY_PROFILE1_NAME[] = { "Profile 1 Calendar File",
		"Enter the first profile calendar file name" };
string CFG_ENTRY_PROFILE1_NAME_ru[] = { "Календарный файл №1",
		"Введите имя файла для профиля №1" };

string CFG_ENTRY_PROFILE2_NAME[] = { "Profile 2 Calendar File",
		"Enter the second profile calendar file name" };
string CFG_ENTRY_PROFILE2_NAME_ru[] = { "Календарный файл №2",
		"Введите имя файла для профиля №2" };

string
		CFG_ENTRY_PROFILE_CUR[] = { "Current Profile", "Choose current profile" };
string CFG_ENTRY_PROFILE_CUR_ru[] = { "Текущий профиль",
		"Выберите текущий профиль" };

string CFG_LANGUAGE[] = { "Interface Language",
		"Choose Calendar interface language" };
string CFG_LANGUAGE_ru[] = { "Язык интерфейса",
		"Выберите язык интерфейса Календаря" };

string CFG_WEEKVIEW_SHOWTIME[] = { "Show time in Week View events",
		"Select if you want to see time along with event names in Week View" };
string CFG_WEEKVIEW_SHOWTIME_ru[] = { "Показывать время в Событиях Недели",
		"Выберите, хотите ли Вы видеть время в Событиях Недели" };

string CFG_WEEKVIEW_EVENTFONT[] = { "Week View Event Font",
		"Select font for event previews in Week View" };
string CFG_WEEKVIEW_EVENTFONT_ru[] = { "Шрифт Событий Недели",
		"Выберите шрифт для показа событий недели" };

string CFG_DAYVIEW_SHOWDESC[] = { "Show description in Day View events",
		"Choose if you want to see descriptions" };
string CFG_DAYVIEW_SHOWDESC_ru[] = { "Показывать детали",
		"Выберите, хотите ли вы видеть детали событий при просмотре дня" };

string CFG_DAYVIEW_SHOWLOC[] = { "Show location in Day View events",
		"Choose if you want to see descriptions" };
string CFG_DAYVIEW_SHOWLOC_ru[] = {
		"Показывать место события",
		"Выберите, хотите ли вы видеть места событий при просмотре дня" };

string CFG_DAYVIEW_SHOWPART[] = { "Show participants in Day View events",
		"Choose if you want to see descriptions" };
string CFG_DAYVIEW_SHOWPART_ru[] = {
		"Показывать участников события",
		"Выберите, хотите ли вы видеть участников событий при просмотре дня" };

string CFG_MONTHVIEW_SHOWWW[] = { "Show Work Week Number",
		"Select if you want Work Week Number to be displayed in Month View" };
string
		CFG_MONTHVIEW_SHOWWW_ru[] = { "Показывать номер рабочей недели",
				"Выберите, хотите ли Вы видеть номер рабочей недели в событиях месяца" };

string CFG_VIEW_STARTVIEW_VNT[] = { "Last View", "Month View", "Week View",
		"Today View", "Year View" };
string CFG_VIEW_STARTVIEW_VNT_ru[] = { "Последний выбор", "События месяца",
		"События недели", "Сегодня", "Годовой календарь" };

string CFG_PROFILE_I = "Profile I";
string CFG_PROFILE_I_ru = "Профиль I";

string CFG_PROFILE_II = "Profile II";
string CFG_PROFILE_II_ru = "Профиль II";

const static std::string LANG_RU = "ru";

Translations::Translations() {

	dbg << "Known languages" << endl;
	char** langs = EnumLanguages();
	for (int i = 0; i < 15; i++) {
		if (langs[i] == NULL)
			break;
		dbg << "-->" << langs[i] << endl;
	}

	dbg << "Translation for " << day_names[0] << " is " << TR(day_names[0])
			<< endl;
	dbg << "Translation for " << month_names[7] << " is " << TR(month_names[7])
			<< endl;
	dbg << "Translation for 'July' is " << TR("July") << endl;
}

Translations::~Translations() {
}

template<int k>
void Translations::add_translations(const std::string(&org)[k],
		const std::string(&trans)[k]) {
	for (int i = 0; i < k; ++i)
		add_translation(org[i], trans[i]);

}

void Translations::add_translation(const std::string& org,
		const std::string &trans) {
	AddTranslation(const_cast<char*> (org.c_str()),
			const_cast<char*> (trans.c_str()));
	dbg << "TRANS " << org << "=" << trans << endl;

}

void Translations::switch_language(const std::string& lang) {
	LoadLanguage(US(lang));

	if (LANG_RU == lang)
		load_lang_ru();
}

void Translations::load_lang_ru() {
	ADD_RU_TRANSLATIONS(YESNO);
	ADD_RU_TRANSLATIONS(LANGUAGES);
	ADD_RU_TRANSLATIONS(ORIENTATIONS);
	ADD_RU_TRANSLATIONS(month_names);
	ADD_RU_TRANSLATIONS(month_names_s);
	ADD_RU_TRANSLATIONS(day_names);
	ADD_RU_TRANSLATIONS(days_s);
	ADD_RU_TRANSLATIONS(days_ss);

	ADD_RU_TRANSLATION(WW);
	ADD_RU_TRANSLATION(YEAR);
	ADD_RU_TRANSLATION(LOCATION);
	ADD_RU_TRANSLATION(DESCRIPTION);
	ADD_RU_TRANSLATION(PARTICIPANTS);
	ADD_RU_TRANSLATION(FULL_DAY);

	ADD_RU_TRANSLATION(CFG_PROFILE_I);
	ADD_RU_TRANSLATION(CFG_PROFILE_II);

	ADD_RU_TRANSLATIONS(evt);

	ADD_RU_TRANSLATIONS(CFG_LANGUAGE);
	ADD_RU_TRANSLATIONS(CFG_ENTRY_WEEKSTART);
	ADD_RU_TRANSLATIONS(CFG_ENTRY_STVIEW);
	ADD_RU_TRANSLATIONS(CFG_ENTRY_PROFILE_DIR);
	ADD_RU_TRANSLATIONS(CFG_ENTRY_PROFILE1_NAME);
	ADD_RU_TRANSLATIONS(CFG_ENTRY_PROFILE2_NAME);
	ADD_RU_TRANSLATIONS(CFG_ENTRY_PROFILE_CUR);
	ADD_RU_TRANSLATIONS(CFG_WEEKVIEW_SHOWTIME);
	ADD_RU_TRANSLATIONS(CFG_WEEKVIEW_EVENTFONT);
	ADD_RU_TRANSLATIONS(CFG_DAYVIEW_SHOWDESC);
	ADD_RU_TRANSLATIONS(CFG_DAYVIEW_SHOWLOC);
	ADD_RU_TRANSLATIONS(CFG_DAYVIEW_SHOWPART);
	ADD_RU_TRANSLATION(CFG_CONFIGURE_TITLE);
	ADD_RU_TRANSLATIONS(CFG_ORIENTATION);
	ADD_RU_TRANSLATIONS(CFG_VIEW_STARTVIEW_VNT);
}
