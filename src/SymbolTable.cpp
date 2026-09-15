#include "SymbolTable.h"

using namespace std;

/* Inserta el simbolo solo si no esta repetido */
int SymbolTable::agregar(const string& nombre) {
    int pos = buscar(nombre);
    if (pos == -1) {
        simbolos.push_back(nombre);
        return static_cast<int>(simbolos.size()) - 1;
    }
    return pos;
}

/* Busqueda secuencial del identificador */
int SymbolTable::buscar(const string& nombre) const {
    for (int i = 0; i < static_cast<int>(simbolos.size()); i++) {
        if (simbolos[i] == nombre) return i;
    }
    return -1;
}

/* Retorna referencia al vector de identificadores */
const vector<string>& SymbolTable::getSimbolos() const {
    return simbolos;
}

/* Comprueba si la tabla esta vacia */
bool SymbolTable::vacia() const {
    return simbolos.empty();
}
