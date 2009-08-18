#ifndef PBCONTROLS_H
#define PBCONTROLS_H

#include <stdio.h>
#include <map>
#include <string>
#include <utility>
#include "inkview.h"
#include "cppevent.h"
#include "cppslot.h"

#define BORDER_SPACE 4 // space between controls

class PBControl
{
protected:
	std::string _name;
	
	int _top;
	int _left;
	int _width;
	int _height;
	
	bool _canBeFocused;
	bool _drawBorder;
	bool _focused;
	bool _leaveOnKeys;
	bool _visible;
	
	ifont *_font;
	bool _useParentFont;
	
	std::string _focusedControl;
	
	PBControl *_parent;
	
	std::vector<std::pair<std::string, PBControl *> > _controls;
	std::string _tag;
	
	virtual int DispatchMsgToControls(int type, int par1, int par2);
	virtual void ClearRegion();
	virtual void PlaceControls();
	
	CppSlot2<PBControl, void, PBControl *, bool> ControlFocusChangedSlot;
	virtual void ControlFocusChangedHandler(PBControl *sender, bool focused);
	
	CppSlot2<PBControl, void, PBControl *, bool> ControlLeaveSlot;
	virtual void ControlLeaveHandler(PBControl *sender, bool next);
	
public:

	PBControl(std::string name, PBControl *parent);
	virtual ~PBControl();
	
	// property accessors
	virtual std::string GetName();
	virtual int GetTop();
	virtual void SetTop(int value);
	virtual int GetLeft();
	virtual void SetLeft(int value);
	virtual int GetWidth();
	virtual void SetWidth(int value);
	virtual int GetHeight();
	virtual void SetHeight(int value);
	virtual void SetSize(int left, int top, int width, int height);
	virtual bool GetCanBeFocused();
	virtual void SetCanBeFocused(bool value);
	virtual bool GetDrawBorder();
	virtual void SetDrawBorder(bool value);
	virtual bool GetFocused();
	virtual void SetFocused(bool value);
	virtual bool GetLeaveOnKeys();
	virtual void SetLeaveOnKeys(bool value);
	virtual void SetControlFont(ifont *value);
	virtual ifont *GetFont();
	virtual PBControl *GetFocusedControl();
	virtual bool GetVisible();
	virtual void SetVisible(bool value);
	virtual PBControl *GetParent();
	virtual std::string GetTag();
	virtual void SetTag(std::string value);
	
	// public methods
	virtual int HandleMsg(int type, int par1, int par2);
	virtual void Draw();
	virtual void DrawIfVisible();
	virtual void Update();
	
	virtual void AddControl(PBControl *control);
	
	// events
	CppSignal2<void, PBControl *, bool> OnFocusChanged;
	CppSignal2<void, PBControl *, bool> OnLeave;	
	CppSignal1<void, PBControl *> OnFocusedControlChanged;
};

typedef std::vector<std::pair<std::string, PBControl *> >::iterator control_it;
typedef std::vector<std::pair<std::string, PBControl *> >::reverse_iterator control_rev_it;

class PBListBoxItem : public PBControl
{
protected:
	std::string _text;
	ibitmap *_image;
	std::string _marker;
	int _maxHeight;
public:
	PBListBoxItem(std::string name, PBControl *parent);
	
	virtual void Draw();
	
	virtual int GetHeight();
	virtual void SetMaxHeight(int value);
	virtual int GetMaxHeight();
	virtual ibitmap * GetImage();
	virtual void SetImage(ibitmap * value);
	virtual std::string GetMarker();
	virtual void SetMarker(std::string value);
	virtual std::string GetText();
	virtual void SetText(std::string value);
};

typedef std::vector<PBListBoxItem *>::iterator lbitem_it;

class PBListBox : public PBControl
{
protected:
	std::vector<PBListBoxItem *> _items;
	int _topItemIndex, _bottomItemIndex;
	std::string _caption;
	
	virtual void PlaceControls();
	virtual int GetPageHeight();
	virtual int GetFooterHeight();
	virtual int GetCaptionHeight();
public:
	PBListBox(std::string name, PBControl *parent);
	~PBListBox();
	
	virtual int HandleMsg(int type, int par1, int par2);
	virtual void Draw();
	
	virtual std::string GetCaption();
	virtual void SetCaption(std::string value);
	virtual std::vector<PBListBoxItem *> GetItems();
	virtual PBListBoxItem *AddItem(std::string text, std::string tag = "");
	virtual PBListBoxItem *AddItem(ibitmap *image, std::string tag = "");
	virtual void Clear();
	virtual int GetSelectedIndex();
	virtual void GetPageItems(int startItemIndex, bool direct);	
};

class PBButton : public PBControl
{
protected:
	ibitmap *_image;
	std::string _text;
	
	int _maxWidth;
	
public:

	PBButton(std::string name, PBControl *parent);
	
	virtual void Draw();
	virtual int HandleMsg(int type, int par1, int par2);
	
	virtual int GetWidth();
	virtual void SetMaxWidth(int value);
	virtual int GetMaxWidth();
	virtual ibitmap *GetImage();
	virtual void SetImage(ibitmap *value);
	virtual std::string GetText();
	virtual void SetText(std::string value);
	
	CppSignal1<void, PBControl *> OnPressed;
};

#endif

