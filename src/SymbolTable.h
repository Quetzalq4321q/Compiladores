#pragma once

#include <string>
#include <vector>

using namespace std;

/* Gestiona identificadores unicos encontrados durante el analisis */
class SymbolTable {
public:
    /* Agrega un identificador si no existe y devuelve su indice */
    int agregar(const string& nombre);

    /* Busca un identificador; retorna posicion o -1 si no existe */
    int buscar(const string& nombre) const;

    /* Lista de simbolos almacenados */
    const vector<string>& getSimbolos() const;

    /* Verifica si la tabla aun no tiene simbolos */
    bool vacia() const;

private:
    vector<string> simbolos;
};