// ---------------------------------------------------------------
// FileConverter.h  –  Lector de archivos de entrada.
//
// Formatos soportados como ENTRADA:
//   .lp   → lectura directa (texto plano)
//   .txt  → lectura directa (texto plano)
//   .docx → extrae texto del ZIP interno (no requiere Office)
//   .pdf  → extrae texto vía Word COM (requiere Office)
//
// NOTA: este header NO usa "using namespace std" para evitar
// colisiones con windows.h (byte vs std::byte).
// ---------------------------------------------------------------
#pragma once

#include <string>

class FileConverter {
public:
    static std::string leer(const std::string& ruta);
    static std::string ultimoError();

private:
    static std::string error_;
    static std::string getExtension(const std::string& ruta);
    static std::string leerTextoPlano(const std::string& ruta);
    static std::string extraerDocx(const std::string& ruta);
    static std::string extraerPdf(const std::string& ruta);
    static std::string ejecutarPS(const std::string& script);
};
