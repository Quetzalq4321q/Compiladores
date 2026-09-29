cmake_minimum_required(VERSION 3.20)

project(Compiladores)

set(CMAKE_CXX_STANDARD 20)

# Aplicacion principal con interfaz visual (WinAPI)
add_executable(Compiladores WIN32
        src/main.cpp
        src/Token.cpp
        src/Lexer.cpp
        src/SymbolTable.cpp
        src/FileConverter.cpp
)

target_link_libraries(Compiladores comctl32 comdlg32 ole32)

# Ejecutable de pruebas automaticas por consola
add_executable(test_lexer
        tests/test_lexer.cpp
        src/Token.cpp
        src/Lexer.cpp
        src/SymbolTable.cpp
)