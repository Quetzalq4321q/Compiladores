// ---------------------------------------------------------------
// Token.h  –  Definición del tipo de token y la estructura Token.
//
// SEMANA 1: solo se reconocen NUM_INT y NUM_DEC.
//   NUM_INT  →  D+          (uno o más dígitos)
//   NUM_DEC  →  D+ '.' D+  (dígitos, punto, dígitos)
//
// Campos de Token:
//   tipo    – categoría léxica (enum LexTokenType)
//   lexema  – texto original encontrado en el fuente
//   linea   – línea donde aparece (base 1)
//   columna – columna donde empieza (base 1)
//
// NOTA: el enum se llama LexTokenType (no TokenType) para evitar
// colisiones con los macros TOKEN_* de la API de Windows.
// ---------------------------------------------------------------
#pragma once

#include <string>

using namespace std;

// ---------------------------------------------------------------
// LexTokenType – Enumeración de categorías léxicas.
// ---------------------------------------------------------------
enum class LexTokenType {
    NUM_INT,      // Número entero:   42, 0, 1000
    NUM_DEC,      // Número decimal:  3.14, 0.5, 10.0
    LEX_ERROR,    // Carácter o secuencia no reconocida.
    LEX_EOF       // Marca interna de fin de texto.
};

// ---------------------------------------------------------------
// Token – Unidad mínima producida por el analizador léxico.
// ---------------------------------------------------------------
struct Token {
    LexTokenType tipo;
    string       lexema;
    int          linea;
    int          columna;

    // Constructor por defecto (token vacío).
    Token();

    // Constructor parametrizado.
    Token(LexTokenType tipo, const string& lexema, int linea, int columna);

    // Devuelve la representación imprimible, ej: "<NUM_INT>"
    string toString() const;
};

// Convierte un LexTokenType a su nombre de texto, ej: "NUM_INT"
string tokenTypeToString(LexTokenType tipo);