// ---------------------------------------------------------------
// Lexer.cpp  –  Analizador léxico LP (Semana 1).
//
// Implementa el reconocimiento de:
//   NUM_INT  →  D+          (expresión regular: [0-9]+)
//   NUM_DEC  →  D+ '.' D+  (expresión regular: [0-9]+\.[0-9]+)
//
// Todo lo que no sea un dígito, espacio o salto de línea
// se registra como LEX_ERROR con su posición exacta.
// ---------------------------------------------------------------
#include "Lexer.h"
#include <cctype>

using namespace std;

// ---------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------
Lexer::Lexer(const string& fuente)
    : fuente(fuente), pos(0), linea(1), columna(1) {}

// ---------------------------------------------------------------
// actual() – Devuelve el carácter en la posición actual.
//            Retorna '\0' si ya se llegó al final del texto.
// ---------------------------------------------------------------
char Lexer::actual() const {
    if (pos >= fuente.size()) return '\0';
    return fuente[pos];
}

// ---------------------------------------------------------------
// siguiente() – Look-ahead de 1: mira el carácter siguiente
//               sin consumirlo (necesario para detectar D+ '.' D+).
// ---------------------------------------------------------------
char Lexer::siguiente() const {
    if (pos + 1 >= fuente.size()) return '\0';
    return fuente[pos + 1];
}

// ---------------------------------------------------------------
// avanzar() – Consume el carácter actual y mueve el cursor.
//             Actualiza línea y columna para el reporte de errores.
// ---------------------------------------------------------------
void Lexer::avanzar() {
    if (pos < fuente.size()) {
        if (fuente[pos] == '\n') {
            linea++;
            columna = 1;  // Reiniciar columna al pasar de línea.
        } else {
            columna++;
        }
        pos++;
    }
}

// ---------------------------------------------------------------
// saltarEspacios() – Salta todos los caracteres de separación
//                    (espacio, tab, retorno de carro, nueva línea).
//                    No generan tokens.
// ---------------------------------------------------------------
void Lexer::saltarEspacios() {
    while (pos < fuente.size() &&
           isspace(static_cast<unsigned char>(fuente[pos]))) {
        avanzar();
    }
}

// ---------------------------------------------------------------
// lexNumero() – Reconoce un literal numérico a partir de la
//               posición actual.
//
// Algoritmo:
//   1. Consumir todos los dígitos consecutivos → parte entera.
//   2. Si el siguiente char es '.' y el char DESPUÉS del punto
//      también es un dígito, entonces es NUM_DEC:
//      consumir '.' y la parte decimal.
//   3. Si no hay punto (o el punto no va seguido de dígito),
//      el token es NUM_INT.
//
// Parámetros linIni/colIni: posición donde inicia el número
// (se capturan antes de llamar a esta función).
// ---------------------------------------------------------------
Token Lexer::lexNumero(int linIni, int colIni) {
    size_t inicio = pos;

    // Paso 1: consumir dígitos de la parte entera.
    while (pos < fuente.size() && isdigit(static_cast<unsigned char>(fuente[pos]))) {
        avanzar();
    }

    // Paso 2: verificar si hay punto decimal seguido de dígito.
    if (actual() == '.' && isdigit(static_cast<unsigned char>(siguiente()))) {
        avanzar();  // Consumir el punto.

        // Consumir dígitos de la parte decimal.
        while (pos < fuente.size() && isdigit(static_cast<unsigned char>(fuente[pos]))) {
            avanzar();
        }

        string lexema = fuente.substr(inicio, pos - inicio);
        return Token(LexTokenType::NUM_DEC, lexema, linIni, colIni);
    }

    // Paso 3: no hay parte decimal → es un entero.
    string lexema = fuente.substr(inicio, pos - inicio);
    return Token(LexTokenType::NUM_INT, lexema, linIni, colIni);
}

// ---------------------------------------------------------------
// analizar() – Bucle principal del analizador léxico.
//
// Recorre todo el texto fuente produciendo tokens hasta llegar
// al final. Los tokens se acumulan en `tokens` y los errores
// en `errores`. La tabla de símbolos queda vacía en Semana 1.
// ---------------------------------------------------------------
void Lexer::analizar() {
    tokens.clear();
    errores.clear();

    while (pos < fuente.size()) {
        saltarEspacios();
        if (pos >= fuente.size()) break;

        int  linIni = linea;
        int  colIni = columna;
        char c      = actual();

        if (isdigit(static_cast<unsigned char>(c))) {
            // ---- Número entero o decimal ----
            tokens.push_back(lexNumero(linIni, colIni));
        } else {
            // ---- Carácter no reconocido → ERROR LÉXICO ----
            string lex(1, c);
            errores.push_back(Token(LexTokenType::LEX_ERROR, lex, linIni, colIni));
            avanzar();  // Avanzar para no quedar en bucle infinito.
        }
    }
}

// ---------------------------------------------------------------
// Accesores
// ---------------------------------------------------------------
const vector<Token>& Lexer::getTokens()  const { return tokens;  }
const SymbolTable&   Lexer::getTabla()   const { return tabla;   }
const vector<Token>& Lexer::getErrores() const { return errores; }
