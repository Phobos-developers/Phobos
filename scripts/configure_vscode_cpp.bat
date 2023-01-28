@if not defined _echo echo off

rem Copies and opens example config files for VSCode.

rem Ensure we're in correct directory.
cd /D "%~dp0"
cd ..\.vscode\

if exist c_cpp_properties.json (
    echo C++ extension configuration file already present, skipping.
) else (
    copy c_cpp_properties.example.json c_cpp_properties.json
    code c_cpp_properties.json
)

if exist settings.json (
    echo Settings configuration file already present, skipping.
) else (
    copy settings.example.json settings.json
    code settings.json
)
