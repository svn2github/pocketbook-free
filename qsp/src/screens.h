#ifndef SCREENS_H
#define SCREENS_H
#include <stdlib.h>
#include "pbcontrols.h"
#include "helper.h"

#include "qsp/qsp.h"

void handleMainMenuItem(int index);


class ObjectsDialog : public PBControl
{
protected:
	PBListBox listBox;
	
	virtual void PlaceControls();
	
	CppSlot1<ObjectsDialog, void, PBControl *> FocusedItemChangedSlot;
	virtual void FocusedItemChangedHandler(PBControl *sender);
public:
	ObjectsDialog(std::string name, PBControl *parent);
	
	virtual int HandleMsg(int type, int par1, int par2);
	void Reload();
	int GetObjectsCount();
	std::string GetCurrentObjectDesc();
};

typedef std::vector<std::pair<std::string, std::string> > links_vector;
typedef std::vector<std::pair<std::string, std::string> >::iterator link_it;

class LocationDescription : public PBControl
{
protected:
	PBListBox listBox;
	
	links_vector _links;
	
	virtual void PlaceControls();
	
public:

	LocationDescription(std::string name, PBControl *parent);
	//virtual ~LocationDescription();
	
	//virtual int HandleMsg(int type, int par1, int par2);
	
	void Reload();
	links_vector GetLinks();
};

class AdditionalDescription : public PBControl
{
protected:
	PBListBox listBox;
	
	links_vector _links;
	
	virtual void PlaceControls();
	
public:

	AdditionalDescription(std::string name, PBControl *parent);

	void Reload();
	links_vector GetLinks();
};

class ActionsDialog : public PBControl
{
protected:
	PBListBox listBox;
	
	virtual void PlaceControls();
	
	CppSlot1<ActionsDialog, void, PBControl *> FocusedItemChangedSlot;
	virtual void FocusedItemChangedHandler(PBControl *sender);
	
public:
	ActionsDialog(std::string name, PBControl *parent);
	
	virtual int HandleMsg(int type, int par1, int par2);
	void Reload();
	void AddLinkItem(std::string text, std::string link);
	
	CppSignal1<void, PBControl *> OnActionExecuted;
};


class ObjectsScreen : public PBControl
{
protected:
	ObjectsDialog objectsDialog;
	AdditionalDescription additionalDescription;
	
	virtual void PlaceControls();
	
public:
	ObjectsScreen(std::string name, PBControl *parent);
	void Reload();
	
	ObjectsDialog *GetObjectsDialog();
};

class GameScreen : public PBControl
{
protected:
	PBButton menuButton;
	PBButton objectsButton;
	LocationDescription locationDescription;
	ActionsDialog actionsDialog;
	ObjectsScreen objectsScreen;
	
	virtual void PlaceControls();
	
	CppSlot1<GameScreen, void, PBControl *> ActionExecutedSlot;
	void ActionExecutedHandler(PBControl * sender);
	
	CppSlot1<GameScreen, void, PBControl *> ButtonPressedSlot;
	void ButtonPressedHandler(PBControl * sender);
	
	CppSlot2<GameScreen, void, PBControl *, bool> DialogLeavedSlot;
	void DialogLeavedHandler(PBControl * sender, bool next);
public:

	GameScreen(std::string name, PBControl *parent);
	
	virtual int HandleMsg(int type, int par1, int par2);
	
	void Reload();
};

class MainScreen : public PBControl
{
protected:
	GameScreen gameScreen;
	virtual void PlaceControls();
	
public:

	MainScreen(std::string name, PBControl *parent);
	virtual int HandleMsg(int type, int par1, int par2);
	
	void UpdateUI();
};



#endif

