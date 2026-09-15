#pragma once

#include <string>

using namespace std;

/* Categorias lexicas para la entrega actual (Semana 1: enteros y decimales) */
enum class LexTokenType {
    NUM_INT,      /* Enteros: 12, 0, 999 */
    NUM_DEC,      /* Decimales: 3.14, 0.5 */
    LEX_ERROR,    /* Caracter o simbolo no reconocido */
    LEX_EOF       /* Fin de archivo */
};

/* Representa un token reconocido en el codigo fuente */
struct Token {
    LexTokenType tipo;
    string       lexema;
    int          linea;

    /* Constructor por defecto */
    Token();

    /* Constructor con datos del token */
    Token(LexTokenType tipo, const string& lexema, int linea);

    /* Retorna formato imprimible del token (ej. <NUM_INT>) */
    string toString() const;
};

/* Convierte el enum a texto legible */
string tokenTypeToString(LexTokenType tipo);