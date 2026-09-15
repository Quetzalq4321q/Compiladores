// ---------------------------------------------------------------
// Lexer.h  –  Declaración del analizador léxico LP (Semana 1).
//
// TOKENS RECONOCIDOS EN ESTA ENTREGA:
//   NUM_INT  →  D+         donde D = [0-9]
//   NUM_DEC  →  D+ '.' D+
//
// El lexer produce tres listas:
//   tokens   – secuencia ordenada de tokens reconocidos.
//   tabla    – tabla de símbolos (vacía en semana 1).
//   errores  – tokens de tipo LEX_ERROR con línea y columna.
// ---------------------------------------------------------------
#pragma once

#include <string>
#include <vector>
#include "Token.h"
#include "SymbolTable.h"

using namespace std;

class Lexer {
public:
    // Constructor: recibe el texto completo del archivo fuente.
    explicit Lexer(const string& fuente);

    // Ejecuta el análisis léxico. Llama a este método antes de leer resultados.
    void analizar();

    // ---- Accesores (usados por la interfaz gráfica) ----

    // Lista de tokens generados (en orden de aparición).
    const vector<Token>& getTokens()  const;

    // Tabla de símbolos (vacía en semana 1).
    const SymbolTable&   getTabla()   const;

    // Errores léxicos con línea y columna.
    const vector<Token>& getErrores() const;

private:
    const string   fuente;   // Texto fuente completo (inmutable).
    size_t         pos;      // Posición actual en el texto.
    int            linea;    // Línea actual (base 1).
    int            columna;  // Columna actual (base 1).

    vector<Token>  tokens;   // Tokens reconocidos.
    SymbolTable    tabla;    // Tabla de símbolos.
    vector<Token>  errores;  // Errores léxicos.

    // ---- Métodos auxiliares ----

    char   actual()       const;  // Carácter en pos actual.
    char   siguiente()    const;  // Carácter en pos+1 (look-ahead).
    void   avanzar();             // Avanza pos, actualiza linea/columna.
    void   saltarEspacios();      // Salta whitespace.

    Token  lexNumero(int linIni, int colIni);  // Reconoce NUM_INT o NUM_DEC.
};