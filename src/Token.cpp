#include "Token.h"

using namespace std;

/* Token vacio por defecto */
Token::Token()
    : tipo(LexTokenType::LEX_EOF), lexema(""), linea(0), columna(1), symbolIndex(-1), mensaje("") {}

/* Inicializa token con tipo, texto, linea, columna, indice de simbolo y mensaje opcional */
Token::Token(LexTokenType tipo, const string& lexema, int linea, int columna, int symbolIndex, const string& mensaje)
    : tipo(tipo), lexema(lexema), linea(linea), columna(columna), symbolIndex(symbolIndex), mensaje(mensaje) {}

/* Mapea cada enum a su etiqueta textual */
string tokenTypeToString(LexTokenType tipo) {
    switch (tipo) {
        case LexTokenType::ID:         return "ID";
        case LexTokenType::TEXTO:      return "TEXTO";
        case LexTokenType::NUM_INT:    return "NUM_INT";
        case LexTokenType::NUM_DEC:    return "NUM_DEC";
        case LexTokenType::KW_INT:     return "INT";
        case LexTokenType::KW_FLOAT:   return "FLOAT";
        case LexTokenType::KW_STRING:  return "STRING";
        case LexTokenType::KW_BOOL:    return "BOOL";
        case LexTokenType::KW_CHAR:    return "CHAR";
        case LexTokenType::KW_IF:      return "IF";
        case LexTokenType::KW_ELSE:    return "ELSE";
        case LexTokenType::KW_WHILE:   return "WHILE";
        case LexTokenType::KW_FOR:     return "FOR";
        case LexTokenType::KW_RETURN:  return "RETURN";
        case LexTokenType::KW_VOID:    return "VOID";
        case LexTokenType::KW_TRUE:    return "TRUE";
        case LexTokenType::KW_FALSE:   return "FALSE";
        case LexTokenType::KW_PRINT:   return "PRINT";
        case LexTokenType::OP_ASSIGN:  return "ASSIGN";
        case LexTokenType::OP_DIV_INT: return "OP_DIV_INT";
        case LexTokenType::OP_SUM:     return "OP_SUM";
        case LexTokenType::OP_SUB:     return "OP_SUB";
        case LexTokenType::OP_MUL:     return "OP_MUL";
        case LexTokenType::OP_DIV:     return "OP_DIV";
        case LexTokenType::OP_MOD:     return "OP_MOD";
        case LexTokenType::OP_EQ:      return "OP_EQ";
        case LexTokenType::OP_NE:      return "OP_NE";
        case LexTokenType::OP_LE:      return "OP_LE";
        case LexTokenType::OP_GE:      return "OP_GE";
        case LexTokenType::OP_LT:      return "OP_LT";
        case LexTokenType::OP_GT:      return "OP_GT";
        case LexTokenType::SEMICOLON:  return "SEMICOLON";
        case LexTokenType::COMMA:      return "COMMA";
        case LexTokenType::LPAREN:     return "LPAREN";
        case LexTokenType::RPAREN:     return "RPAREN";
        case LexTokenType::LBRACE:     return "LBRACE";
        case LexTokenType::RBRACE:     return "RBRACE";
        case LexTokenType::LEX_ERROR:  return "ERROR_LEXICO";
        case LexTokenType::LEX_EOF:    return "FIN_ENTRADA";
        default:                       return "???";
    }
}

/* Representacion de salida exacta: <INT>, <ID,0>, <=>, <NUM_INT>, <;>, <//>, etc. */
string Token::toString() const {
    if (tipo == LexTokenType::LEX_ERROR) return "ERROR_LEXICO";

    if (tipo == LexTokenType::ID) {
        int idx = (symbolIndex >= 0) ? symbolIndex : 0;
        return "<ID," + to_string(idx) + ">";
    }

    if (tipo == LexTokenType::TEXTO)   return "<TEXTO>";
    if (tipo == LexTokenType::NUM_INT) return "<NUM_INT>";
    if (tipo == LexTokenType::NUM_DEC) return "<NUM_DEC>";

    // Palabras reservadas
    if (tipo == LexTokenType::KW_INT)    return "<INT>";
    if (tipo == LexTokenType::KW_FLOAT)  return "<FLOAT>";
    if (tipo == LexTokenType::KW_STRING) return "<STRING>";
    if (tipo == LexTokenType::KW_BOOL)   return "<BOOL>";
    if (tipo == LexTokenType::KW_CHAR)   return "<CHAR>";
    if (tipo == LexTokenType::KW_IF)     return "<IF>";
    if (tipo == LexTokenType::KW_ELSE)   return "<ELSE>";
    if (tipo == LexTokenType::KW_WHILE)  return "<WHILE>";
    if (tipo == LexTokenType::KW_FOR)    return "<FOR>";
    if (tipo == LexTokenType::KW_RETURN) return "<RETURN>";
    if (tipo == LexTokenType::KW_VOID)   return "<VOID>";
    if (tipo == LexTokenType::KW_TRUE)   return "<TRUE>";
    if (tipo == LexTokenType::KW_FALSE)  return "<FALSE>";
    if (tipo == LexTokenType::KW_PRINT)  return "<PRINT>";

    // Operadores
    if (tipo == LexTokenType::OP_ASSIGN)  return "<=>";
    if (tipo == LexTokenType::OP_DIV_INT) return "<//>";
    if (tipo == LexTokenType::OP_SUM)     return "<+>";
    if (tipo == LexTokenType::OP_SUB)     return "<->";
    if (tipo == LexTokenType::OP_MUL)     return "<*>";
    if (tipo == LexTokenType::OP_DIV)     return "</>";
    if (tipo == LexTokenType::OP_MOD)     return "<%>";
    if (tipo == LexTokenType::OP_EQ)      return "<==>";
    if (tipo == LexTokenType::OP_NE)      return "<!=>";
    if (tipo == LexTokenType::OP_LE)      return "<<=>";
    if (tipo == LexTokenType::OP_GE)      return "<>=>";
    if (tipo == LexTokenType::OP_LT)      return "<<>";
    if (tipo == LexTokenType::OP_GT)      return "<>>";

    // Delimitadores
    if (tipo == LexTokenType::SEMICOLON) return "<;>";
    if (tipo == LexTokenType::COMMA)     return "<,>";
    if (tipo == LexTokenType::LPAREN)    return "<(>";
    if (tipo == LexTokenType::RPAREN)    return "<)>";
    if (tipo == LexTokenType::LBRACE)    return "<{>";
    if (tipo == LexTokenType::RBRACE)    return "<}>";

    return "<" + tokenTypeToString(tipo) + ">";
}
