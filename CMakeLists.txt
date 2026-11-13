cmake_minimum_required(VERSION 3.20)

project(Compiladores)

set(CMAKE_CXX_STANDARD 20)

# Optimizacion y enlace estatico de librerias de runtime C++ (elimina errores de libgcc y libstdc++ faltantes)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
if (WIN32 AND MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    # Copia automatica de libwinpthread-1.dll al directorio de salida para evitar errores de DLL faltante
    get_filename_component(MINGW_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    find_file(WINPTHREAD_DLL "libwinpthread-1.dll" PATHS ${MINGW_BIN_DIR})
    if (WINPTHREAD_DLL)
        file(COPY "${WINPTHREAD_DLL}" DESTINATION "${CMAKE_BINARY_DIR}")
    endif()
endif()

# 1. Aplicacion principal con interfaz visual (Windows WinAPI con 6 Pestanas y Ejecucion)
if (WIN32)
    add_executable(Compiladores WIN32
            src/main.cpp
            src/Token.cpp
            src/Lexer.cpp
            src/SymbolTable.cpp
            src/FileConverter.cpp
            src/ErrorHandler.cpp
            src/AST.cpp
            src/Parser.cpp
            src/Interpreter.cpp
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
        src/AST.cpp
        src/Parser.cpp
        src/Interpreter.cpp
)

# 3. Suite de pruebas unitarias del Analizador Lexico
add_executable(test_lexer
        tests/test_lexer.cpp
        src/Token.cpp
        src/Lexer.cpp
        src/SymbolTable.cpp
        src/ErrorHandler.cpp
)

# 4. Suite de pruebas unitarias del Analizador Sintactico e Interprete
add_executable(test_parser
        tests/test_parser.cpp
        src/Token.cpp
        src/Lexer.cpp
        src/SymbolTable.cpp
        src/ErrorHandler.cpp
        src/AST.cpp
        src/Parser.cpp
        src/Interpreter.cpp
)
