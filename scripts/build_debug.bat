@if not defined _echo echo off

rem Builds Phobos DevBuild.

rem Ensure we're in correct directory.
cd /D "%~dp0"

call run_msbuild /maxCpuCount /consoleloggerparameters:NoSummary /property:Configuration=Debug
