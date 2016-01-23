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

#ifndef __refdb_h
#define __refdb_h

#include <vector>
#include <string>
#include <map>
#include <iostream>

#include "v8regexp.h"

// -----------------------------------------------------------------------
class TRefDB {
public:
	TRefDB(void);
	~TRefDB(void);
	
	bool previewFile(const std::string &FileName_);
	bool makeNumbers(void);
	bool save(const std::string &FileName_);
	
	bool load(const std::string &FileName_);
	int processFile(const std::string &FileName_);
	
	void setDebugOutputFolder(const std::string &DebugOutputFolder_);

private:
	typedef std::vector<std::string> TChildren;
	struct TTitleElement {
		std::string ParentTitle;
		std::string DoxygenLabel; // The doxygen label for the chapter.
		std::string Caption;
		TChildren ChildrenTitles; // The children of the chapter.
		TChildren Figures;        // The list of figures in the chapter.
		std::string Number;
		};
	typedef std::map<std::string, TTitleElement> TTiles;
	TTiles m_Titles;

	// ----
	struct TFigureElement {
		std::string Caption;
		std::string Number;
		};
	typedef std::map<std::string, TFigureElement> TFigures;
	TFigures m_Figures;
	
	std::string m_CurrentTitle; // Current title used for generating figures numbers.
	//
	regexp *m_TitleFigureNumberRE, *m_TitleFigureRE, *m_TitleContents, *m_FigureContents, 
		*m_NumberContents, *m_ImageContents;
	
	std::string m_DebugOutputFolder;
	
	int previewTitle(char *Pos_);
	int previewFigure(char *Pos_);
	//
	int processTitle(char *Pos_, std::ostream *DebugStream_);
	int processFigure(char *Pos_, std::ostream *DebugStream_);
	int processNumber(char *Pos_, std::ostream *DebugStream_);
	int processContents(char *Pos_, std::ostream *DebugStream_);
	int processImage(char *Pos_, std::ostream *DebugStream_);
	};

#endif // #ifndef __refdb_h