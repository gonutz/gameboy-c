@echo off

if not exist tetris.gb (
    echo File not found: tetris.gb
    exit /b
)

call %~dp0\build.bat && %~dp0\gameboy.exe -m 4 -f tetris.gb
