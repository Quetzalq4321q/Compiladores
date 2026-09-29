#include "SymbolTable.h"
#include <algorithm>

using namespace std;

/* Inserta el simbolo solo si no esta repetido; si ya existe, actualiza apariciones */
int SymbolTable::agregar(const string& nombre, const string& tipo, int linea) {
    int pos = buscar(nombre);
    if (pos == -1) {
        SymbolEntry entry;
        entry.pos = static_cast<int>(entradas.size());
        entry.nombre = nombre;
        entry.tipo = tipo.empty() ? "desconocido" : tipo;
        entry.lineaIni = linea;
        entry.apariciones.push_back(linea);

        entradas.push_back(entry);
        return entry.pos;
    }

    // Ya existe: agregar la linea si aun no esta registrada
    auto& apars = entradas[pos].apariciones;
    if (find(apars.begin(), apars.end(), linea) == apars.end()) {
        apars.push_back(linea);
    }
    // Actualizar tipo si no estaba definido
    if ((entradas[pos].tipo == "desconocido" || entradas[pos].tipo.empty()) && !tipo.empty()) {
        entradas[pos].tipo = tipo;
    }

    return pos;
}

/* Busqueda secuencial del identificador */
int SymbolTable::buscar(const string& nombre) const {
    for (int i = 0; i < static_cast<int>(entradas.size()); i++) {
        if (entradas[i].nombre == nombre) return i;
    }
    return -1;
}

/* Retorna vector con solo los nombres de los identificadores */
vector<string> SymbolTable::getSimbolos() const {
    vector<string> nombres;
    nombres.reserve(entradas.size());
    for (const auto& e : entradas) {
        nombres.push_back(e.nombre);
    }
    return nombres;
}

/* Retorna vector con todas las entradas completas */
const vector<SymbolEntry>& SymbolTable::getEntradas() const {
    return entradas;
}

/* Comprueba si la tabla esta vacia */
bool SymbolTable::vacia() const {
    return entradas.empty();
}

/* Limpia la tabla de simbolos */
void SymbolTable::limpiar() {
    entradas.clear();
}
