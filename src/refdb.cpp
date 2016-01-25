/*
* This file is part of Doxynum.
*
* Copyright (c) 2014 Sergey Vasyutin.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will `be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this software. If not, see <http://www.gnu.org/licenses/>.
*/

#include "refdb.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <sstream>
#include <fstream>

#ifdef _MSC_VER
	#pragma warning(disable: 4996) // VS warning: This function or variable may be unsafe
#endif

// -----------------------------------------------------------------------
struct TAutoFile {
public:
	TAutoFile(const char *FileName_, const char *Mode_): m_P(fopen(FileName_, Mode_)) {}
	~TAutoFile(void) {if(m_P) fclose(m_P);}
	
	FILE* file(void) const {return m_P;}
	
private:
	FILE *m_P;
	};

// -----------------------------------------------------------------------
TRefDB::TRefDB(void)
{
#define STRINGHACK (char*)
m_TitleFigureRE = v8regcomp(STRINGHACK"#(title|figure)[ \t\n\r]*[{]");
assert(m_TitleFigureNumberRE);

m_TitleFigureNumberRE = v8regcomp(STRINGHACK"#(title|figure|number|contents|image)[ \t\n\r]*[{]");
assert(m_TitleFigureNumberRE);
//
m_TitleContents = v8regcomp(STRINGHACK
	/* Name */ "[ \t\n\r]*" /**/ "([^| \t\n\r]+)" /**/ "[ \t\n\r]*" /**/ "[|]"
	/* ParentName */ "[ \t\n\r]*" /**/ "([^| \t\n\r]*)" /**/ "[ \t\n\r]*" /**/ "[|]"
	/* Doxygen Label */ "[ \t\n\r]*" /**/ "([^| \t\n\r]*)" /**/ "[ \t\n\r]*" /**/ "[|]"
	/* Text */ "[ \t\n\r]*" /**/ "([^}]+)" /**/ "[ \t\n\r]*" /**/ "[}]");
assert(m_TitleContents);
//
m_FigureContents = v8regcomp(STRINGHACK
	/* Name */ "[ \t\n\r]*" /**/ "([^| \t\n\r]+)" /**/ "[ \t\n\r]*" /**/ "[|]"
	/* Text */ "[ \t\n\r]*" /**/ "([^}]+)" /**/ "[ \t\n\r]*" /**/ "[}]");
assert(m_FigureContents);
//
m_NumberContents = v8regcomp(STRINGHACK
	/* Text */ "[ \t\n\r]*" /**/ "([^| \t\n\r]+)" /**/ "[ \t\n\r]*" /**/ "[}]");
assert(m_NumberContents);

m_ImageContents = v8regcomp(STRINGHACK
	/* FileName */ "[ \t\n\r]*" /**/ "([^| \t\n\r]+)" /**/ "[ \t\n\r]*" /**/ "[|]"
	/* Width in percents */ "[ \t\n\r]*" /**/ "([^}]+)" /**/ "[ \t\n\r]*" /**/ "[}]");
assert(m_NumberContents);

m_Titles.insert(std::make_pair(std::string(), TTiles::mapped_type()));
}

// -----------------------------------------------------------------------
TRefDB::~TRefDB(void)
{
free(m_TitleFigureRE);
free(m_TitleFigureNumberRE);
free(m_TitleContents);
free(m_FigureContents);
free(m_NumberContents);
free(m_ImageContents);
}

// -----------------------------------------------------------------------
bool TRefDB::previewFile(const std::string &FileName_)
{
FILE *Input = fopen(FileName_.c_str(), "rb");
if(!Input) {
	std::cerr << "Can't open file '" << FileName_ << "'." << std::endl;
	return false;
	}
//
long Size;
bool Fail = fseek(Input, 0, SEEK_END) != 0 || 
	((Size = ftell(Input)), (fseek(Input, 0, SEEK_SET) != 0));
if(Fail || !Size) {
	fclose(Input);
	if(Fail) {
		std::cerr << "Error reading file '" << FileName_ << "'." << std::endl;
		return false;
		}
	return true;
	}

char *Data = (char*)malloc(Size + 1);
Fail = (fread(Data, 1, Size, Input) != (size_t)Size);
fclose(Input);
if(Fail) {
	free(Data);
	std::cerr << "Error reading file '" << FileName_ << "'." << std::endl;
	return false;
	}

Data[Size] = (char)0;
char *Pos = Data, *End = Data + Size;
while(Pos < End) {
	if(!v8regexec(m_TitleFigureRE, Pos)) break;
	Pos = m_TitleFigureRE->endp[0];
	int Len = (*(m_TitleFigureRE->startp[1]) == 't')? previewTitle(Pos): previewFigure(Pos);
	if(!Len) {
		std::cerr << "Error in file '" << FileName_ << "'." << std::endl;
		return false;
		}
	Pos += Len;
	}

free(Data);
return true;
}

// -----------------------------------------------------------------------
int TRefDB::previewTitle(char *Pos_)
{
if(!v8regexec(m_TitleContents, Pos_)) 
	return 0;
//
std::string Name(m_TitleContents->startp[1], m_TitleContents->endp[1]);
std::pair<TTiles::iterator, bool> Pair = m_Titles.insert(
	std::make_pair(Name, TTiles::mapped_type()));
if(!Pair.second) {
	std::cerr << "The title '" << Name << "' has been described twice." << std::endl;
	return 0;
	}
//
TTitleElement &Element = Pair.first->second;
Element.ParentTitle.assign(m_TitleContents->startp[2], m_TitleContents->endp[2]);
Element.DoxygenLabel.assign(m_TitleContents->startp[3], m_TitleContents->endp[3]);
Element.Caption.assign(m_TitleContents->startp[4], m_TitleContents->endp[4]);
//
TTiles::iterator tit = m_Titles.find(Element.ParentTitle);
if(tit == m_Titles.end()) {
	std::cerr << "The parent title '" << Element.ParentTitle << "' for the title '" << Name << 
		"' was not described." << std::endl;
	return 0;
	}
tit->second.ChildrenTitles.push_back(Name);
//
m_CurrentTitle = Name;
return int(m_TitleContents->endp[0] - Pos_);
}

// -----------------------------------------------------------------------
int TRefDB::previewFigure(char *Pos_)
{
if(!v8regexec(m_FigureContents, Pos_)) 
	return 0;
//
std::string Name(m_FigureContents->startp[1], m_FigureContents->endp[1]);
std::pair<TFigures::iterator, bool> Pair = m_Figures.insert(
	std::make_pair(Name, TFigures::mapped_type()));
if(!Pair.second) {
	std::cerr << "The figure '" << Name << "' has been described twice." << std::endl;
	return 0;
	}
//
TFigureElement &Element = Pair.first->second;
Element.Caption.assign(m_FigureContents->startp[2], m_FigureContents->endp[2]);
//
if(m_CurrentTitle.empty()) {
	std::cerr << "The figure '" << Name << "' has no parent title." << std::endl;
	return 0;
	}

TTiles::iterator tit = m_Titles.find(m_CurrentTitle);
if(tit == m_Titles.end()) {
	std::cerr << "The parent title '" << m_CurrentTitle << "' for the figure '" << Name << 
		"' was not described." << std::endl;
	return 0;
	}
tit->second.Figures.push_back(Name);
return int(m_FigureContents->endp[0] - Pos_);
}

// -----------------------------------------------------------------------
bool TRefDB::makeNumbers(void)
{
struct THelper {
	static bool numerateChildren(TRefDB &This_, TTitleElement &Element_) {
		int Index = 1;
		char NumberBuffer[128];
		for(TChildren::iterator it = Element_.ChildrenTitles.begin(); 
			it != Element_.ChildrenTitles.end(); ++it, ++Index) {
			TTiles::iterator tit = This_.m_Titles.find(*it);
			assert(tit != This_.m_Titles.end());
			if(!Element_.Number.empty()) 
				tit->second.Number = Element_.Number + '.';
			sprintf(NumberBuffer, "%d", Index);
			tit->second.Number.append(NumberBuffer);
			if(!numerateChildren(This_, tit->second)) 
				return false;
			}
		//
		Index = 1;
		for(TChildren::iterator it = Element_.Figures.begin(); 
			it != Element_.Figures.end(); ++it, ++Index) {
			TFigures::iterator fit = This_.m_Figures.find(*it);
			assert(fit != This_.m_Figures.end());
			sprintf(NumberBuffer, "%s.%d", Element_.Number.c_str(), Index);
			fit->second.Number.append(NumberBuffer);
			}
		return true;
		}
	};
//
if(!THelper::numerateChildren(*this, m_Titles[std::string()])) 
	return false;
//
return true;
}

// -----------------------------------------------------------------------
bool TRefDB::save(const std::string &FileName_)
{
struct THelper {
	static bool write(const std::string &Value_, FILE *File_) {
		unsigned Size = (unsigned)Value_.size();
		if(fwrite(&Size, 1, sizeof(Size), File_) != sizeof(Size) ||
			fwrite(Value_.c_str(), 1, Size, File_) != Size) return false;
		return true;
		}
	//
	static bool write(const TChildren &Value_, FILE *File_) {
		unsigned Size = (unsigned)Value_.size();
		if(fwrite(&Size, 1, sizeof(Size), File_) != sizeof(Size)) 
			return false;
		//
		for(TChildren::const_iterator it = Value_.begin(); it != Value_.end(); ++it) {
			if(!write(*it, File_)) return false;
			}
		return true;
		}
	};

// ----
TAutoFile Out(FileName_.c_str(), "wb");
if(!Out.file()) return false;
//
unsigned Size = (unsigned)m_Titles.size();
if(fwrite(&Size, 1, sizeof(Size), Out.file()) != sizeof(Size)) return false;
for(TTiles::const_iterator it = m_Titles.begin(); it != m_Titles.end(); ++it) {
	if(!THelper::write(it->first, Out.file()) || 
		!THelper::write(it->second.ParentTitle, Out.file()) ||
		!THelper::write(it->second.DoxygenLabel, Out.file()) ||
		!THelper::write(it->second.Caption, Out.file()) ||
		!THelper::write(it->second.ChildrenTitles, Out.file()) ||
		!THelper::write(it->second.Figures, Out.file()) ||
		!THelper::write(it->second.Number, Out.file()))
		return false;
	}
//
Size = (unsigned)m_Figures.size();
if(fwrite(&Size, 1, sizeof(Size), Out.file()) != sizeof(Size)) return false;
for(TFigures::const_iterator it = m_Figures.begin(); it != m_Figures.end(); ++it) {
	/*std::cout << it->first << ": number: " <<
		it->second.Number << ", caption: " << it->second.Caption << std::endl;*/
	if(!THelper::write(it->first, Out.file()) || 
		!THelper::write(it->second.Caption, Out.file()) ||
		!THelper::write(it->second.Number, Out.file()))
		return false;
	}
return true;
}

// -----------------------------------------------------------------------
bool TRefDB::load(const std::string &FileName_)
{
struct THelper {
	static bool read(FILE *File_, std::string &Value_) {
		unsigned Size;
		if(fread(&Size, 1, sizeof(Size), File_) != sizeof(Size)) 
			return false;
		if(!Size) {
			Value_.clear();
			return true;
			}
		char *Data = (char*)malloc(Size);
		bool Success = fread(Data, 1, Size, File_) == Size;
		if(Success) 
			Value_.assign(Data, Data + Size);
		free(Data);
		return Success;
		}
	//
	static bool read(FILE *File_, TChildren &Value_) {
		unsigned Size;
		if(fread(&Size, 1, sizeof(Size), File_) != sizeof(Size)) 
			return false;
		Value_.clear();
		if(!Size) return true;
		//
		for(unsigned i = 0; i < Size; i++) {
			Value_.push_back(std::string());
			if(!read(File_, Value_.back())) return false;
			}
		return true;
		}
	};

// ----
TAutoFile In(FileName_.c_str(), "rb");
if(!In.file()) return false;
//
unsigned Size;
if(fread(&Size, 1, sizeof(Size), In.file()) != sizeof(Size)) 
	return false;
m_Titles.clear();
//
std::string Key;
for(unsigned i = 0; i < Size; i++) {
	if(!THelper::read(In.file(), Key)) return false;
	TTiles::iterator it = m_Titles.insert(std::make_pair(Key, TTiles::mapped_type())).first;
	TTitleElement &Element = it->second;
	if(!THelper::read(In.file(), Element.ParentTitle) ||
		!THelper::read(In.file(), Element.DoxygenLabel) ||
		!THelper::read(In.file(), Element.Caption) ||
		!THelper::read(In.file(), Element.ChildrenTitles) ||
		!THelper::read(In.file(), Element.Figures) ||
		!THelper::read(In.file(), Element.Number))
		return false;
	}
	
if(fread(&Size, 1, sizeof(Size), In.file()) != sizeof(Size)) 
	return false;
m_Figures.clear();
for(unsigned i = 0; i < Size; i++) {
	if(!THelper::read(In.file(), Key)) return false;
	TFigures::iterator it = m_Figures.insert(std::make_pair(Key, TFigures::mapped_type())).first;
	TFigureElement &Element = it->second;
	if(!THelper::read(In.file(), Element.Caption) ||
		!THelper::read(In.file(), Element.Number))
		return false;
	}
return true;
}

// -----------------------------------------------------------------------
int TRefDB::processFile(const std::string &FileName_)
{
FILE *Input = fopen(FileName_.c_str(), "rb");
if(!Input) {
	std::cerr << "Can't open file '" << FileName_ << "'." << std::endl;
	return 0;
	}
long Size;
bool Fail = fseek(Input, 0, SEEK_END) != 0 || 
	((Size = ftell(Input)), (fseek(Input, 0, SEEK_SET) != 0));
if(Fail || !Size) {
	fclose(Input);
	if(Fail) {
		std::cerr << "Error reading file '" << FileName_ << "'." << std::endl;
		return false;
		}
	return true;
	}

char *Data = (char*)malloc(Size + 1);
Fail = (fread(Data, 1, Size, Input) != (size_t)Size);
fclose(Input);
if(Fail) {
	free(Data);
	std::cerr << "Error reading file '" << FileName_ << "'." << std::endl;
	return false;
	}

std::ofstream *DebugStream = NULL;
if(!m_DebugOutputFolder.empty()) {
	std::string DebugFileName = m_DebugOutputFolder + "doxynum_";
	std::string::size_type LastOf = FileName_.find_last_of('/');
	if(LastOf == std::string::npos)
		DebugFileName += FileName_;
	else {
		DebugFileName += (FileName_.c_str() + (LastOf + 1));
		}
	DebugStream = new std::ofstream(DebugFileName.c_str());
	if(DebugStream->bad()) {
		delete DebugStream;
		DebugStream = NULL;
		}
	}

Data[Size] = (char)0;
char *Pos = Data, *End = Pos + Size;
while(Pos < End) {
	if(!v8regexec(m_TitleFigureNumberRE, Pos)) break;
	*(m_TitleFigureNumberRE->startp[0]) = char(0);
	std::cout << Pos;
	if(DebugStream) *DebugStream << Pos;
	//
	Pos = m_TitleFigureNumberRE->endp[0];
	int Len;
	switch(*(m_TitleFigureNumberRE->startp[1])) {
	case 't': Len = processTitle(Pos, DebugStream); break;
	case 'n': Len = processNumber(Pos, DebugStream); break;
	case 'f': Len = processFigure(Pos, DebugStream); break;
	case 'c': Len = processContents(Pos, DebugStream); break;
	case 'i': Len = processImage(Pos, DebugStream); break;
		}
	if(!Len) {
		free(Data);
		std::cerr << "Error in stream: '" << Pos << "'" << std::endl;
		return 1;
		}
	Pos += Len;
	}
std::cout << Pos;
if(DebugStream) *DebugStream << Pos;
//
if(DebugStream) delete DebugStream;
free(Data);
return 0;
}

// -----------------------------------------------------------------------
int TRefDB::processTitle(char *Pos_, std::ostream *DebugStream_)
{
if(!v8regexec(m_TitleContents, Pos_)) 
	return 0;
//
std::string Name(m_TitleContents->startp[1], m_TitleContents->endp[1]);
TTiles::const_iterator it = m_Titles.find(Name);
if(it == m_Titles.end()) return 0;
const TTitleElement &Element = it->second;
if(!Element.DoxygenLabel.empty()) {
	std::cout << Element.DoxygenLabel << ' ';
	if(DebugStream_) *DebugStream_ << Element.DoxygenLabel << ' ';
	}
std::cout << Element.Number << ".&nbsp;" << Element.Caption;
if(DebugStream_) *DebugStream_ << Element.Number << ".&nbsp;" << Element.Caption;
//
return int(m_TitleContents->endp[0] - Pos_);
}

// -----------------------------------------------------------------------
int TRefDB::processFigure(char *Pos_, std::ostream *DebugStream_)
{
if(!v8regexec(m_FigureContents, Pos_)) 
	return 0;
//
std::string Name(m_FigureContents->startp[1], m_FigureContents->endp[1]);
TFigures::const_iterator it = m_Figures.find(Name);
if(it == m_Figures.end()) return 0;
const TFigureElement &Element = it->second;
//
std::cout << Element.Number << ".&nbsp;" << Element.Caption;
if(DebugStream_) *DebugStream_ << Element.Number << ".&nbsp;" << Element.Caption;
//
return int(m_FigureContents->endp[0] - Pos_);
}

// -----------------------------------------------------------------------
int TRefDB::processNumber(char *Pos_, std::ostream *DebugStream_)
{
if(!v8regexec(m_NumberContents, Pos_)) 
	return 0;
//
std::string Name(m_NumberContents->startp[1], m_NumberContents->endp[1]);
TTiles::const_iterator it = m_Titles.find(Name);
if(it != m_Titles.end()) {
	const TTitleElement &Element = it->second;
	std::cout << Element.Number;
	if(DebugStream_) *DebugStream_ << Element.Number;
	}
else {
	TFigures::const_iterator fit = m_Figures.find(Name);
	if(fit == m_Figures.end()) return 0;
	const TFigureElement &Element = fit->second;
	std::cout << Element.Number;
	if(DebugStream_) *DebugStream_ << Element.Number;
	}
return int(m_NumberContents->endp[0] - Pos_);
}

// -----------------------------------------------------------------------
int TRefDB::processContents(char *Pos_, std::ostream *DebugStream_)
{
struct THelper {
	static bool listChildren(const TRefDB &This_, const TTitleElement &Element_, int CurrentLevel_,
		int MaxLevel_, std::ostream *DebugStream_) {
		if(CurrentLevel_ > MaxLevel_) return true;
		//
		for(TChildren::const_iterator it = Element_.ChildrenTitles.begin(); 
			it != Element_.ChildrenTitles.end(); ++it) {
			//
			TTiles::const_iterator tit = This_.m_Titles.find(*it);
			assert(tit != This_.m_Titles.end());
			const TTitleElement &iElement = tit->second;
			
			std::cout << iElement.Number << ". \\link " << iElement.DoxygenLabel << '\n' <<
				iElement.Caption << "\n\\endlink \\n\n\n";
				
			if(DebugStream_) {
				*DebugStream_ << iElement.Number << ". \\link " << iElement.DoxygenLabel << '\n' << 
					iElement.Caption << "\n\\endlink \\n\n\n";
				}
			if(!iElement.ChildrenTitles.empty()) {
				if(!listChildren(This_, iElement, CurrentLevel_ + 1, MaxLevel_, DebugStream_)) 
					return false;
				}
			}
		return true;		
		}
	};

if(!v8regexec(m_NumberContents, Pos_)) 
	return 0;
//
std::string LevelString(m_NumberContents->startp[1], m_NumberContents->endp[1]);
int Level = atoi(LevelString.c_str());
//
THelper::listChildren(*this, m_Titles[std::string()], 1, Level, DebugStream_);
return int(m_NumberContents->endp[0] - Pos_);
}

// -----------------------------------------------------------------------
int TRefDB::processImage(char *Pos_, std::ostream *DebugStream_)
{
if(!v8regexec(m_ImageContents, Pos_)) 
	return 0;
//
std::string FileName(m_ImageContents->startp[1], m_ImageContents->endp[1]);

std::cout << "\\image html " << FileName << "\n"
	"\\image rtf " << FileName;
//
if(DebugStream_) 
	*DebugStream_ << "\\image html " << FileName << "\n\\image rtf " << FileName;

return int(m_ImageContents->endp[0] - Pos_);
}

// -----------------------------------------------------------------------
void TRefDB::setDebugOutputFolder(const std::string &DebugOutputFolder_)
{
#ifdef _WIN32
	const char Separator = '\\';
#else
	const char Separator = '/';
#endif
//
if(DebugOutputFolder_.empty()) return;
//
m_DebugOutputFolder = DebugOutputFolder_;
char LastChar = m_DebugOutputFolder[m_DebugOutputFolder.size() - 1];
if(LastChar != '\\' && LastChar != '/')
	m_DebugOutputFolder += Separator;
}