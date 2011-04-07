class	BaseCard
{
/* */
public:
	bool			m_bKnowing;
	int				m_iSidesCount;
	virtual void	Parse(std::string Str) = 0;
	virtual void	Show(int iSide, irect *irRect) = 0;
	void			Dublicate();
	virtual void Sound()
	{
	}

	virtual~BaseCard()
	{
	}
};
class QandACard :
	public BaseCard
{
	std::string m_sSide[2];

/* */
public:
	virtual void	Parse(std::string Str);
	virtual void	Show(int iSide, irect *irRect);
	virtual~QandACard()
	{
	}
};
class MCLCard :
	public BaseCard
{
	std::string m_sSide[3];
	std::string m_sType[3];
	std::string m_sSound;

/* */
public:
	virtual void	Parse(std::string Str);
	virtual void	Show(int iSide, irect *irRect);
	virtual void	Sound();
	virtual~MCLCard()
	{
	}
};

class	BaseImpliment;
class	CARDLIST
{
	friend class						BaseCard;
	friend class						BaseImpliment;
	friend class						SmplImpliment;
	friend class						MemoImpliment;
	friend class						TestImpliment;
	friend class						MCLCard;
	friend void							UpdateData();
	friend void CALENDAR::				AddCurrCL(time_t ts);
	friend void							KA_mem_11(int par1, int par2);
	BaseImpliment						*m_MainImpliment;
	std::string							m_sFullFileName;
	std::string							m_sCardListName;
	std::vector<BaseCard *>				m_vList;
	std::vector<BaseCard *>				m_vData;
	std::vector<BaseCard *>::iterator	m_CurCard;
	int m_CurSide[3];
	int m_CurSell;
	int m_CurShowdSide;

/* */
public:
	bool m_Need16;
	void SetFile(std::string sName);
	void Init();
	void Repaint();
	void SetSmplImpl();
	void SetMemoImpl();
	void SetTestImpl();
	void ChangeSides();
	void ChangeSides2();
	void Randomise();
	void Reset();
	void MoveNext();
	void MovePrev();
	void MoveFirst();
	void MoveLast();
	void Expand();
	void Sound();
	int GetPercent();
	bool SetSell(int par1, int par2);
	bool IsSell();
	bool SellNext();
	bool SellPrev();
	bool SellNext2();
	bool SellOK();
};

class BaseImpliment
{
	friend class CARDLIST;
	virtual void ShowCurSide() = 0;
	virtual void ShowOthSide() = 0;
	virtual bool ShowMidSide() = 0;
	virtual void UpdateHeader() = 0;
	virtual void SellOK()
	{
	}

/* */
protected:
	virtual void	EoF();
	virtual void	BoF();
};
class SmplImpliment :
	public BaseImpliment
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
class MemoImpliment :
	public BaseImpliment
{
	virtual void	ShowCurSide();
	virtual void	ShowOthSide();
	virtual bool	ShowMidSide();
	virtual void	UpdateHeader();
	virtual void	SellOK();
};
class TestImpliment :
	public BaseImpliment
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
