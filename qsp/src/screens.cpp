#include "screens.h"
#include "convert.h"

MainScreen mainScreen("mainScreen", 0);
ifont *defaultFont(0);

enum
	{
	MAINMENU_OPEN = 100,
	MAINMENU_FONTSIZE,
	MAINMENU_QUICKSAVE,
	MAINMENU_QUICKLOAD,
	MAINMENU_RESTART,
	MAINMENU_EXIT
	};
	
static imenu fontSizeMenu[] = 
	{
		{ ITEM_ACTIVE, 12, "12", NULL },
		{ ITEM_ACTIVE, 14, "14", NULL },
		{ ITEM_ACTIVE, 16, "16", NULL },
		{ ITEM_ACTIVE, 18, "18", NULL },
		{ 0, 0, NULL, NULL }
	};
	
static imenu mainMenu[] = 
	{
		{ ITEM_HEADER,   0, "QSP", NULL },
		{ ITEM_ACTIVE, MAINMENU_OPEN, "Открыть книгу", NULL },
		{ ITEM_SUBMENU, MAINMENU_FONTSIZE, "Размер шрифта", fontSizeMenu },
		{ ITEM_ACTIVE, MAINMENU_QUICKSAVE, "Быстрое сохранение", NULL },
		{ ITEM_ACTIVE, MAINMENU_QUICKLOAD, "Быстрая загрузка", NULL },
		{ ITEM_ACTIVE, MAINMENU_RESTART, "Начать заново", NULL },
		{ ITEM_SEPARATOR, 0, NULL, NULL },
		{ ITEM_ACTIVE, MAINMENU_EXIT, "Выход", NULL },
		{ 0, 0, NULL, NULL }
	};
	
void dir_selected(char *path)
{
	std::string fileName;
	
	DIR *dir = iv_opendir(path);
    if (dir != 0)
    {
		struct dirent *ep;
		while (ep = iv_readdir(dir))
		{
			std::string ext = GetFileExtension(ep->d_name);
            if (ext == "QSP" || ext == "qsp" || ext == "GAM" || ext == "gam")
			{
				fileName += path;
				fileName += "/";
				fileName += ep->d_name;
				break;
			}
		}
        iv_closedir(dir);
    }
    else
        Message(ICON_ERROR, "Error", "Не удалось открыть выбранный каталог", 2000);

	if (fileName.size() == 0)
		Message(ICON_ERROR, "Error", "Не найден файл qsp или gam", 2000);
	else
	{
		QSPLoadGameWorld(fileName.c_str());
		QSPRestartGame(QSP_TRUE);
	}
		
}

bool GetVarValue(const QSP_CHAR *name, long *num, QSP_CHAR **str)
{
	if (QSPGetVarValuesCount(name, num) && *num)
	{
		QSPGetVarValues(name, 0, num, str);
		return true;
	}
	return false;
}

static char dirbuf[1024];
void handleMainMenuItem(int index)
{
	switch(index)
	{
		case MAINMENU_OPEN:
			OpenDirectorySelector("Выберите каталог", dirbuf, 1024, dir_selected);
			
			// PC hack
			//QSPLoadGameWorld("filimon\\filimon.qsp");
			//QSPRestartGame(QSP_TRUE);
			break;
		case MAINMENU_QUICKSAVE:
			long numVal;
			QSP_CHAR *strVal;
			if (!(GetVarValue(QSP_FMT("NOSAVE"), &numVal, &strVal) && numVal))
			{
				if (!QSPSaveGame((QSP_CHAR*)(GetQuestPath()+="quicksave.sav").c_str(), QSP_FALSE))
					ShowError();
			}
			else
			{
				Message(ICON_INFORMATION, "Cохранение невозможно", "Возможность сохранения отключена", 3000);
			}
			break;
		case MAINMENU_QUICKLOAD:
			if (!QSPOpenSavedGame((QSP_CHAR*)(GetQuestPath()+="quicksave.sav").c_str(), QSP_TRUE))
				ShowError();
			break;
		case MAINMENU_RESTART:
			if (!QSPRestartGame(QSP_TRUE))
				ShowError();
			break;
		case MAINMENU_EXIT:
			CloseApp();
			break;
		default:
			CloseFont(defaultFont);
			defaultFont = OpenFont("times", index, 1);
			mainScreen.SetControlFont(defaultFont);
			mainScreen.UpdateUI();
			break;
	}
}

MainScreen::MainScreen(std::string name, PBControl *parent) : PBControl(name, parent), gameScreen("gameScreen", this)
{
	SetDrawBorder(false);
	
	AddControl(&gameScreen);
	
	gameScreen.SetVisible(false);
	gameScreen.SetLeaveOnKeys(false);
	gameScreen.SetDrawBorder(false);
}

int MainScreen::HandleMsg(int type, int par1, int par2)
{
	if (type == EVT_EXIT)
	{
		CloseApp();
	}
	else if (type == EVT_SHOW)
	{
		_left = 0;
		_top = 0;
		_width = ScreenWidth();
		_height = ScreenHeight();
		SetFocused(true);
		
		gameScreen.SetVisible(true);
		gameScreen.SetFocused(true);
		UpdateUI();
		//DispatchMsgToControls(type, par1, par2);
	}
	else if (type == EVT_KEYPRESS)
	{
		int handled = DispatchMsgToControls(type, par1, par2);
	}
	else
	{
		//DispatchMsgToControls(type, par1, par2);
	}
	
	return 0;
}

void MainScreen::PlaceControls()
{
	gameScreen.SetSize(GetLeft(), GetTop(), GetWidth(), GetHeight());
}

long oldFullRefreshCount = 0;
bool IsFullRefresh()
{
	return oldFullRefreshCount != QSPGetFullRefreshCount();
}

void MainScreen::UpdateUI()
{
	gameScreen.Reload();
	Update();
	oldFullRefreshCount = QSPGetFullRefreshCount();
}

GameScreen::GameScreen(std::string name, PBControl *parent) : PBControl(name, parent),
	menuButton("menuButton", this), objectsButton("objectsButton", this),
	locationDescription("locationDescription", this), actionsDialog("actionsDialog", this),
	objectsScreen("objectsScreen", this),
	ActionExecutedSlot(this, &GameScreen::ActionExecutedHandler),
	ButtonPressedSlot(this,  &GameScreen::ButtonPressedHandler),
	DialogLeavedSlot(this,  &GameScreen::DialogLeavedHandler)
{
	objectsScreen.OnLeave.connect(&DialogLeavedSlot);
	
	AddControl(&actionsDialog);
	AddControl(&menuButton);
	AddControl(&objectsButton);
	AddControl(&locationDescription);
	//AddControl(&objectsDialog);
	//AddControl(&additionalDescription);
	AddControl(&objectsScreen);
	
	//objectsDialog.SetVisible(false);
	//additionalDescription.SetVisible(false);
	objectsScreen.SetVisible(false);
	objectsScreen.SetDrawBorder(false);
	objectsButton.OnPressed.connect(&ButtonPressedSlot);
	menuButton.OnPressed.connect(&ButtonPressedSlot);

	menuButton.SetFocused(true);
	//actionsDialog.OnActionExecuted.connect(&ActionExecutedSlot);
}

void GameScreen::ActionExecutedHandler(PBControl *sender)
{
	Reload();
	Update();
}

void GameScreen::ButtonPressedHandler(PBControl *sender)
{
	if (sender == &menuButton)
	{
		OpenMenu(mainMenu, 0, menuButton.GetLeft(), menuButton.GetTop(), handleMainMenuItem);
	}
	else if (sender == &objectsButton)
	{
		objectsScreen.SetVisible(true);
		objectsScreen.SetFocused(true);
		//menuButton.SetVisible(false);
		//objectsButton.SetVisible(false);
		//locationDescription.SetVisible(false);
		//actionsDialog.SetVisible(false);
		Update();
	}
}

void GameScreen::DialogLeavedHandler(PBControl *sender, bool next)
{
	sender->SetVisible(false);
	actionsDialog.SetFocused(true);
	Update();
}

void GameScreen::PlaceControls()
{
	int buttonsHeight = 25;
	int left = GetLeft()+BORDER_SPACE, top = GetTop()+BORDER_SPACE, width = GetWidth()-BORDER_SPACE*2, height = GetHeight()-BORDER_SPACE*2;
	//int left = GetLeft(), top = GetTop(), width = GetWidth(), height = GetHeight();
	
	menuButton.SetMaxWidth(width/2);
	menuButton.SetSize(left, top, 0, buttonsHeight);
	objectsButton.SetSize(menuButton.GetLeft() + menuButton.GetWidth() + BORDER_SPACE, top, width - menuButton.GetWidth() - BORDER_SPACE, buttonsHeight);
	locationDescription.SetSize(left, top+buttonsHeight+BORDER_SPACE, width, height*2/3-buttonsHeight-BORDER_SPACE);
	actionsDialog.SetSize(left, locationDescription.GetTop() + locationDescription.GetHeight() + BORDER_SPACE, width, height - locationDescription.GetHeight() - BORDER_SPACE);
	//objectsDialog.SetSize(left, top, width, height/2);
	//additionalDescription.SetSize(left, objectsDialog.GetTop() + objectsDialog.GetHeight(), width, height/2);
	objectsScreen.SetSize(left, top, width, height);
}

int GameScreen::HandleMsg(int type, int par1, int par2)
{
	DispatchMsgToControls(type, par1, par2);
	
	return 1;
}

void GameScreen::Reload()
{
	//fprintf(stderr, "\n gamescreen reload. QSPIsMainDescChanged = %d", QSPIsMainDescChanged());
	//fprintf(stderr, "\n gamescreen reload. QSPIsObjectsChanged = %d", QSPIsObjectsChanged());
	//fprintf(stderr, "\n gamescreen reload. QSPIsActionsChanged = %d", QSPIsActionsChanged());
	//fprintf(stderr, "\n gamescreen reload. QSPIsVarsDescChanged = %d", QSPIsVarsDescChanged());
	menuButton.SetText("QSP");
	
	char objButtonCaptionBuf[40];
	sprintf(objButtonCaptionBuf, "Предметы: %d", objectsScreen.GetObjectsDialog()->GetObjectsCount());
	std::string objButtonCaption(objButtonCaptionBuf);
	std::string curObject (objectsScreen.GetObjectsDialog()->GetCurrentObjectDesc());
	if (curObject.size() > 0)
	{
		objButtonCaption += " | ";
		objButtonCaption += curObject;
	}
	objectsButton.SetText(objButtonCaption);
	
	//if (QSPIsObjectsChanged() || QSPIsVarsDescChanged())
	{
		objectsScreen.Reload();
	}
	//if (QSPIsMainDescChanged() || QSPIsActionsChanged())
	{
		locationDescription.Reload();
		
		if (QSPIsActionsChanged() || locationDescription.GetLinks().size() > 0)
		{
			actionsDialog.Reload();
			links_vector links = locationDescription.GetLinks();
			for(link_it it = links.begin(); it != links.end(); it++)
				actionsDialog.AddLinkItem(it->first, it->second);
		}
	}	
	
}

ObjectsScreen::ObjectsScreen(std::string name, PBControl *parent) : PBControl(name, parent),
	objectsDialog("objectsDialog", this), additionalDescription("additionalDescription", this)
{
	AddControl(&objectsDialog);
	AddControl(&additionalDescription);
}

void ObjectsScreen::PlaceControls()
{
	objectsDialog.SetSize(GetLeft(), GetTop(), GetWidth(), GetHeight()/2);
	additionalDescription.SetSize(GetLeft(), objectsDialog.GetTop() + objectsDialog.GetHeight() + BORDER_SPACE, GetWidth(), GetHeight() - objectsDialog.GetHeight() - BORDER_SPACE);
}

void ObjectsScreen::Reload()
{
	if (QSPIsObjectsChanged())	
	{
		objectsDialog.Reload();
	}
	//if (QSPIsVarsDescChanged())
	{
		additionalDescription.Reload();
	}
}

ObjectsDialog *ObjectsScreen::GetObjectsDialog()
{
	return &objectsDialog;
}

LocationDescription::LocationDescription(std::string name, PBControl *parent) : PBControl(name, parent),
	listBox("listBox", this)
{
	AddControl(&listBox);
}

void LocationDescription::PlaceControls()
{
	listBox.SetSize(GetLeft(), GetTop(), GetWidth(), GetHeight());
}

void LocationDescription::Reload()
{
	//fprintf(stderr, "\n LocationDescription reload");
	listBox.Clear();
	
	ParseText(QSPGetMainDesc(), listBox, _links);
	
	// scroll if text was added
	if (QSPIsMainDescChanged() && !IsFullRefresh() && listBox.GetItems().size() > 0)
		listBox.GetPageItems(listBox.GetItems().size()-1, false);
		
}

links_vector LocationDescription::GetLinks()
{
	return _links;
}

AdditionalDescription::AdditionalDescription(std::string name, PBControl *parent) : PBControl(name, parent),
	listBox("listBox", this)
{
	AddControl(&listBox);
	
	listBox.SetCaption("Дополнительное описание");
}

void AdditionalDescription::PlaceControls()
{
	listBox.SetSize(GetLeft(), GetTop(), GetWidth(), GetHeight());
}

void AdditionalDescription::Reload()
{
	//fprintf(stderr, "\n AdditionalDescription reload");
	listBox.Clear();
	
	ParseText(QSPGetVarsDesc(), listBox, _links);
}

links_vector AdditionalDescription::GetLinks()
{
	return _links;
}

ObjectsDialog::ObjectsDialog(std::string name, PBControl *parent) : PBControl(name, parent),
	listBox("listBox", this),
	FocusedItemChangedSlot(this, &ObjectsDialog::FocusedItemChangedHandler)
{
	AddControl(&listBox);
	listBox.SetCaption("Предметы");
	
	listBox.OnFocusedControlChanged.connect(&FocusedItemChangedSlot);
}

void ObjectsDialog::FocusedItemChangedHandler(PBControl *sender)
{
/*
	int index = -1;
	if (listBox.GetFocusedControl() != 0)
	{
		std::string tag (((PBListBoxItem *)listBox.GetFocusedControl())->GetTag());
		if (tag.size() > 0)
		{
			index = atoi(tag.c_str());
		}
		
	}
	
	if (index != QSPGetSelObjectIndex())
		if (!QSPSetSelObjectIndex(index, QSP_TRUE))
			ShowError();
*/
}

void ObjectsDialog::PlaceControls()
{
	listBox.SetSize(GetLeft(), GetTop(), GetWidth(), GetHeight());
}

int ObjectsDialog::HandleMsg(int type, int par1, int par2)
{
	DispatchMsgToControls(type, par1, par2);
	if (type == EVT_KEYPRESS)
	{
		
		switch (par1)
		{
			case KEY_OK:
				int index = -1;
				if (listBox.GetFocusedControl() != 0)
				{
					PBListBoxItem *focusedItem = (PBListBoxItem *)listBox.GetFocusedControl();
					std::string tag (focusedItem->GetTag());
					if (tag.size() > 0)
					{
						index = atoi(tag.c_str());
					}
						
					if (index != QSPGetSelObjectIndex())
					{
						if (!QSPSetSelObjectIndex(index, QSP_TRUE))
							ShowError();
						/*
						else
							for (lbitem_it it = listBox.GetItems().begin(); it != listBox.GetItems().end(); it++)
							{
								if (*it == focusedItem)
								{
									(*it)->SetMarker("<");
								}
								else
								{
									(*it)->SetMarker("");
								}
							}
							*/
					}
					else
						OnLeave.emit_sig(this, false);
				}
				else			
					OnLeave.emit_sig(this, false);
				break;
		}
	}
	return 0;
}

void ObjectsDialog::Reload()
{
	//fprintf(stderr, "\n ObjectsDialog reload");
	listBox.Clear();
	
	long n_objects = QSPGetObjectsCount();
	long sel_index = QSPGetSelObjectIndex();

	//listBox.AddItem("");
	for (long i = 0; i < n_objects; i++)
	{
		char *obj_image, *obj_desc;
		QSPGetObjectData(i, &obj_image, &obj_desc);
		std::string str_desc;
		to_utf8((unsigned char *)obj_desc, &str_desc, koi8_to_unicode);
		char tag[20];
		sprintf(tag, "%d", i);
		PBListBoxItem *newItem = listBox.AddItem(ClearHTMLTags(str_desc), std::string(tag));
		
		if (i == sel_index)
		{
			newItem->SetFocused(true);
			//newItem->SetMarker("<");
		}
	}
}

int ObjectsDialog::GetObjectsCount()
{
	return QSPGetObjectsCount();
}

std::string ObjectsDialog::GetCurrentObjectDesc()
{
	long sel_index = QSPGetSelObjectIndex();
	char *obj_image, *obj_desc;
	std::string str_desc;
	
	if (sel_index >= 0)
	{
		QSPGetObjectData(sel_index, &obj_image, &obj_desc);
		to_utf8((unsigned char *)obj_desc, &str_desc, koi8_to_unicode);
	}
	return str_desc;
}

ActionsDialog::ActionsDialog(std::string name, PBControl *parent) : PBControl(name, parent),
	listBox("listBox", this),
	FocusedItemChangedSlot(this, &ActionsDialog::FocusedItemChangedHandler)
{
	AddControl(&listBox);
	//listBox.SetCaption("Действия");
	listBox.OnFocusedControlChanged.connect(&FocusedItemChangedSlot);
}

void ActionsDialog::FocusedItemChangedHandler(PBControl *sender)
{
	int index = -1;
	if (listBox.GetFocusedControl() != 0)
	{
		std::string tag (((PBListBoxItem *)listBox.GetFocusedControl())->GetTag());
		index = atoi(tag.c_str());
	}
	
	QSPSetSelActionIndex(index, QSP_FALSE);
}

void ActionsDialog::PlaceControls()
{
	listBox.SetSize(GetLeft(), GetTop(), GetWidth(), GetHeight());
}

int ActionsDialog::HandleMsg(int type, int par1, int par2)
{
	DispatchMsgToControls(type, par1, par2);
	if (type == EVT_KEYPRESS)
	{
		
		switch (par1)
		{
			case KEY_OK:
				if (listBox.GetFocusedControl() == 0)
					break;
					
				PBListBoxItem * item = (PBListBoxItem *)listBox.GetFocusedControl();

				if (item->GetTag().size() > 5)
				{
					if (item->GetTag().substr(0, 5) == "EXEC:" || item->GetTag().substr(0, 5) == "exec:")
					{
						if (!QSPExecString((const QSP_CHAR *)item->GetTag().substr(5).c_str(), QSP_TRUE))
							ShowError();
						//else
							//OnActionExecuted.emit_sig(this);
					}
				}
				else
				{
					if (!QSPExecuteSelActionCode(QSP_TRUE))
						ShowError();
					//OnActionExecuted.emit_sig(this);
				}
				break;
		}
	}
	return 0;
}

void ActionsDialog::Reload()
{
	//fprintf(stderr, "\n ActionsDialog reload");
	listBox.Clear();
	
	long n_actions = QSPGetActionsCount();
	long sel_index = QSPGetSelActionIndex();
	for (long i = 0; i < n_actions; i++)
	{
		char *act_image, *act_desc;
		QSPGetActionData(i, &act_image, &act_desc);
		
		std::string str_desc;
		to_utf8((unsigned char *)act_desc, &str_desc, koi8_to_unicode);
		char tag[20];
		sprintf(tag, "%d", i);
		PBListBoxItem *newItem = listBox.AddItem(ClearHTMLTags(str_desc), std::string(tag));
		
		if (i == sel_index)
			newItem->SetFocused(true);
	}
}

void ActionsDialog::AddLinkItem(std::string text, std::string link)
{
	PBListBoxItem *newItem = listBox.AddItem(text);
	newItem->SetTag(link);
}


