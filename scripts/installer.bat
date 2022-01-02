@echo off
powershell -Command "& {[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12}"; "& {Invoke-WebRequest -UseBasicParsing 'https://raw.githubusercontent.com/R3nzTheCodeGOD/Spotify-Internal/master/scripts/install.ps1' | Invoke-Expression}"
pause
exit