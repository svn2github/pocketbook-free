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

#include <stdlib.h>

#include <cctype>

#include <ZLFile.h>
#include <ZLInputStream.h>

#include "RtfReader.h"

std::map<std::string, RtfCommand*> RtfReader::ourKeywordMap;

static const int rtfStreamBufferSize = 4096;

RtfReader::RtfReader(const std::string &encoding) : EncodedTextReader(encoding) {
}

RtfReader::~RtfReader() {
}

RtfCommand::~RtfCommand() {
}

void RtfNewParagraphCommand::run(RtfReader &reader, int*) const {
	reader.newParagraph();
}

RtfFontPropertyCommand::RtfFontPropertyCommand(RtfReader::FontProperty property) : myProperty(property) {
}

void RtfFontPropertyCommand::run(RtfReader &reader, int *parameter) const {
	bool start = (parameter == 0) || (*parameter != 0);
	switch (myProperty) {
		case RtfReader::FONT_BOLD:
			if (reader.myState.Bold != start) {
				reader.myState.Bold = start;
				reader.setFontProperty(RtfReader::FONT_BOLD);
			}
			break;
		case RtfReader::FONT_ITALIC:
			if (reader.myState.Italic != start) {
				reader.myState.Italic = start;
				reader.setFontProperty(RtfReader::FONT_ITALIC);
			}
			break;
		case RtfReader::FONT_UNDERLINED:
			if (reader.myState.Underlined != start) {
				reader.myState.Underlined = start;
				reader.setFontProperty(RtfReader::FONT_UNDERLINED);
			}
			break;
	}
}

RtfAlignmentCommand::RtfAlignmentCommand(ZLTextAlignmentType alignment) : myAlignment(alignment) {
}

void RtfAlignmentCommand::run(RtfReader &reader, int*) const {
	if (reader.myState.Alignment != myAlignment) {
		reader.myState.Alignment = myAlignment;
		reader.setAlignment();
	}
}

RtfCharCommand::RtfCharCommand(const std::string &chr) : myChar(chr) {
}

void RtfCharCommand::run(RtfReader &reader, int*) const {
	reader.processCharData(myChar.data(), myChar.length(), false);
}

RtfDestinationCommand::RtfDestinationCommand(RtfReader::DestinationType destination) : myDestination(destination) {
}

void RtfDestinationCommand::run(RtfReader &reader, int*) const {
	if (reader.myState.Destination == myDestination) {
		return;
	}
	reader.myState.Destination = myDestination;
	if (myDestination == RtfReader::DESTINATION_PICTURE) {
		reader.myState.ReadDataAsHex = true;
	}
	reader.switchDestination(myDestination, true);
}

void RtfStyleCommand::run(RtfReader &reader, int*) const {
	if (reader.myState.Destination == RtfReader::DESTINATION_STYLESHEET) {
		//std::cerr << "Add style index: " << val << "\n";
		
		//sprintf(style_attributes[0], "%i", val);
	} else /*if (myState.Destination == rdsContent)*/ {
		//std::cerr << "Set style index: " << val << "\n";

		//sprintf(style_attributes[0], "%i", val);
	}
}

void RtfCodepageCommand::run(RtfReader &reader, int *parameter) const {
	if (parameter != 0) {
		reader.setEncoding(*parameter);
	}
}

void RtfSpecialCommand::run(RtfReader &reader, int*) const {
	reader.mySpecialMode = true;
}

RtfPictureCommand::RtfPictureCommand(const std::string &mimeType) : myMimeType(mimeType) {
}

void RtfPictureCommand::run(RtfReader &reader, int*) const {
	reader.myNextImageMimeType = myMimeType;
}

void RtfFontResetCommand::run(RtfReader &reader, int*) const {
	if (reader.myState.Bold) {
		reader.myState.Bold = false;
		reader.setFontProperty(RtfReader::FONT_BOLD);
	}
	if (reader.myState.Italic) {
		reader.myState.Italic = false;
		reader.setFontProperty(RtfReader::FONT_ITALIC);
	}
	if (reader.myState.Underlined) {
		reader.myState.Underlined = false;
		reader.setFontProperty(RtfReader::FONT_UNDERLINED);
	}
}

void RtfReader::addAction(const std::string &tag, RtfCommand *command) {
	ourKeywordMap.insert(std::pair<std::string,RtfCommand*>(tag, command));
}

void RtfReader::fillKeywordMap() {
	if (ourKeywordMap.empty()) {
		addAction("*",	new RtfSpecialCommand());
		addAction("ansicpg",	new RtfCodepageCommand());

		static const char *keywordsToSkip[] = {"buptim", "colortbl", "comment", "creatim", "doccomm", "fonttbl", "footer", "footerf", "footerl", "footerr", "ftncn", "ftnsep", "ftnsepc", "header", "headerf", "headerl", "headerr", "keywords", "operator", "printim", "private1", "revtim", "rxe", "subject", "tc", "txe", "xe", 0};
		RtfCommand *skipCommand = new RtfDestinationCommand(RtfReader::DESTINATION_SKIP);
		for (const char **i = keywordsToSkip; *i != 0; ++i) {
			addAction(*i,	skipCommand);
		}
		addAction("info",	new RtfDestinationCommand(RtfReader::DESTINATION_INFO));
		addAction("title",	new RtfDestinationCommand(RtfReader::DESTINATION_TITLE));
		addAction("author",	new RtfDestinationCommand(RtfReader::DESTINATION_AUTHOR));
		addAction("pict",	new RtfDestinationCommand(RtfReader::DESTINATION_PICTURE));
		addAction("stylesheet",	new RtfDestinationCommand(RtfReader::DESTINATION_STYLESHEET));
		addAction("footnote",	new RtfDestinationCommand(RtfReader::DESTINATION_FOOTNOTE));

		RtfCommand *newParagraphCommand = new RtfNewParagraphCommand();
		addAction("\n",	newParagraphCommand);
		addAction("\r",	newParagraphCommand);
		addAction("par",	newParagraphCommand);

		addAction("\x09",	new RtfCharCommand("\x09"));
		addAction("_",	new RtfCharCommand("-"));
		addAction("\\",	new RtfCharCommand("\\"));
		addAction("{",	new RtfCharCommand("{"));
		addAction("}",	new RtfCharCommand("}"));
		addAction("bullet",	new RtfCharCommand("\xE2\x80\xA2"));		 // &bullet;
		addAction("endash",	new RtfCharCommand("\xE2\x80\x93"));		 // &ndash;
		addAction("emdash",	new RtfCharCommand("\xE2\x80\x94"));		 // &mdash;
		addAction("~",	new RtfCharCommand("\xC0\xA0"));							// &nbsp;
		addAction("enspace",	new RtfCharCommand("\xE2\x80\x82"));		// &emsp;
		addAction("emspace",	new RtfCharCommand("\xE2\x80\x83"));		// &ensp;
		addAction("lquote",	new RtfCharCommand("\xE2\x80\x98"));		 // &lsquo;
		addAction("rquote",	new RtfCharCommand("\xE2\x80\x99"));		 // &rsquo;
		addAction("ldblquote",	new RtfCharCommand("\xE2\x80\x9C"));	// &ldquo;
		addAction("rdblquote",	new RtfCharCommand("\xE2\x80\x9D"));	// &rdquo;

		addAction("jpegblip",	new RtfPictureCommand("image/jpeg"));
		addAction("pngblip",	new RtfPictureCommand("image/png"));

		addAction("s",	new RtfStyleCommand());

		addAction("qc",	new RtfAlignmentCommand(ALIGN_CENTER));
		addAction("ql",	new RtfAlignmentCommand(ALIGN_LEFT));
		addAction("qr",	new RtfAlignmentCommand(ALIGN_RIGHT));
		addAction("qj",	new RtfAlignmentCommand(ALIGN_JUSTIFY));
		addAction("pard",	new RtfAlignmentCommand(ALIGN_UNDEFINED));

		addAction("b",	new RtfFontPropertyCommand(RtfReader::FONT_BOLD));
		addAction("i",	new RtfFontPropertyCommand(RtfReader::FONT_ITALIC));
		addAction("u",	new RtfFontPropertyCommand(RtfReader::FONT_UNDERLINED));
		addAction("plain",	new RtfFontResetCommand());
	}
}

bool RtfReader::parseDocument() {
	enum {
		READ_NORMAL_DATA,
		READ_BINARY_DATA,
		READ_HEX_SYMBOL,
		READ_KEYWORD,
		READ_KEYWORD_PARAMETER
	} parserState = READ_NORMAL_DATA;

	char keywordbuf[64], parambuf[64];
	int keywordpos=0, parampos=0;

	int imageStartOffset = -1;
	char hsbuf[4];
	int hspos=0;

	while (!myIsInterrupted) {
		const char *ptr = myStreamBuffer;
		const char *end = myStreamBuffer + myStream->read(myStreamBuffer, rtfStreamBufferSize);
		if (ptr == end) {
			break;
		}
		const char *dataStart = ptr;
		bool readNextChar = true;
		while (ptr != end) {
			switch (parserState) {
				case READ_BINARY_DATA:
					// TODO: optimize
					processCharData(ptr, 1);
					--myBinaryDataSize;
					if (myBinaryDataSize == 0) {
						parserState = READ_NORMAL_DATA;
					}
					break;
				case READ_NORMAL_DATA:
					switch (*ptr) {
						case '{':
							if (ptr > dataStart) {
								processCharData(dataStart, ptr - dataStart);
							}
							dataStart = ptr + 1;
							myStateStack.push(myState);
							myState.ReadDataAsHex = false;
							break;
						case '}':
						{
							if (ptr > dataStart) {
								processCharData(dataStart, ptr - dataStart);
							}
							dataStart = ptr + 1;

							if (imageStartOffset >= 0) {
								int imageSize = myStream->offset() + (ptr - end) - imageStartOffset;
								insertImage(myNextImageMimeType, myFileName, imageStartOffset, imageSize);
								imageStartOffset = -1;
							}

							if (myStateStack.empty()) {
								return false;
							}
							
							if (myState.Destination != myStateStack.top().Destination) {
								switchDestination(myState.Destination, false);
								switchDestination(myStateStack.top().Destination, true);
							}
							
							bool oldItalic = myState.Italic;
							bool oldBold = myState.Bold;
							bool oldUnderlined = myState.Underlined;
							ZLTextAlignmentType oldAlignment = myState.Alignment;
							myState = myStateStack.top();
							myStateStack.pop();
					
							if (myState.Italic != oldItalic) {
								setFontProperty(RtfReader::FONT_ITALIC);
							}
							if (myState.Bold != oldBold) {
								setFontProperty(RtfReader::FONT_BOLD);
							}
							if (myState.Underlined != oldUnderlined) {
								setFontProperty(RtfReader::FONT_UNDERLINED);
							}
							if (myState.Alignment != oldAlignment) {
								setAlignment();
							}
							
							break;
						}
						case '\\':
							if (ptr > dataStart) {
								processCharData(dataStart, ptr - dataStart);
							}
							dataStart = ptr + 1;
							parserState = READ_KEYWORD;
							keywordpos = 0;
							break;
						case 0x0d:
						case 0x0a:			// cr and lf are noise characters...
							if (ptr > dataStart) {
								processCharData(dataStart, ptr - dataStart);
							}
							dataStart = ptr + 1;
							break;
						default:
							if (myState.ReadDataAsHex) {
								if (imageStartOffset == -1) {
									imageStartOffset = myStream->offset() + (ptr - end);
								}
							}
							break;
					}
					break;
				case READ_HEX_SYMBOL:
					hsbuf[hspos++] = *ptr;
					if (hspos == 2) {
						hsbuf[hspos] = 0;
						char ch = strtol(hsbuf, 0, 16);
						processCharData(&ch, 1);
						parserState = READ_NORMAL_DATA;
						dataStart = ptr + 1;
					}
					break;
				case READ_KEYWORD:
					if (!isalpha(*ptr)) {
						if ((ptr == dataStart) && (keywordpos == 0)) {
							if (*ptr == '\'') {
								parserState = READ_HEX_SYMBOL;
								hspos = 0;
							} else {
								keywordbuf[0] = *ptr;
								keywordbuf[1] = 0;
								processKeyword(keywordbuf);
								parserState = READ_NORMAL_DATA;
							}
							dataStart = ptr + 1;
						} else {
							if (keywordpos + (ptr - dataStart) > 63) {
								parserState = READ_NORMAL_DATA;
								break;
							}
							strncpy(keywordbuf+keywordpos, dataStart, ptr - dataStart);
							keywordpos += (ptr - dataStart);
							keywordbuf[keywordpos] = 0;
							if ((*ptr == '-') || isdigit(*ptr)) {
								dataStart = ptr;
								parserState = READ_KEYWORD_PARAMETER;
								parampos = 0;
							} else {
								readNextChar = (*ptr == ' ');
								processKeyword(keywordbuf);
								parserState = READ_NORMAL_DATA;
								dataStart = readNextChar ? ptr + 1 : ptr;
							}
						}
					}
					break;
				case READ_KEYWORD_PARAMETER:
					if (!isdigit(*ptr)) {
						memcpy(parambuf+parampos, dataStart, ptr - dataStart);
						parampos += (ptr - dataStart);
						parambuf[parampos] = 0;
						int parameter = atoi(parambuf);
						readNextChar = (*ptr == ' ');
						if ((strcmp(keywordbuf, "bin") == 0) && (parameter > 0)) {
							myBinaryDataSize = parameter;
							parserState = READ_BINARY_DATA;
						} else {
							processKeyword(keywordbuf, &parameter);
							parserState = READ_NORMAL_DATA;
						}
						dataStart = readNextChar ? ptr + 1 : ptr;
					}
					break;
			}
			if (readNextChar) {
				++ptr;
			} else {
				readNextChar = true;
			}
		}
		if (dataStart < end) {
			switch (parserState) {
				case READ_NORMAL_DATA:
					processCharData(dataStart, end - dataStart);
					break;
				case READ_KEYWORD:
					memcpy(keywordbuf+keywordpos, dataStart, end - dataStart);
					keywordpos += (end - dataStart);
					break;
				case READ_KEYWORD_PARAMETER:
					memcpy(parambuf+parampos, dataStart, end - dataStart);
					parampos += (end - dataStart);
					break;
				default:
					break;
			}
		}
	}
	
	return myIsInterrupted || myStateStack.empty();
}

void RtfReader::processKeyword(const char *keyword, int *parameter) {
	bool wasSpecialMode = mySpecialMode;
	mySpecialMode = false;
	if (myState.Destination == RtfReader::DESTINATION_SKIP) {
		return;
	}

	std::map<std::string, RtfCommand*>::const_iterator it = ourKeywordMap.find(keyword);
	
	if (strcmp(keyword, "shppict") == 0) wasSpecialMode = false;

	if (it == ourKeywordMap.end()) {
		if (wasSpecialMode || strcmp(keyword, "nonshppict") == 0)
			myState.Destination = RtfReader::DESTINATION_SKIP;
		return;
	}

	it->second->run(*this, parameter);
}


inline void RtfReader::processCharData(const char *data, size_t len, bool convert) {
	if (myState.Destination != RtfReader::DESTINATION_SKIP) {
		addCharData(data, len, convert);
	}
}

void RtfReader::interrupt() {
	myIsInterrupted = true;
}

bool RtfReader::readDocument(const std::string &fileName) {
	myFileName = fileName;
	myStream = ZLFile(fileName).inputStream();
	if (myStream.isNull() || !myStream->open()) {
			return false;
	}

	fillKeywordMap();

	myStreamBuffer = new char[rtfStreamBufferSize];
	
	myIsInterrupted = false;

	mySpecialMode = false;

	myState.Alignment = ALIGN_UNDEFINED;
	myState.Italic = false;
	myState.Bold = false;
	myState.Underlined = false;
	myState.Destination = RtfReader::DESTINATION_NONE;
	myState.ReadDataAsHex = false;

	bool code = parseDocument();

	while (!myStateStack.empty()) {
		myStateStack.pop();
	}
	
	delete[] myStreamBuffer;
	myStream->close();
	
	return code;
}
