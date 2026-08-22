@if not defined _echo echo off

rem Builds Phobos Release.

rem Ensure we're in correct directory.
cd /D "%~dp0"

call build Release
