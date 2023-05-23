@if not defined _echo echo off

rem Finds and starts Visual Studio Developer Command Prompt using VS Locator.
rem https://github.com/microsoft/vswhere/wiki/Start-Developer-Command-Prompt

rem Ensure we're in correct directory.
cd /D "%~dp0"

for /f "usebackq delims=" %%i in (`vswhere.exe -products * -prerelease -latest -property installationPath`) do (
  if exist "%%i\Common7\Tools\vsdevcmd.bat" (
    call "%%i\Common7\Tools\vsdevcmd.bat" %*
    exit /b
  )
)

echo VS Locator is unable to locate Visual Studio Developer Command Prompt. Ensure you have VS or VS Build Tools installed.
exit /b 2
