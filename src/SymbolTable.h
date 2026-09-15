// ---------------------------------------------------------------
// SymbolTable.h  –  Tabla de símbolos.
//
// SEMANA 1: Solo se reconocen NUM_INT y NUM_DEC, por lo que
// la tabla de símbolos permanece vacía en esta entrega.
// Las semanas siguientes agregarán identificadores (ID).
// ---------------------------------------------------------------
#pragma once

#include <string>
#include <vector>

using namespace std;

class SymbolTable {
public:
    // Agrega un identificador si no existe. Retorna su posición.
    int agregar(const string& nombre);

    // Busca un identificador. Retorna su posición o -1 si no existe.
    int buscar(const string& nombre) const;

    // Devuelve todos los símbolos (para mostrarlos en la interfaz).
    const vector<string>& getSimbolos() const;

    // Indica si la tabla está vacía.
    bool vacia() const;

private:
    vector<string> simbolos;  // Lista de identificadores únicos.
};