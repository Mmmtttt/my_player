echo off
cls
echo %1
for %%a in ("%1\*.ts") do %1\ffmpeg -i "%%a" -f mp4 -codec copy "%1\%%~na.mp4"
pause