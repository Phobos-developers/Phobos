@if not defined _echo echo off

rem Cleans build folders.

rem Ensure we're in correct directory.
cd /D "%~dp0"
cd ..

if exist DevBuild\ rmdir /S /Q DevBuild\
if exist Release\ rmdir /S /Q Release\