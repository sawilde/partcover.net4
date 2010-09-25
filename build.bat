@echo off

echo [Preparing 'release' directory]
echo.
if exist release\nul rd /q /s release

echo. 
echo [Building PartCover]
echo.
pushd main
msbuild /target:Rebuild /property:Configuration=Setup PartCover.sln
if errorlevel 1 goto BuildError
popd

echo.
echo [Copying files to 'release' directory]
md release
xcopy main\bin release /s
del /q release\*.pdb
del /q release\*.log

echo. 
echo Build successful. Output is in 'release' directory.
echo.
pause
goto Exit

:BuildError
popd
echo.
echo Build failed.
echo.
pause
goto Exit

:Exit
