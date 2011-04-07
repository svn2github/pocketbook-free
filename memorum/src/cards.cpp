#include "main.h"

/* */
void BaseCard::Dublicate()
{
	int i = CardList.m_CurCard - CardList.m_vList.begin();
	CardList.m_vList.push_back(this);
	CardList.m_CurCard = CardList.m_vList.begin() + i;
}

/* */
void QandACard::Parse(std::string Str)
{
	m_bKnowing = false;
	m_iSidesCount = 2;

	int find = Str.find("a: ");
	m_sSide[0] = Str.substr(3, find - 3);
	m_sSide[1] = Str.substr(find + 3);
}

/* */
void QandACard::Show(int iSide, irect *irRect)
{
	SetFont(Config.Cardfont, BLACK);
	DrawTextRect2(irRect, sz(m_sSide[iSide]));
}

/* */
void MCLCard::Parse(std::string Str)
{
	m_bKnowing = false;

	unsigned int	iStartSide, iEndSide, iStartContent, iEndContent, iCurSide = 0;
	char			buf[255];
	sscanf(Str.c_str(), "<card sides=\"%d\"", &m_iSidesCount);
	if (m_iSidesCount == 3) SetSState(ITEM_ACTIVE);
	iStartSide = Str.find("<side");
	while (iStartSide != std::string::npos)
	{
		iEndSide = Str.find("</side>", iStartSide);

		std::string side(Str.substr(iStartSide, iEndSide - iStartSide));
		sscanf(side.c_str(), "<side type=\"%s", buf);
		buf[3] = 0;
		m_sType[iCurSide] = buf;
		iStartContent = side.find('>') + 1;
		iEndContent = side.find("</side>", iStartContent);
		if (m_sType[iCurSide] == "txt")
			m_sSide[iCurSide] = side.substr(iStartContent, iEndContent - iStartContent);
		else
		{
			std::string path = CardList.m_sFullFileName;
			m_sSide[iCurSide] = path.substr(0, path.rfind('/') + 1) + side.substr
				(
					iStartContent,
					iEndContent - iStartContent
				);
		}

		iStartSide = Str.find("<side", iEndSide);
		iCurSide++;
	}

	iStartContent = Str.find("<sound>");
	if (iStartContent != std::string::npos)
	{
		iStartContent += 7;
		iEndContent = Str.find("</sound>", iStartContent);

		std::string path = CardList.m_sFullFileName;
		m_sSound = path.substr(0, path.rfind('/') + 1) + Str.substr(iStartContent, iEndContent - iStartContent);
	}
	else
		m_sSound = "";
}

/* */
void MCLCard::Show(int iSide, irect *irRect)
{
	if (iSide == 2 && m_iSidesCount == 2) iSide = CardList.m_CurSide[2];
	SetFont(Config.Cardfont, BLACK);
	if (m_sType[iSide] == "txt")
	{
		if (TextRectHeight(irRect->w, sz(m_sSide[iSide]), irRect->flags) > irRect->h)
		{
			for
			(
				int i = (int)
					(
						irRect->h /
							(
								TextRectHeight(irRect->w, sz(m_sSide[iSide]), irRect->flags) /
									(Config.Cardfont->size + Config.Cardfont->size / 6.0)
							)
					);
				i < 50;
				i++
			)
			{
				SetFont(OpenFont(Config.Cardfont->name, i, 0), BLACK);
				if (TextRectHeight(irRect->w, sz(m_sSide[iSide]), irRect->flags) > irRect->h)
				{
					SetFont(OpenFont(Config.Cardfont->name, i - 1, 0), BLACK);
					break;
				}
			}
		}

		DrawTextRect2(irRect, sz(m_sSide[iSide]));
	}
	else
	{
		ibitmap *img = LoadJPEG(sz(m_sSide[iSide]), irRect->w - 2, irRect->h - 2, 128, 128, Config.Proportional);
		if (!img) img = LoadBitmap(sz(m_sSide[iSide]));
		if (img)
		{
			DrawBitmapRect2(irRect, img);
			CardList.m_Need16 = Config.Grayscale16;
		}
		else
			DrawTextRect2(irRect, sz(m_sSide[iSide]));
	}
}

/* */
void MCLCard::Sound()
{
	if (!m_sSound.empty()) PlayFile(sz(m_sSound));
}

/* */
void CARDLIST::SetFile(std::string sName)
{
	m_sFullFileName = sName;
}

/* */
void CARDLIST::Init()
{
	for (unsigned int i = 0; i < m_vData.size(); i++) delete m_vData[i];
	m_vData.clear();
	m_vList.clear();

	FILE	*f = iv_fopen(sz(m_sFullFileName), "r");
	std::string sFileContent("");

	char			buf[256];
	unsigned int	i, iStart, iEnd;
	while ((bool) (i = iv_fread(buf, 1, 255, f)) == true)
	{
		buf[i] = 0;
		sFileContent += buf;
	}

	SetSState(ITEM_INACTIVE);

	if (m_sFullFileName.find(".q&a") != std::string::npos)
	{
		int t = m_sFullFileName.rfind('/') + 1;
		m_sCardListName = m_sFullFileName.substr(t, m_sFullFileName.rfind('.') - t);
		iStart = sFileContent.find("q: ");
		while (iStart != std::string::npos)
		{
			iEnd = sFileContent.find("q: ", iStart + 1);

			BaseCard	*temp = new QandACard;
			temp->Parse(sFileContent.substr(iStart, iEnd - iStart));
			m_vData.push_back(temp);
			m_vList.push_back(temp);
			iStart = iEnd;
		}
	}
	else if (m_sFullFileName.find(".mcl") != std::string::npos)
	{
		unsigned int	t;
		while ((t = sFileContent.find("\n")) != std::string::npos) sFileContent.replace(t, 1, " ");
		while ((t = sFileContent.find("\t")) != std::string::npos) sFileContent.replace(t, 1, " ");
		while ((t = sFileContent.find("  ")) != std::string::npos) sFileContent.replace(t, 1, "");
		while ((t = sFileContent.find("<br/>")) != std::string::npos) sFileContent.replace(t, 5, "\n");
		while ((t = sFileContent.find("<br />")) != std::string::npos) sFileContent.replace(t, 6, "\n");
		t = sFileContent.find("cardlist name=\"") + 15;
		m_sCardListName = sFileContent.substr(t, sFileContent.find('\"', t) - t);
		iStart = sFileContent.find("<card ");
		while (iStart != std::string::npos)
		{
			iEnd = sFileContent.find("<card ", iStart + 1);

			BaseCard	*temp = new MCLCard;
			temp->Parse(sFileContent.substr(iStart, iEnd - iStart));
			m_vData.push_back(temp);
			m_vList.push_back(temp);
			iStart = iEnd;
		}
	}

	iv_fclose(f);
	m_CurCard = m_vList.begin();
	if (Config.Autorand) random_shuffle(m_vList.begin(), m_vList.end());
	m_CurSide[0] = 0;
	m_CurSide[1] = 1;
	m_CurSide[2] = 2;
	m_CurSell = -1;
	SetSmplImpl();
}

/* */
void CARDLIST::Repaint()
{
	m_MainImpliment->ShowCurSide();
}

/* */
void CARDLIST::SetSmplImpl()
{
	if (m_MainImpliment) delete m_MainImpliment;

	SmplImpliment	*temp = new SmplImpliment;
	temp->m_iPercent = Config.Percentstart;
	temp->m_bNext = 1;
	m_MainImpliment = temp;
	m_CurSell = -1;
}

/* */
void CARDLIST::SetTestImpl()
{
	if (m_vData.size() < 4) Message(ICON_ERROR, "Memorum", T(@Less4cards), 2000);
	m_vList = m_vData;
	m_CurCard = m_vList.begin();
	if (m_MainImpliment) delete m_MainImpliment;

	TestImpliment	*temp = new TestImpliment;
	temp->m_iCorect = 0;
	m_MainImpliment = temp;
	CardList.m_CurSell = 2;
}

/* */
void CARDLIST::SetMemoImpl()
{
	if (m_MainImpliment) delete m_MainImpliment;
	m_MainImpliment = new MemoImpliment;
	m_CurSell = -1;
}

/* */
void CARDLIST::ChangeSides()
{
	int t = m_CurSide[0];
	m_CurSide[0] = m_CurSide[1];
	m_CurSide[1] = t;
}

/* */
void CARDLIST::ChangeSides2()
{
	int t = m_CurSide[0];
	m_CurSide[0] = m_CurSide[2];
	m_CurSide[2] = t;
}

/* */
void CARDLIST::Randomise()
{
	random_shuffle(m_vList.begin(), m_vList.end());
	MoveFirst();
}

/* */
void CARDLIST::Reset()
{
	m_vList = m_vData;
	m_CurCard = m_vList.begin();
	if (Config.Autorand) random_shuffle(m_vList.begin(), m_vList.end());
	MoveFirst();
}

/* */
void CARDLIST::MoveNext()
{
	if (m_CurCard != m_vList.end() - 1)
		m_CurCard++;
	else
		m_MainImpliment->EoF();

	SmplImpliment	*temp = dynamic_cast<SmplImpliment *>(m_MainImpliment);
	if (temp && temp->m_bNext == 1) temp->m_bNext = 0;
	Repaint();
}

/* */
void CARDLIST::MovePrev()
{
	if (m_CurCard != m_vList.begin())
		m_CurCard--;
	else
		m_MainImpliment->BoF();
	Repaint();
}

/* */
void CARDLIST::MoveFirst()
{
	m_CurCard = m_vList.begin();
}

/* */
void CARDLIST::MoveLast()
{
	m_CurCard = m_vList.end() - 1;
}

/* */
void CARDLIST::Expand()
{
	if (!m_MainImpliment->ShowMidSide()) m_MainImpliment->ShowOthSide();
}

/* */
void CARDLIST::Sound()
{
	SetVolume(100);
	CurrentCard->Sound();
}

/* */
bool CARDLIST::SetSell(int par1, int par2)
{
	bool	tst = dynamic_cast<TestImpliment *>(m_MainImpliment);
	if (tst && par2 > Config.m_Rects[CardTst11].y)
	{
		DrawSelection(RCTVAL(CardTst11 + m_CurSell), WHITE);
		DrawRect(RCTVAL(CardTst11 + m_CurSell), BLACK);
		m_CurSell = 2 *
			(
				par2 -
				Config.m_Rects[CardTst11].y > (Config.m_Rects[FullScreen].h - Config.m_Rects[CardTst11].y) /
				2
			) +
					(par1 > Config.m_Rects[FullScreen].w / 2);
		DrawSelection(RCTVAL(CardTst11 + m_CurSell), BLACK);
		PartialUpdateBW(RCTVAL(FullScreen));
		SellOK();
		return true;
	}
	else if (par2 > Config.m_Rects[Footer].y)
	{
		DrawSelection(RCTVAL(SelButt1 + m_CurSell), WHITE);
		m_CurSell = par1 / (Config.m_Rects[FullScreen].w / 4);
		DrawSelection(RCTVAL(SelButt1 + m_CurSell), BLACK);
		PartialUpdateBW(RCTVAL(Footer));
		SellOK();
		return true;
	}

	return false;
}

/* */
bool CARDLIST::IsSell()
{
	return m_CurSell != -1;
}

/* */
bool CARDLIST::SellNext()
{
	bool	tst = dynamic_cast<TestImpliment *>(m_MainImpliment);
	if (m_CurSell == -1) return false;
	if (tst)
	{
		DrawSelection(RCTVAL(CardTst11 + m_CurSell), WHITE);
		DrawRect(RCTVAL(CardTst11 + m_CurSell), BLACK);
	}
	else
		DrawSelection(RCTVAL(SelButt1 + m_CurSell), WHITE);
	m_CurSell = (m_CurSell + 1) % 4;
	if (tst)
		DrawSelection(RCTVAL(CardTst11 + m_CurSell), BLACK);
	else
		DrawSelection(RCTVAL(SelButt1 + m_CurSell), BLACK);
	PartialUpdateBW(RCTVAL(FullScreen));
	return true;
}

/* */
bool CARDLIST::SellPrev()
{
	bool	tst = dynamic_cast<TestImpliment *>(m_MainImpliment);
	if (m_CurSell == -1) return false;
	if (tst)
	{
		DrawSelection(RCTVAL(CardTst11 + m_CurSell), WHITE);
		DrawRect(RCTVAL(CardTst11 + m_CurSell), BLACK);
	}
	else
		DrawSelection(RCTVAL(SelButt1 + m_CurSell), WHITE);
	m_CurSell = (m_CurSell + 3) % 4;
	if (tst)
		DrawSelection(RCTVAL(CardTst11 + m_CurSell), BLACK);
	else
		DrawSelection(RCTVAL(SelButt1 + m_CurSell), BLACK);
	PartialUpdateBW(RCTVAL(FullScreen));
	return true;
}

/* */
bool CARDLIST::SellNext2()
{
	bool	tst = dynamic_cast<TestImpliment *>(m_MainImpliment);
	if (!tst) return false;
	DrawSelection(RCTVAL(CardTst11 + m_CurSell), WHITE);
	DrawRect(RCTVAL(CardTst11 + m_CurSell), BLACK);
	m_CurSell = (m_CurSell + 2) % 4;
	DrawSelection(RCTVAL(CardTst11 + m_CurSell), BLACK);
	PartialUpdateBW(RCTVAL(FullScreen));
	return true;
}

/* */
bool CARDLIST::SellOK()
{
	if (m_CurSell == -1) return false;
	m_MainImpliment->SellOK();
	MoveNext();
	return true;
}

/* */
int CARDLIST::GetPercent()
{
	if (dynamic_cast<SmplImpliment *>(m_MainImpliment))
		return dynamic_cast<SmplImpliment *>(m_MainImpliment)->m_iPercent;
	else
		return 0;
}

/* */
void BaseImpliment::EoF()
{
	CardList.MoveFirst();
}

/* */
void BaseImpliment::BoF()
{
	CardList.MoveLast();
}

/* */
void SmplImpliment::ShowCurSide()
{
	if (!m_bNext)
	{
		BaseCard	*Prev;
		if (CardList.m_CurCard == CardList.m_vList.begin())
			Prev = (*(CardList.m_vList.end() - 1));
		else
			Prev = (*(CardList.m_CurCard - 1));
		Prev->m_bKnowing = true;
		if (!(rand() % (100 / m_iPercent))) Prev->Dublicate();
	}

	m_bNext = 1;
	FillArea(RCTVAL(FullScreen), LGRAY);
	for (int i = 0; i < CurrentCard->m_iSidesCount; i++)
	{
		int id = CurrentCard->m_iSidesCount * 3 + i;
		FillArea(RCTVAL(id), WHITE);
		DrawRect(RCTVAL(id), BLACK);
	}

	CurrentCard->Show(CardList.m_CurSide[0], &Config.m_Rects[CurrentCard->m_iSidesCount * 3]);
	DrawRect(RCTVAL(CurrentCard->m_iSidesCount * 3), BLACK);
	UpdateHeader();
	CardList.m_CurShowdSide = 0;
	FullUpdate();
	if (CardList.m_Need16 && FineUpdateSupported()) FineUpdate();
	CardList.m_Need16 = false;
}

/* */
void SmplImpliment::ShowOthSide()
{
	CurrentCard->Show(CardList.m_CurSide[1], &Config.m_Rects[CurrentCard->m_iSidesCount * 3 + 1]);
	DrawRect(RCTVAL(CurrentCard->m_iSidesCount * 3 + 1), BLACK);
	PartialUpdate(RCTVAL(CurrentCard->m_iSidesCount * 3 + 1));
	if (CardList.m_Need16 && FineUpdateSupported()) FineUpdate();
	CardList.m_Need16 = false;
	CardList.m_CurShowdSide = 1;
	CurrentCard->Dublicate();
	UpdateHeader();
	PartialUpdateBW(RCTVAL(Header));
	if (CurrentCard->m_bKnowing) m_iPercent += Config.Percentsteap;
	CurrentCard->m_bKnowing = false;
	m_bNext = 2;
}

/* */
bool SmplImpliment::ShowMidSide()
{
	if (CurrentCard->m_iSidesCount != 3) return false;
	if (CardList.m_CurShowdSide == 2) return false;
	CurrentCard->Show(CardList.m_CurSide[2], &Config.m_Rects[Card3Sd2]);
	DrawRect(RCTVAL(Card3Sd2), BLACK);
	PartialUpdate(RCTVAL(Card3Sd2));
	if (CardList.m_Need16 && FineUpdateSupported()) FineUpdate();
	CardList.m_Need16 = false;
	CardList.m_CurShowdSide = 2;
	return true;
}

/* */
void SmplImpliment::UpdateHeader()
{
	char	sPosInfo[10];
	sprintf(sPosInfo, "%d/%d ", CardList.m_CurCard - CardList.m_vList.begin() + 1, CardList.m_vList.size());
	FillArea(RCTVAL(Header), WHITE);
	DrawSelection(RCTVAL(Header), DGRAY);
	SetFont(Config.Headerfont, BLACK);
	DrawTextRect(RCTVAL(Header), sz(" " + CardList.m_sCardListName), ALIGN_LEFT | VALIGN_MIDDLE);
	DrawTextRect(RCTVAL(Header), sPosInfo, ALIGN_RIGHT | VALIGN_MIDDLE);
}

/* */
void SmplImpliment::EoF()
{
	CardList.m_vList = CardList.m_vData;
	CardList.m_CurCard = CardList.m_vList.begin();
	CardList.ChangeSides();
	BaseImpliment::EoF();
}

/* */
void SmplImpliment::BoF()
{
	CardList.ChangeSides();
	BaseImpliment::BoF();
}

/* */
void MemoImpliment::ShowCurSide()
{
	FillArea(RCTVAL(FullScreen), LGRAY);
	for (int i = 0; i < CurrentCard->m_iSidesCount; i++)
	{
		int id = CurrentCard->m_iSidesCount * 3 + i;
		FillArea(RCTVAL(id), WHITE);
		DrawRect(RCTVAL(id), BLACK);
	}

	CurrentCard->Show(CardList.m_CurSide[0], &Config.m_Rects[CurrentCard->m_iSidesCount * 3]);
	DrawRect(RCTVAL(CurrentCard->m_iSidesCount * 3), BLACK);
	UpdateHeader();
	CardList.m_CurShowdSide = 0;
	FullUpdate();
	if (CardList.m_Need16 && FineUpdateSupported()) FineUpdate();
	CardList.m_Need16 = false;
}

/* */
void MemoImpliment::ShowOthSide()
{
	CurrentCard->Show(CardList.m_CurSide[1], &Config.m_Rects[CurrentCard->m_iSidesCount * 3 + 1]);
	DrawRect(RCTVAL(CurrentCard->m_iSidesCount * 3 + 1), BLACK);
	PartialUpdate(RCTVAL(CurrentCard->m_iSidesCount * 3 + 1));
	if (CardList.m_Need16 && FineUpdateSupported()) FineUpdate();
	CardList.m_Need16 = false;
	CardList.m_CurShowdSide = 1;

	FillArea(RCTVAL(Footer), WHITE);
	DrawLine
	(
		Config.m_Rects[Footer].x,
		Config.m_Rects[Footer].y,
		Config.m_Rects[Footer].x + Config.m_Rects[Footer].w,
		Config.m_Rects[Footer].y,
		BLACK
	);
	for (int i = 0; i < 4; i++)
	{
		char	buf[20];
		sprintf(buf, "@Knowing%d", i + 1);

		int id = SelButt1 + i;
		SetFont(Config.Footerfont, BLACK);
		DrawTextRect2(Config.m_Rects + id, GetLangText(buf));
	}

	CardList.m_CurSell = 2;
	DrawSelection(RCTVAL(SelButt3), BLACK);
	PartialUpdateBW(RCTVAL(Footer));
}

/* */
bool MemoImpliment::ShowMidSide()
{
	if (CurrentCard->m_iSidesCount != 3) return false;
	if (CardList.m_CurShowdSide == 2) return false;
	CurrentCard->Show(CardList.m_CurSide[2], &Config.m_Rects[Card3Sd2]);
	DrawRect(RCTVAL(Card3Sd2), BLACK);
	PartialUpdate(RCTVAL(Card3Sd2));
	if (CardList.m_Need16 && FineUpdateSupported()) FineUpdate();
	CardList.m_Need16 = false;
	CardList.m_CurShowdSide = 2;
	return true;
}

/* */
void MemoImpliment::UpdateHeader()
{
	char	sPosInfo[10];
	sprintf(sPosInfo, "%d/%d ", CardList.m_CurCard - CardList.m_vList.begin() + 1, CardList.m_vList.size());
	FillArea(RCTVAL(Header), WHITE);
	DrawSelection(RCTVAL(Header), DGRAY);
	SetFont(Config.Headerfont, BLACK);
	DrawTextRect(RCTVAL(Header), sz(" " + CardList.m_sCardListName), ALIGN_LEFT | VALIGN_MIDDLE);
	DrawTextRect(RCTVAL(Header), sPosInfo, ALIGN_RIGHT | VALIGN_MIDDLE);
}

/* */
void MemoImpliment::SellOK()
{
	CurrentCard->m_bKnowing = CardList.m_CurSell / 2;
	if (!CurrentCard->m_bKnowing) CurrentCard->Dublicate();
	CardList.m_CurSell = -1;
}

/* */
void TestImpliment::ShowCurSide()
{
	FillArea(RCTVAL(FullScreen), LGRAY);
	for (int i = 0; i < 5; i++)
	{
		int id = CardTst0 + i;
		FillArea(RCTVAL(id), WHITE);
		DrawRect(RCTVAL(id), BLACK);
	}

	CurrentCard->Show(CardList.m_CurSide[0], &Config.m_Rects[CardTst0]);
	DrawRect(RCTVAL(CardTst0), BLACK);
	UpdateHeader();
	DrawSelection(RCTVAL(CardTst11 + CardList.m_CurSell), BLACK);
	ShowOthSide();
}

/* */
void TestImpliment::ShowOthSide()
{
	BaseCard	*Variants[4];
	Variants[0] = CurrentCard;

	bool	repeat = false;
	for (int i = 1; i < 4; i++)
	{
		do
		{
			Variants[i] = CardList.m_vData[rand() % CardList.m_vData.size()];
			repeat = false;
			for (int j = 0; j < i; j++)
				if (Variants[i] == Variants[j])
				{
					repeat = true;
					break;
				}
		} while (repeat);
	}

	random_shuffle(Variants, Variants + 4);
	for (int i = 0; i < 4; i++)
	{
		Variants[i]->Show(CardList.m_CurSide[1], Config.m_Rects + CardTst11 + i);
		DrawRect(RCTVAL(CardTst11 + i), BLACK);
		if (Variants[i] == CurrentCard) m_iCurCorSell = i;
	}

	FullUpdate();
	if (CardList.m_Need16 && FineUpdateSupported()) FineUpdate();
	CardList.m_Need16 = false;
}

/* */
bool TestImpliment::ShowMidSide()
{
	return true;
}

/* */
void TestImpliment::UpdateHeader()
{
	char	sPosInfo[10];
	sprintf
	(
		sPosInfo,
		"%d %d/%d ",
		m_iCorect,
		CardList.m_CurCard - CardList.m_vList.begin() + 1,
		CardList.m_vList.size()
	);
	FillArea(RCTVAL(Header), WHITE);
	DrawSelection(RCTVAL(Header), DGRAY);
	SetFont(Config.Headerfont, BLACK);
	DrawTextRect(RCTVAL(Header), sz(" " + CardList.m_sCardListName), ALIGN_LEFT | VALIGN_MIDDLE);
	DrawTextRect(RCTVAL(Header), sPosInfo, ALIGN_RIGHT | VALIGN_MIDDLE);
}

/* */
void TestImpliment::SellOK()
{
	if (m_iCurCorSell == CardList.m_CurSell)
	{
		m_iCorect++;
		CurrentCard->m_bKnowing = true;
	}
	else
		CurrentCard->m_bKnowing = false;
}

/* */
void TestImpliment::EoF()
{
	char	buf[256];
	sprintf(buf, T(@Test_comlete), m_iCorect, CardList.m_vList.size());
	Message(ICON_INFORMATION, "Memorum", buf, 5000);
	m_iCorect = 0;
	BaseImpliment::EoF();
}
