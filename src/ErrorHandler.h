#pragma once

#include <string>
#include <vector>
#include "Token.h"
#include "SymbolTable.h"

using namespace std;

/* Tipos de problemas detectados en la contencion de entradas y archivos */
enum class TipoFalloEntrada {
    NINGUNO,              /* Entrada valida y procesable */
    ARCHIVO_NO_EXISTE,    /* La ruta no apunta a ningun archivo existente */
    ARCHIVO_VACIO,        /* Archivo de 0 bytes (valor muerto) */
    TEXTO_VACIO,          /* Solo espacios o saltos de linea (valor muerto) */
    IMAGEN,               /* Formatos de imagen (.png, .jpg, etc.) */
    BINARIO_EJECUTABLE,   /* Binarios compilados o ejecutables (.exe, .dll, etc.) */
    MULTIMEDIA,           /* Audio o video (.mp3, .mp4, etc.) */
    CONTENIDO_BINARIO,    /* Archivo de texto que contiene bytes nulos o basura */
    FORMATO_NO_SOPORTADO  /* Extension ajena a .lp, .txt, .docx, .pdf */
};

/* Diagnostico completo del validador de entradas */
struct DiagnosticoEntrada {
    bool             esValido;
    TipoFalloEntrada tipo;
    string           titulo;
    string           mensaje;
    string           sugerencia;
};

/* Modulo unico para contencion, validacion y reporte de errores */
class ErrorHandler {
public:
    /* Valida un archivo antes de intentar leerlo o procesarlo */
    static DiagnosticoEntrada validarArchivo(const string& ruta);

    /* Valida el texto extraido o escrito en el editor para evitar valores muertos */
    static DiagnosticoEntrada validarCodigoFuente(const string& codigo);

    /* Genera el reporte formateado de errores lexicos */
    static string generarReporteErrores(const vector<Token>& errores);

    /* Genera la salida detallada de tokens basada en la especificacion del profesor */
    static string generarReporteTokens(const vector<Token>& tokens, const string& secuenciaLineas);

    /* Genera la salida detallada de la tabla de simbolos */
    static string generarReporteTablaSimbolos(const SymbolTable& tabla);

private:
    /* Extrae la extension en minusculas */
    static string extraerExtension(const string& ruta);

    /* Comprueba si la extension corresponde a imagenes */
    static bool esImagen(const string& ext);

    /* Comprueba si la extension corresponde a binarios o ejecutables */
    static bool esBinarioOEjecutable(const string& ext);

    /* Comprueba si la extension corresponde a archivos multimedia */
    static bool esMultimedia(const string& ext);

    /* Comprueba si el archivo contiene caracteres binarios no legibles */
    static bool contieneBytesBinarios(const string& ruta);
};
