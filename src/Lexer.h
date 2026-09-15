#pragma once

#include <string>
#include <vector>
#include "Token.h"
#include "SymbolTable.h"

using namespace std;

/* Analizador lexico para el lenguaje LP */
class Lexer {
public:
    /* Inicializa con el contenido del archivo fuente */
    explicit Lexer(const string& fuente);

    /* Recorre el texto y genera la lista de tokens y errores */
    void analizar();

    /* Devuelve los tokens reconocidos */
    const vector<Token>& getTokens() const;

    /* Devuelve la tabla de simbolos */
    const SymbolTable& getTabla() const;

    /* Devuelve los errores lexicos detectados */
    const vector<Token>& getErrores() const;

private:
    const string   fuente;
    size_t         pos;
    int            linea;

    vector<Token>  tokens;
    SymbolTable    tabla;
    vector<Token>  errores;

    /* Retorna el caracter actual */
    char actual() const;

    /* Retorna el caracter siguiente sin consumirlo */
    char siguiente() const;

    /* Avanza un caracter y actualiza el contador de linea */
    void avanzar();

    /* Salta espacios en blanco y saltos de linea */
    void saltarEspacios();

    /* Extrae un numero entero o decimal */
    Token lexNumero(int linIni);
};