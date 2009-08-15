/*
 * translations.hpp
 *
 *  Created on: Jun 28, 2009
 *      Author: fax
 */

#ifndef TRANSLATIONS_HPP_
#define TRANSLATIONS_HPP_

const char* TR(const std::string& label);

extern std::string LANGUAGES[];
extern std::string ORIENTATIONS[];

extern std::string month_names[];
extern std::string day_names[];
extern std::string days_s[];
extern std::string days_ss[];
extern std::string WW;
extern std::string YEAR;
extern std::string evt[];
extern std::string LOCATION;
extern std::string DESCRIPTION;
extern std::string PARTICIPANTS;
extern std::string FULL_DAY;

extern std::string CFG_PROFILE_I;
extern std::string CFG_PROFILE_II;

extern std::string CFG_CONFIGURE_TITLE;
extern std::string CFG_ENTRY_WEEKSTART[];
extern std::string CFG_ENTRY_STVIEW[];
extern std::string CFG_VIEW_STARTVIEW_VNT[];
extern std::string CFG_LANGUAGE[];
extern std::string CFG_ORIENTATION[];

extern std::string CFG_WEEKVIEW_EVENTFONT[];
extern std::string CFG_WEEKVIEW_SHOWTIME[];
extern std::string CFG_DAYVIEW_SHOWDESC[];
extern std::string CFG_DAYVIEW_SHOWLOC[];
extern std::string CFG_DAYVIEW_SHOWPART[];
extern std::string CFG_MONTHVIEW_SHOWWW[];

extern std::string CFG_ENTRY_PROFILE_DIR[];
extern std::string CFG_ENTRY_PROFILE1_NAME[];
extern std::string CFG_ENTRY_PROFILE2_NAME[];
extern std::string CFG_ENTRY_PROFILE_CUR[];


class Translations {
public:
	Translations();
	virtual ~Translations();

	template<int k>
	void add_translations(const std::string(&org)[k],
			const std::string(&trans)[k]);

	void add_translation(const std::string& org, const std::string &trans);
	void switch_language( const std::string& lang );

	static Translations& get_instance();
private:
 	void load_lang_ru();
private:
	static Translations instance;
};

#endif /* TRANSLATIONS_HPP_ */
