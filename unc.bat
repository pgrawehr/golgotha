rem Updates crustification (source beautifying) of whole code (includes absolute paths for my system, so adapt as required)


for /R %i in (*.cpp *.h) do ..\Programme\uncrustify\Uncrustify.exe -c ..\Programme\uncrustify\defaults.cfg %i --replace