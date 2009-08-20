#include "pbcontrols.h"

PBControl::PBControl(std::string name, PBControl *parent) :
	ControlFocusChangedSlot(this, &PBControl::ControlFocusChangedHandler), ControlLeaveSlot(this, &PBControl::ControlLeaveHandler)
{
	_name = name;
	_parent = parent;
	
	if (parent == 0)
	{
		_top = 0;
		_left = 0;
		_useParentFont = false;
	}
	else
	{
		_top = parent->GetTop();
		_left = parent->GetLeft();
		_useParentFont = true;
		_font = parent->GetFont();
	}
	_width = 10;
	_height = 10;
	_canBeFocused = true;
	_drawBorder = true;
	_focused = false;
	_leaveOnKeys = true;
	_visible = true;
}

PBControl::~PBControl()
{
}

int PBControl::DispatchMsgToControls(int type, int par1, int par2)
{
	PBControl *focused = GetFocusedControl();
	if (focused != 0)
		return focused->HandleMsg(type, par1, par2);
		
	return 0;
}

void PBControl::PlaceControls()
{
}
	
void PBControl::ControlFocusChangedHandler(PBControl *sender, bool focused)
{
	// only one child control can have focus
	if (focused)
	{
		if (GetFocusedControl() != 0 && GetFocusedControl() != sender)
			GetFocusedControl()->SetFocused(false);
			
		_focusedControl.assign(sender->GetName());
		
		OnFocusedControlChanged.emit_sig(this);
	}
	else
	{
		
	}
}

void PBControl::ControlLeaveHandler(PBControl *sender, bool next)
{
	//fprintf(stderr, "\nleave %s", sender->GetName().c_str());

	// check if focus already moved
	if (!sender->GetFocused())
		return;
		
	// get iterator for sender
	control_it it;
	for (it = _controls.begin(); it != _controls.end(); it++)
		if (it->first == sender->GetName()) break;

	control_rev_it last_it = _controls.rbegin();
			
	// cycle focus through visible controls
	if (next)
	{
		do
		{
			if (it->first == last_it->first)
			{
				if (GetLeaveOnKeys())
				{
					OnLeave.emit_sig(this, next);
					return;
				}
				else
					it = _controls.begin();
			}
			else
				it++;
	
			if (it->second->GetVisible() && it->second->GetCanBeFocused())
			{
				it->second->SetFocused(true);
				Update();
				break;
			}
		
		}
		while(it->second != sender);
	}
	else
	{
		do
		{
			if (it == _controls.begin())
			{
				if (GetLeaveOnKeys())
				{
					OnLeave.emit_sig(this, next);
					return;
				}
				else
				{
					for (it = _controls.begin(); it != _controls.end(); it++)
						if (it->first == last_it->first) break;
					//it = _controls.find(last_it->first);
				}
			}
			else
				it--;
						
			if (it->second->GetVisible() && it->second->GetCanBeFocused())
			{
				it->second->SetFocused(true);
				Update();
				break;
			}
		}
		while(it->second != sender);
	}

}

std::string PBControl::GetName()
{
	return _name;
}

int PBControl::GetTop()
{
	return _top;
}

void PBControl::SetTop(int value)
{
	_top = value;
}

int PBControl::GetLeft()
{
	return _left;
}

void PBControl::SetLeft(int value)
{
	_left = value;
}

int PBControl::GetWidth()
{
	return _width;
}

void PBControl::SetWidth(int value)
{
	_width = value;
}

int PBControl::GetHeight()
{
	return _height;
}

void PBControl::SetHeight(int value)
{
	_height = value;
}

void PBControl::SetSize(int left, int top, int width, int height)
{
	SetLeft(left);
	SetTop(top);
	SetWidth(width);
	SetHeight(height);
}

bool PBControl::GetCanBeFocused()
{
	return _canBeFocused;
}

void PBControl::SetCanBeFocused(bool value)
{
	_canBeFocused = value;
	
	if (!_canBeFocused && GetFocused())
		SetFocused(false);
}
	
bool PBControl::GetDrawBorder()
{
	return _drawBorder;
}

void PBControl::SetDrawBorder(bool value)
{
	_drawBorder = value;
}
	
bool PBControl::GetFocused()
{
	return _focused;
}

void PBControl::SetFocused(bool value)
{
	if (_focused == value)
		return;
		
	// if lost focus, clear focus of child controls
	if (!value)
	{
		for (control_it it = _controls.begin(); it != _controls.end(); it++)
		{
			it->second->SetFocused(false);
		}
	}
	// if got focus
	else
	{	
		if (!GetCanBeFocused())
			return;
		
		if(GetFocusedControl() != 0)
		{
			GetFocusedControl()->SetFocused(true);
		}
		else
		{
			// focus first visible child control
			for (control_it it = _controls.begin(); it != _controls.end(); it++)
			{
				if(it->second->GetVisible() && it->second->GetCanBeFocused())
				{
					it->second->SetFocused(true);
					break;
				}
			}
		}
	}
	_focused = value;
			
	OnFocusChanged.emit_sig(this, value);
}

bool PBControl::GetLeaveOnKeys()
{
	return _leaveOnKeys;
}

void PBControl::SetLeaveOnKeys(bool value)
{
	_leaveOnKeys = value;
}

void PBControl::SetControlFont(ifont *value)
{
	if (_font == value)
		return;
		
	_font = value;
	_useParentFont = false;
}

ifont *PBControl::GetFont()
{
	if (_useParentFont && _parent != 0)
		return _parent->GetFont();
		
	return _font;
}

PBControl *PBControl::GetFocusedControl()
{
	control_it it;
	//= _controls.find(_focusedControl);
	for (it = _controls.begin(); it != _controls.end(); it++)
		if (it->first == _focusedControl) break;

	if (it == _controls.end())
		return 0;
		
	return it->second;
	
	//for (control_it it = _controls.begin(); it != _controls.end(); it++)
	//{
	//	if(it->second->GetFocused())
	//		return it->second;
	//}
	
	return 0;
}

bool PBControl::GetVisible()
{

	if (_parent != 0 && !_parent->GetVisible())
	{
		return false;
	}
		
	return _visible;
}

void PBControl::SetVisible(bool value)
{
	_visible = value;
}

PBControl *PBControl::GetParent()
{
	return _parent;
}

std::string PBControl::GetTag()
{
	return _tag;
}

void PBControl::SetTag(std::string value)
{
	_tag.assign(value);
}
	
int PBControl::HandleMsg(int type, int par1, int par2)
{
	DispatchMsgToControls(type, par1, par2);
	
	return 0;
}

void PBControl::ClearRegion()
{
	FillArea(GetLeft(), GetTop(), GetWidth(), GetHeight(), WHITE);
}
	
void PBControl::Draw()
{
	ClearRegion();
	PlaceControls();
	if (GetDrawBorder())
		DrawRect(GetLeft(), GetTop(), GetWidth(), GetHeight(), BLACK);
	for (control_it it = _controls.begin(); it != _controls.end(); it++)
	{
		it->second->DrawIfVisible();
	}
	
	if (GetDrawBorder() && GetFocused())
		DrawRect(GetLeft() + BORDER_SPACE/2, GetTop() + BORDER_SPACE/2, GetWidth() - BORDER_SPACE, GetHeight() - BORDER_SPACE, BLACK);
}

void PBControl::DrawIfVisible()
{
	if (GetVisible())
	{
		SetFont(GetFont(), BLACK);
		Draw();
	}
}

void PBControl::Update()
{
//fprintf(stderr, "\n *** update %s ***", GetName().c_str());
	DrawIfVisible();
	PartialUpdate(GetLeft(), GetTop(), GetWidth(), GetHeight());
}

void PBControl::AddControl(PBControl *control)
{
	_controls.push_back(std::make_pair(control->GetName(), control));
	control->OnFocusChanged.connect(&ControlFocusChangedSlot);
	control->OnLeave.connect(&ControlLeaveSlot);
}

PBListBoxItem::PBListBoxItem(std::string name, PBControl *parent) : PBControl(name, parent)
{
	_image = 0;
}

void PBListBoxItem::Draw()
{
	ClearRegion();
	int markerWidth = StringWidth((char*)GetMarker().c_str());
	if (GetImage() != 0)
		DrawBitmapRect(GetLeft()+3, GetTop()+3, GetWidth()-markerWidth-3, GetHeight()-3, GetImage(), ALIGN_CENTER);
	else
		DrawTextRect(GetLeft()+3, GetTop()+3, GetWidth()-markerWidth-3, GetHeight()-3, (char*)GetText().c_str(), ALIGN_LEFT);
	
	if (GetMarker().size() > 0)
	{
		DrawTextRect(GetLeft()+3, GetTop()+3, GetWidth()-3, GetHeight()-3, (char*)GetMarker().c_str(), ALIGN_RIGHT);
	}
		
	if (GetFocused())
		DrawSelection(GetLeft(), GetTop(), GetWidth(), GetHeight(), BLACK);
}

int PBListBoxItem::GetHeight()
{
	if (_height <= 0)
	{
		int height;
		if (GetImage() != 0)
			height = GetImage()->height;
		else
			height = TextRectHeight(GetWidth()-6-StringWidth((char*)GetMarker().c_str()), (char*)GetText().c_str(), ALIGN_LEFT)+6;
		
		if (GetMaxHeight() > 0 && height > GetMaxHeight())
			return GetMaxHeight();
			
		return height;
	}
	
	return PBControl::GetHeight();
}

int PBListBoxItem::GetMaxHeight()
{
	return _maxHeight;
}

void PBListBoxItem::SetMaxHeight(int value)
{
	_maxHeight = value;
}
	
ibitmap *PBListBoxItem::GetImage()
{
	return _image;
}

void PBListBoxItem::SetImage(ibitmap *value)
{
	_image = value;
}
	
std::string PBListBoxItem::GetText()
{
	return _text;
}

void PBListBoxItem::SetText(std::string value)
{
	_text.assign(value);
}

std::string PBListBoxItem::GetMarker()
{
	return _marker;
}

void PBListBoxItem::SetMarker(std::string value)
{
	_marker.assign(value);
}
	
PBListBox::PBListBox(std::string name, PBControl *parent) : PBControl(name, parent)
{
	_topItemIndex = _bottomItemIndex = -1;
}

PBListBox::~PBListBox()
{
	for (lbitem_it it = _items.begin(); it != _items.end(); it++)
		delete (*it);
}

std::vector<PBListBoxItem *> PBListBox::GetItems()
{
	return _items;
}

int PBListBox::HandleMsg(int type, int par1, int par2)
{
	if (type == EVT_KEYPRESS)
	{
		switch (par1)
		{
			case KEY_DOWN:
				if (GetFocusedControl() == 0 || _controls.size() == 0)
				{
					OnLeave.emit_sig(this, true);
				}
				else if (GetFocusedControl() != 0)
				{
					// if bottom item selected and there's next page, go to this page
					if (GetFocusedControl() == _items[_bottomItemIndex] && _bottomItemIndex < (int)_items.size()-1)
					{
						_items[_bottomItemIndex + 1]->SetFocused(true);
						GetPageItems(_bottomItemIndex + 1, true);
						Update();
					}
					else
					{
						ControlLeaveHandler(GetFocusedControl(), true);
						//Update();
					}
				}	
				break;
				
			case KEY_UP:
				if (GetFocusedControl() == 0 || _controls.size() == 0)
				{
					OnLeave.emit_sig(this, false);
				}
				else if (GetFocusedControl() != 0)
				{
					// if top item selected and there's previous page, go to this page
					if (GetFocusedControl() == _items[_topItemIndex] && _topItemIndex > 0)
					{
						_items[_topItemIndex - 1]->SetFocused(true);
						GetPageItems(_topItemIndex - 1, false);
						Update();
					}
					else
					{
						ControlLeaveHandler(GetFocusedControl(), false);
						//Update();
					}
				}	
				break;
				
			case KEY_RIGHT:
				if (_controls.size() == 0)
				{
					OnLeave.emit_sig(this, true);
				}
				// if there's no next pages, leave
				else if (_bottomItemIndex == (int)_items.size()-1)
				{
					OnLeave.emit_sig(this, true);
				}
				// show next page
				else
				{
					GetPageItems(_bottomItemIndex + 1, true);
					_items[_topItemIndex]->SetFocused(true);
					Update();
				}
				break;
				
			case KEY_LEFT:
				if (_controls.size() == 0)
				{
					OnLeave.emit_sig(this, false);
				}
				// if there's no previous pages, leave
				else if (_topItemIndex == 0)
				{
					OnLeave.emit_sig(this, false);
				}
				// show previous page
				else
				{
					GetPageItems(_topItemIndex - 1, false);
					_items[_topItemIndex]->SetFocused(true);
					Update();
				}
				break;
		}
	}
	
	return 0;
}

void PBListBox::PlaceControls()
{
	if (_items.size() == 0)
		return;
	
	GetPageItems(_topItemIndex, true);
	
	int top = GetTop() + BORDER_SPACE + GetCaptionHeight() + BORDER_SPACE;
	
	for (int i = _topItemIndex; i <= _bottomItemIndex; i++)
	{
		_items[i]->SetMaxHeight(GetPageHeight());
		_items[i]->SetSize(GetLeft()+BORDER_SPACE, top, GetWidth()-BORDER_SPACE*2, 0);
		top += _items[i]->GetHeight();
	}
}

int PBListBox::GetPageHeight()
{
	return GetHeight() - BORDER_SPACE*3 - GetCaptionHeight() - GetFooterHeight();
}

int PBListBox::GetCaptionHeight()
{
	return GetCaption().size() > 0 ? TextRectHeight(GetWidth() - BORDER_SPACE*2, (char *)GetCaption().c_str(), ALIGN_CENTER) : 0;
}

int PBListBox::GetFooterHeight()
{
	if (_topItemIndex > 0 || _bottomItemIndex < (int)_items.size()-1)
		return TextRectHeight(GetWidth(), "<<<", ALIGN_LEFT) + BORDER_SPACE;
		
	return 0;
}

void PBListBox::GetPageItems(int startItemIndex, bool direct)
{
	_topItemIndex = _bottomItemIndex = -1;
	int height = 0;
	int pageHeight = GetPageHeight();
	if (direct)
	{
		_topItemIndex = startItemIndex;
		for (int i = startItemIndex; i < (int)_items.size(); i++)
		{
			_items[i]->SetMaxHeight(pageHeight);
			_items[i]->SetSize(0, 0, GetWidth()-BORDER_SPACE*2, 0);
			height += _items[i]->GetHeight();
			
			if (height > pageHeight)
				break;
				
			_bottomItemIndex = i;
		}
	}
	else
	{
		_bottomItemIndex = startItemIndex;
		for (int i = startItemIndex; i >= 0; i--)
		{
			_items[i]->SetMaxHeight(pageHeight);
			_items[i]->SetSize(0, 0, GetWidth(), 0);
			height += _items[i]->GetHeight();
			
			if (height > pageHeight)
				break;
				
			_topItemIndex = i;
		}
	}	
}

void PBListBox::Draw()
{
	ClearRegion();
	PlaceControls();
	
	if (GetDrawBorder())
		DrawRect(GetLeft(), GetTop(), GetWidth(), GetHeight(), BLACK);
	
	if (GetCaption().size()>0)
	{
		DrawTextRect(GetLeft() + BORDER_SPACE, GetTop() + BORDER_SPACE, GetWidth() - BORDER_SPACE*2, GetCaptionHeight(), (char *)GetCaption().c_str(), ALIGN_CENTER);
		DrawLine(GetLeft() + BORDER_SPACE, GetTop() + BORDER_SPACE + GetCaptionHeight(), GetLeft() + GetWidth() - BORDER_SPACE, GetTop() + BORDER_SPACE + GetCaptionHeight(), BLACK);
	}
	
	if (_topItemIndex >= 0)
	{
		for (int i = _topItemIndex; i <= _bottomItemIndex; i++)
		{
			_items[i]->DrawIfVisible();
		}
	}
	
	if (_topItemIndex > 0)
		DrawTextRect(GetLeft() + BORDER_SPACE, GetTop() + GetHeight() - GetFooterHeight(), GetWidth() - BORDER_SPACE*2, GetFooterHeight(), "<<<", ALIGN_LEFT);
		
	if (_bottomItemIndex < (int)_items.size()-1)
		DrawTextRect(GetLeft() + BORDER_SPACE, GetTop() + GetHeight() - GetFooterHeight(), GetWidth() - BORDER_SPACE*2, GetFooterHeight(), ">>>", ALIGN_RIGHT);
	
	if (GetDrawBorder() && GetFocused())
		DrawRect(GetLeft() + BORDER_SPACE/2, GetTop() + BORDER_SPACE/2, GetWidth() - BORDER_SPACE, GetHeight() - BORDER_SPACE, BLACK);
}

std::string PBListBox::GetCaption()
{
	return _caption;
}

void PBListBox::SetCaption(std::string value)
{
	_caption.assign(value);
}

PBListBoxItem *PBListBox::AddItem(std::string text, std::string tag)
{
	char itemName[20];
	sprintf(itemName, "item%d", _items.size()+1);
	PBListBoxItem * newItem = new PBListBoxItem(itemName, this);
	newItem->SetText(text);
	newItem->SetTag(tag);
	
	_items.push_back(newItem);
	AddControl(newItem);
	
	if (GetFocusedControl() == 0 && GetFocused())
		newItem->SetFocused(true);
	
	if (_topItemIndex < 0)
		_topItemIndex = _bottomItemIndex = 0;
		
	return newItem;
}

PBListBoxItem *PBListBox::AddItem(ibitmap *image, std::string tag)
{
	PBListBoxItem * newItem = AddItem("", tag);
	newItem->SetImage(image);
	return newItem;
}

void PBListBox::Clear()
{
	_controls.clear();
	_items.clear();
	_focusedControl.clear();
	_topItemIndex = _bottomItemIndex = -1;
}

int PBListBox::GetSelectedIndex()
{
	int i = 0;
	for (control_it it = _controls.begin(); it != _controls.end(); it++, i++)
	{
		if(it->second->GetFocused())
		return i;
	}
	
	return -1;
}

PBButton::PBButton(std::string name, PBControl *parent) : PBControl(name, parent)
{
	_image = 0;
}

void PBButton::Draw()
{
	ClearRegion();
	
	DrawTextRect(GetLeft()+4, GetTop(), GetWidth()-4, GetHeight(), (char *)_text.c_str(), ALIGN_LEFT | VALIGN_MIDDLE);

	if (GetFocused())
		DrawSelection(GetLeft(), GetTop(), GetWidth(), GetHeight(), BLACK);
	else
		DrawRect(GetLeft(), GetTop(), GetWidth(), GetHeight(), BLACK);
}

int PBButton::HandleMsg(int type, int par1, int par2)
{
	if (type == EVT_KEYPRESS)
	{
		switch (par1)
		{
			case KEY_OK:
				OnPressed.emit_sig(this);
				break;
			case KEY_LEFT:
			case KEY_UP:
				OnLeave.emit_sig(this, false);
				break;
			case KEY_RIGHT:
			case KEY_DOWN:
				OnLeave.emit_sig(this, true);
				break;
		}
	}
	
	return 0;
}

int PBButton::GetWidth()
{
	if (_width <= 0)
	{
		int width;
		
		width = StringWidth((char*)GetText().c_str())+8;
		
		if (GetMaxWidth() > 0 && width > GetMaxWidth())
			return GetMaxWidth();
			
		return width;
	}
	
	return PBControl::GetWidth();
}

void PBButton::SetMaxWidth(int value)
{
	_maxWidth = value;
}

int PBButton::GetMaxWidth()
{
	return _maxWidth;
}

ibitmap *PBButton::GetImage()
{
	return _image;
}

void PBButton::SetImage(ibitmap *value)
{
	_image = value;
}

std::string PBButton::GetText()
{
	return _text;
}

void PBButton::SetText(std::string value)
{
	_text = value;
}

// PBLabel
PBLabel::PBLabel(std::string name, PBControl *parent) : PBControl(name, parent)
{
	_image = 0;
	_canBeFocused = false;
}

void PBLabel::Draw()
{
	ClearRegion();
	
	DrawTextRect(GetLeft()+4, GetTop(), GetWidth()-4, GetHeight(), (char *)_text.c_str(), ALIGN_CENTER | VALIGN_MIDDLE);

	if (GetFocused())
		DrawSelection(GetLeft(), GetTop(), GetWidth(), GetHeight(), BLACK);
	else if (GetDrawBorder())
		DrawRect(GetLeft(), GetTop(), GetWidth(), GetHeight(), BLACK);
}

int PBLabel::HandleMsg(int type, int par1, int par2)
{
	if (type == EVT_KEYPRESS)
	{
		switch (par1)
		{
			case KEY_LEFT:
			case KEY_UP:
				OnLeave.emit_sig(this, false);
				break;
			case KEY_RIGHT:
			case KEY_DOWN:
				OnLeave.emit_sig(this, true);
				break;
		}
	}
	
	return 0;
}

int PBLabel::GetWidth()
{
	if (_width <= 0)
	{
		int width;
		
		width = StringWidth((char*)GetText().c_str())+8;
		
		if (GetMaxWidth() > 0 && width > GetMaxWidth())
			return GetMaxWidth();
			
		return width;
	}
	
	return PBControl::GetWidth();
}

void PBLabel::SetMaxWidth(int value)
{
	_maxWidth = value;
}

int PBLabel::GetMaxWidth()
{
	return _maxWidth;
}

ibitmap *PBLabel::GetImage()
{
	return _image;
}

void PBLabel::SetImage(ibitmap *value)
{
	_image = value;
}

std::string PBLabel::GetText()
{
	return _text;
}

void PBLabel::SetText(std::string value)
{
	_text = value;
}

