#include "Token.h"

using namespace std;

/* Token vacio por defecto */
Token::Token()
    : tipo(LexTokenType::LEX_EOF), lexema(""), linea(0) {}

/* Inicializa token con tipo, texto y numero de linea */
Token::Token(LexTokenType tipo, const string& lexema, int linea)
    : tipo(tipo), lexema(lexema), linea(linea) {}

/* Mapea cada enum a su etiqueta textual */
string tokenTypeToString(LexTokenType tipo) {
    switch (tipo) {
        case LexTokenType::NUM_INT:   return "NUM_INT";
        case LexTokenType::NUM_DEC:   return "NUM_DEC";
        case LexTokenType::LEX_ERROR: return "ERROR_LEXICO";
        case LexTokenType::LEX_EOF:   return "FIN_ENTRADA";
        default:                      return "???";
    }
}

/* Representacion de salida: <NUM_INT> o ERROR_LEXICO */
string Token::toString() const {
    if (tipo == LexTokenType::LEX_ERROR) return "ERROR_LEXICO";
    return "<" + tokenTypeToString(tipo) + ">";
}
