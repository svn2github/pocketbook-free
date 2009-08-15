#include <boost/date_time/gregorian_calendar.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <iostream>
#include <sstream>

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <inkview.h>

#include "month-view.hpp"
#include "week-view.hpp"
#include "year-view.hpp"
#include "day-view.hpp"
#include "ical-parser.hpp"

#include "view-manager.hpp"
#include "translations.hpp"

using namespace std;
using namespace ical;

#define dbg cout

// TO DO LIST:
// DONE: Add day labels
// DONE: Add option to starting day Sun/Mon
// DONE: Add more drawing functions to Screen
// DONE: Read about localizations
// DONE: Add zoom in by side buttons
// DONE: Localization
// TODO: Profiles for different people
// TODO: Daily view should show full records or only summary
// DONE: Instead of round-robin in week view and month view, allow for moving to the next month/week
// TODO: Add copyright and about information
// TODO: Zooming out from year view should leave/suspend program
// TODO: Check how can we run in background
// TODO: In month view and day view use SetClip to make redrawing smoother
// TODO: add set_clip function to Rect;
// TODO: Think of other type of selection (thick line?)
// TODO: Add fully independent profiles
// TODO: !Add navigation to an arbitrary date
// TODO: ! Check if dual-profiles work
// DONE: !Check time correctness for ical parser
// DONE: Year View
// TODO: All configuration settings work

static iconfig *config = NULL;

static char *weekstart_vnt[] = { "Sunday", "Monday", NULL };

static char *stview_vnt[] = { "Last View", "Month View", "Week View",
		"Today View", "Year View", NULL };

static char *ort_vnt[] = { "Portrait", "Landscape", "Reverse Landscape",
		"Reverse Portrait", "Auto", NULL };

static char *eview_vnt[] = { "Summary", "Preview", NULL };

static char* dirview_vnt[] = { "", NULL };
static char* p1_name_vnt[] = { "basic.ics", "profile1.ics" };
static char* p2_name_vnt[] = { "basic.ics", "profile2.ics" };
static char* profile_vnt[] = { "Profile I", "Profile II", NULL };

iconfigedit mkentry(int type, const ibitmap *icon,
		const std::string desc_hint[2], char *name, char **variants) {
	iconfigedit ce = { type, icon, US(desc_hint[0]), US(desc_hint[1]), name,
			variants[0], variants };
	return ce;
}

iconfigedit mk_yn_entry(const ibitmap *icon, const std::string desc_hint[2],
		char *name) {
	static char* variants[] = { "Yes", "No", NULL };

	return mkentry(CFG_CHOICE, icon, desc_hint, name, variants);

}

iconfigedit mk_ft_entry(const ibitmap *icon, const std::string desc_hint[2],
		char *name, char *ft_name) {
	char* ftconfig[] = { ft_name, NULL };

	return mkentry(CFG_FONT, icon, desc_hint, name, ftconfig);
}

static char** langs = EnumLanguages();

static iconfigedit calce[] = {

mkentry(CFG_CHOICE, NULL, CFG_ENTRY_PROFILE_CUR, "profile.cur", profile_vnt),

mkentry(CFG_CHOICE, NULL, CFG_ORIENTATION, "view.orientation", ort_vnt),

mkentry(CFG_CHOICE, NULL, CFG_LANGUAGE, "view.language", langs),

		mkentry(CFG_CHOICE, NULL, CFG_ENTRY_WEEKSTART, "view.weekstart",
				weekstart_vnt),

		mkentry(CFG_CHOICE, NULL, CFG_ENTRY_STVIEW, "view.stview", stview_vnt),
		mkentry(CFG_HIDDEN, NULL, CFG_ENTRY_STVIEW, "view.last_view",
				stview_vnt),

		mk_yn_entry(NULL, CFG_MONTHVIEW_SHOWWW, "monthview.showww"),

		mk_yn_entry(NULL, CFG_WEEKVIEW_SHOWTIME, "weekview.showtime"),

		mk_yn_entry(NULL, CFG_DAYVIEW_SHOWDESC, "dayview.show_description"),

		mk_yn_entry(NULL, CFG_DAYVIEW_SHOWLOC, "dayview.show_location"),

		mk_yn_entry(NULL, CFG_DAYVIEW_SHOWPART, "dayview.show_participants"),

		//FONTS

		mk_ft_entry(NULL, CFG_WEEKVIEW_EVENTFONT, "weekview.event_font",
				"LiberationSerif,10"),

		mkentry(CFG_TEXT, NULL, CFG_ENTRY_PROFILE_DIR, "profile.dir",
				dirview_vnt),

		mkentry(CFG_TEXT, NULL, CFG_ENTRY_PROFILE1_NAME, "profile.file1",
				p1_name_vnt),

		mkentry(CFG_TEXT, NULL, CFG_ENTRY_PROFILE2_NAME, "profile.file2",
				p2_name_vnt),

		//				mkentry(CFG_),

		//{ "View events as", "param.eview", CFG_CHOICE, eview_vnt[0], eview_vnt },

		//{ "Event font", "param.event_font", CFG_FONT, "Arial,24", NULL },

		{ 0, NULL, NULL, 0, NULL, NULL }

};

namespace bg = boost::gregorian;

MonthView *mv;
DayView* dv;
WeekView* wv;
YearView* yv;
ViewManager *vm;
//ical_parser p("basic.ics");

void file_selected(char *path) {
	cout << "Selected path " << path << endl;
}

void save_config() {
	dbg << "Saving configuration" << endl;
	dbg << "Setting last view param" << endl;
	WriteInt(config, "view.last_view", vm->config().view.last_view);
	dbg << "Status=" << SaveConfig(config) << endl;
	dbg << "Config saved" << endl;

}

void config_ok() {
	save_config();
	vm -> update_configuration();
	vm -> rotate();
}

void change_handler(char* name) {
	dbg << "Change handler called for " << name << endl;

	return;

	if (string(name) == "profile.dir") {
		dbg << "opening selection dialog" << endl;
		char buf[256] = "Select file";
		OpenDirectorySelector("Test", buf, sizeof(buf), file_selected);
		cout << "Directory selector opened" << endl;

	}
}

void open_configuration() {

	dbg << "Running configuration" << endl;
	OpenConfigEditor(US(CFG_CONFIGURE_TITLE.c_str()), config, calce, config_ok,
			change_handler);
}

int main_handler(int type, int par1, int par2);

#if 0
void msg(const char *message) {
	int x = 10;

	Screen s = Screen::get_default();
	int y = s.height() - 150;

	s.set_font(event_f, BLACK);
	Rect r(x, y, s.width(), 50);
	s.clear_rect(r);
	s.draw_text_rect(r, message, ALIGN_LEFT | VALIGN_MIDDLE);
	cout << "Drawing at (" << x << ", " << y << ")" << " string " << message
	<< endl;

	s.partial_update(r);
}
#endif

void restore_view(int param) {
	dbg << "Restoring last view for value of param " << param << endl;
	switch (param) {
	case Config::LastView:
		restore_view(vm -> config().view.last_view);
		break;
	case Config::MonthView:
		dbg << "Initially, setting Month View" << endl;
		vm -> set_view(mv, false);
		break;

	case Config::TodayView:
		dbg << "Initially, setting Day View" << endl;
		vm -> set_view(dv, false);
		break;

	case Config::WeekView:
		dbg << "Initially, setting Week View" << endl;
		vm -> set_view(wv, false);
		break;

	case Config::YearView:
		dbg << "Initially, setting Year View" << endl;
		vm -> set_view(yv, false);
		break;
	}

}

int main_handler(int type, int par1, int par2) {
	fprintf(stderr, "[%i %i %i]\n", type, par1, par2);

	switch (type) {
	case EVT_INIT:
		// occurs once at startup, only in main handler
		SetKeyboardRate(1000, 1500);

		config = OpenConfig(USERDATA "/calendar/config.cfg", calce);
		dbg << "Read config" << config << endl;

		{
			Translations trans;
		}

		// TODO: Add file selection
		// TODO: Add error handling/exceptions
		vm = new ViewManager(config);

		mv = new MonthView(config, vm);
		dbg << "Created MonthView" << endl;
		dv = new DayView(config, vm);
		dbg << "Created DayView" << endl;
		wv = new WeekView(config, vm);
		dbg << "Created WeekView" << endl;
		yv = new YearView(config, vm);
		dbg << "Created YearView" << endl;

		vm->addView(dv);
		vm->addView(wv);
		vm->addView(mv);
		vm->addView(yv);

		vm->set_date(bg::day_clock::local_day());

		restore_view(vm -> config().view.starting_view);
		break;

	case EVT_SHOW:
		// occurs when this event handler becomes active
		vm->rotate();
		vm->cur_view()->draw();
		break;

	case EVT_ORIENTATION:
		// TODO: orientation, if not auto, do nothing
		if (vm->config().view.orientation == Config::Auto) {
			vm->change_orientation(par1);
		}
		break;

	case EVT_KEYPRESS:
		if (KEY_MENU == par1)
			open_configuration();
		return 1;

	case EVT_KEYRELEASE:
	case EVT_KEYREPEAT: {
		ViewAction va = vm ->cur_view() -> handle_keypress(type, par1, par2);
		switch (va) {
		case PROCESSED:
			return 1;
		case ZOOM_IN:
			vm -> zoom_in();
			return 1;
		case ZOOM_OUT:
			if (par2)
				CloseApp();
			else
				vm -> zoom_out();
			return 1;

		case NOT_PROCESSED:
		default:
			break;
		}

		switch (par1) {
		case KEY_MINUS:
			vm -> zoom_out();
			return 1;
		case KEY_PLUS:
			vm -> zoom_in();
			return 1;

		case KEY_DELETE:
			CloseApp();
			break;
		default:
			break;

		}
	}
	case EVT_EXIT:
		// occurs only in main handler when exiting or when SIGINT received.
		// save configuration here, if needed
		dbg << "Saving settings..." << endl;
		save_config();
		dbg << "Exitting..." << endl;

		break;
	}

	return 0;
}

int main(int argc, char **argv) {
	InkViewMain(main_handler);
	return 0;
}

