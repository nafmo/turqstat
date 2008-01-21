@echo off
setlocal
cd ..

if "%1" == "" goto help
if not exist visualc\Release\turqstat.exe goto build

call git archive --format=zip -9 --prefix=src/ HEAD > visualc\turqstat-%1.zip
zip -9j visualc/turqstat-%1.zip visualc/Release/turqstat.exe turqstat.doc turqstat.dok README.Win32

goto end

:help
echo Usage: %0 version
goto end

:build
echo Must build with Visual C++ first
goto end

:end
endlocal
