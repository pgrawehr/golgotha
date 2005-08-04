@echo off
echo This file creates a source code distribution of Golgotha.
if "%1"=="" goto :needfile
goto doit
:needfile
echo Please add the Date of the file where the data should go.
echo I.e: mkdistr 030627 if it is the 27th of June 2003.
goto :end
:doit
wzzip -a -ex -whs -jhrs -p -r G%1.zip *.cpp *.h *.scm Readme.txt resource.res Makefile* *.dsw *.dsp *.sln *.vcproj Makefile*.* *.rc -xbackup/*.* -xcvs/*.* -xmaxtool/sdk_inc/*.* -xsdk_inc/*.* -xtictactoe/*.* 
wzzip -a -ex -whs -jhrs -p -r R%1.zip resource/*.*

:end
