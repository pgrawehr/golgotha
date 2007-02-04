@echo off
echo This file creates a distribution of Golgotha's music. 
if "%1"=="" goto :needfile
goto doit
:needfile
echo Please add the Date of the file where the data should go.
echo I.e: mkdistr 030627 if it is the 27th of June 2003.
goto :end
:doit
wzzip -a -ex -Whs -jhrs -p -r SF%1.zip sfx/*.*

:end
