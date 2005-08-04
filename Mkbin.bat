@echo off
echo This file creates a source code distribution of Golgotha.
if "%1"=="" goto :needfile
goto doit
:needfile
echo Please add the Date of the file where the data should go.
echo I.e: mkdistr 030627 if it is the 27th of June 2003.
goto :end
:doit
wzzip -a -ex -Whs -jhrs -p -r B%1.zip Golgotha.exe *.scm *.level Readme.txt resource.res -xbackup/*.* -xcvs/*.* -xmaxtool/sdk_inc/*.* -xsdk_inc/*.* -xtictactoe/*.* -xRelease/*.*
wzzip -a -ex -Whs -jhrs -P -r B%1.zip resource/*.* bitmaps/*.*  textures/*.* objects/*.* ivcon/ivcon.exe 

:end
