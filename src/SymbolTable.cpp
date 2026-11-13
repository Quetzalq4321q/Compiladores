#include "SymbolTable.h"

using namespace std;

/* Agrega un identificador si no existe y devuelve su posicion en tiempo O(1) */
int SymbolTable::agregar(const string& nombre, int linea) {
    auto it = mapaIndices.find(nombre);
    if (it != mapaIndices.end()) {
        int posExistente = it->second;
        entradas[posExistente].apariciones.push_back(linea);
        return posExistente;
    }

    int nuevaPos = static_cast<int>(entradas.size());
    SymbolEntry entrada;
    entrada.pos = nuevaPos;
    entrada.nombre = nombre;
    entrada.lineaIni = linea;
    entrada.apariciones.push_back(linea);

    entradas.push_back(entrada);
    mapaIndices[nombre] = nuevaPos;
    return nuevaPos;
}

/* Busca el identificador por nombre en tiempo O(1); retorna posicion o -1 */
int SymbolTable::buscar(const string& nombre) const {
    auto it = mapaIndices.find(nombre);
    if (it != mapaIndices.end()) {
        return it->second;
    }
    return -1;
}

/* Retorna las entradas completas */
const vector<SymbolEntry>& SymbolTable::getEntradas() const {
    return entradas;
}

/* Retorna solo los nombres de los identificadores */
vector<string> SymbolTable::getSimbolos() const {
    vector<string> nombres;
    nombres.reserve(entradas.size());
    for (const auto& e : entradas) {
        nombres.push_back(e.nombre);
    }
    return nombres;
}

/* Verifica si no hay simbolos registrados */
bool SymbolTable::vacia() const {
    return entradas.empty();
}

/* Limpia la tabla y el indice */
void SymbolTable::limpiar() {
    entradas.clear();
    mapaIndices.clear();
}
