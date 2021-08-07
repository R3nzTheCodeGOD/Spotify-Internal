@echo off
color 6
cls
SET original="%appdata%\Spotify\chrome_elf.dll"
SET original_backup="%appdata%\Spotify\_chrome_elf.dll"
SET modified="%cd%\chrome_elf.dll"

taskkill /F /IM Spotify.exe

If not exist %original% (
	echo [R3nzError] not found spotify folder
    GOTO end_code
)

If not exist %modified% (
	echo [R3nzError] modified file not found
    GOTO end_code
)

If exist %original_backup% (

	echo original already exists in backup

    echo f | xcopy /y /f %modified% %original%

    echo [R3nzSuccess] successful installing
    
    GOTO end_code
)

echo f | xcopy /y /f %original% %original_backup%

echo f | xcopy /y /f %modified% %original%

echo [R3nzSuccess] successful installing

:end_code
pause > nul