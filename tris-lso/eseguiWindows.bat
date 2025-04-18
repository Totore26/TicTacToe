@echo off
setlocal

REM Percorso completo allo script PowerShell
set "ps1File=%~dp0runWin.ps1"

REM Sblocca lo script se è stato bloccato da Windows (file scaricato da internet)
powershell -Command "Unblock-File -Path '%ps1File%'"

REM Avvia runWin.ps1 con ExecutionPolicy Bypass (solo per questa esecuzione)
powershell -NoExit -ExecutionPolicy Bypass -File "%ps1File%"

endlocal
pause
