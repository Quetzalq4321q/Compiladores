#include "Lexer.h"
#include <cctype>

using namespace std;

/* Constructor: inicia en la primera linea y caracter */
Lexer::Lexer(const string& fuente)
    : fuente(fuente), pos(0), linea(1) {}

/* Caracter actual en la posicion de lectura */
char Lexer::actual() const {
    if (pos >= fuente.size()) return '\0';
    return fuente[pos];
}

/* Look-ahead de 1 caracter para distinguir punto decimal */
char Lexer::siguiente() const {
    if (pos + 1 >= fuente.size()) return '\0';
    return fuente[pos + 1];
}

/* Avanza una posicion y controla el conteo de lineas */
void Lexer::avanzar() {
    if (pos < fuente.size()) {
        if (fuente[pos] == '\n') linea++;
        pos++;
    }
}

/* Consume espacios, tabulaciones y saltos de linea */
void Lexer::saltarEspacios() {
    while (pos < fuente.size() && isspace(static_cast<unsigned char>(fuente[pos]))) {
        avanzar();
    }
}

/* Reconoce NUM_INT (D+) o NUM_DEC (D+\.D+) */
Token Lexer::lexNumero(int linIni) {
    size_t inicio = pos;

    /* Consume la secuencia de digitos entera */
    while (pos < fuente.size() && isdigit(static_cast<unsigned char>(fuente[pos]))) {
        avanzar();
    }

    /* Si hay punto seguido de digito, es decimal */
    if (actual() == '.' && isdigit(static_cast<unsigned char>(siguiente()))) {
        avanzar(); /* Consume el punto */

        while (pos < fuente.size() && isdigit(static_cast<unsigned char>(fuente[pos]))) {
            avanzar();
        }

        string lexema = fuente.substr(inicio, pos - inicio);
        return Token(LexTokenType::NUM_DEC, lexema, linIni);
    }

    string lexema = fuente.substr(inicio, pos - inicio);
    return Token(LexTokenType::NUM_INT, lexema, linIni);
}

/* Proceso principal de analisis lexico */
void Lexer::analizar() {
    tokens.clear();
    errores.clear();

    while (pos < fuente.size()) {
        saltarEspacios();
        if (pos >= fuente.size()) break;

        int linIni = linea;
        char c = actual();

        if (isdigit(static_cast<unsigned char>(c))) {
            tokens.push_back(lexNumero(linIni));
        } else {
            /* Cualquier otro caracter no numerico se registra como error lexico */
            string lex(1, c);
            errores.push_back(Token(LexTokenType::LEX_ERROR, lex, linIni));
            avanzar();
        }
    }
}

/* Accesores de resultados */
const vector<Token>& Lexer::getTokens()  const { return tokens;  }
const SymbolTable&   Lexer::getTabla()   const { return tabla;   }
const vector<Token>& Lexer::getErrores() const { return errores; }
