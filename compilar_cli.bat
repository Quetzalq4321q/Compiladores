@echo off
chcp 65001 > nul
echo ========================================================
echo  Compilando compilador_lp.exe (Version Consola / CLI)
echo ========================================================

set MINGW_BIN=C:\Users\User\AppData\Local\Programs\CLion\bin\mingw\bin
if exist "%MINGW_BIN%" set PATH=%MINGW_BIN%;%PATH%

g++ -std=c++20 -O3 ^
    src/cli_main.cpp ^
    src/Token.cpp ^
    src/Lexer.cpp ^
    src/SymbolTable.cpp ^
    src/FileConverter.cpp ^
    src/ErrorHandler.cpp ^
    src/AST.cpp ^
    src/Parser.cpp ^
    src/Interpreter.cpp ^
    -I src ^
    -static-libgcc -static-libstdc++ ^
    -o compilador_lp.exe

if %ERRORLEVEL% equ 0 (
    echo.
    echo [EXITO] compilador_lp.exe generado correctamente.
    echo.
    echo Probando con input/ejemplo.lp:
    compilador_lp.exe input/ejemplo.lp
) else (
    echo.
    echo [ERROR] La compilacion fallo.
)
