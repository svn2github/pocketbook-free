/*
 * Copyright (C) 2004-2008 Geometer Plus <contact@geometerplus.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "ZLGtkOptionView.h"
#include "ZLGtkOptionViewHolder.h"
#include "../dialogs/ZLGtkDialogManager.h"
#include "../dialogs/ZLGtkUtil.h"
#include "../util/ZLGtkKeyUtil.h"

static GtkLabel *gtkLabel(const std::string &name) {
	GtkLabel *label = GTK_LABEL(gtk_label_new((gtkString(name) + ":").c_str()));
	gtk_label_set_justify(label, GTK_JUSTIFY_RIGHT);
	return label;
}

void ZLGtkOptionView::_onValueChanged(GtkWidget*, gpointer self) {
	((ZLGtkOptionView*)self)->onValueChanged();
}

void BooleanOptionView::_createItem() {
	myCheckBox = GTK_CHECK_BUTTON(gtk_check_button_new_with_mnemonic(gtkString(name()).c_str()));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(myCheckBox), ((ZLBooleanOptionEntry&)*myOption).initialState());
	g_signal_connect(GTK_WIDGET(myCheckBox), "toggled", G_CALLBACK(_onValueChanged), this);
	myHolder.attachWidget(*this, GTK_WIDGET(myCheckBox));
}

void BooleanOptionView::_show() {
	gtk_widget_show(GTK_WIDGET(myCheckBox));
}

void BooleanOptionView::_hide() {
	gtk_widget_hide(GTK_WIDGET(myCheckBox));
}

void BooleanOptionView::_onAccept() const {
	((ZLBooleanOptionEntry&)*myOption).onAccept(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(myCheckBox)));
}

void BooleanOptionView::onValueChanged() {
	((ZLBooleanOptionEntry&)*myOption).onStateChanged(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(myCheckBox)));
}

void Boolean3OptionView::_onReleased(GtkButton *button, gpointer self) {
	Boolean3OptionView &view = *(Boolean3OptionView*)self;
	switch (view.myState) {
		case B3_TRUE:
			view.setState(B3_UNDEFINED);
			break;
		case B3_FALSE:
			view.setState(B3_TRUE);
			break;
		case B3_UNDEFINED:
			view.setState(B3_FALSE);
			break;
	}
	view.onValueChanged();
}

void Boolean3OptionView::setState(ZLBoolean3 state) {
	if (myState != state) {
		myState = state;
		bool active = false;
		bool inconsistent = false;
		switch (myState) {
			case B3_TRUE:
				active = true;
				break;
			case B3_FALSE:
				break;
			case B3_UNDEFINED:
				inconsistent = true;
				break;
		}
		gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(myCheckBox), inconsistent);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(myCheckBox), active);
	}
}

void Boolean3OptionView::_createItem() {
	myCheckBox = GTK_CHECK_BUTTON(gtk_check_button_new_with_mnemonic(gtkString(name()).c_str()));
	setState(((ZLBoolean3OptionEntry&)*myOption).initialState());
	g_signal_connect(GTK_WIDGET(myCheckBox), "released", G_CALLBACK(_onReleased), this);
	myHolder.attachWidget(*this, GTK_WIDGET(myCheckBox));
}

void Boolean3OptionView::_show() {
	gtk_widget_show(GTK_WIDGET(myCheckBox));
}

void Boolean3OptionView::_hide() {
	gtk_widget_hide(GTK_WIDGET(myCheckBox));
}

void Boolean3OptionView::_onAccept() const {
	((ZLBoolean3OptionEntry&)*myOption).onAccept(myState);
}

void Boolean3OptionView::onValueChanged() {
	((ZLBoolean3OptionEntry&)*myOption).onStateChanged(myState);
}

void ChoiceOptionView::_createItem() {
	myFrame = GTK_FRAME(gtk_frame_new(name().c_str()));
	myVBox = GTK_BOX(gtk_vbox_new(true, 10));
	gtk_container_set_border_width(GTK_CONTAINER(myVBox), 5);

	int num = ((ZLChoiceOptionEntry&)*myOption).choiceNumber();
	myButtons = new GtkRadioButton* [num];
	GSList *group = 0;
	for (int i = 0; i < num; ++i) {
		myButtons[i] = GTK_RADIO_BUTTON(gtk_radio_button_new_with_label(group, ((ZLChoiceOptionEntry&)*myOption).text(i).c_str()));
		group = gtk_radio_button_get_group(myButtons[i]);
		gtk_box_pack_start (myVBox, GTK_WIDGET(myButtons[i]), true, true, 0);
	}
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(myButtons[((ZLChoiceOptionEntry&)*myOption).initialCheckedIndex()]), true);
	gtk_container_add(GTK_CONTAINER(myFrame), GTK_WIDGET(myVBox));
	myHolder.attachWidget(*this, GTK_WIDGET(myFrame));
}

void ChoiceOptionView::_show() {
	gtk_widget_show(GTK_WIDGET(myFrame));
	gtk_widget_show(GTK_WIDGET(myVBox));
	for (int i = 0; i < ((ZLChoiceOptionEntry&)*myOption).choiceNumber(); ++i) {
		gtk_widget_show(GTK_WIDGET(myButtons[i]));
	}
}

void ChoiceOptionView::_hide() {
	gtk_widget_hide(GTK_WIDGET(myFrame));
	gtk_widget_hide(GTK_WIDGET(myVBox));
	for (int i = 0; i < ((ZLChoiceOptionEntry&)*myOption).choiceNumber(); ++i) {
		gtk_widget_hide(GTK_WIDGET(myButtons[i]));
	}
}

void ChoiceOptionView::_onAccept() const {
	for (int i = 0; i < ((ZLChoiceOptionEntry&)*myOption).choiceNumber(); ++i) {
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(myButtons[i]))) {
			((ZLChoiceOptionEntry&)*myOption).onAccept(i);
			return;
		}
	}
}

void ComboOptionView::_createItem() {
	const ZLComboOptionEntry &comboOptionEntry = (ZLComboOptionEntry&)*myOption;
	myLabel = gtkLabel(name());
	myComboBox = comboOptionEntry.isEditable() ?
		GTK_COMBO_BOX(gtk_combo_box_entry_new_text()) : 
		GTK_COMBO_BOX(gtk_combo_box_new_text());

	g_signal_connect(GTK_WIDGET(myComboBox), "changed", G_CALLBACK(_onValueChanged), this);

	myHolder.attachWidgets(*this, GTK_WIDGET(myLabel), GTK_WIDGET(myComboBox));

	reset();
}

void ComboOptionView::_show() {
	gtk_widget_show(GTK_WIDGET(myLabel));
	gtk_widget_show(GTK_WIDGET(myComboBox));
}

void ComboOptionView::_hide() {
	gtk_widget_hide(GTK_WIDGET(myLabel));
	gtk_widget_hide(GTK_WIDGET(myComboBox));
}

void ComboOptionView::_setActive(bool active) {
	gtk_widget_set_sensitive(GTK_WIDGET(myComboBox), active);
}

void ComboOptionView::_onAccept() const {
	((ZLComboOptionEntry&)*myOption).onAccept(gtk_combo_box_get_active_text(myComboBox));
}

void ComboOptionView::reset() {
	if (myComboBox == 0) {
		return;
	}

	for (; myListSize > 0; --myListSize) {
		gtk_combo_box_remove_text(myComboBox, 0);
	}
	const ZLComboOptionEntry &comboOptionEntry = (ZLComboOptionEntry&)*myOption;
	const std::vector<std::string> &values = comboOptionEntry.values();
	const std::string &initial = comboOptionEntry.initialValue();
	myListSize = values.size();
	mySelectedIndex = -1;
	int index = 0;
	for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it, ++index) {
		if (*it == initial) {
			mySelectedIndex = index;
		}
		gtk_combo_box_append_text(myComboBox, it->c_str());
	}
	if (mySelectedIndex >= 0) {
		gtk_combo_box_set_active(myComboBox, mySelectedIndex);
	}
}

void ComboOptionView::onValueChanged() {
	int index = gtk_combo_box_get_active(myComboBox);
	ZLComboOptionEntry &o = (ZLComboOptionEntry&)*myOption;
	if ((index != mySelectedIndex) && (index >= 0) && (index < (int)o.values().size())) {
		mySelectedIndex = index;
  	o.onValueSelected(mySelectedIndex);
	} else if (o.useOnValueEdited()) {
		std::string text = gtk_combo_box_get_active_text(myComboBox);
  	o.onValueEdited(text);
	}
}

void SpinOptionView::_createItem() {
	ZLSpinOptionEntry &option = (ZLSpinOptionEntry&)*myOption;

	myLabel = gtkLabel(name());

	GtkAdjustment *adjustment = GTK_ADJUSTMENT(gtk_adjustment_new(option.initialValue(), option.minValue(), option.maxValue(), option.step(), option.step(), 0));
	mySpinBox = GTK_SPIN_BUTTON(gtk_spin_button_new(adjustment, 1, 0));

	myHolder.attachWidgets(*this, GTK_WIDGET(myLabel), GTK_WIDGET(mySpinBox));
}

void SpinOptionView::_show() {
	gtk_widget_show(GTK_WIDGET(myLabel));
	gtk_widget_show(GTK_WIDGET(mySpinBox));
}

void SpinOptionView::_hide() {
	gtk_widget_hide(GTK_WIDGET(myLabel));
	gtk_widget_hide(GTK_WIDGET(mySpinBox));
}

void SpinOptionView::_onAccept() const {
	gtk_spin_button_update(GTK_SPIN_BUTTON(mySpinBox));
	((ZLSpinOptionEntry&)*myOption).onAccept((int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(mySpinBox)));
}

void StringOptionView::_createItem() {
	myLineEdit = GTK_ENTRY(gtk_entry_new());
	g_signal_connect(myLineEdit, "changed", G_CALLBACK(_onValueChanged), this);

	if (!name().empty()) {
		myLabel = gtkLabel(name());
		myHolder.attachWidgets(*this, GTK_WIDGET(myLabel), GTK_WIDGET(myLineEdit));
	} else {
		myLabel = 0;
		myHolder.attachWidget(*this, GTK_WIDGET(myLineEdit));
	}

	reset();
}

void StringOptionView::reset() {
	if (myLineEdit == 0) {
		return;
	}

	gtk_entry_set_text(myLineEdit, ((ZLStringOptionEntry&)*myOption).initialValue().c_str());
}

void StringOptionView::onValueChanged() {
	ZLStringOptionEntry &o = (ZLStringOptionEntry&)*myOption;
	if (o.useOnValueEdited()) {
		o.onValueEdited(gtk_entry_get_text(myLineEdit));
	}
}

void StringOptionView::_show() {
	if (myLabel != 0) {
		gtk_widget_show(GTK_WIDGET(myLabel));
	}
	gtk_widget_show(GTK_WIDGET(myLineEdit));
}

void StringOptionView::_hide() {
	if (myLabel != 0) {
		gtk_widget_hide(GTK_WIDGET(myLabel));
	}
	gtk_widget_hide(GTK_WIDGET(myLineEdit));
}

void StringOptionView::_setActive(bool active) {
	gtk_entry_set_editable(myLineEdit, active);
}

void StringOptionView::_onAccept() const {
	((ZLStringOptionEntry&)*myOption).onAccept(gtk_entry_get_text(myLineEdit));
}

static GdkColor convertColor(const ZLColor &color) {
	GdkColor gdkColor;
	gdkColor.red = color.Red * 65535 / 255;
	gdkColor.blue = color.Blue * 65535 / 255;
	gdkColor.green = color.Green * 65535 / 255;
	return gdkColor;
}

static ZLColor convertColor(const GdkColor &color) {
	ZLColor zlColor;
	zlColor.Red = color.red * 255 / 65535;
	zlColor.Blue = color.blue * 255 / 65535;
	zlColor.Green = color.green * 255 / 65535;
	return zlColor;
}

void ColorOptionView::_createItem() {
	GdkColor initialColor = convertColor(((ZLColorOptionEntry&)*myOption).initialColor());
	GdkColor currentColor = convertColor(((ZLColorOptionEntry&)*myOption).color());

	myColorSelection = GTK_COLOR_SELECTION(gtk_color_selection_new());
	gtk_color_selection_set_has_opacity_control(myColorSelection, false);
	gtk_color_selection_set_has_palette(myColorSelection, true);
	gtk_color_selection_set_current_color(myColorSelection, &currentColor);
	gtk_color_selection_set_previous_color(myColorSelection, &initialColor);

	GtkContainer *container = GTK_CONTAINER(gtk_vbox_new(true, 0));
	gtk_container_set_border_width(container, 5);
	gtk_container_add(container, GTK_WIDGET(myColorSelection));
	myHolder.attachWidget(*this, GTK_WIDGET(container));
	gtk_widget_show(GTK_WIDGET(container));
}

void ColorOptionView::reset() {
	if (myColorSelection == 0) {
		return;
	}

	ZLColorOptionEntry &colorEntry = (ZLColorOptionEntry&)*myOption;

	GdkColor gdkColor;
	gtk_color_selection_get_current_color(myColorSelection, &gdkColor);
	colorEntry.onReset(convertColor(gdkColor));

	GdkColor initialColor = convertColor(((ZLColorOptionEntry&)*myOption).initialColor());
	GdkColor currentColor = convertColor(((ZLColorOptionEntry&)*myOption).color());
	gtk_color_selection_set_current_color(myColorSelection, &currentColor);
	gtk_color_selection_set_previous_color(myColorSelection, &initialColor);
}

void ColorOptionView::_show() {
	gtk_widget_show(GTK_WIDGET(myColorSelection));
}

void ColorOptionView::_hide() {
	gtk_widget_hide(GTK_WIDGET(myColorSelection));
}

void ColorOptionView::_onAccept() const {
	GdkColor gdkColor;
	gtk_color_selection_get_current_color(myColorSelection, &gdkColor);
	((ZLColorOptionEntry&)*myOption).onAccept(convertColor(gdkColor));
}

static bool key_view_focus_in_event(GtkWidget *entry, GdkEventFocus*, gpointer) {
	gdk_keyboard_grab(entry->window, true, GDK_CURRENT_TIME);
	((ZLGtkDialogManager&)ZLGtkDialogManager::instance()).grabKeyboard(true);
	return false;
}

static bool key_view_focus_out_event(GtkWidget*, GdkEventFocus*, gpointer) {
	((ZLGtkDialogManager&)ZLGtkDialogManager::instance()).grabKeyboard(false);
	gdk_keyboard_ungrab(GDK_CURRENT_TIME);
	return false;
}

static bool key_view_key_press_event(GtkWidget *entry, GdkEventKey *event, gpointer data) {
	gtk_entry_set_text(GTK_ENTRY(entry), ZLGtkKeyUtil::keyName(event).c_str());
	((KeyOptionView*)data)->setKey(ZLGtkKeyUtil::keyName(event));
	return true;
}

void KeyOptionView::_createItem() {
	myKeyEntry = GTK_ENTRY(gtk_entry_new());
	gtk_signal_connect(GTK_OBJECT(myKeyEntry), "focus_in_event", G_CALLBACK(key_view_focus_in_event), 0);
	gtk_signal_connect(GTK_OBJECT(myKeyEntry), "focus_out_event", G_CALLBACK(key_view_focus_out_event), 0);
	gtk_signal_connect(GTK_OBJECT(myKeyEntry), "key_press_event", G_CALLBACK(key_view_key_press_event), this);
	key_view_focus_out_event(GTK_WIDGET(myKeyEntry), 0, 0);

	myLabel = GTK_LABEL(gtkLabel(ZLResource::resource("keyOptionView")["actionFor"].value()));

	myComboBox = GTK_COMBO_BOX(gtk_combo_box_new_text());
	const std::vector<std::string> &actions = ((ZLKeyOptionEntry&)*myOption).actionNames();
	for (std::vector<std::string>::const_iterator it = actions.begin(); it != actions.end(); ++it) {
		gtk_combo_box_append_text(myComboBox, it->c_str());
	}

	myTable = GTK_TABLE(gtk_table_new(2, 2, false));
	gtk_table_set_col_spacings(myTable, 5);
	gtk_table_set_row_spacings(myTable, 5);
	gtk_table_attach_defaults(myTable, GTK_WIDGET(myLabel), 0, 1, 0, 1);
	gtk_table_attach_defaults(myTable, GTK_WIDGET(myKeyEntry), 1, 2, 0, 1);
	gtk_table_attach_defaults(myTable, GTK_WIDGET(myComboBox), 0, 2, 1, 2);
	g_signal_connect(GTK_WIDGET(myComboBox), "changed", G_CALLBACK(_onValueChanged), this);

	myHolder.attachWidget(*this, GTK_WIDGET(myTable));
}

void KeyOptionView::onValueChanged() {
	if (!myCurrentKey.empty()) {
		((ZLKeyOptionEntry&)*myOption).onValueChanged(
			myCurrentKey,
			gtk_combo_box_get_active(myComboBox)
		);
	}
}

void KeyOptionView::setKey(const std::string &key) {
	myCurrentKey = key;
	if (!key.empty()) {
		gtk_combo_box_set_active(myComboBox, ((ZLKeyOptionEntry&)*myOption).actionIndex(key));
		gtk_widget_show(GTK_WIDGET(myComboBox));
	}
	((ZLKeyOptionEntry&)*myOption).onKeySelected(myCurrentKey);
}

void KeyOptionView::reset() {
	if (myTable == 0) {
		return;
	}
	myCurrentKey.erase();
	gtk_entry_set_text(myKeyEntry, "");
	((ZLKeyOptionEntry&)*myOption).onKeySelected(myCurrentKey);
	gtk_widget_hide(GTK_WIDGET(myComboBox));
}

void KeyOptionView::_show() {
	gtk_widget_show(GTK_WIDGET(myTable));
	gtk_widget_show(GTK_WIDGET(myKeyEntry));
	gtk_widget_show(GTK_WIDGET(myLabel));
	if (!myCurrentKey.empty()) {
		gtk_widget_show(GTK_WIDGET(myComboBox));
	} else {
		gtk_widget_hide(GTK_WIDGET(myComboBox));
	}
}

void KeyOptionView::_hide() {
	gtk_widget_hide(GTK_WIDGET(myTable));
	myCurrentKey.erase();
	gtk_entry_set_text(myKeyEntry, "");
	((ZLKeyOptionEntry&)*myOption).onKeySelected(myCurrentKey);
}

void KeyOptionView::_onAccept() const {
	((ZLKeyOptionEntry&)*myOption).onAccept();
}
