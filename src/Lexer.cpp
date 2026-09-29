#include "Lexer.h"
#include <cctype>
#include <sstream>
#include <map>

using namespace std;

/* Diccionario de palabras reservadas del lenguaje LP */
static const map<string, LexTokenType> PALABRAS_RESERVADAS = {
    {"int",     LexTokenType::KW_INT},
    {"float",   LexTokenType::KW_FLOAT},
    {"char",    LexTokenType::KW_CHAR},
    {"boolean", LexTokenType::KW_BOOLEAN},
    {"bool",    LexTokenType::KW_BOOLEAN},
    {"void",    LexTokenType::KW_VOID},
    {"if",      LexTokenType::KW_IF},
    {"else",    LexTokenType::KW_ELSE},
    {"for",     LexTokenType::KW_FOR},
    {"while",   LexTokenType::KW_WHILE},
    {"scanf",   LexTokenType::KW_SCANF},
    {"println", LexTokenType::KW_PRINTLN},
    {"print",   LexTokenType::KW_PRINTLN},
    {"main",    LexTokenType::KW_MAIN},
    {"return",  LexTokenType::KW_RETURN},
    {"string",  LexTokenType::KW_STRING}
};

/* Constructor: inicializa posicion y linea */
Lexer::Lexer(const string& fuente)
    : fuente(fuente), pos(0), linea(1) {}

/* Caracter actual en el flujo */
char Lexer::actual() const {
    if (pos >= fuente.size()) return '\0';
    return fuente[pos];
}

/* Look-ahead de 1 caracter */
char Lexer::siguiente() const {
    if (pos + 1 >= fuente.size()) return '\0';
    return fuente[pos + 1];
}

/* Avanza la posicion de lectura y actualiza numero de linea */
void Lexer::avanzar() {
    if (pos < fuente.size()) {
        if (fuente[pos] == '\n') linea++;
        pos++;
    }
}

/* Consume espacios en blanco y descarta comentarios segun la especificacion //.*\n */
void Lexer::saltarEspaciosYComentarios() {
    while (pos < fuente.size()) {
        char c = actual();
        if (isspace(static_cast<unsigned char>(c))) {
            avanzar();
            continue;
        }

        /* Comentario de una linea: //.*\n */
        if (c == '/' && siguiente() == '/') {
            avanzar(); /* primer '/' */
            avanzar(); /* segundo '/' */
            while (pos < fuente.size() && actual() != '\n') {
                avanzar();
            }
            continue;
        }

        /* Comentario de bloque: /* ... * / */
        if (c == '/' && siguiente() == '*') {
            int linCom = linea;
            avanzar(); /* '/' */
            avanzar(); /* '*' */
            bool cerrado = false;
            while (pos < fuente.size()) {
                if (actual() == '*' && siguiente() == '/') {
                    avanzar(); /* '*' */
                    avanzar(); /* '/' */
                    cerrado = true;
                    break;
                }
                avanzar();
            }
            if (!cerrado) {
                errores.push_back(Token(LexTokenType::LEX_ERROR, "/*", linCom, -1, "Comentario de bloque sin cerrar"));
            }
            continue;
        }

        break;
    }
}

/* Reconoce literales de texto entre comillas */
Token Lexer::lexTexto(int linIni, char delim) {
    avanzar(); /* Consume comilla inicial */
    size_t inicio = pos;
    bool cerrado = false;

    while (pos < fuente.size()) {
        char c = actual();
        if (c == '\\') {
            avanzar();
            if (pos < fuente.size()) avanzar();
            continue;
        }
        if (c == delim) {
            cerrado = true;
            break;
        }
        if (c == '\n') break;
        avanzar();
    }

    if (cerrado) {
        string contenido = fuente.substr(inicio, pos - inicio);
        avanzar(); /* Consume comilla final */
        string lexema = string(1, delim) + contenido + string(1, delim);
        return Token(LexTokenType::TEXTO, lexema, linIni);
    }

    string incompleto = string(1, delim) + fuente.substr(inicio, pos - inicio);
    errores.push_back(Token(LexTokenType::LEX_ERROR, incompleto, linIni, -1, "Cadena de texto sin cerrar"));
    return Token(LexTokenType::LEX_ERROR, incompleto, linIni, -1, "Cadena de texto sin cerrar");
}

/* Reconoce NUM_INT (D+) o NUM_DEC (D+\.D+) con deteccion de anomalias */
Token Lexer::lexNumero(int linIni) {
    size_t inicio = pos;
    int puntos = 0;

    while (pos < fuente.size()) {
        char c = actual();
        if (isdigit(static_cast<unsigned char>(c))) {
            avanzar();
        } else if (c == '.') {
            if (isdigit(static_cast<unsigned char>(siguiente()))) {
                puntos++;
                if (puntos > 1) {
                    avanzar();
                    while (pos < fuente.size() && (isalnum(static_cast<unsigned char>(actual())) || actual() == '.')) {
                        avanzar();
                    }
                    string lexErr = fuente.substr(inicio, pos - inicio);
                    errores.push_back(Token(LexTokenType::LEX_ERROR, lexErr, linIni, -1, "Numero con multiples puntos decimales"));
                    return Token(LexTokenType::LEX_ERROR, lexErr, linIni, -1, "Numero con multiples puntos decimales");
                }
                avanzar();
            } else {
                avanzar();
                while (pos < fuente.size() && (isalnum(static_cast<unsigned char>(actual())) || actual() == '.')) {
                    avanzar();
                }
                string lexErr = fuente.substr(inicio, pos - inicio);
                errores.push_back(Token(LexTokenType::LEX_ERROR, lexErr, linIni, -1, "Numero decimal mal formado (falta parte decimal)"));
                return Token(LexTokenType::LEX_ERROR, lexErr, linIni, -1, "Numero decimal mal formado (falta parte decimal)");
            }
        } else {
            break;
        }
    }

    /* Valida que no empiece un identificador invalido con digitos (ej. 20edad) */
    if (pos < fuente.size() && (isalpha(static_cast<unsigned char>(actual())) || actual() == '_')) {
        while (pos < fuente.size() && (isalnum(static_cast<unsigned char>(actual())) || actual() == '_')) {
            avanzar();
        }
        string lexErr = fuente.substr(inicio, pos - inicio);
        errores.push_back(Token(LexTokenType::LEX_ERROR, lexErr, linIni, -1, "Identificador invalido: no puede comenzar con digitos"));
        return Token(LexTokenType::LEX_ERROR, lexErr, linIni, -1, "Identificador invalido: no puede comenzar con digitos");
    }

    string lexema = fuente.substr(inicio, pos - inicio);
    if (puntos == 1) {
        return Token(LexTokenType::NUM_DEC, lexema, linIni);
    }
    return Token(LexTokenType::NUM_INT, lexema, linIni);
}

/* Reconoce identificadores y palabras reservadas */
Token Lexer::lexIdentificadorOPalabra(int linIni) {
    size_t inicio = pos;
    while (pos < fuente.size() && (isalnum(static_cast<unsigned char>(actual())) || actual() == '_')) {
        avanzar();
    }
    string lexema = fuente.substr(inicio, pos - inicio);

    auto it = PALABRAS_RESERVADAS.find(lexema);
    if (it != PALABRAS_RESERVADAS.end()) {
        return Token(it->second, lexema, linIni);
    }

    /* Identificador valido: registrar en la tabla de simbolos */
    int idx = tabla.agregar(lexema, linIni);
    return Token(LexTokenType::ID, lexema, linIni, idx);
}

/* Bucle principal del analizador lexico */
void Lexer::analizar() {
    tokens.clear();
    errores.clear();
    tabla.limpiar();
    pos = 0;
    linea = 1;

    while (pos < fuente.size()) {
        saltarEspaciosYComentarios();
        if (pos >= fuente.size()) break;

        int linIni = linea;
        char c = actual();
        char sig = siguiente();

        /* 1. Cadenas de texto */
        if (c == '"' || c == '\'') {
            Token t = lexTexto(linIni, c);
            if (t.tipo != LexTokenType::LEX_ERROR) tokens.push_back(t);
            continue;
        }

        /* 2. Numeros enteros o decimales */
        if (isdigit(static_cast<unsigned char>(c))) {
            Token t = lexNumero(linIni);
            if (t.tipo != LexTokenType::LEX_ERROR) tokens.push_back(t);
            continue;
        }

        /* 3. Puntos sueltos no numericos */
        if (c == '.') {
            avanzar();
            errores.push_back(Token(LexTokenType::LEX_ERROR, ".", linIni, -1, "Punto aislado no reconocido"));
            continue;
        }

        /* 4. Identificadores y palabras reservadas */
        if (isalpha(static_cast<unsigned char>(c)) || c == '_') {
            tokens.push_back(lexIdentificadorOPalabra(linIni));
            continue;
        }

        /* 5. Operadores relacionales de 2 caracteres (==, !=, <=, >=) -> COMP */
        if (c == '=' && sig == '=') {
            avanzar(); avanzar();
            tokens.push_back(Token(LexTokenType::OP_COMP, "==", linIni));
            continue;
        }
        if (c == '!' && sig == '=') {
            avanzar(); avanzar();
            tokens.push_back(Token(LexTokenType::OP_COMP, "!=", linIni));
            continue;
        }
        if (c == '<' && sig == '=') {
            avanzar(); avanzar();
            tokens.push_back(Token(LexTokenType::OP_COMP, "<=", linIni));
            continue;
        }
        if (c == '>' && sig == '=') {
            avanzar(); avanzar();
            tokens.push_back(Token(LexTokenType::OP_COMP, ">=", linIni));
            continue;
        }

        /* 6. Operadores logicos de 2 caracteres (&&, ||) */
        if (c == '&' && sig == '&') {
            avanzar(); avanzar();
            tokens.push_back(Token(LexTokenType::OP_AND, "&&", linIni));
            continue;
        }
        if (c == '|' && sig == '|') {
            avanzar(); avanzar();
            tokens.push_back(Token(LexTokenType::OP_OR, "||", linIni));
            continue;
        }

        /* 7. Operadores relacionales simples (<, >) -> COMP */
        if (c == '<') {
            avanzar();
            tokens.push_back(Token(LexTokenType::OP_COMP, "<", linIni));
            continue;
        }
        if (c == '>') {
            avanzar();
            tokens.push_back(Token(LexTokenType::OP_COMP, ">", linIni));
            continue;
        }

        /* 8. Operador de asignacion (=) */
        if (c == '=') {
            avanzar();
            tokens.push_back(Token(LexTokenType::OP_ASSIGN, "=", linIni));
            continue;
        }

        /* 9. Operador de negacion logica (!) */
        if (c == '!') {
            avanzar();
            tokens.push_back(Token(LexTokenType::OP_NOT, "!", linIni));
            continue;
        }

        /* 10. Operadores aritmeticos (+, -, *, /, %) */
        if (c == '+') { avanzar(); tokens.push_back(Token(LexTokenType::OP_SUM, "+", linIni)); continue; }
        if (c == '-') { avanzar(); tokens.push_back(Token(LexTokenType::OP_SUB, "-", linIni)); continue; }
        if (c == '*') { avanzar(); tokens.push_back(Token(LexTokenType::OP_MUL, "*", linIni)); continue; }
        if (c == '/') { avanzar(); tokens.push_back(Token(LexTokenType::OP_DIV, "/", linIni)); continue; }
        if (c == '%') { avanzar(); tokens.push_back(Token(LexTokenType::OP_MOD, "%", linIni)); continue; }

        /* 11. Delimitadores y simbolos especiales */
        if (c == '(') { avanzar(); tokens.push_back(Token(LexTokenType::LPAREN, "(", linIni)); continue; }
        if (c == ')') { avanzar(); tokens.push_back(Token(LexTokenType::RPAREN, ")", linIni)); continue; }
        if (c == '[') { avanzar(); tokens.push_back(Token(LexTokenType::LBRACKET, "[", linIni)); continue; }
        if (c == ']') { avanzar(); tokens.push_back(Token(LexTokenType::RBRACKET, "]", linIni)); continue; }
        if (c == '{') { avanzar(); tokens.push_back(Token(LexTokenType::LBRACE, "{", linIni)); continue; }
        if (c == '}') { avanzar(); tokens.push_back(Token(LexTokenType::RBRACE, "}", linIni)); continue; }
        if (c == ',') { avanzar(); tokens.push_back(Token(LexTokenType::COMMA, ",", linIni)); continue; }
        if (c == ';') { avanzar(); tokens.push_back(Token(LexTokenType::SEMICOLON, ";", linIni)); continue; }

        /* 12. Operadores logicos incompletos */
        if (c == '&') {
            avanzar();
            errores.push_back(Token(LexTokenType::LEX_ERROR, "&", linIni, -1, "Operador logico incompleto (esperaba '&&')"));
            continue;
        }
        if (c == '|') {
            avanzar();
            errores.push_back(Token(LexTokenType::LEX_ERROR, "|", linIni, -1, "Operador logico incompleto (esperaba '||')"));
            continue;
        }

        /* 13. Caracter no reconocido (ej: @, $, #, etc.) */
        string lex(1, c);
        errores.push_back(Token(LexTokenType::LEX_ERROR, lex, linIni, -1, "Caracter no reconocido '" + lex + "'"));
        avanzar();
    }
}

/* Genera la secuencia exacta de tokens agrupados por linea */
string Lexer::getSalidaPorLineas() const {
    if (tokens.empty()) return "(Sin tokens reconocidos)";

    map<int, vector<Token>> porLinea;
    for (const auto& t : tokens) {
        porLinea[t.linea].push_back(t);
    }

    ostringstream ss;
    bool primera = true;
    for (const auto& p : porLinea) {
        if (!primera) ss << "\r\n";
        primera = false;
        bool primerToken = true;
        for (const auto& t : p.second) {
            if (!primerToken) ss << " ";
            primerToken = false;
            ss << t.toString();
        }
    }
    return ss.str();
}

/* Accesores */
const vector<Token>& Lexer::getTokens()  const { return tokens;  }
const SymbolTable&   Lexer::getTabla()   const { return tabla;   }
const vector<Token>& Lexer::getErrores() const { return errores; }
