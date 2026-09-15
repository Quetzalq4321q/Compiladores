cmake_minimum_required(VERSION 3.20)

project(Compiladores)

set(CMAKE_CXX_STANDARD 20)

# WIN32 = aplicación de ventana (sin consola).
add_executable(Compiladores WIN32
        src/main.cpp
        src/Token.cpp
        src/Lexer.cpp
        src/SymbolTable.cpp
        src/FileConverter.cpp
)

# comctl32 = TabControl, comdlg32 = GetOpenFileName, ole32 = COM (PDF).
target_link_libraries(Compiladores comctl32 comdlg32 ole32)