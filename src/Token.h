#pragma once

#include <string>

using namespace std;

/* Categorias lexicas para el lenguaje LP */
enum class LexTokenType {
    // Identificador y literales
    ID,           /* Identificador: <ID,index> */
    TEXTO,        /* Cadena de texto: "..." o '...' -> <TEXTO> */
    NUM_INT,      /* Enteros: 12, 0, 999 -> <NUM_INT> */
    NUM_DEC,      /* Decimales: 3.14, 0.5 -> <NUM_DEC> */

    // Palabras reservadas
    KW_INT,       /* int -> <INT> */
    KW_FLOAT,     /* float -> <FLOAT> */
    KW_STRING,    /* string -> <STRING> */
    KW_BOOL,      /* bool -> <BOOL> */
    KW_CHAR,      /* char -> <CHAR> */
    KW_IF,        /* if -> <IF> */
    KW_ELSE,      /* else -> <ELSE> */
    KW_WHILE,     /* while -> <WHILE> */
    KW_FOR,       /* for -> <FOR> */
    KW_RETURN,    /* return -> <RETURN> */
    KW_VOID,      /* void -> <VOID> */
    KW_TRUE,      /* true -> <TRUE> */
    KW_FALSE,     /* false -> <FALSE> */
    KW_PRINT,     /* print -> <PRINT> */

    // Operadores
    OP_ASSIGN,    /* = -> <=> */
    OP_DIV_INT,   /* // -> <//> */
    OP_SUM,       /* + -> <+> */
    OP_SUB,       /* - -> <-> */
    OP_MUL,       /* * -> <*> */
    OP_DIV,       /* / -> </> */
    OP_MOD,       /* % -> <%> */
    OP_EQ,        /* == -> <==> */
    OP_NE,        /* != -> <!=> */
    OP_LE,        /* <= -> <<=> */
    OP_GE,        /* >= -> <>=> */
    OP_LT,        /* < -> <<> */
    OP_GT,        /* > -> <>> */

    // Delimitadores
    SEMICOLON,    /* ; -> <;> */
    COMMA,        /* , -> <,> */
    LPAREN,       /* ( -> <(> */
    RPAREN,       /* ) -> <)> */
    LBRACE,       /* { -> <{> */
    RBRACE,       /* } -> <}> */

    // Control
    LEX_ERROR,    /* Caracter o simbolo no reconocido */
    LEX_EOF       /* Fin de archivo */
};

/* Representa un token reconocido en el codigo fuente */
struct Token {
    LexTokenType tipo;
    string       lexema;
    int          linea;
    int          columna;
    int          symbolIndex; /* Indice en la tabla de simbolos (-1 si no aplica) */
    string       mensaje;     /* Descripcion o mensaje de error */

    /* Constructores */
    Token();
    Token(LexTokenType tipo, const string& lexema, int linea, int columna = 1, int symbolIndex = -1, const string& mensaje = "");

    /* Retorna formato imprimible del token (ej. <INT>, <ID,0>, <=>, <NUM_INT>, <;>) */
    string toString() const;
};

/* Convierte el enum a texto legible */
string tokenTypeToString(LexTokenType tipo);