@echo off
rem 检测是否有Python 3环境
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo 没有找到python3环境，正在下载安装包...
    rem 下载python 3.10.0安装包
    bitsadmin /transfer python_download /priority high https://mirrors.huaweicloud.com/python/3.10.0/python-3.10.0-amd64.exe %temp%\python-3.10.0-amd64.exe
    rem 安装python 3，并添加到环境变量
    echo 正在安装python 3，请稍候...
    %temp%\python-3.10.0-amd64.exe /quiet InstallAllUsers=1 PrependPath=1 Include_test=0
    rem 删除安装包
    del %temp%\python-3.10.0-amd64.exe
)
rem 安装成功，显示版本信息
echo Python 3 is installed, version: 
python --version

rem 检测是否有pip工具
pip --version >nul 2>&1
if %errorlevel% neq 0 (
    echo 没有找到pip库，正在下载get-pip.py文件...
    rem 下载get-pip.py文件
    bitsadmin /transfer pip_download /priority high https://bootstrap.pypa.io/get-pip.py %temp%\get-pip.py
    rem 以管理员身份运行get-pip.py文件
    echo 正在安装pip库，请稍候...
    powershell -Command "Start-Process -Verb RunAs %temp%\get-pip.py"
    rem 删除get-pip.py文件
    del %temp%\get-pip.py
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