#pragma once

#include <string>
#include <vector>

using namespace std;

/* Entrada detallada en la tabla de simbolos */
struct SymbolEntry {
    int          pos;
    string       nombre;
    string       tipo;
    int          lineaIni;
    vector<int>  apariciones;
};

/* Gestiona identificadores unicos encontrados durante el analisis */
class SymbolTable {
public:
    /* Agrega un identificador si no existe y devuelve su indice (base 0).
       Si ya existe, registra la nueva linea de aparicion. */
    int agregar(const string& nombre, const string& tipo = "", int linea = 1);

    /* Busca un identificador; retorna posicion o -1 si no existe */
    int buscar(const string& nombre) const;

    /* Lista de simbolos almacenados (solo nombres) */
    vector<string> getSimbolos() const;

    /* Lista completa de entradas con metadatos */
    const vector<SymbolEntry>& getEntradas() const;

    /* Verifica si la tabla aun no tiene simbolos */
    bool vacia() const;

    /* Reinicia la tabla de simbolos */
    void limpiar();

private:
    vector<SymbolEntry> entradas;
};