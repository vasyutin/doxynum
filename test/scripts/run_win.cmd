echo Creating example HTML set in $(TEMP)\doxynum_example

@echo off
cd ..
set CURRENT_DIR=%CD%
doxynum -l files.lst -d %TEMP%\doxynum-wap.db -i %TEMP%\doxynum-wap.list

del %TEMP%\wap-rtf-doxyfile > nul 2> nul
copy scripts\Doxyfile.win %TEMP%\wap-html-doxyfile > nul 2> nul
echo # >> %TEMP%\wap-html-doxyfile
type %TEMP%\doxynum-wap.list >> %TEMP%\wap-html-doxyfile
doxygen %TEMP%\wap-html-doxyfile
