@echo off
color 6
cls
SET original="%appdata%\Spotify\chrome_elf.dll"
SET original_backup="%appdata%\Spotify\_chrome_elf.dll"

taskkill /F /IM Spotify.exe

echo f | xcopy /y /f %original_backup% %original%

echo [R3nzSuccess] successful uninstalling

:end_code
pause > nul