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

#include <ZLUnicodeUtil.h>

#include "ZLTextView.h"
#include "ZLTextLineInfo.h"

struct xxx_link {
	int x1, y1, x2, y2;
	int kind;
	std::string id;
	bool next;
};
extern std::vector<std::string> xxx_notes;
extern std::vector<xxx_link> xxx_page_links;
extern int lock_drawing;
bool link_not_terminated;

void ZLTextView::paint() {

	preparePaintInfo();

	myTextElementMap.clear();
	myTreeNodeMap.clear();
	context().clear(ZLTextStyleCollection::instance().baseStyle().BackgroundColorOption.value());

	if (empty()) {
		return;
	}

	link_not_terminated = false;

	std::vector<size_t> labels;
	labels.reserve(myLineInfos.size() + 1);
	labels.push_back(0);
	context().moveYTo(topMargin());
	for (std::vector<ZLTextLineInfoPtr>::const_iterator it = myLineInfos.begin(); it != myLineInfos.end(); ++it) {
		prepareTextLine(**it);
		labels.push_back(myTextElementMap.size());
	}
	mySelectionModel.update();
	context().moveYTo(topMargin());
	int index = 0;
	for (std::vector<ZLTextLineInfoPtr>::const_iterator it = myLineInfos.begin(); it != myLineInfos.end(); ++it) {
		drawTextLine(**it, labels[index], labels[index + 1]);
		++index;
	}

/*
	shared_ptr<ZLTextPositionIndicatorInfo> indicatorInfo = this->indicatorInfo();
	if (!indicatorInfo.isNull() && indicatorInfo->isVisible()) {
		positionIndicator()->draw();
	}
*/

	ZLTextParagraphCursorCache::cleanup();
}

static bool operator <= (const ZLTextElementArea &area, const ZLTextSelectionModel::BoundElement &element) {
	return
		(area.ParagraphNumber < element.ParagraphNumber) ||
		((area.ParagraphNumber == element.ParagraphNumber) &&
		 (area.TextElementNumber <= element.TextElementNumber));
}

static bool operator > (const ZLTextElementArea &area, const ZLTextSelectionModel::BoundElement &element) {
	return !(area <= element);
}

static bool operator < (const ZLTextWordCursor &cursor, const ZLTextSelectionModel::BoundElement &element) {
	int pn = cursor.paragraphCursor().index();
	return
		(pn < element.ParagraphNumber) ||
		((pn == element.ParagraphNumber) &&
		 (((int)cursor.wordNumber() < element.TextElementNumber) ||
		  (((int)cursor.wordNumber() == element.TextElementNumber) &&
			 (cursor.charNumber() < element.CharNumber))));
}

static bool operator >= (const ZLTextWordCursor &cursor, const ZLTextSelectionModel::BoundElement &element) {
	return !(cursor < element);
}

static bool operator > (const ZLTextWordCursor &cursor, const ZLTextSelectionModel::BoundElement &element) {
	int pn = cursor.paragraphCursor().index();
	return
		(pn > element.ParagraphNumber) ||
		((pn == element.ParagraphNumber) &&
		 (((int)cursor.wordNumber() > element.TextElementNumber) ||
		  (((int)cursor.wordNumber() == element.TextElementNumber) &&
			 (cursor.charNumber() > element.CharNumber))));
}

static bool operator <= (const ZLTextWordCursor &cursor, const ZLTextSelectionModel::BoundElement &element) {
	return !(cursor > element);
}

static ZLTextElementIterator findLast(ZLTextElementIterator from, ZLTextElementIterator to, const ZLTextSelectionModel::BoundElement &bound) {
	if (*from > bound) {
		return from;
	}
	for (++from; (from != to) && (*from <= bound); ++from) {
	}
	return --from;
}

int ZLTextView::areaLength(const ZLTextParagraphCursor &paragraph, const ZLTextElementArea &area, int toCharNumber) {
	myStyle.setTextStyle(area.Style);
	const ZLTextWord &word = (const ZLTextWord&)paragraph[area.TextElementNumber];
	int length = toCharNumber - area.StartCharNumber;
	bool selectHyphenationSign = false;
	if (length >= area.Length) {
		selectHyphenationSign = area.AddHyphenationSign;
		length = area.Length;
	}
	if (length > 0) {
		return myStyle.wordWidth(word, area.StartCharNumber, length, selectHyphenationSign);
	}
	return 0;
}

void ZLTextView::drawTextLine(const ZLTextLineInfo &info, size_t from, size_t to) {

if (lock_drawing) return;

// Denis //
	Hebrew=false;
	NeedHebrewTest=true;
// Denis //

	const ZLTextParagraphCursor &paragraph = info.RealStart.paragraphCursor();
	const ZLTextElementIterator fromIt = myTextElementMap.begin() + from;
	const ZLTextElementIterator toIt = myTextElementMap.begin() + to;

	if (!mySelectionModel.isEmpty() && (from != to)) {
		std::pair<ZLTextSelectionModel::BoundElement,ZLTextSelectionModel::BoundElement> range = mySelectionModel.range();

		int left = viewWidth() + leftMargin() - 1;
		if (info.Start > range.first) {
			left = leftMargin();
		} else if (info.End >= range.first) {
			ZLTextElementIterator jt = findLast(fromIt, toIt, range.first);
			left = jt->XStart;
			if (jt->Kind == ZLTextElement::WORD_ELEMENT) {
				left += areaLength(paragraph, *jt, range.first.CharNumber);
			}
		}

		const int top = context().y() + 1;
		int bottom = context().y() + info.Height + info.Descent;
		int right = leftMargin();
		if (info.End < range.second) {
			right = viewWidth() + leftMargin() - 1;
			bottom += info.VSpaceAfter;
		} else if (info.Start <= range.second) {
			ZLTextElementIterator jt = findLast(fromIt, toIt, range.second);
			if (jt->Kind == ZLTextElement::WORD_ELEMENT) {
				right = jt->XStart + areaLength(paragraph, *jt, range.second.CharNumber) - 1;
			} else {
				right = jt->XEnd - 1;
			}
		}

		if (left < right) {
			context().setFillColor(ZLTextStyleCollection::instance().baseStyle().SelectionBackgroundColorOption.value());
			context().fillRectangle(left, top, right, bottom);
		}
	}


	context().moveY(info.Height);
	int maxY = topMargin() + textAreaHeight();
	if (context().y() > maxY) {
	  context().moveYTo(maxY);
	}
	context().moveXTo(leftMargin());
	if (!info.NodeInfo.isNull()) {
		drawTreeLines(*info.NodeInfo, info.Height, info.Descent + info.VSpaceAfter);
	}
	ZLTextElementIterator it = fromIt;

	struct xxx_link cur_link;
	cur_link.id.erase();
	cur_link.x1 = 0;
	cur_link.x2 = 0;
	cur_link.y1 = 0;
	cur_link.y2 = 0;
	cur_link.next = false;

//	printf("xxxxxxxxxxxxxxxxxxxxxxxxx\n");

	bool started = false;

	
	for (ZLTextWordCursor pos = info.Start; //RealStart; 
			!pos.equalWordNumber(info.End); pos.nextWord()) {
		const ZLTextElement &element = paragraph[pos.wordNumber()];
		ZLTextElement::Kind kind = element.kind();
	

//		printf("kind: %d\n", element.kind());
		if(pos.equalWordNumber(info.RealStart))
			started = true;
		if (started && (kind == ZLTextElement::WORD_ELEMENT) || (kind == ZLTextElement::IMAGE_ELEMENT)) {
			if (it->ChangeStyle) {
				myStyle.setTextStyle(it->Style);
			}

// Denis //
// ”брать коментарии чтобы правильно работало //

//			const int x = it->XStart;
//			const int y = it->YEnd - myStyle.elementDescent(element) - myStyle.textStyle()->verticalShift();
			const int xStart = it->XStart;
			const int xEnd = it->XEnd;
			const int y = it->YEnd - myStyle.elementDescent(element) - myStyle.textStyle()->verticalShift();

// Denis //
			if (kind == ZLTextElement::WORD_ELEMENT) {
// Denis //
// ”брать коментарии чтобы правильно работало //
//				drawWord(x, y, (const ZLTextWord&)element, pos.charNumber(), -1, false);
				drawWord(xStart, xEnd, y, (ZLTextWord&)element, pos.charNumber(), -1, false);
// Denis //

			} else {
				context().drawImage(xStart, y, ((const ZLTextImageElement&)element).image());
			}
			++it;
		} else if(kind == ZLTextElement::CONTROL_ELEMENT) {

			const ZLTextControlEntry &control = ((const ZLTextControlElement&)element).entry();
//			printf("control.kind(): %d, isHyperlink: %s\n", control.kind(), control.isHyperlink()?"true":"false");

			if (control.isHyperlink()) {
				if(control.kind() == 15 || control.kind() == 16 || control.kind() == 37) {
					//INTERNAL_HYPERLINK
					//printf("hyperlink start\n");
					if(!cur_link.id.empty()) {
						fprintf(stderr, "Denis - ZLTextView_paint.cpp - drawTextLine - empty1 - not support\n");
						const int y = it->YEnd - myStyle.elementDescent(element) - myStyle.textStyle()->verticalShift();

						cur_link.x2 = it->XEnd;
						cur_link.y2 = y;
						cur_link.y1 = y - info.Height;
						xxx_page_links.push_back(cur_link);
	
						context().drawLine(cur_link.x1, cur_link.y2, cur_link.x2, cur_link.y2);
					}

					cur_link.id = ((const ZLTextHyperlinkControlEntry&)control).label();
					cur_link.kind = control.kind();
// Denis //
// ”брать коментарии чтобы правильно работало //
					PrevHebrew=Hebrew;
					if (Hebrew){
						cur_link.x1 = viewWidth()+leftMargin()-it->XEnd;
					} else {
						cur_link.x1 = it->XStart;
					}

//						cur_link.x1 = it->XStart;
// Denis //					
				} 
				else {
					//printf("control.kind() %d, id: %s\n", control.kind(), ((const ZLTextHyperlinkControlEntry&)control).label().c_str());
				}
				
			} else {

				if(control.kind() == 15 || control.kind() == 16 || control.kind() == 37) {

					if(cur_link.id.empty() || link_not_terminated) {
						cur_link.x1 = fromIt->XStart;
						fprintf(stderr, "Denis - ZLTextView_paint.cpp - drawTextLine - empty2 - not support\n");
					}

					ZLTextElementIterator lit = it;
					if(lit != fromIt) {
						lit--;
					
						const int y = lit->YEnd - myStyle.elementDescent(element) - myStyle.textStyle()->verticalShift();

// Denis //
// ”брать коментарии чтобы правильно работало //
						if (Hebrew || (PrevHebrewForLink && NeedHebrewTest)){
							if (PrevHebrew){
								cur_link.x2 = viewWidth()+leftMargin()-lit->XStart;
							} else {
								cur_link.x2 = viewWidth()+leftMargin()-cur_link.x1;
								cur_link.x1 = viewWidth()+leftMargin()-lit->XEnd;
							}
						} else {
							cur_link.x2 = lit->XEnd;
						}

//						cur_link.x2 = lit->XEnd;
// Denis //
						cur_link.y2 = y;
						cur_link.y1 = y - info.Height;

						xxx_page_links.push_back(cur_link);
						
						context().drawLine(cur_link.x1, cur_link.y2,  cur_link.x2, cur_link.y2);
					}

					cur_link.id.erase();

					link_not_terminated = false;

				}
			}
			
		}
		
	}


	if(!cur_link.id.empty() || link_not_terminated) {
	fprintf(stderr, "Denis - ZLTextView_paint.cpp - drawTextLine - empty3 - not support link_not_terminated = %d cur_link.id.empty() = %d\n", link_not_terminated, cur_link.id.empty());
		ZLTextElementIterator lit = it;
		if(lit != fromIt)
			lit--;

		ZLTextWordCursor pos = info.RealStart;		

		const ZLTextElement &element = paragraph[info.End.wordNumber()];
		const int y = lit->YEnd - myStyle.elementDescent(element) - myStyle.textStyle()->verticalShift();
		if(cur_link.id.empty()) {
			cur_link.x1 = fromIt->XStart;
		}

		cur_link.x2 = lit->XEnd;
		cur_link.y2 = y;
		cur_link.y1 = y - info.Height;
		cur_link.next = true;

		xxx_page_links.push_back(cur_link);	


		context().drawLine(cur_link.x1, cur_link.y2, cur_link.x2, cur_link.y2);

		link_not_terminated = true;
	}
	

	if (it != toIt) {
		if (it->ChangeStyle) {
			myStyle.setTextStyle(it->Style);
		}
		int start = 0;
		if (info.Start.equalWordNumber(info.End)) {
			start = info.Start.charNumber();
		}
		int len = info.End.charNumber() - start;

// Denis //
// ”брать коментарии чтобы правильно работало //
//		const ZLTextWord &word = (const ZLTextWord&)info.End.element();
		ZLTextWord &word = (ZLTextWord&)info.End.element();

		context().setColor(myStyle.textStyle()->color());
//		const int x = it->XStart;
//		const int y = it->YEnd - myStyle.elementDescent(word) - myStyle.textStyle()->verticalShift();
//		drawWord(x, y, word, start, len, it->AddHyphenationSign);
		const int xStart = it->XStart;
		const int xEnd = it->XEnd;
		const int y = it->YEnd - myStyle.elementDescent(word) - myStyle.textStyle()->verticalShift();
		drawWord(xStart, xEnd, y, word, start, len, it->AddHyphenationSign);
// Denis //
	}
	context().moveY(info.Descent + info.VSpaceAfter);
}

void ZLTextView::prepareTextLine(const ZLTextLineInfo &info) {
	myStyle.setTextStyle(info.StartStyle);
	const int y = std::min(context().y() + info.Height, topMargin() + textAreaHeight());
	int spaceCounter = info.SpaceCounter;
	int fullCorrection = 0;
	const bool endOfParagraph = info.End.isEndOfParagraph();
	bool wordOccured = false;
	bool changeStyle = true;

	context().moveXTo(leftMargin() + info.LeftIndent);

	switch (myStyle.textStyle()->alignment()) {
		case ALIGN_RIGHT:
			context().moveX(leftMargin() + viewWidth() - myStyle.textStyle()->rightIndent() - info.Width);
			break;
		case ALIGN_CENTER:
			context().moveX(leftMargin() + (viewWidth() - myStyle.textStyle()->rightIndent() - info.Width) / 2);
			break;
		case ALIGN_JUSTIFY:
			if (!endOfParagraph && (info.End.element().kind() != ZLTextElement::AFTER_PARAGRAPH_ELEMENT)) {
				fullCorrection = viewWidth() - myStyle.textStyle()->rightIndent() - info.Width;
			}
			break;
		case ALIGN_LEFT:
		case ALIGN_UNDEFINED:
			break;
	}

	const ZLTextParagraphCursor &paragraph = info.RealStart.paragraphCursor();
	int paragraphNumber = paragraph.index();
	for (ZLTextWordCursor pos = info.RealStart; !pos.equalWordNumber(info.End); pos.nextWord()) {
		const ZLTextElement &element = paragraph[pos.wordNumber()];
		ZLTextElement::Kind kind = element.kind();
		const int x = context().x();
		int width = myStyle.elementWidth(element, pos.charNumber());
	
		switch (kind) {
			case ZLTextElement::WORD_ELEMENT:
			case ZLTextElement::IMAGE_ELEMENT:
			{
				const int height = myStyle.elementHeight(element);
				const int descent = myStyle.elementDescent(element);
				const int length = (kind == ZLTextElement::WORD_ELEMENT) ? ((const ZLTextWord&)element).Length - pos.charNumber() : 0;
				myTextElementMap.push_back(
					ZLTextElementArea(
						paragraphNumber, pos.wordNumber(), pos.charNumber(), length, false,
						changeStyle, myStyle.textStyle(), kind,
						x, x + width - 1, y - height + 1, y + descent
					)
				);
				changeStyle = false;
				wordOccured = true;
				break;
			}
			case ZLTextElement::CONTROL_ELEMENT:
				myStyle.applyControl((const ZLTextControlElement&)element);
				changeStyle = true;
				break;
			case ZLTextElement::FORCED_CONTROL_ELEMENT:
				myStyle.applyControl((const ZLTextForcedControlElement&)element);
				changeStyle = true;
				break;
			case ZLTextElement::HSPACE_ELEMENT:
			case ZLTextElement::NB_HSPACE_ELEMENT:
				if (wordOccured && (spaceCounter > 0)) {
					int correction = fullCorrection / spaceCounter;
					context().moveX(context().spaceWidth() + correction);
					fullCorrection -= correction;
					wordOccured = false;
					--spaceCounter;
				}
				break;
			case ZLTextElement::INDENT_ELEMENT:
			case ZLTextElement::BEFORE_PARAGRAPH_ELEMENT:
			case ZLTextElement::AFTER_PARAGRAPH_ELEMENT:
			case ZLTextElement::EMPTY_LINE_ELEMENT:
			case ZLTextElement::FIXED_HSPACE_ELEMENT:
				break;
		}

		context().moveX(width);
	}
	if (!endOfParagraph && (info.End.element().kind() == ZLTextElement::WORD_ELEMENT)) {
		int start = 0;
		if (info.End.equalWordNumber(info.RealStart)) {
			start = info.RealStart.charNumber();
		}
		const int len = info.End.charNumber() - start;
		if (len > 0) {
			const ZLTextWord &word = (const ZLTextWord&)info.End.element();
			ZLUnicodeUtil::Ucs2String ucs2string;
			ZLUnicodeUtil::utf8ToUcs2(ucs2string, word.Data, word.Size);
			const bool addHyphenationSign = ucs2string[start + len - 1] != '-';
			const int x = context().x(); 
			const int width = myStyle.wordWidth(word, start, len, addHyphenationSign);
			const int height = myStyle.elementHeight(word);
			const int descent = myStyle.elementDescent(word);
			myTextElementMap.push_back(
				ZLTextElementArea(
					paragraphNumber, info.End.wordNumber(), start, len, addHyphenationSign,
					changeStyle, myStyle.textStyle(), ZLTextElement::WORD_ELEMENT,
					x, x + width - 1, y - height + 1, y + descent
				)
			);
		}
	}

	context().moveY(info.Height + info.Descent + info.VSpaceAfter);
}
