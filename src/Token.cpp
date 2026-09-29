#include "Token.h"

using namespace std;

/* Token vacio por defecto */
Token::Token()
    : tipo(LexTokenType::LEX_EOF), lexema(""), linea(0), symbolIndex(-1), mensaje("") {}

/* Token inicializado con tipo, lexema, linea, indice de simbolo y mensaje opcional */
Token::Token(LexTokenType tipo, const string& lexema, int linea, int symbolIndex, const string& mensaje)
    : tipo(tipo), lexema(lexema), linea(linea), symbolIndex(symbolIndex), mensaje(mensaje) {}

/* Mapea cada categoria lexica a su etiqueta textual */
string tokenTypeToString(LexTokenType tipo) {
    switch (tipo) {
        case LexTokenType::ID:         return "ID";
        case LexTokenType::TEXTO:      return "TEXTO";
        case LexTokenType::NUM_INT:    return "NUM_INT";
        case LexTokenType::NUM_DEC:    return "NUM_DEC";
        case LexTokenType::KW_INT:     return "INT";
        case LexTokenType::KW_FLOAT:   return "FLOAT";
        case LexTokenType::KW_CHAR:    return "CHAR";
        case LexTokenType::KW_BOOLEAN: return "BOOLEAN";
        case LexTokenType::KW_VOID:    return "VOID";
        case LexTokenType::KW_IF:      return "IF";
        case LexTokenType::KW_ELSE:    return "ELSE";
        case LexTokenType::KW_FOR:     return "FOR";
        case LexTokenType::KW_WHILE:   return "WHILE";
        case LexTokenType::KW_SCANF:   return "SCANF";
        case LexTokenType::KW_PRINTLN: return "PRINTLN";
        case LexTokenType::KW_MAIN:    return "MAIN";
        case LexTokenType::KW_RETURN:  return "RETURN";
        case LexTokenType::KW_STRING:  return "STRING";
        case LexTokenType::OP_ASSIGN:  return "=";
        case LexTokenType::OP_SUM:     return "+";
        case LexTokenType::OP_SUB:     return "-";
        case LexTokenType::OP_MUL:     return "*";
        case LexTokenType::OP_DIV:     return "/";
        case LexTokenType::OP_MOD:     return "%";
        case LexTokenType::OP_AND:     return "&&";
        case LexTokenType::OP_OR:      return "||";
        case LexTokenType::OP_NOT:     return "!";
        case LexTokenType::OP_COMP:    return "COMP";
        case LexTokenType::LPAREN:     return "(";
        case LexTokenType::RPAREN:     return ")";
        case LexTokenType::LBRACKET:   return "[";
        case LexTokenType::RBRACKET:   return "]";
        case LexTokenType::LBRACE:     return "{";
        case LexTokenType::RBRACE:     return "}";
        case LexTokenType::COMMA:      return ",";
        case LexTokenType::SEMICOLON:  return ";";
        case LexTokenType::LEX_ERROR:  return "ERROR_LEXICO";
        case LexTokenType::LEX_EOF:    return "FIN_ENTRADA";
        default:                       return "???";
    }
}

/* Formato exacto de token segun la especificacion del lenguaje LP */
string Token::toString() const {
    if (tipo == LexTokenType::LEX_ERROR) return "ERROR_LEXICO";

    /* Identificadores incluyen la posicion en la tabla de simbolos: <ID,pos> */
    if (tipo == LexTokenType::ID) {
        int idx = (symbolIndex >= 0) ? symbolIndex : 0;
        return "<ID," + to_string(idx) + ">";
    }

    /* Literales */
    if (tipo == LexTokenType::TEXTO)   return "<TEXTO>";
    if (tipo == LexTokenType::NUM_INT) return "<NUM_INT>";
    if (tipo == LexTokenType::NUM_DEC) return "<NUM_DEC>";

    /* Palabras reservadas */
    if (tipo == LexTokenType::KW_INT)     return "<INT>";
    if (tipo == LexTokenType::KW_FLOAT)   return "<FLOAT>";
    if (tipo == LexTokenType::KW_CHAR)    return "<CHAR>";
    if (tipo == LexTokenType::KW_BOOLEAN) return "<BOOLEAN>";
    if (tipo == LexTokenType::KW_VOID)    return "<VOID>";
    if (tipo == LexTokenType::KW_IF)      return "<IF>";
    if (tipo == LexTokenType::KW_ELSE)    return "<ELSE>";
    if (tipo == LexTokenType::KW_FOR)     return "<FOR>";
    if (tipo == LexTokenType::KW_WHILE)   return "<WHILE>";
    if (tipo == LexTokenType::KW_SCANF)   return "<SCANF>";
    if (tipo == LexTokenType::KW_PRINTLN) return "<PRINTLN>";
    if (tipo == LexTokenType::KW_MAIN)    return "<MAIN>";
    if (tipo == LexTokenType::KW_RETURN)  return "<RETURN>";
    if (tipo == LexTokenType::KW_STRING)  return "<STRING>";

    /* Operador de asignacion */
    if (tipo == LexTokenType::OP_ASSIGN) return "<=>";

    /* Operadores aritmeticos */
    if (tipo == LexTokenType::OP_SUM) return "<+>";
    if (tipo == LexTokenType::OP_SUB) return "<->";
    if (tipo == LexTokenType::OP_MUL) return "<*>";
    if (tipo == LexTokenType::OP_DIV) return "</>";
    if (tipo == LexTokenType::OP_MOD) return "<%>";

    /* Operadores logicos */
    if (tipo == LexTokenType::OP_AND) return "<&&>";
    if (tipo == LexTokenType::OP_OR)  return "<||>";
    if (tipo == LexTokenType::OP_NOT) return "<!>";

    /* Operadores relacionales (todos se representan como <COMP>) */
    if (tipo == LexTokenType::OP_COMP) return "<COMP>";

    /* Delimitadores y simbolos especiales */
    if (tipo == LexTokenType::LPAREN)    return "<(>";
    if (tipo == LexTokenType::RPAREN)    return "<)>";
    if (tipo == LexTokenType::LBRACKET)  return "<[>";
    if (tipo == LexTokenType::RBRACKET)  return "<]>";
    if (tipo == LexTokenType::LBRACE)    return "<{>";
    if (tipo == LexTokenType::RBRACE)    return "<}>";
    if (tipo == LexTokenType::COMMA)     return "<,>";
    if (tipo == LexTokenType::SEMICOLON) return "<;>";

    return "<" + tokenTypeToString(tipo) + ">";
}
