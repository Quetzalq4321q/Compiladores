#pragma once

#include <string>

using namespace std;

/* Categorias lexicas del lenguaje LP */
enum class LexTokenType {
    /* Identificador y literales */
    ID,           /* Identificador: <ID,pos> */
    TEXTO,        /* Cadena de texto: "..." -> <TEXTO> */
    NUM_INT,      /* Numero entero: 20 -> <NUM_INT> */
    NUM_DEC,      /* Numero decimal: 15.5 -> <NUM_DEC> */

    /* Palabras reservadas oficiales de LP */
    KW_INT,       /* int -> <INT> */
    KW_FLOAT,     /* float -> <FLOAT> */
    KW_CHAR,      /* char -> <CHAR> */
    KW_BOOLEAN,   /* boolean -> <BOOLEAN> */
    KW_VOID,      /* void -> <VOID> */
    KW_IF,        /* if -> <IF> */
    KW_ELSE,      /* else -> <ELSE> */
    KW_FOR,       /* for -> <FOR> */
    KW_WHILE,     /* while -> <WHILE> */
    KW_SCANF,     /* scanf -> <SCANF> */
    KW_PRINTLN,   /* println -> <PRINTLN> */
    KW_MAIN,      /* main -> <MAIN> */
    KW_RETURN,    /* return -> <RETURN> */
    KW_STRING,    /* string -> <STRING> (adicional) */

    /* Operador de asignacion */
    OP_ASSIGN,    /* = -> <=> */

    /* Operadores aritmeticos */
    OP_SUM,       /* + -> <+> */
    OP_SUB,       /* - -> <-> */
    OP_MUL,       /* * -> <*> */
    OP_DIV,       /* / -> </> */
    OP_MOD,       /* % -> <%> */

    /* Operadores logicos */
    OP_AND,       /* && -> <&&> */
    OP_OR,        /* || -> <||> */
    OP_NOT,       /* ! -> <!> */

    /* Operadores relacionales / comparacion */
    OP_COMP,      /* >, >=, <, <=, !=, == -> <COMP> */

    /* Simbolos especiales y delimitadores */
    LPAREN,       /* ( -> <(> */
    RPAREN,       /* ) -> <)> */
    LBRACKET,     /* [ -> <[> */
    RBRACKET,     /* ] -> <]> */
    LBRACE,       /* { -> <{> */
    RBRACE,       /* } -> <}> */
    COMMA,        /* , -> <,> */
    SEMICOLON,    /* ; -> <;> */

    /* Control y errores */
    LEX_ERROR,    /* Error lexico */
    LEX_EOF       /* Fin de archivo */
};

/* Representa un token individual reconocido en el codigo fuente */
struct Token {
    LexTokenType tipo;
    string       lexema;
    int          linea;
    int          symbolIndex; /* Posicion en la tabla de simbolos (-1 si no es ID) */
    string       mensaje;     /* Descripcion del error si aplica */

    /* Constructores */
    Token();
    Token(LexTokenType tipo, const string& lexema, int linea, int symbolIndex = -1, const string& mensaje = "");

    /* Formato de salida segun especificacion LP (ej: <INT>, <ID,0>, <=>, <COMP>, etc.) */
    string toString() const;
};

/* Retorna el nombre textual de la categoria */
string tokenTypeToString(LexTokenType tipo);