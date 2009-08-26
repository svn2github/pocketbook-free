#include "screens.h"
#include "convert.h"

MainScreen mainScreen("mainScreen", 0);
ifont *defaultFont(0);

enum
	{
	MAINMENU_OPEN = 100,
	MAINMENU_FONT,
	MAINMENU_SELECTFONT,
	MAINMENU_ORIENTATION,
	MAINMENU_QUICKSAVE,
	MAINMENU_QUICKLOAD,
	MAINMENU_RESTART,
	MAINMENU_EXIT
	};
	
static imenu fontSizeMenu[] = 
	{
		{ ITEM_ACTIVE, 16, "16", NULL },
		{ ITEM_ACTIVE, 18, "18", NULL },
		{ ITEM_ACTIVE, 20, "20", NULL },
		{ ITEM_ACTIVE, 22, "22", NULL },
		{ ITEM_ACTIVE, MAINMENU_SELECTFONT, "Выбрать шрифт", NULL },
		{ 0, 0, NULL, NULL }
	};
	
static imenu mainMenu[] = 
	{
		{ ITEM_HEADER,   0, "QSP", NULL },
		{ ITEM_ACTIVE, MAINMENU_OPEN, "Открыть книгу", NULL },
		{ ITEM_SUBMENU, MAINMENU_FONT, "Шрифт", fontSizeMenu },
		{ ITEM_ACTIVE, MAINMENU_ORIENTATION, "Ориентация", NULL },
		{ ITEM_ACTIVE, MAINMENU_QUICKSAVE, "Быстрое сохранение", NULL },
		{ ITEM_ACTIVE, MAINMENU_QUICKLOAD, "Быстрая загрузка", NULL },
		{ ITEM_ACTIVE, MAINMENU_RESTART, "Начать заново", NULL },
		{ ITEM_SEPARATOR, 0, NULL, NULL },
		{ ITEM_ACTIVE, MAINMENU_EXIT, "Выход", NULL },
		{ 0, 0, NULL, NULL }
	};

bool IsQuestOpened()
{
	return QSPGetCurLoc() != 0;
}

bool CompareStr(std::string str1, char* str2)
{
	if (str2 == 0)
		return str1.size() == 0;
		
	return str1 == str2;
}
	
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
		//fileName = utf8_to((const unsigned char *)fileName.c_str(), koi8_to_unicode);
		if (!QSPLoadGameWorld(fileName.c_str()))
				ShowError();
		chdir(GetQuestPath().c_str());
		QSPRestartGame(QSP_TRUE);
	}
		
}

void SetDefaultFont(std::string name, int size)
{
	if (defaultFont != 0)
		try
		{
			CloseFont(defaultFont);
		}
		catch(...)
		{
			fprintf(stderr, "\nerror closing default font");
		}
	defaultFont = OpenFont((char*)name.c_str(), size, 1);
	mainScreen.SetControlFont(defaultFont);
	mainScreen.UpdateUI();
}

void font_selected(char *fontr, char *fontb, char *fonti, char *fontbi)
{
	std::string font(fontr);
	
	size_t div_pos = font.find_first_of(',');
	if (div_pos != std::string::npos)
	{
		int size = atoi(font.substr(div_pos+1).c_str());
		SetDefaultFont(font.substr(0, div_pos), size);
	}
}

void orientation_selected(int direction)
{
	SetOrientation(direction);
	mainScreen.UpdateUI();
}

void HandleRestartDialog(int button)
{
	if (button == 1 && IsQuestOpened())
    {
		if (!QSPRestartGame(QSP_TRUE))
				ShowError();
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
void HandleMainMenuItem(int index)
{
std::string fileName;
	switch(index)
	{
		case MAINMENU_OPEN:
			OpenDirectorySelector("Выберите каталог", dirbuf, 1024, dir_selected);
			
			// PC hack
			//QSPLoadGameWorld("filimon\\filimon.qsp");
			//QSPLoadGameWorld("darkcstl\\dark.gam");
			//fileName = "pirates\\pirates_bmp.qsp";
			//fileName = "box\\box.gam";
			//fileName = "zone\\zone.qsp";
			//fileName = "komm\\launch.qsp";
			//fileName = utf8_to((const unsigned char *)fileName.c_str(), koi8_to_unicode);
			/*
			if (!QSPLoadGameWorld(fileName.c_str()))
				ShowError();
			else
			{
				chdir(GetQuestPath().c_str());
				QSPRestartGame(QSP_TRUE);
			}*/
			break;
		case MAINMENU_QUICKSAVE:
			if (!IsQuestOpened())
			{
				Message(ICON_INFORMATION, "QSP", "Перед загрузкой состояния необходимо открыть книгу", 3000);
				break;
			}
			long numVal;
			QSP_CHAR *strVal;
			if (!(GetVarValue(QSP_FMT("NOSAVE"), &numVal, &strVal) && numVal))
			{
				if (!QSPSaveGame((QSP_CHAR*)(/*GetQuestPath()+=*/"quicksave.sav")/*.c_str()*/, QSP_FALSE))
					ShowError();
			}
			else
			{
				Message(ICON_INFORMATION, "QSP", "Возможность сохранения отключена", 3000);
			}
			break;
		case MAINMENU_QUICKLOAD:
			if (!IsQuestOpened())
			{
				Message(ICON_INFORMATION, "QSP", "Нет открытой книги", 3000);
				break;
			}
			if (!QSPOpenSavedGame((QSP_CHAR*)(/*GetQuestPath()+=*/"quicksave.sav")/*.c_str()*/, QSP_TRUE))
				ShowError();
			break;
		case MAINMENU_RESTART:
			if (!IsQuestOpened())
			{
				Message(ICON_INFORMATION, "QSP", "Нет открытой книги", 3000);
				break;
			}
			Dialog(ICON_QUESTION, "QSP", "Вы действительно хотите начать заново?", "Да", "Нет", HandleRestartDialog);
			
			break;
		case MAINMENU_EXIT:
			CloseApp();
			break;
		case MAINMENU_SELECTFONT:
			OpenFontSelector("Выберите шрифт", (char*)std::string(defaultFont->name).c_str(), defaultFont->size, font_selected);
			break;
		case MAINMENU_ORIENTATION:
			OpenRotateBox(orientation_selected);
			break;
		default:
			SetDefaultFont(defaultFont->name, index);
			break;
	}
}


MainScreen::MainScreen(std::string name, PBControl *parent) : PBControl(name, parent),
	gameScreen("gameScreen", this)
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
	_left = 0;
	_top = 0;
	_width = ScreenWidth();
	_height = ScreenHeight();

	gameScreen.SetSize(GetLeft(), GetTop(), GetWidth(), GetHeight());
}

long oldFullRefreshCount = 0;
bool IsFullRefresh()
{
	return oldFullRefreshCount != QSPGetFullRefreshCount();
}

void MainScreen::UpdateUI(bool forceUpdate)
{
	if (IsFullRefresh())
		QSPSaveGame((QSP_CHAR*)(/*GetQuestPath()+=*/"autosave.sav")/*.c_str()*/, QSP_FALSE);
	
	bool updateNeeded = gameScreen.Reload();
	if (forceUpdate || updateNeeded)
		Update();
	oldFullRefreshCount = QSPGetFullRefreshCount();
}

GameScreen *MainScreen::GetGameScreen()
{
	return &gameScreen;
}

GameScreen::GameScreen(std::string name, PBControl *parent) : PBControl(name, parent),
	menuButton("menuButton", this), commandBoxButton("commandBoxButton", this), objectsButton("objectsButton", this),
	versionLabel("versionLabel", this), locationDescription("locationDescription", this), actionsDialog("actionsDialog", this),
	objectsScreen("objectsScreen", this), imageScreen("imageScreen", this),
	ActionExecutedSlot(this, &GameScreen::ActionExecutedHandler),
	ButtonPressedSlot(this,  &GameScreen::ButtonPressedHandler),
	DialogLeavedSlot(this,  &GameScreen::DialogLeavedHandler)
{
	objectsScreen.OnLeave.connect(&DialogLeavedSlot);
	imageScreen.OnLeave.connect(&DialogLeavedSlot);
	
	AddControl(&actionsDialog);
	AddControl(&menuButton);
	AddControl(&commandBoxButton);
	AddControl(&objectsButton);
	AddControl(&locationDescription);
	//AddControl(&objectsDialog);
	//AddControl(&additionalDescription);
	AddControl(&versionLabel);
	AddControl(&objectsScreen);
	AddControl(&imageScreen);
	
	//objectsDialog.SetVisible(false);
	//additionalDescription.SetVisible(false);
	objectsScreen.SetVisible(false);
	objectsScreen.SetDrawBorder(false);
	objectsButton.OnPressed.connect(&ButtonPressedSlot);
	
	imageScreen.SetVisible(false);
	//imageScreen.SetDrawBorder(false);
	
	menuButton.OnPressed.connect(&ButtonPressedSlot);
	menuButton.SetFocused(true);
	
	commandBoxButton.OnPressed.connect(&ButtonPressedSlot);
	//actionsDialog.OnActionExecuted.connect(&ActionExecutedSlot);
		
	versionLabel.SetText(APP_VERSION);
	//versionLabel.SetDrawBorder(false);

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
		OpenMenu(mainMenu, 0, menuButton.GetLeft(), menuButton.GetTop(), HandleMainMenuItem);
	}
	else if (sender == &commandBoxButton)
	{
		ShowCommandBox();
	}
	else if (sender == &objectsButton)
	{
		SwitchObjectsScreen();
	}
}

void GameScreen::DialogLeavedHandler(PBControl *sender, bool next)
{
	if (sender == &objectsScreen)
		SwitchObjectsScreen();
	else if (sender == &imageScreen)
	{
		sender->SetVisible(false);
		if (objectsScreen.GetVisible())
			objectsScreen.SetFocused(true);
		else
			actionsDialog.SetFocused(true);
			
		Update();
	}
}

void GameScreen::PlaceControls()
{
	int buttonsHeight = 38;
	int left = GetLeft()+BORDER_SPACE, top = GetTop()+BORDER_SPACE, width = GetWidth()-BORDER_SPACE*2, height = GetHeight()-BORDER_SPACE*2;
	//int left = GetLeft(), top = GetTop(), width = GetWidth(), height = GetHeight();
	
	menuButton.SetMaxWidth(width/4);
	menuButton.SetSize(left, top, 0, buttonsHeight);
	if (!commandBoxButton.GetVisible())
		objectsButton.SetSize(menuButton.GetLeft() + menuButton.GetWidth() + BORDER_SPACE, top, width - menuButton.GetWidth() - BORDER_SPACE, buttonsHeight);
	else
	{
		commandBoxButton.SetMaxWidth(width/4);
		commandBoxButton.SetSize(menuButton.GetLeft() + menuButton.GetWidth() + BORDER_SPACE, top, 0, buttonsHeight);
		objectsButton.SetSize(menuButton.GetLeft() + menuButton.GetWidth() + commandBoxButton.GetWidth() + BORDER_SPACE*2, top, width - menuButton.GetWidth() - commandBoxButton.GetWidth() - BORDER_SPACE*2, buttonsHeight);
	}
	locationDescription.SetSize(left, top+buttonsHeight+BORDER_SPACE, width, height*5/7-buttonsHeight-BORDER_SPACE);
	actionsDialog.SetSize(left, locationDescription.GetTop() + locationDescription.GetHeight() + BORDER_SPACE, width, height - locationDescription.GetHeight() - BORDER_SPACE);
	objectsScreen.SetSize(left, top, width, height);
	imageScreen.SetSize(left, top, width, height);
	
	versionLabel.SetSize(objectsButton.GetLeft()+objectsButton.GetWidth()-140-BORDER_SPACE, objectsButton.GetTop()+BORDER_SPACE, 140, objectsButton.GetHeight()-BORDER_SPACE*2);
	versionLabel.SetVisible(!IsQuestOpened());
}

void GameScreen::SwitchObjectsScreen()
{
	if (objectsScreen.GetVisible())
	{
		objectsScreen.SetVisible(false);
		actionsDialog.SetFocused(true);
		Update();
	}
	else
	{
		objectsScreen.SetVisible(true);
		objectsScreen.SetFocused(true);
		Update();
	}
}


static char commandBuf[COMMAND_BUF_SIZE+1] = "";
static std::string lastCommand;
void HandleCommandBox(char *s)
{
	std::string text = utf8_to((const unsigned char *)s, koi8_to_unicode);
	
	QSPSetInputStrText(text.c_str());
	
	if (!QSPExecUserInput(QSP_TRUE))
		ShowError();
	
	if (s != 0)
		lastCommand.assign(s);
	else
		lastCommand.clear();
}

void GameScreen::ShowCommandBox()
{
	SetStringToCharString(commandBuf, lastCommand, COMMAND_BUF_SIZE);

	OpenKeyboard("Введите команду", commandBuf, COMMAND_BUF_SIZE/2, 0, HandleCommandBox);
}

int GameScreen::HandleMsg(int type, int par1, int par2)
{

	int handled = DispatchMsgToControls(type, par1, par2);
	
	if (!handled && type == EVT_KEYPRESS)
	{
		switch(par1)
		{
			case KEY_MENU:
				OpenMenu(mainMenu, 0, menuButton.GetLeft(), menuButton.GetTop(), HandleMainMenuItem);
				break;
			case KEY_BACK:
				ShowCommandBox();
				break;
			case KEY_DELETE:
				// show|hide objectsScreen
				SwitchObjectsScreen();
				break;
		}
	}
	
	return handled;
}

void GameScreen::Update()
{
	PBControl::Update();
	if (imageScreen.GetVisible())
		FineUpdate();
}

bool GameScreen::Reload()
{
	//fprintf(stderr, "\n gamescreen reload. QSPIsMainDescChanged = %d", QSPIsMainDescChanged());
	//fprintf(stderr, "\n gamescreen reload. QSPIsObjectsChanged = %d", QSPIsObjectsChanged());
	//fprintf(stderr, "\n gamescreen reload. QSPIsActionsChanged = %d", QSPIsActionsChanged());
	//fprintf(stderr, "\n gamescreen reload. QSPIsVarsDescChanged = %d", QSPIsVarsDescChanged());
	bool updateNeeded = false;
	menuButton.SetText("QSP");
	
	commandBoxButton.SetText(" K ");
	
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
	
	updateNeeded = locationDescription.Reload();
	if (updateNeeded)
	{
		actionsDialog.Reload(true);
		links_vector links = locationDescription.GetLinks();
		for(link_it it = links.begin(); it != links.end(); it++)
			actionsDialog.AddLinkItem(it->first, it->second);
	}
	else
		updateNeeded = actionsDialog.Reload() || updateNeeded;
		
	updateNeeded = objectsScreen.Reload() || updateNeeded;
	
	return updateNeeded;
}

std::string GameScreen::GetLastCommand()
{
	return lastCommand;
}

void GameScreen::SetLastCommand(std::string value)
{
	lastCommand = value;
}

void GameScreen::ShowWindow(int window, bool show)
{
	switch (window)
	{
		case QSP_WIN_INPUT:
			//commandBoxButton.SetVisible(show);
		break;
	}
}

void GameScreen::ShowImage(ibitmap *image)
{
	imageScreen.SetImage(image);
	imageScreen.SetVisible(true);
	imageScreen.SetFocused(true);
	Update();
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

bool ObjectsScreen::Reload()
{
	bool updateNeeded = false;
	updateNeeded = objectsDialog.Reload() || updateNeeded;
	updateNeeded = additionalDescription.Reload() || updateNeeded;
	return updateNeeded;
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

bool LocationDescription::Reload()
{
	if (QSPGetMainDesc() == 0)
	{
		listBox.Clear();
		bool updateNeeded = _rawValue.size() > 0;
		_rawValue.clear();
		return updateNeeded;
	}
	
	if (QSPIsMainDescChanged() || _rawValue != QSPGetMainDesc())
	{
		listBox.Clear();
		_rawValue = QSPGetMainDesc();
		ParseText(QSPGetMainDesc(), listBox, _links);

		// scroll if text was added
		if (QSPIsMainDescChanged() && !IsFullRefresh() && listBox.GetItems().size() > 0)
			listBox.GetPageItems(listBox.GetItems().size()-1, false);

		return true;
	}
	return false;
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

bool AdditionalDescription::Reload()
{
	//fprintf(stderr, "\n AdditionalDescription reload");
	if (QSPGetVarsDesc() == 0)
	{
		listBox.Clear();
		bool updateNeeded = _rawValue.size() > 0;
		_rawValue.clear();
		return updateNeeded;
	}
	
	if (QSPIsVarsDescChanged() || _rawValue != QSPGetVarsDesc())
	{
		listBox.Clear();
		_rawValue = QSPGetVarsDesc();
		ParseText(QSPGetVarsDesc(), listBox, _links);
		//fprintf(stderr, "\n AdditionalDescription reloaded");
		return true;
	}
	
	return false;
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
									//(*it)->SetMarker("");
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

bool ObjectsDialog::Reload()
{
	//fprintf(stderr, "\n ObjectsDialog reload");
	bool updateNeeded = false;
	
	long n_objects = QSPGetObjectsCount();
	long sel_index = QSPGetSelObjectIndex();
	char *obj_image, *obj_desc;

	if (QSPIsObjectsChanged())
		updateNeeded = true;
	else
	{
		if (_rawValues.size() != n_objects)
			updateNeeded = true;
		else
			for (long i = 0; i < _rawValues.size(); i++)
			{
				QSPGetObjectData(i, &obj_image, &obj_desc);
				if (!CompareStr(_rawValues[i], obj_desc))
				{
					updateNeeded = true;
					break;
				}
			}
	}
	
	if (updateNeeded)
	{
		listBox.Clear();
		_rawValues.clear();
		for (long i = 0; i < n_objects; i++)
		{
			QSPGetObjectData(i, &obj_image, &obj_desc);
			_rawValues.push_back(std::string(obj_desc));
			
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
		//fprintf(stderr, "\n ObjectsDialog reloaded");
	}
	
	return updateNeeded;
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
					if (item->GetTag().substr(5, 5) == "EXEC:" || item->GetTag().substr(5, 5) == "exec:")
					{
						if (!QSPExecString((const QSP_CHAR *)item->GetTag().substr(5+5).c_str(), QSP_TRUE))
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

bool ActionsDialog::Reload(bool force)
{
	//fprintf(stderr, "\n ActionsDialog reload");
	bool updateNeeded = false;
	
	long n_actions = QSPGetActionsCount();
	long sel_index = QSPGetSelActionIndex();
	char *act_image, *act_desc;
	
	if (QSPIsActionsChanged())
		updateNeeded = true;
	else
	{
		if (_rawValues.size() != n_actions)
			updateNeeded = true;
		else
			for (long i = 0; i < _rawValues.size(); i++)
			{
				QSPGetActionData(i, &act_image, &act_desc);
				if (!CompareStr(_rawValues[i], act_desc))
				{
					updateNeeded = true;
					break;
				}
			}
	}
	
	if (force || updateNeeded)
	{
		listBox.Clear();
		_rawValues.clear();
		for (long i = 0; i < n_actions; i++)
		{
			QSPGetActionData(i, &act_image, &act_desc);
			_rawValues.push_back(std::string(act_desc));
			
			std::string str_desc;
			to_utf8((unsigned char *)act_desc, &str_desc, koi8_to_unicode);
			char tag[20];
			sprintf(tag, "%d", i);
			PBListBoxItem *newItem = listBox.AddItem(ClearHTMLTags(str_desc), std::string(tag));
			
			if (i == sel_index)
				newItem->SetFocused(true);
		}
		//fprintf(stderr, "\n ActionsDialog reloaded");
	}
	return updateNeeded;
}

void ActionsDialog::AddLinkItem(std::string text, std::string link)
{
	PBListBoxItem *newItem = listBox.AddItem(text);
	std::string link_tag ("link:"+link);
	newItem->SetTag(link_tag);
}

ImageScreen::ImageScreen(std::string name, PBControl *parent): PBControl(name, parent)
{
	_image = 0;
}

void ImageScreen::Draw()
{
	ClearRegion();
	if (GetDrawBorder())
		DrawRect(GetLeft(), GetTop(), GetWidth(), GetHeight(), BLACK);
	if (GetImage() != 0)
		DrawBitmapRect(GetLeft()+3, GetTop()+3, GetWidth()-3, GetHeight()-3, GetImage(), ALIGN_CENTER | VALIGN_MIDDLE);
	if (GetDrawBorder() && GetFocused())
		DrawRect(GetLeft() + BORDER_SPACE/2, GetTop() + BORDER_SPACE/2, GetWidth() - BORDER_SPACE, GetHeight() - BORDER_SPACE, BLACK);
}

int ImageScreen::HandleMsg(int type, int par1, int par2)
{
	if (type == EVT_KEYPRESS)
	{
		switch (par1)
		{
			case KEY_LEFT:
			case KEY_UP:
				OnLeave.emit_sig(this, false);
				break;
			case KEY_OK:
			case KEY_RIGHT:
			case KEY_DOWN:
				OnLeave.emit_sig(this, true);
				break;
		}
	}
	
	return 1;
}

ibitmap *ImageScreen::GetImage()
{
	return _image;
}

void ImageScreen::SetImage(ibitmap *value)
{
	_image = value;
}

