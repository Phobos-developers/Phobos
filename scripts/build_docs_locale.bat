@if not defined _echo echo off

rem Builds Phobos docs with VitePress.

rem Ensure we're in correct directory.
cd /D "%~dp0"
cd ..\docs

if not exist node_modules (
	call npm ci
	if errorlevel 1 exit /b %errorlevel%
)

echo Building docs bundle with zh_CN pages from PO files.
call npm run build
