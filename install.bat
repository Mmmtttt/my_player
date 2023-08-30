@echo off
rem 检测是否有Python 3环境
python --version >nul 2>&1
if %errorlevel% neq 0 (
    rem 没有安装Python 3，从官网下载并安装
    echo Python 3 is not installed, downloading and installing...
    powershell -Command "(New-Object Net.WebClient).DownloadFile('https://www.python.org/ftp/python/3.11.5/python-3.11.5-amd64.exe', 'C:\Tools\python-3.11.5.exe')"
    C:\Tools\python-3.11.5.exe /quiet InstallAllUsers=1 PrependPath=1 Include_test=0 TargetDir=C:\Tools\Python311
    if %errorlevel% neq 0 (
        rem 安装失败，显示错误信息并退出
        echo Failed to install Python 3, error code: %errorlevel%
        exit /b %errorlevel%
    )
)
rem 安装成功，显示版本信息
echo Python 3 is installed, version: 
python --version

rem 检测是否有pip工具
pip --version >nul 2>&1
if %errorlevel% neq 0 (
    rem 没有安装pip，从官网下载并安装
    echo pip is not installed, downloading and installing...
    powershell -Command "(New-Object Net.WebClient).DownloadFile('https://bootstrap.pypa.io/get-pip.py', 'C:\Tools\get-pip.py')"
    python C:\Tools\get-pip.py
    if %errorlevel% neq 0 (
        rem 安装失败，显示错误信息并退出
        echo Failed to install pip, error code: %errorlevel%
        exit /b %errorlevel%
    )
)
rem 安装成功，显示版本信息
echo pip is installed, version: 
pip --version

rem 使用pip工具安装Flask组件
echo Installing Flask...
pip install flask
if %errorlevel% neq 0 (
    rem 安装失败，显示错误信息并退出
    echo Failed to install Flask, error code: %errorlevel%
    exit /b %errorlevel%
)
rem 安装成功，显示版本信息
echo Flask is installed, version: 
pip show flask | findstr Version

rem 结束脚本
echo All done!
exit /b 0
