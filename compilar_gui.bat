@echo off
chcp 65001 > nul
echo ========================================================
echo  Compilando Compiladores.exe (Interfaz Visual WinAPI)
echo ========================================================

set MINGW_BIN=C:\Users\User\AppData\Local\Programs\CLion\bin\mingw\bin
if exist "%MINGW_BIN%" set PATH=%MINGW_BIN%;%PATH%

g++ -std=c++20 -O3 ^
    src/main.cpp ^
    src/Token.cpp ^
    src/Lexer.cpp ^
    src/SymbolTable.cpp ^
    src/FileConverter.cpp ^
    src/ErrorHandler.cpp ^
    src/AST.cpp ^
    src/Parser.cpp ^
    src/Interpreter.cpp ^
    -I src ^
    -lgdi32 -lcomctl32 -lcomdlg32 -mwindows ^
    -static-libgcc -static-libstdc++ ^
    -o Compiladores.exe

if %ERRORLEVEL% equ 0 (
    echo.
    echo [EXITO] Compiladores.exe generado correctamente.
    echo Ejecutando...
    start Compiladores.exe
) else (
    echo.
    echo [ERROR] La compilacion fallo.
)
