#pragma once

class BaseCard
{
public:
	bool m_bKnowing;
	int m_iSidesCount;
	virtual void Parse(string Str) = 0;
	virtual void Show(int iSide, irect* irRect) = 0;
	virtual void Sound(){}
	void Dublicate();
	virtual ~BaseCard(){}
};
class QandACard: public BaseCard
{
	string m_sSide[2];
public:
	virtual void Parse(string Str);
	virtual void Show(int iSide, irect* irRect);
	virtual ~QandACard(){}
};
class MCLCard: public BaseCard
{
	string m_sSide[3];
	string m_sType[3];
	string m_sSound;
public:
	virtual void Parse(string Str);
	virtual void Show(int iSide, irect* irRect);
	virtual void Sound();
	virtual ~MCLCard(){}
};

class BaseImpliment;
class CARDLIST
{
	friend class BaseCard;
	friend class BaseImpliment;
	friend class SmplImpliment;
	friend class MemoImpliment;
	friend class TestImpliment;
	friend void UpdateData();
	friend void MCLCard::Parse(string Str);
	friend void CALENDAR::AddCurrCL();
	BaseImpliment* m_MainImpliment;
	std::string m_sFullFileName;
	std::string m_sCardListName;
	std::vector<BaseCard*> m_vList;
	std::vector<BaseCard*> m_vData;
	std::vector<BaseCard*>::iterator m_CurCard;
	bool m_CurSide;
	int m_CurSell;
	int m_CurShowdSide;

public:
	void SetFile(string sName);
	void Init();
	void Repaint();
	void SetSmplImpl();
	void SetMemoImpl();
	void SetTestImpl();
	void ChangeSides();
	void Randomise();
	void Reset();
	void MoveNext();
	void MovePrev();
	void MoveFirst();
	void MoveLast();
	void Expand();
	void Sound();
	void SetSell(int sel);
	bool IsSell();
	bool SellNext();
	bool SellPrev();
	bool SellOK();
};

class BaseImpliment
{
	friend class CARDLIST;
	virtual void ShowCurSide() = 0;
	virtual void ShowOthSide() = 0;
	virtual bool ShowMidSide() = 0;
	virtual void UpdateHeader() = 0;
	virtual void SellOK(){}
protected:
	virtual void EoF();
	virtual void BoF();
};
class SmplImpliment: public BaseImpliment
{
	friend class CARDLIST;
	int m_iPercent;
	int m_bNext;
	virtual void ShowCurSide();
	virtual void ShowOthSide();
	virtual bool ShowMidSide();
	virtual void UpdateHeader();
	virtual void EoF();
	virtual void BoF();
};
class MemoImpliment: public BaseImpliment
{
	virtual void ShowCurSide();
	virtual void ShowOthSide();
	virtual bool ShowMidSide();
	virtual void UpdateHeader();
	virtual void SellOK();
};
class TestImpliment: public BaseImpliment
{
	friend class CARDLIST;
	int m_iCorect;
	int m_iCurCorSell;
	virtual void ShowCurSide();
	virtual void ShowOthSide();
	virtual bool ShowMidSide();
	virtual void UpdateHeader();
	virtual void SellOK();
	virtual void EoF();
};
