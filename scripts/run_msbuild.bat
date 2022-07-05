@if not defined _echo echo off

rem Executes MSBuild found using VS Locator and VS Dev. Cmd. Prompt.

rem Ensure we're in correct directory.
cd /D "%~dp0"

run_vsdevcmd.bat & cd .. & msbuild %*
