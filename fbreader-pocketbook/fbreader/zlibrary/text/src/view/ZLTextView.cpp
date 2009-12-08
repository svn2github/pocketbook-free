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

#include <algorithm>

#include <ZLUnicodeUtil.h>
#include <ZLApplication.h>

#include <ZLTextModel.h>
#include <ZLTextParagraph.h>

#include "ZLTextView.h"
#include "ZLTextLineInfo.h"
#include "ZLTextParagraphCursor.h"
#include "ZLTextWord.h"
#include "ZLTextSelectionModel.h"

// Denis // -->
// Для отмены изменений 08.09.2009
//static char Data2[5*1024*1024];
static char *Data2;
// Denis // <--
// Denis //
extern bool link_not_terminated;
extern int hyph_override;
// Denis //

ZLTextView::ZLTextView(ZLApplication &application, shared_ptr<ZLPaintContext> context) : ZLView(application, context), myPaintState(NOTHING_TO_PAINT), myOldWidth(-1), myOldHeight(-1), myStyle(context), mySelectionModel(*this, application), myTreeStateIsFrozen(false) {
}

ZLTextView::~ZLTextView() {
	clear();
}

void ZLTextView::setPaintContext(shared_ptr<ZLPaintContext> context) {
	ZLView::setPaintContext(context);
	myStyle.setPaintContext(context);
}

void ZLTextView::clear() {
	mySelectionModel.clear();

	myStartCursor = 0;
	myEndCursor = 0;
	myLineInfos.clear();
	myPaintState = NOTHING_TO_PAINT;

	myTextElementMap.clear();
	myTreeNodeMap.clear();
	myTextSize.clear();
	myTextBreaks.clear();

	ZLTextParagraphCursorCache::clear();
}

void ZLTextView::setModel(shared_ptr<ZLTextModel> model) {
	clear();

	myModel = model;

	if (!myModel.isNull() && (myModel->paragraphsNumber() != 0)) {
		setStartCursor(ZLTextParagraphCursor::cursor(*myModel));

		size_t size = myModel->paragraphsNumber();
		myTextSize.reserve(size + 1);
		myTextSize.push_back(0);
		for (size_t i= 0; i < size; ++i) {
			myTextSize.push_back(myTextSize.back() + (*myModel)[i]->textLength());
			if ((*myModel)[i]->kind() == ZLTextParagraph::END_OF_TEXT_PARAGRAPH) {
				myTextBreaks.push_back(i);
			}
		}
	}
}

void ZLTextView::scrollPage(bool forward, ScrollingMode mode, unsigned int value) {
	preparePaintInfo();
	if (myPaintState == READY) {
		myPaintState = forward ? TO_SCROLL_FORWARD : TO_SCROLL_BACKWARD;
		myScrollingMode = mode;
		myOverlappingValue = value;
	}
}

std::vector<size_t>::const_iterator ZLTextView::nextBreakIterator() const {
	ZLTextWordCursor cursor = endCursor();
	if (cursor.isNull()) {
		cursor = startCursor();
	}
	if (cursor.isNull()) {
		return myTextBreaks.begin();
	}
	return std::lower_bound(myTextBreaks.begin(), myTextBreaks.end(), cursor.paragraphCursor().index());
}

void ZLTextView::scrollToStartOfText() {
	if (endCursor().isNull()) {
		return;
	}

	if (!startCursor().isNull() &&
			startCursor().isStartOfParagraph() &&
			startCursor().paragraphCursor().isFirst()) {
		return;
	}

	std::vector<size_t>::const_iterator i = nextBreakIterator();
	gotoParagraph((i != myTextBreaks.begin()) ? *(i - 1) : 0, false);

	application().refreshWindow();
}

void ZLTextView::scrollToEndOfText() {
	if (endCursor().isNull() || myModel.isNull()) {
		return;
	}

	if (endCursor().isEndOfParagraph() &&
			endCursor().paragraphCursor().isLast()) {
		return;
	}

	std::vector<size_t>::const_iterator i = nextBreakIterator();
	if (i == myTextBreaks.end()) {
		gotoParagraph(myModel->paragraphsNumber(), true);
		myEndCursor.nextParagraph();
	} else {
		gotoParagraph(*i - 1, true);
	}
	myEndCursor.moveToParagraphEnd();

	application().refreshWindow();
}

int ZLTextView::paragraphIndexByCoordinate(int y) const {
	int indexBefore = -1;
	for (ZLTextElementIterator it = myTextElementMap.begin(); it != myTextElementMap.end(); ++it) {
		if (it->YEnd < y) {
			indexBefore = it->ParagraphNumber;
		} else if ((it->YStart <= y) || (it->ParagraphNumber == indexBefore)) {
			return it->ParagraphNumber;
		} else {
			return -1;
		}
	}
	return -1;
}

const ZLTextElementArea *ZLTextView::elementByCoordinates(int x, int y) const {
	ZLTextElementIterator it =
		std::find_if(myTextElementMap.begin(), myTextElementMap.end(), ZLTextElementArea::RangeChecker(x, y));
	return (it != myTextElementMap.end()) ? &*it : 0;
}

void ZLTextView::gotoMark(ZLTextMark mark) {
	if (mark.ParagraphNumber < 0) {
		return;
	}
	bool doRepaint = false;
	if (startCursor().isNull()) {
		doRepaint = true;
		preparePaintInfo();
	}
	if (startCursor().isNull()) {
		return;
	}
	if (((int)startCursor().paragraphCursor().index() != mark.ParagraphNumber) ||
			(startCursor().position() > mark)) {
		doRepaint = true;
		gotoParagraph(mark.ParagraphNumber);
		preparePaintInfo();
	}
	if (endCursor().isNull()) {
		preparePaintInfo();
	}
	while (mark > endCursor().position()) {
		doRepaint = true;
		scrollPage(true, NO_OVERLAPPING, 0);
		preparePaintInfo();
	}
	if (doRepaint) {
		application().refreshWindow();
	}
}

void ZLTextView::gotoParagraph(int num, bool last) {
	if (myModel.isNull()) {
		return;
	}

	if (myModel->kind() == ZLTextModel::TREE_MODEL) {
		if ((num >= 0) && (num < (int)myModel->paragraphsNumber())) {
			ZLTextTreeParagraph *tp = (ZLTextTreeParagraph*)(*myModel)[num];
			if (myTreeStateIsFrozen) {
				int corrected = num;
				ZLTextTreeParagraph *parent = tp->parent();
				while ((corrected > 0) && (parent != 0) && !parent->isOpen()) {
					for (--corrected; ((corrected > 0) && parent != (*myModel)[corrected]); --corrected);
					parent = parent->parent();
				}
				if (last && (corrected != num)) {
					++corrected;
				}
				num = corrected;
			} else {
				tp->openTree();
				rebuildPaintInfo(true);
			}
		}
	}

	if (last) {
		if ((num > 0) && (num <= (int)myModel->paragraphsNumber())) {
			moveEndCursor(num);
		}
	} else {
		if ((num >= 0) && (num < (int)myModel->paragraphsNumber())) {
			moveStartCursor(num);
		}
	}
}

void ZLTextView::gotoPosition(int paragraphNumber, int wordNumber, int charNumber) {
	gotoParagraph(paragraphNumber, false);
	if (!myStartCursor.isNull() && 
			((int)myStartCursor.paragraphCursor().index() == paragraphNumber)) {
		moveStartCursor(paragraphNumber, wordNumber, charNumber);
	}
}

bool ZLTextView::hasMultiSectionModel() const {
	return !myTextBreaks.empty();
}

void ZLTextView::search(const std::string &text, bool ignoreCase, bool wholeText, bool backward, bool thisSectionOnly) {
	if (text.empty()) {
		return;
	}

	size_t startIndex = 0;
	size_t endIndex = myModel->paragraphsNumber();
	if (thisSectionOnly) {
		std::vector<size_t>::const_iterator i = nextBreakIterator();
		if (i != myTextBreaks.begin()) {
			startIndex = *(i - 1);
		}
		if (i != myTextBreaks.end()) {
			endIndex = *i;
		}
	}

	myModel->search(text, startIndex, endIndex, ignoreCase);
	if (!startCursor().isNull()) {
		rebuildPaintInfo(true);
		ZLTextMark position = startCursor().position();
		gotoMark(wholeText ?
							(backward ? myModel->lastMark() : myModel->firstMark()) :
							(backward ? myModel->previousMark(position) : myModel->nextMark(position)));
		application().refreshWindow();
	}
}

bool ZLTextView::canFindNext() const {
	return !endCursor().isNull() && (myModel->nextMark(endCursor().position()).ParagraphNumber > -1);
}

void ZLTextView::findNext() {
	if (!endCursor().isNull()) {
		gotoMark(myModel->nextMark(endCursor().position()));
	}
}

bool ZLTextView::canFindPrevious() const {
	return !startCursor().isNull() && (myModel->previousMark(startCursor().position()).ParagraphNumber > -1);
}

void ZLTextView::findPrevious() {
	if (!startCursor().isNull()) {
		gotoMark(myModel->previousMark(startCursor().position()));
	}
}

bool ZLTextView::onStylusPress(int x, int y) {
	mySelectionModel.deactivate();

  if (myModel.isNull()) {
	  return false;
	}

	shared_ptr<ZLTextPositionIndicatorInfo> indicatorInfo = this->indicatorInfo();
	if (!indicatorInfo.isNull() &&
			indicatorInfo->isVisible() &&
			indicatorInfo->isSensitive()) {
		myTreeStateIsFrozen = true;
		bool indicatorAnswer = positionIndicator()->onStylusPress(x, y);
		myTreeStateIsFrozen = false;
		if (indicatorAnswer) {
			application().refreshWindow();
			return true;
		}
	}

	if (myModel->kind() == ZLTextModel::TREE_MODEL) {
		ZLTextTreeNodeMap::const_iterator it =
			std::find_if(myTreeNodeMap.begin(), myTreeNodeMap.end(), ZLTextTreeNodeArea::RangeChecker(x, y));
		if (it != myTreeNodeMap.end()) {
			int paragraphNumber = it->ParagraphNumber;
			ZLTextTreeParagraph *paragraph = (ZLTextTreeParagraph*)(*myModel)[paragraphNumber];

			paragraph->open(!paragraph->isOpen());
			rebuildPaintInfo(true);
			preparePaintInfo();
			if (paragraph->isOpen()) {
				int nextParagraphNumber = paragraphNumber + paragraph->fullSize();
				int lastParagraphNumber = endCursor().paragraphCursor().index();
				if (endCursor().isEndOfParagraph()) {
					++lastParagraphNumber;
				}
				if (lastParagraphNumber < nextParagraphNumber) {
					gotoParagraph(nextParagraphNumber, true);
					preparePaintInfo();
				}
			}
			int firstParagraphNumber = startCursor().paragraphCursor().index();
			if (startCursor().isStartOfParagraph()) {
				--firstParagraphNumber;
			}
			if (firstParagraphNumber >= paragraphNumber) {
				gotoParagraph(paragraphNumber);
				preparePaintInfo();
			}
			application().refreshWindow();

			return true;
		}
	}

	return false;
}

void ZLTextView::activateSelection(int x, int y) {
	if (isSelectionEnabled()) {
		mySelectionModel.activate(x, y);
		application().refreshWindow();
	}
}

bool ZLTextView::onStylusMovePressed(int x, int y) {
	if (mySelectionModel.extendTo(x, y)) {
		copySelectedTextToClipboard(ZLDialogManager::CLIPBOARD_SELECTION);
		application().refreshWindow();
	}
	return true;
}

void ZLTextView::copySelectedTextToClipboard(ZLDialogManager::ClipboardType type) const {
	if (ZLDialogManager::instance().isClipboardSupported(type)) {
		std::string text = mySelectionModel.getText();
		if (!text.empty()) {
			ZLDialogManager::instance().setClipboardText(text, type);
		}
	}
}

bool ZLTextView::onStylusRelease(int, int) {
	mySelectionModel.deactivate();
	return true;
}

// Denis //	

bool HebrewTest(ZLTextWord &word){

	int unic1=0;
	int i=0;
	int min=strlen(word.Data)-2;
	if (min>word.Size+7){
		min=word.Size+7;
	}
	while (i<min-3){
		if ((((word.Data[i] & 0xE0) ^ 0xC0) == 0)&&(((word.Data[i+1] & 0xC0) ^ 0x80) == 0)){
				unic1=(word.Data[i] & 0x1F)<<6;
				unic1=unic1 | (word.Data[i+1] & 0x3F);
		}
		if ((((word.Data[i] & 0xF0) ^ 0xE0) == 0)&&(((word.Data[i+1] & 0xC0) ^ 0x80) == 0)&&(((word.Data[i+2] & 0xC0) ^ 0x80) == 0)){
			unic1=(word.Data[i] & 0x0F)<<12;
			unic1=unic1 | ((word.Data[i+1] & 0x3F)<<6);
			unic1=unic1 | (word.Data[i+2] & 0x3F);
		}
		if (((unic1>=0x05D0)&&(unic1<=0x05EA)) || ((unic1>=0x05F0)&&(unic1<=0x05F4)) || ((unic1>=0x05B0)&&(unic1<=0x05C3)) ||
			((unic1>=0xFB1D)&&(unic1<=0xFB4F))){
			return true;
		} else {
			unic1=0;
			i++;
		}
	}
	return false;
}

bool ZLTextView::HebrewTestWord(char *Data, int len){

	int unic1=0;
	int i=0;
	while (i<len-2){
		if ((((Data[i] & 0xE0) ^ 0xC0) == 0)&&(((Data[i+1] & 0xC0) ^ 0x80) == 0)){
				unic1=(Data[i] & 0x1F)<<6;
				unic1=unic1 | (Data[i+1] & 0x3F);
		}
		if ((((Data[i] & 0xF0) ^ 0xE0) == 0)&&(((Data[i+1] & 0xC0) ^ 0x80) == 0)&&(((Data[i+2] & 0xC0) ^ 0x80) == 0)){
			unic1=(Data[i] & 0x0F)<<12;
			unic1=unic1 | ((Data[i+1] & 0x3F)<<6);
			unic1=unic1 | (Data[i+2] & 0x3F);
		}
		if (((unic1>=0x05D0)&&(unic1<=0x05EA)) || 
			((unic1>=0x05F0)&&(unic1<=0x05F4)) || 
			((unic1>=0x05B0)&&(unic1<=0x05C3)) ||
			((unic1>=0xFB1D)&&(unic1<=0xFB4F))){
			return true;
		} else {
			unic1=0;
			i++;
		}
	}
	return false;
}

void HebrewInvert(ZLTextWord &word){
	
	unsigned int i=0;
	int unic1=0;
	int unic2=0;
	unsigned int k=0;
	unsigned int bk;

	while (((Data2[k]=='(') || 
		(Data2[k]=='[') || 
		(Data2[k]=='<') ||
		(Data2[k]=='{') ||
		(Data2[k]=='\"'))
		 && (k<word.Size-1)){
		k++;	
	}
	
	if ( ((Data2[k]>='0') && (Data2[k]<='9')) || 
		((Data2[k]>='A') && (Data2[k]<='Z')) || 
		((Data2[k]>='a') && (Data2[k]<='z')) || 
		(word.Size<=2) ||
		(k>word.Size-2)
		){
		return;
	}

	k=0;		
	unsigned int size=word.Size;
	while (
		((Data2[0]=='(') ||
		(Data2[0]=='[') ||
		(Data2[0]=='<') ||
		(Data2[0]=='{'))
			&& (k<size)
		){
		if ((Data2[0]=='(') && (k<size)){
			memcpy(&Data2[0], &Data2[1], size-1);
			Data2[size-1]=')';
			k++;
			size--;
		}
		if ((Data2[0]=='[') && (k<size)){
			memcpy(&Data2[0], &Data2[1], size-1);
			Data2[size-1]=']';
			k++;
			size--;
		}
		if ((Data2[0]=='<') && (k<size)){
			memcpy(&Data2[0], &Data2[1], size-1);
			Data2[size-1]='>';
			k++;
			size--;
		}
		if ((Data2[0]=='{') && (k<size)){
			memcpy(&Data2[0], &Data2[1], size-1);
			Data2[size-1]='}';
			k++;
			size--;
		}
	}
	
	bk=0;
	while (
		((Data2[size+bk-1]==')') ||
		(Data2[size+bk-1]==']') ||
		(Data2[size+bk-1]=='>') ||
		(Data2[size+bk-1]=='}') ||
		(Data2[size+bk-1]==':') ||
		(Data2[size+bk-1]==';') ||
		(Data2[size+bk-1]=='.') ||
		(Data2[size+bk-1]==',') ||
		(Data2[size+bk-1]=='!') ||
		(Data2[size+bk-1]=='?'))
			&& (bk<size)
		){
		if ((Data2[size+bk-1]==')') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]='(';
			bk++;
			size--;
		}
		if ((Data2[size+bk-1]==']') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]='[';
			bk++;
			size--;
		}
		if ((Data2[size+bk-1]=='>') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]='<';
			bk++;
			size--;
		}
		if ((Data2[size+bk-1]=='}') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]='{';
			bk++;
			size--;
		}
		if ((Data2[size+bk-1]==':') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]=':';
			bk++;
			size--;
		}
		if ((Data2[size+bk-1]==';') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]=';';
			bk++;
			size--;
		}
		if ((Data2[size+bk-1]=='.') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]='.';
			bk++;
			size--;
		}
		if ((Data2[size+bk-1]==',') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]=',';
			bk++;
			size--;
		}
		if ((Data2[size+bk-1]=='!') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]='!';
			bk++;
			size--;
		}
		if ((Data2[size+bk-1]=='?') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]='?';
			bk++;
			size--;
		}
	}

	i=bk;
	if (size>1){
		if ((((Data2[i] & 0xE0) ^ 0xC0) == 0)&&(((Data2[i+1] & 0xC0) ^ 0x80) == 0)){
			unic1=(Data2[i] & 0x1F)<<6;
			unic1=unic1 | (Data2[i+1] & 0x3F);
		}
		if ((((Data2[i] & 0xF0) ^ 0xE0) == 0)&&(((Data2[i+1] & 0xC0) ^ 0x80) == 0)&&(((Data2[i+2] & 0xC0) ^ 0x80) == 0)){
			unic1=(Data2[i] & 0x1F)<<12;
			unic1=unic1 | ((Data2[i+1] & 0x3F)<<6);
			unic1=unic1 | (Data2[i+2] & 0x3F);
		}
	}

	if (((unic1>=0x05D0)&&(unic1<=0x05EA)) || 
		((unic1>=0x05F0)&&(unic1<=0x05F4)) || 
		((unic1>=0x05B0)&&(unic1<=0x05C3)) || 
		((unic1>=0xFB1D)&&(unic1<=0xFB4F)) ||
   		((unic1>=0xA1) && (unic1<=0xBF)) ||
   		((unic1>=0x2010) && (unic1<=0x2046)) ||
		(Data2[i]>0)){
 		int unic1size, unic2size;
		while ((size>2)&&(size<1000)){
			char buf1[5], buf2[5];
			
			unic1=0;
			unic1size=0;
			unic2size=0;
			
		   	if (Data2[i]>0){
		   		unic1size=1;
		   	}
			if ((((Data2[i] & 0xE0) ^ 0xC0) == 0)&&(((Data2[i+1] & 0xC0) ^ 0x80) == 0)){
				unic1=(Data2[i] & 0x1F)<<6;
				unic1=unic1 | (Data2[i+1] & 0x3F);
				if (((unic1>=0x05D0)&&(unic1<=0x05EA)) || 
					((unic1>=0x05F0)&&(unic1<=0x05F4)) || 
					((unic1>=0x05B0)&&(unic1<=0x05C3)) ||
			   		((unic1>=0xA1) && (unic1<=0xBF))){
					unic1size=2;
				}
			}
			if ((((Data2[i] & 0xF0) ^ 0xE0) == 0)&&(((Data2[i+1] & 0xC0) ^ 0x80) == 0)&&(((Data2[i+2] & 0xC0) ^ 0x80) == 0)){
				unic1=(Data2[i] & 0x0F)<<12;
				unic1=unic1 | ((Data2[i+1] & 0x3F)<<6);
				unic1=unic1 | (Data2[i+2] & 0x3F);
			   	if (((unic1>=0xFB1D)&&(unic1<=0xFB4F)) ||
			   		((unic1>=0x2010) && (unic1<=0x2046))
			   		){
					unic1size=3;
				}
			}
			unic2=0;
		   	if (Data2[i+size-1]>0){
		   		unic2size=1;
		   	}
			if ((((Data2[i+size-2] & 0xE0) ^ 0xC0) == 0)&&(((Data2[i+size-1] & 0xC0) ^ 0x80) == 0)){
				unic2=(Data2[i+size-2] & 0x1F)<<6;
				unic2=unic2 | (Data2[i+size-1] & 0x3F);
				if (((unic2>=0x05D0)&&(unic2<=0x05EA)) || 
					((unic2>=0x05F0)&&(unic2<=0x05F4)) || 
					((unic2>=0x05B0)&&(unic2<=0x05C3)) ||
			   		((unic2>=0xA1) && (unic2<=0xBF))){
					unic2size=2;
				}
			}
			if ((((Data2[i+size-3] & 0xF0) ^ 0xE0) == 0)&&
				(((Data2[i+size-2] & 0xC0) ^ 0x80) == 0)&&
				(((Data2[i+size-1] & 0xC0) ^ 0x80) == 0)){
				unic2=(Data2[i+size-3] & 0x0F)<<12;
				unic2=unic2 | ((Data2[i+size-2] & 0x3F)<<6);
				unic2=unic2 | (Data2[i+size-1] & 0x3F);
			   	if (((unic2>=0xFB1D)&&(unic2<=0xFB4F)) ||
			   		((unic2>=0x2010) && (unic2<=0x2046))
			   		){
					unic2size=3;
				}
			}
			if ((unic1size==0)||(unic2size==0)){
				fprintf(stderr, "Denis - ZLTextView.cpp - HebrewInvert - Not Support Symbols - Invert was break\n\t unic1=%d unic2=%d\n", unic1, unic2);
				return;
			}
			if (unic1size>unic2size){
				memcpy(&buf1[0], &Data2[i], unic1size);
				memcpy(&buf2[0], &Data2[i+size-unic2size], unic2size);
				memcpy(&Data2[i+unic2size], &Data2[i+unic2size+(unic1size-unic2size)], size-unic2size-unic1size);
				memcpy(&Data2[i], &buf2[0], unic2size);
				memcpy(&Data2[i+size-unic1size], &buf1[0], unic1size);
				i+=unic2size;
				size-=(unic1size+unic2size);
			} else {
				if (unic1size<unic2size){
					memcpy(&buf1[0], &Data2[i], unic1size);
					memcpy(&buf2[0], &Data2[i+size-unic2size], unic2size);
					unsigned int l;
					for (l=0; l<size-unic1size-unic2size; l++)
						Data2[i+size-unic1size-l-1]=Data2[i+size-unic1size-l-(unic2size-unic1size)-1];
					memcpy(&Data2[i], &buf2[0], unic2size);
					memcpy(&Data2[i+size-unic1size], &buf1[0], unic1size);
					i+=unic2size;
					size-=(unic1size+unic2size);
				} else {
					memcpy(&buf1[0], &Data2[i], unic1size);
					memcpy(&buf2[0], &Data2[i+size-unic2size], unic2size);
					memcpy(&Data2[i], &buf2[0], unic2size);
					memcpy(&Data2[i+size-unic1size], &buf1[0], unic1size);					
					i+=unic2size;
					size-=(unic1size+unic2size);
				}
			}

		}
	} else {
		if (word.Size>3){
			fprintf(stderr, "Denis - ZLTextView.cpp - HebrewInvert - Not Support Symbols - Invert was break\n\t unic1=%d\n", unic1);
		}
	}
	return;
}

/*
int HebrewPos(ZLTextWord &word){
	int unic1=0;
	if (((Data2[0] & 0xF0) ^ 0xE0) == 0){
		unic1=(Data2[02] & 0x0F)<<12;
		unic1=unic1 | ((Data2[1] & 0x3F)<<6);
		unic1=unic1 | (Data2[2] & 0x3F);
		if (unic1==0xFEFF){
			return 3;
		}
	}
	return 0;
}*/

// Denis //	

void ZLTextView::drawString(int x, int y, const char *str, int len, const ZLTextWord::Mark *mark, int shift) {
	context().setColor(myStyle.textStyle()->color());

	if (mark == 0) {
		context().drawString(x, y, str, len);
	} else {
		int pos = 0;
// Denis //
		int Hx=x+context().stringWidth(str, len);
		bool wordHebrew = HebrewTestWord((char *)str, len);
// Denis //
		for (; (mark != 0) && (pos < len); mark = mark->next()) {
			
			int markStart = mark->start() - shift;
			int markLen = mark->length();
// Denis //			
			if (wordHebrew){

				if (markLen <= 0) {
					continue;
				}
				markStart=len-markStart-markLen;
				if (markStart+markLen < len-pos) {
					Hx -= context().stringWidth(str + markStart + markLen, len - markStart - markLen - pos);
					context().drawString(Hx, y, str + markStart + markLen, len - markStart - markLen - pos);
				}
				if (markStart >= 0) {
					context().setColor(ZLTextStyleCollection::instance().baseStyle().SelectedTextColorOption.value());
					{
						Hx -= context().stringWidth(str + markStart, markLen);
						context().drawString(Hx, y, str + markStart, markLen);
					}
					context().setColor(myStyle.textStyle()->color());
				}
				pos = len - markStart;

			} else {
			
				if (markStart < pos) {
					markLen += markStart - pos;
					markStart = pos;
				}
				if (markLen <= 0) {
					continue;
				}
				if (markStart > pos) {
					int endPos = std::min(markStart, len);
					context().drawString(x, y, str + pos, endPos - pos);
					x += context().stringWidth(str + pos, endPos - pos);
				}
				if (markStart < len) {
					context().setColor(ZLTextStyleCollection::instance().baseStyle().SelectedTextColorOption.value());
					{
						int endPos = std::min(markStart + markLen, len);
						context().drawString(x, y, str + markStart, endPos - markStart);
						x += context().stringWidth(str + markStart, endPos - markStart);
					}
					context().setColor(myStyle.textStyle()->color());
				}
				pos = markStart + markLen;
			}
		}
		if (pos < len) {
// Denis //
			if (wordHebrew){
				Hx -= context().stringWidth(str, len - pos);
				context().drawString(Hx, y, str, len - pos);
			} else {
// Denis //
				context().drawString(x, y, str + pos, len - pos);
			}
		}
	}
}

void ZLTextView::drawWord(int xStart, int xEnd, int y, ZLTextWord &word, int start, int length, bool addHyphenationSign) {

	if ((start == 0) && (length == -1)) {

// Denis //
		if (NeedHebrewTest){
			NeedHebrewTest=false;
//			PrevHebrewForLink=Hebrew;
			Hebrew=HebrewTest(word);
		} else {
			PrevHebrewForLink=Hebrew;
		}
//fprintf(stderr, "Denis - ZLTextView.cpp - drawWord - 3 - strlen(word.Data) = %d\n", strlen(word.Data));	

// Denis // -->
// Для отмены изменений 08.09.2009
//		memcpy(&Data2[0], &word.Data[0], strlen(word.Data));
		Data2 = (char *)&word.Data[0];
// Denis // <--
		if (HebrewTestWord(&Data2[0], word.Size)){
				HebrewInvert(word);
		}

		if (Hebrew){
// Denis //
// Отключение переносов для нормального отображения англ. слов.
			hyph_override=0;
// Denis //
//			int i1=0;
//			i1=HebrewPos(word);
//			drawString(viewWidth()+leftMargin()-xEnd, y, &Data2[i1], word.Size-i1, word.mark(), 0);			
			drawString(viewWidth()+leftMargin()-xEnd, y, Data2, word.Size, word.mark(), 0);			
// Denis // -->
// Для отмены изменений 08.09.2009
			HebrewInvert(word);
// Denis // <--
		} else {
			if (((word.Data[word.Size]==3)||(word.Data[word.Size+1]==30))&&(PrevHebrewForLink)){
				NeedHebrewTest=true;
				drawString(viewWidth()+leftMargin()-xEnd, y, Data2, word.Size, word.mark(), 0);			
			} else {
				drawString(xStart, y, Data2, word.Size, word.mark(), 0);
			}
		}
// Denis //

	} else {
		int startPos = ZLUnicodeUtil::length(word.Data, start);
		int endPos = (length == -1) ? word.Size : ZLUnicodeUtil::length(word.Data, start + length);
		if (!addHyphenationSign) {
			drawString(xStart, y, word.Data + startPos, endPos - startPos, word.mark(), startPos);
		} else {
			std::string substr;
			substr.append(word.Data + startPos, endPos - startPos);
			substr += '-';
			drawString(xStart, y, substr.data(), substr.length(), word.mark(), startPos);
		}
	}
}

void ZLTextView::clearCaches() {
	rebuildPaintInfo(true);
}

void ZLTextView::highlightParagraph(int paragraphNumber) {
	myModel->selectParagraph(paragraphNumber);
	rebuildPaintInfo(true);
}

int ZLTextView::infoSize(const ZLTextLineInfo &info, SizeUnit unit) {
	return (unit == PIXEL_UNIT) ? (info.Height + info.Descent + info.VSpaceAfter) : (info.IsVisible ? 1 : 0);
}

int ZLTextView::textAreaHeight() const {
	shared_ptr<ZLTextPositionIndicatorInfo> indicatorInfo = this->indicatorInfo();
	if (!indicatorInfo.isNull() && indicatorInfo->isVisible()) {
		return viewHeight() - indicatorInfo->height() - indicatorInfo->offset();
	} else {
		return viewHeight();
	}
}
/*
 *
size_t ZLTextView::PositionIndicator::sizeOfTextBeforeParagraph(size_t paragraphNumber) const {
	return myTextView.myTextSize[paragraphNumber] - myTextView.myTextSize[startTextIndex()];
}

size_t ZLTextView::PositionIndicator::sizeOfParagraph(size_t paragraphNumber) const {
	return myTextView.myTextSize[paragraphNumber + 1] - myTextView.myTextSize[paragraphNumber];
}
*/


void ZLTextView::gotoPage(size_t index) {
	const size_t symbolIndex = index * 2048 - 128;
	std::vector<size_t>::const_iterator it = std::lower_bound(myTextSize.begin(), myTextSize.end(), symbolIndex);
	std::vector<size_t>::const_iterator i = nextBreakIterator();
	const size_t startIndex = (i != myTextBreaks.begin()) ? *(i - 1) : 0;
	const size_t endIndex = (i != myTextBreaks.end()) ? *i : myModel->paragraphsNumber();
	size_t paragraphNumber = std::min((size_t)(it - myTextSize.begin()), endIndex) - 1;
	gotoParagraph(paragraphNumber, true);
	preparePaintInfo();
	const ZLTextWordCursor &cursor = endCursor();
	if (!cursor.isNull() && (paragraphNumber == cursor.paragraphCursor().index())) {
		if (!cursor.paragraphCursor().isLast() || !cursor.isEndOfParagraph()) {
			size_t paragraphLength = cursor.paragraphCursor().paragraphLength();
			if (paragraphLength > 0) {
				size_t wordNum =
					(myTextSize[startIndex] + symbolIndex - myTextSize[paragraphNumber]) *
					paragraphLength / (myTextSize[endIndex] - myTextSize[startIndex]);
				moveEndCursor(cursor.paragraphCursor().index(), wordNum, 0);
			}
		}
	}
}

size_t ZLTextView::pageNumber() const {
	if (empty()) {
		return 0;
	}
	std::vector<size_t>::const_iterator i = nextBreakIterator();
/*	const size_t startIndex = (i != myTextBreaks.begin()) ? *(i - 1) : 0;
	const size_t endIndex = (i != myTextBreaks.end()) ? *i : myModel->paragraphsNumber();*/
	return /*(myTextSize[endIndex] - myTextSize[startIndex]) / 2048 + 1;*/myModel->paragraphsNumber();

}
