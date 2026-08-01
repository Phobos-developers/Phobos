@if not defined _echo echo off

rem Builds a provided build config, optionally with a provided build type
rem (NIGHTLY, PRERELEASE or RELEASE - none means a plain local build).

rem Ensure we're in correct directory.
cd /D "%~dp0"

call run_msbuild /maxCpuCount /consoleloggerparameters:NoSummary /property:Configuration=%1 /property:BuildType=%2
