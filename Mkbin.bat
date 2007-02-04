@echo off
echo This file creates a windows binary (release) distribution of Golgotha.
if "%1"=="" goto :needfile
goto doit
:needfile
echo Please add the Date of the file where the data should go.
echo I.e: mkdistr 030627 if it is the 27th of June 2003.
goto :end
:doit
@echo on
if NOT EXIST Release/Golgotha.exe goto fail
rem Copy the debug version to the root dir. (for zipping)
copy Release\Golgotha.exe Golgotha.exe
copy ivcon\Release\ivcon.exe ivcon\ivcon.exe

del Golgotha.ilk
wzzip -a -ex -Whs -jhrs -p -r B%1.zip Golgotha.exe *.scm *.level Readme.txt resource.res -xbackup/*.* -xcvs/*.* -xmaxtool/sdk_inc/*.* -xsdk_inc/*.* -xRelease/*.* -xmax_plugin/*.* -xtests/*.* -xLandkarten/*.*
if errorlevel 1 goto fail
wzzip -a -ex -Whs -jhrs -P -r B%1.zip resource/*.* bitmaps/*.*  textures/*.* objects/*.* ivcon/ivcon.exe 
if errorlevel 1 goto fail
goto end
:fail
@echo off
Echo Failure: Either no release build available or winzip encountered errors. Check the output.
:end
