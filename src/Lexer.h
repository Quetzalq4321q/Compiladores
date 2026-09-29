#pragma once

#include <string>
#include <vector>
#include "Token.h"
#include "SymbolTable.h"

using namespace std;

/* Analizador lexico para el lenguaje LP con soporte para ID, TEXTO, palabras reservadas y tabla */
class Lexer {
public:
    /* Inicializa con el contenido del archivo fuente */
    explicit Lexer(const string& fuente);

    /* Recorre el texto y genera la lista de tokens, errores y tabla de simbolos */
    void analizar();

    /* Devuelve los tokens reconocidos */
    const vector<Token>& getTokens() const;

    /* Devuelve la tabla de simbolos */
    const SymbolTable& getTabla() const;

    /* Devuelve los errores lexicos detectados */
    const vector<Token>& getErrores() const;

    /* Devuelve la salida formateada agrupada por linea:
       <INT> <ID,0> <=> <NUM_INT> <;>
       <FLOAT> <ID,1> <=> <ID,0> <//> <NUM_INT> <;> */
    string getSalidaPorLineas() const;

private:
    const string   fuente;
    size_t         pos;
    int            linea;
    int            columna;

    vector<Token>  tokens;
    SymbolTable    tabla;
    vector<Token>  errores;
    string         ultimoTipoLeido;
    Token          ultimoToken;

    /* Retorna el caracter actual */
    char actual() const;

    /* Retorna el caracter siguiente sin consumirlo */
    char siguiente() const;

    /* Busca el siguiente caracter no vacio sin consumir espacios */
    char mirarSiguienteNoEspacio() const;

    /* Avanza un caracter y actualiza contadores de linea y columna */
    void avanzar();

    /* Salta espacios en blanco y comentarios (# y comentarios multilineales) */
    void saltarEspaciosYComentarios();

    /* Extrae un numero entero o decimal con validacion de puntos y sufijos invalidos */
    Token lexNumero(int linIni, int colIni);

    /* Extrae identificadores o palabras reservadas */
    Token lexIdentificadorOPalabra(int linIni, int colIni);

    /* Extrae cadenas de texto delimitadas por comillas */
    Token lexTexto(int linIni, int colIni, char delim);

    /* Registra token y actualiza ultimoToken */
    void agregarToken(const Token& t);
};