@if not defined _echo echo off

rem Copies and opens example config file for VSCode C++ extension.

rem Ensure we're in correct directory.
cd /D "%~dp0"
cd ..\.vscode\

if exist c_cpp_properties.json (
    echo Configuration file already present, exiting.
    exit /b 0
) else (
    copy c_cpp_properties.example.json c_cpp_properties.json
    code c_cpp_properties.json
)