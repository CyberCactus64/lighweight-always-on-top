@echo off

echo Compilation started...
g++ Light-AlwaysOnTop.cpp -o "AlwaysOnTop.exe" -mwindows

:: check if there are errors during compilation
if %ERRORLEVEL% neq 0 (
    echo Error during compilation.
    exit /b %ERRORLEVEL%
)


echo You can now run AlwaysOnTop.exe!
:: AlwaysOnTop.exe

:: check if there are errors when running
if %ERRORLEVEL% neq 0 (
    echo Errore during the program execution.
    exit /b %ERRORLEVEL%
)