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

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

#include <assert.h>

#ifndef _WIN32
	#include <strings.h>
#endif

// -----------------------------------------------------------------------
int Usage(void)
{
std::cout << 
"Doxynum 0.1." << (int)

#include "doxynum_version.h"

<< " is a program for automatic numbering of sections and drawings,\n"
"as well as for creating contents in documentation, generated using the\n"
"doxygen software (www.doxygen.org). The author is Sergey Vasyutin\n"
"(svpro [at] outlook.com).\n"
"\n"                                                                                    
"During the pre-processing the program starts from the command line using\n"
"the following arguments:\n"
"\n"
"doxynum -l <file with the list of files to be processed> \\\n"
"   -d <database of sections and drawings> \\\n"
"   [-i <list of source files for Doxygen configuration file>]\n"
"\n"
"-l <file with the list of files to be processed> - name of the files,\n"
"   each line of which contains the name of the source file for\n"
"   pre-processing. Determines the order of processing files, which\n"
"   influences the numbering of sections (sections, who came first will\n"
"   receive smaller number).\n"
"\n"
"-d <database of sections and drawings> - database with numbers of\n"
"   sections and drawings, gained as the result of pre-processing. Is\n"
"	used during the filtering stage.\n"
"\n"
"-i <list of source files for Doxygen configuration file> - name of the\n"
"   file, into which we will write list of source files in a format of\n"
"   INPUT directive of the configuration file for Doxygen (optional\n"
"   parameter). The text of form 'INPUT=<list of files>' is written to\n"
"   the file. It can be added to the Doxygen configuration file to\n"
"   specify the list of files to be processes in the specified order\n"
"   (see how the argument is use in the example, provided with the\n"
"   program).\n"
"\n"
"In the filtering mode the program is launched by Doxygen, each time\n"
"passing it the name of the source file to be processed. To start the\n"
"program in this mode, you must add the the specified line to the Doxygen\n"
"configuration file (this will tell Doxygen to use Doxynum as a filter).\n"
"\n"
"INPUT_FILTER = \"doxynum -d <database of sections and drawings> \\\n"
"   [-o <directory for debugging>] -f\"\n"
"\n"
"-d <database of sections and drawings> - database with numbers of\n"
"   sections and drawings, gained as the result of pre-processing stage.\n"
"\n"
"-o <directory for debugging> - directory, to which saved the results of\n"
"   filtration for each of the processed files (optional parameter). Can\n"
"   be used for debugging purposes to view what gets into Doxygen from\n"
"   the output of Doxynum.\n"
"\n"
"Sample scripts and configuration files for Doxygen, used to launch\n"
"Doxynum, are supplied with the program.\n";

return 1;
}

// -----------------------------------------------------------------------
bool g_PreviewMode = false;
std::string g_FilesList, g_DataBase, g_InputFile, g_GeneratedFileList, g_DebugOutputDirectory;

// -----------------------------------------------------------------------
bool ParseFileList(std::vector<std::string> &FileList_)
{
assert(FileList_.empty());
std::ifstream FilesStream(g_FilesList.c_str());
if(FilesStream.fail()) {
	std::cerr << "Can't open file '" << g_FilesList << "'." << std::endl;
	return false;
	}
std::string FileName;
while(std::getline(FilesStream, FileName)) {
	if(!FileName.empty() && FileName[0] == '#') continue;
	//
	std::string::iterator EndOfCR = std::remove(FileName.begin(), FileName.end(), '\r');
	if(EndOfCR != FileName.end())
		FileName.erase(EndOfCR, FileName.end());
	//
	if(FileName.empty()) continue;
	FileList_.push_back(FileName);
   }
if(FileList_.empty()) {
	std::cerr << "File list '" << g_FilesList << "' contains no files." << std::endl;
	return false;
	}
return true;
}

// -----------------------------------------------------------------------
int RunPreviewMode(void)
{
std::vector<std::string> FileList;
if(!ParseFileList(FileList)) 
	return 1;
//
if(!g_GeneratedFileList.empty()) {
	std::ofstream GeneratedFile(g_GeneratedFileList.c_str());
	if(GeneratedFile.fail()) {
		std::cerr << "Can't open file '" << GeneratedFile << "'." << std::endl;
		return false;
		}
	GeneratedFile << "INPUT = ";
	for(std::vector<std::string>::const_iterator it = FileList.begin(); it != FileList.end(); 
		++it) {
		if(it != FileList.begin()) GeneratedFile << " \\\n\t";
		GeneratedFile << *it;
		if(GeneratedFile.fail()) {
			std::cerr << "Can't write file '" << GeneratedFile << "'." << std::endl;
			return false;
			}
		}
	}
//
TRefDB DB;	
for(std::vector<std::string>::const_iterator it = FileList.begin(); it != FileList.end(); ++it) {
	if(!DB.previewFile(*it)) return 1;
	}
if(!DB.makeNumbers()) {
	std::cerr << "Can't numerate titles and figures." << std::endl;
	return 1;
	}
if(!DB.save(g_DataBase)) {
	std::cerr << "Error writing preview DB file '" << g_DataBase << "'." << std::endl;
	return 1;
	}
return 0;
}

// -----------------------------------------------------------------------
int RunFilterMode(void)
{
TRefDB DB;
if(!DB.load(g_DataBase)) {
	std::cerr << "Error loading preview DB file '" << g_DataBase << "'." << std::endl;
	return 1;
	}
if(!g_DebugOutputDirectory.empty())
	DB.setDebugOutputFolder(g_DebugOutputDirectory);
	
return DB.processFile(g_InputFile);
}

// -----------------------------------------------------------------------
int main(int Argv_, char **Argc_)
{
#ifdef _WIN32
	#define STRCMPI _strcmpi
#else
	#define STRCMPI strcasecmp
#endif

for(int i = 1; i < Argv_; ++i) {
	if(!STRCMPI(Argc_[i], "-l") && (++i) < Argv_) {
		if(!g_FilesList.empty()) 
			return Usage();
		g_PreviewMode = true;
		g_FilesList = Argc_[i];
		}
	else if(!STRCMPI(Argc_[i], "-f") && (++i) < Argv_) {
		if(!g_InputFile.empty())
			return Usage();
		g_PreviewMode = false;
		g_InputFile = Argc_[i];
		}
	else if(!STRCMPI(Argc_[i], "-d") && (++i) < Argv_) {
		if(!g_DataBase.empty()) 
			return Usage();
		g_DataBase = Argc_[i];
		}
	else if(!STRCMPI(Argc_[i], "-i") && (++i) < Argv_) {
		g_GeneratedFileList = Argc_[i];
		}
	else if(!STRCMPI(Argc_[i], "-o") && (++i) < Argv_) {
		g_DebugOutputDirectory = Argc_[i];
		}
	else 
		return Usage();
	} 
// Checking for the invalid combination of the options.
if(g_DataBase.empty() || 
	(g_PreviewMode && g_FilesList.empty()) ||
	(!g_PreviewMode && g_InputFile.empty()) ||
	(!g_GeneratedFileList.empty() && !g_PreviewMode)) 
	return Usage();

return g_PreviewMode? RunPreviewMode(): RunFilterMode();
}
