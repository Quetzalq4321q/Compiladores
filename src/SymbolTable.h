#pragma once

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

/* Entrada en la tabla de simbolos */
struct SymbolEntry {
    int         pos;
    string      nombre;
    int         lineaIni;
    vector<int> apariciones;
};

/* Gestiona identificadores unicos segun la especificacion del lenguaje LP con busqueda O(1) */
class SymbolTable {
public:
    /* Agrega identificador si no existe y devuelve su posicion (base 0).
       Si ya existe, retorna su posicion previa y registra la linea de aparicion. */
    int agregar(const string& nombre, int linea = 1);

    /* Busca un identificador; retorna posicion o -1 si no existe en tiempo O(1) */
    int buscar(const string& nombre) const;

    /* Lista completa de entradas con metadatos */
    const vector<SymbolEntry>& getEntradas() const;

    /* Lista de nombres de identificadores */
    vector<string> getSimbolos() const;

    /* Comprueba si la tabla esta vacia */
    bool vacia() const;

    /* Limpia la tabla */
    void limpiar();

private:
    vector<SymbolEntry> entradas;
    unordered_map<string, int> mapaIndices;
};
