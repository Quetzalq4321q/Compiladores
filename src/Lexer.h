#pragma once

#include <string>
#include <vector>
#include "Token.h"
#include "SymbolTable.h"

using namespace std;

/* Analizador lexico completo para el lenguaje LP */
class Lexer {
public:
    /* Inicializa con el codigo fuente */
    explicit Lexer(const string& fuente);

    /* Ejecuta el analisis lexico generando tokens, tabla de simbolos y errores */
    void analizar();

    /* Devuelve la lista secuencial de tokens reconocidos */
    const vector<Token>& getTokens() const;

    /* Devuelve la tabla de simbolos (identificadores unicos) */
    const SymbolTable& getTabla() const;

    /* Devuelve los errores lexicos detectados */
    const vector<Token>& getErrores() const;

    /* Devuelve la secuencia de tokens formateada por lineas:
       <VOID> <MAIN> <(> <)> <{>
       <INT> <ID,0> <=> <NUM_INT> <;>
       ... */
    string getSalidaPorLineas() const;

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

    /* Avanza una posicion en el texto y actualiza contador de lineas */
    void avanzar();

    /* Salta espacios en blanco y descarta comentarios (// y multilineas) */
    void saltarEspaciosYComentarios();

    /* Reconoce cadenas de texto ("...") */
    Token lexTexto(int linIni, char delim);

    /* Reconoce numeros enteros (NUM_INT) y decimales (NUM_DEC) */
    Token lexNumero(int linIni);

    /* Reconoce identificadores y palabras reservadas */
    Token lexIdentificadorOPalabra(int linIni);
};