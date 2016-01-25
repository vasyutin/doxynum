# Doxynum
Doxynum is a program for automatic numbering of sections and drawings, as well as for creating contents in documentation, generated using the Doxygen software (www.doxygen.org).

## 1. Introduction

Doxygen is a powerful tool for the development of documentation. It is used in many projects for preparation of API descriptions and manuals for programmers. However, it doesn't have the function of auto numbering of sections and drawings, creating contents for the sections that are located in different files.

These features are almost not necessary when preparing API descriptions. However, in many cases it is convenient to use Doxygen for the creation of electronic documents of other types, for example, user manuals, descriptions of products and others. It is associated with ability of Doxygen to support powerful features of formatting documents, which are well-known among programmers and used in the process of preparing API descriptions. The use of Doxygen for writing any other project documentation can make life of developers a little easier in case when the documentation and code are written by the same people. In this case the auto numbering of sections and drawings, which currently are not presented in Doxygen, will be very useful.


## 2. Operation principle

The usage of the program consists of two phases: pre-processing and filtering. During the pre-processing stage the program searches sections and images in source files. Then it numbers them and writes the numbers in a special file, which is used during filtration stage. 

At the stage of filtration Doxygen filters the contents of all processed files through Doxynum program. At the same time the program adds to the text numbers of sections and drawings from the file received at the first stage. Also, if it was set, inserts the table of contents, containing the sections titles up to a specified level of hierarchy.


## 3. Tags

To ensure that the program is able to find sections and images in text, we use special tags: #title, #figure. To insert the link with the number of section or drawing into text, we apply the tag #number. For easier insertion of images into text, use the tag #image.

The tag is accompanied by one or more parameters, which are followed in curly brackets and separated by vertical line, for example,

```
#title{main_section|top_section|main_section_ref|Main section}
```

### 3.1. Tag #title

Tag #title is used to denote the header section. The tag is accompanied by the following parameters:
1) The mark of section for references to it in tags, for example, in tag #number.
2) The mark of the parent section (section of an upper level, which includes the current section). If described section of upper level of the hierarchy, this parameter is left blank.
3) The mark, which is used in Doxygen's tags \link for the cross-reference to the section. Also the tag is used to create a reference to the section in the table of contents.
4) Name of section in the text.

When processing text instead of tag #title Doxynum inserts section number and its name.

### 3.2. Tag #figure

Tag #figure is used to denote caption. The tag is accompanied by two parameters:
1) The mark of image for references to it in the tag #number.
2) Caption text.

When processing text instead of tag #figure Doxynum inserts figure number and its name.

### 3.3. Tag #number

The tag #number is used to insert section or drawing number into the text. The tag uses only one parameter - the section or picture tag. When processing text instead of tag #number Doxynum inserts number of specified section of drawing.

### 3.4. Tag #toc

Tag #toc is used to insert in the text a table of contents. This tag uses the only integer parameter - the number of header levels, which fall into the table of contents. For example, for tag #toc{2} will be created the contents consisting of two levels.

### 3.5. Tag #image

Tag #image is used to generate commands for insertion of the specified image in the text. The tag is accompanied by two parameters:
1. Name of the image file (supported formats and types of files that are available in Doxygen);
2. Width of image in a percentage of the width of the page. Only used when generating documentation in RTF format.

Examples of tags in documentation files can be viewed in the example, which comes with the software.


##4. Running the program 

During the pre-processing the program starts from the command line using the following arguments:

```
doxynum -l <file with the list of files to be processed> -d <database of sections and drawings>
[-i <list of source files for Doxygen configuration file>]

-l <file with the list of files to be processed> - name of the files, each line of which contains the name of the source file for pre-processing. Determines the order of processing files, which influences the numbering of sections (sections, who came first will receive smaller number).

-d <database of sections and drawings> - database with numbers of sections and drawings, 
gained as the result of pre-processing. Is used during the filtering stage.

-i <list of source files for Doxygen configuration file> - name of the file, into which we will write list of source files in a format of INPUT directive of the configuration file for Doxygen (optional parameter). The text of form 'INPUT=<list of files>' is written to the file. It can be added to the Doxygen configuration file to specify the list of files to be processes in the specified order (see how the argument is use in the example, provided with the program).
```

In the filtering mode the program is launched by Doxygen, each time passing it the name of the source file to be processed. To start the program in this mode, you must add the the specified line to the Doxygen configuration file (this will tell Doxygen to use Doxynum as a filter).

```

INPUT_FILTER = "doxynum -d <database of sections and drawings> [-o <directory for debugging>] -f"

-d <database of sections and drawings> - database with numbers of sections and drawings, gained as the result of pre-processing stage.

-o <directory for debugging> - directory, to which saved the results of filtration for each of the processed files (optional parameter). Can be used for debugging purposes to view what gets into Doxygen from the output of Doxynum.
```

Each time you start the program Doxygen adds the name of the source file to the line, indicated in the directive INPUT_FILTER. The result of filtration is supplied to the input of Doxygen. To view the result of filtering for debugging purposes you can use the argument -o in the Doxynum command prompt. 

Sample scripts and configuration files for Doxygen, used to launch Doxynum, are supplied with the program.


## 5. Installing the software

The executable file for Windows can be downloaded at http://sourceforge.net/projects/doxynum/files. The file is distributed as a ZIP archive, with the name Doxynum_win_executable_X.X. zip, where X.X is the version number. Extract the executable file from the archive and put it in the directory that is listed in your PATH environment variable, to ensure that it can be called from a command prompt.

Installing the program in the Linux operating system is made from the source. Download archive with source code of the program at http://sourceforge.net/projects/doxynum/files. Source texts are located in the archive with the name Doxynum_src_X.X. zip, where X.X is the version number of file source texts. Unzip the sources in any convenient directory. Then chdir to the subdirectory project/linux and run the commands

make
make install

The last command will require administrator rights.


## 6. Software Requirements

The program has been tested with the version 1.8.6 of Doxygen.


## 7. The author

Sergey Vasyutin (svpro [at] outlook.com)
