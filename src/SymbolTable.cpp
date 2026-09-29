#include "SymbolTable.h"

using namespace std;

/* Agrega un identificador si no existe y devuelve su posicion; si ya existe devuelve la existente */
int SymbolTable::agregar(const string& nombre, int linea) {
    int posExistente = buscar(nombre);
    if (posExistente != -1) {
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
    return nuevaPos;
}

/* Busca el identificador por nombre; retorna posicion o -1 */
int SymbolTable::buscar(const string& nombre) const {
    for (size_t i = 0; i < entradas.size(); ++i) {
        if (entradas[i].nombre == nombre) {
            return static_cast<int>(i);
        }
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

/* Reinicia la tabla de simbolos */
void SymbolTable::limpiar() {
    entradas.clear();
}
