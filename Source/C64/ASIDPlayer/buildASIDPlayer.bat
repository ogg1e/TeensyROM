cls

@echo off

set MainFilename=ASIDPlayer
set toolPath="D:\MyData\Geek Stuff\Projects\Commodore 64\Software\PC Utils-SW"

setlocal EnableDelayedExpansion
SET clean=del
SET cleanArgs=/F /Q 
SET buildPath=build
SET sourcePath=source
SET compilerPath=%toolPath%\C64-devkit\compiler\win32
SET compiler=acme.exe


SET MainBuild=%MainFilename%.prg
SET MainCompilerArgs=-r %buildPath%\MainBuildReport --vicelabels %buildPath%\buildPath --msvc --color --format cbm -v3 --outfile
rem --format plain leaves off the 2 byte address from the start of the file.  "cbm" includes it

SET bin2headerPath=%toolPath%\bin2header
SET bin2header=bin2header.exe
SET bin2headerROMPath=..\..\Teensy\ROMs

SET emulatorPath=%toolPath%\Emulation\GTK3VICE-3.6.1-win64\bin
SET emulator=x64sc.exe
SET emulatorArgs=-autostart

::***************************************************************************************************************

@echo on
echo ***Start...
%clean% %cleanArgs% %buildPath%\*.*

echo ***Compile Main...
%compilerPath%\%compiler% %MainCompilerArgs% %buildPath%\%MainBuild% %sourcePath%\%MainFilename%.asm
if NOT %ERRORLEVEL% == 0 exit /b

echo ***bin2header
%bin2headerPath%\%bin2header% %buildPath%\%MainBuild%
copy %buildPath%\%MainBuild%.h %bin2headerROMPath%\%MainBuild%.h

@echo .
@echo Completed: %date% %time%
@echo ************************************************************************************
@echo *** Remember to add "PROGMEM" before "static const unsigned char XXXXXXXXX_prg[] = {"
@echo ************************************************************************************

cmd.exe /c start /b notepad++.exe %bin2headerROMPath%\%MainBuild%.h
::"C:\Program Files\Notepad++\notepad++.exe"

::pause
@exit /b

::only some features can be emulated without the associated TeensyROM hardware, not very useful
::
echo ***Emulate...
start "" %emulatorPath%\%emulator% %emulatorArgs% %buildPath%\%MainBuild%
::
::pause
@exit /b