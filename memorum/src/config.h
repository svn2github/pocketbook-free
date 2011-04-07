enum RectID
{
	FullScreen,
	SelButt1,
	SelButt2,
	SelButt3,
	SelButt4,
	Footer,
	Card2Sd1,
	Card2Sd2,
	Header,
	Card3Sd1,
	Card3Sd3,
	Card3Sd2,
	CardTst0,
	CardTst11,
	CardTst12,
	CardTst21,
	CardTst22,
	MaxId
};

typedef void (*KeyHandlerFunc) (int par1, int par2);

class	InputConfig
{
/* */
protected:
	int				evt_type;
	KeyHandlerFunc	func;

/* */
public:
	virtual bool	RunIf(int evt, int par1, int par2) = 0;
};

class KeyConfig :
	public InputConfig
{
	int				key;
	KeyHandlerFunc	func2;

/* */
public:
	KeyConfig(int i);
	virtual bool	RunIf(int evt, int par1, int par2);
};

class PointerTapConfig :
	public InputConfig
{
	irect	rect;
	bool	move;

/* */
public:
	PointerTapConfig(int i, int evt, irect rect);
	virtual bool	RunIf(int evt, int par1, int par2);
};

class PointerGestConfig :
	public InputConfig
{
	int		x, y, w, h;
	bool	fl;

/* */
public:
	PointerGestConfig(int i, int w, int h, bool flip);
	virtual bool	RunIf(int evt, int par1, int par2);
};

class	CONFIG
{
	iconfig					*m_Config;
	iconfigedit				*m_ConfigEditor;
	iv_confighandler		m_ConfigHandler;
	iv_itemchangehandler	m_ItemChangeHandler;
	friend void				UpdateData();
	friend class			KeyConfig;
	friend class			PointerGestConfig;
	friend class			PointerTapConfig;

/* */
public:
	irect						m_Rects[MaxId];

	bool						Autorand;
	ifont						*Cardfont;
	ifont						*Boldfont;
	ifont						*Headerfont;
	ifont						*Footerfont;
	ifont						*Delftfontb;
	ifont						*Delftfont;
	int							Pagemargins;
	bool						Proportional;
	bool						Grayscale16;
	int							Percentstart;
	int							Percentsteap;
	std::vector<int>			Graphic;
	std::vector<InputConfig *>	MainHandlerDescription;
	KeyHandlerFunc				*MainHandlerDefenition;

	void						Init();
	void						Show();
	void						RecalcRects();
};
