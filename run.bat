@echo off

python --version >nul 2>&1
if %errorlevel% neq 0 (
    .\install.bat
)
cd bin
.\Main.exe