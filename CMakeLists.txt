cmake_minimum_required(VERSION 3.20)

project(Compiladores)

set(CMAKE_CXX_STANDARD 20)

# 1. Aplicacion principal con interfaz visual (Windows WinAPI)
if (WIN32)
    add_executable(Compiladores WIN32
            src/main.cpp
            src/Token.cpp
            src/Lexer.cpp
            src/SymbolTable.cpp
            src/FileConverter.cpp
            src/ErrorHandler.cpp
            res/resource.rc
    )
    target_link_libraries(Compiladores comctl32 comdlg32 ole32)
endif()

# 2. Aplicacion de linea de comandos CLI (Multiplataforma: Linux / Docker / Windows)
add_executable(compilador_lp
        src/cli_main.cpp
        src/Token.cpp
        src/Lexer.cpp
        src/SymbolTable.cpp
        src/FileConverter.cpp
        src/ErrorHandler.cpp
)

# 3. Suite de pruebas unitarias automatizadas
add_executable(test_lexer
        tests/test_lexer.cpp
        src/Token.cpp
        src/Lexer.cpp
        src/SymbolTable.cpp
        src/ErrorHandler.cpp
)