// ---------------------------------------------------------------
// SymbolTable.cpp  –  Implementación de la tabla de símbolos.
// ---------------------------------------------------------------
#include "SymbolTable.h"
#include <algorithm>

using namespace std;

// Agrega un identificador. Si ya existe retorna su posición actual.
int SymbolTable::agregar(const string& nombre) {
    int pos = buscar(nombre);
    if (pos == -1) {
        simbolos.push_back(nombre);
        return static_cast<int>(simbolos.size()) - 1;
    }
    return pos;
}

// Busca por nombre lineal. Retorna -1 si no se encuentra.
int SymbolTable::buscar(const string& nombre) const {
    for (int i = 0; i < static_cast<int>(simbolos.size()); i++) {
        if (simbolos[i] == nombre) return i;
    }
    return -1;
}

const vector<string>& SymbolTable::getSimbolos() const {
    return simbolos;
}

bool SymbolTable::vacia() const {
    return simbolos.empty();
}
