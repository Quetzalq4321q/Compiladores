// ---------------------------------------------------------------
// Token.cpp  –  Implementación de la estructura Token.
// ---------------------------------------------------------------
#include "Token.h"

using namespace std;

// Constructor por defecto.
Token::Token()
    : tipo(LexTokenType::LEX_EOF), lexema(""), linea(0), columna(0) {}

// Constructor parametrizado.
Token::Token(LexTokenType tipo, const string& lexema, int linea, int columna)
    : tipo(tipo), lexema(lexema), linea(linea), columna(columna) {}

// ---------------------------------------------------------------
// tokenTypeToString() – Nombre del tipo de token.
// Se usa tanto en la lista de tokens como en los errores.
// ---------------------------------------------------------------
string tokenTypeToString(LexTokenType tipo) {
    switch (tipo) {
        case LexTokenType::NUM_INT:   return "NUM_INT";
        case LexTokenType::NUM_DEC:   return "NUM_DEC";
        case LexTokenType::LEX_ERROR: return "ERROR_LEXICO";
        case LexTokenType::LEX_EOF:   return "FIN_ENTRADA";
        default:                      return "???";
    }
}

// ---------------------------------------------------------------
// Token::toString() – Formato de salida del token.
//
// Tokens normales  →  <NUM_INT>
// Errores léxicos  →  ERROR_LEXICO
// ---------------------------------------------------------------
string Token::toString() const {
    if (tipo == LexTokenType::LEX_ERROR) return "ERROR_LEXICO";
    return "<" + tokenTypeToString(tipo) + ">";
}
