#include "ErrorHandler.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

using namespace std;

/* Extrae la extension del archivo en minusculas */
string ErrorHandler::extraerExtension(const string& ruta) {
    size_t punto = ruta.rfind('.');
    if (punto == string::npos) return "";
    string ext = ruta.substr(punto);
    transform(ext.begin(), ext.end(), ext.begin(),
              [](unsigned char c){ return tolower(c); });
    return ext;
}

/* Verifica si la extension corresponde a formatos graficos de imagen */
bool ErrorHandler::esImagen(const string& ext) {
    static const vector<string> EXT_IMAGENES = {
        ".png", ".jpg", ".jpeg", ".gif", ".bmp", ".webp",
        ".svg", ".ico", ".tiff", ".tif", ".raw", ".psd"
    };
    for (const auto& e : EXT_IMAGENES) {
        if (ext == e) return true;
    }
    return false;
}

/* Verifica si la extension corresponde a binarios, ejecutables o comprimidos */
bool ErrorHandler::esBinarioOEjecutable(const string& ext) {
    static const vector<string> EXT_BINARIOS = {
        ".exe", ".dll", ".sys", ".bin", ".iso", ".zip",
        ".rar", ".7z", ".tar", ".gz", ".o", ".obj", ".class"
    };
    for (const auto& e : EXT_BINARIOS) {
        if (ext == e) return true;
    }
    return false;
}

/* Verifica si la extension corresponde a audio o video */
bool ErrorHandler::esMultimedia(const string& ext) {
    static const vector<string> EXT_MULTIMEDIA = {
        ".mp3", ".mp4", ".wav", ".avi", ".mkv", ".mov", ".flv"
    };
    for (const auto& e : EXT_MULTIMEDIA) {
        if (ext == e) return true;
    }
    return false;
}

/* Comprueba si los primeros bytes del archivo contienen caracteres nulos */
bool ErrorHandler::contieneBytesBinarios(const string& ruta) {
    ifstream f(ruta, ios::binary);
    if (!f) return false;

    char buffer[2048];
    f.read(buffer, sizeof(buffer));
    streamsize bytesLeidos = f.gcount();

    for (streamsize i = 0; i < bytesLeidos; ++i) {
        if (buffer[i] == '\0') {
            return true;
        }
    }
    return false;
}

/* Valida exhaustivamente el archivo antes de que el compilador lo lea */
DiagnosticoEntrada ErrorHandler::validarArchivo(const string& ruta) {
    DiagnosticoEntrada diag;
    diag.esValido = false;

    if (ruta.empty()) {
        diag.tipo = TipoFalloEntrada::ARCHIVO_NO_EXISTE;
        diag.titulo = "No se selecciono ningun archivo";
        diag.mensaje = "Debe seleccionar o escribir un archivo para procesar.";
        diag.sugerencia = "Presione [Abrir Archivo] o ingrese codigo directamente en la pestana de Entrada.";
        return diag;
    }

    string ext = extraerExtension(ruta);

    /* 1. Deteccion de imagenes */
    if (esImagen(ext)) {
        diag.tipo = TipoFalloEntrada::IMAGEN;
        diag.titulo = "Entrada Invalida: Archivo de Imagen";
        diag.mensaje = "El archivo seleccionado es una imagen grafica ('" + ext + "').\n\n"
                       "El compilador lexico unicamente procesa codigo fuente en texto plano (.lp, .txt) "
                       "o documentos con texto (.docx, .pdf).\n"
                       "No es posible interpretar pixeles o datos graficos como instrucciones del lenguaje LP.";
        diag.sugerencia = "Seleccione un archivo de codigo fuente (.lp o .txt) o un documento (.docx o .pdf) que contenga texto.";
        return diag;
    }

    /* 2. Deteccion de binarios / ejecutables */
    if (esBinarioOEjecutable(ext)) {
        diag.tipo = TipoFalloEntrada::BINARIO_EJECUTABLE;
        diag.titulo = "Entrada Invalida: Archivo Binario / Ejecutable";
        diag.mensaje = "El archivo seleccionado es un ejecutable compilado o archivo binario ('" + ext + "').\n\n"
                       "El compilador requiere codigo fuente editable en lenguaje LP, no binarios de maquina o paquetes comprimidos.";
        diag.sugerencia = "Abra el codigo fuente original (.lp o .txt) antes de compilar.";
        return diag;
    }

    /* 3. Deteccion de multimedia */
    if (esMultimedia(ext)) {
        diag.tipo = TipoFalloEntrada::MULTIMEDIA;
        diag.titulo = "Entrada Invalida: Archivo Multimedia";
        diag.mensaje = "El archivo seleccionado es un archivo de audio o video ('" + ext + "').\n\n"
                       "El compilador solo acepta archivos de texto que contengan codigo fuente LP.";
        diag.sugerencia = "Seleccione un archivo con codigo fuente (.lp o .txt).";
        return diag;
    }

    /* 4. Formatos soportados (.lp, .txt, .docx, .pdf) */
    if (ext != ".lp" && ext != ".txt" && ext != ".docx" && ext != ".pdf") {
        diag.tipo = TipoFalloEntrada::FORMATO_NO_SOPORTADO;
        diag.titulo = "Formato no compatible";
        diag.mensaje = "La extension '" + ext + "' no es un formato compatible con este compilador.\n\n"
                       "Formatos admitidos:\n"
                       "  - Archivos LP (.lp)\n"
                       "  - Archivos de Texto (.txt)\n"
                       "  - Documentos Word (.docx)\n"
                       "  - Documentos PDF (.pdf)";
        diag.sugerencia = "Guarde su programa con extension .lp o .txt para continuar.";
        return diag;
    }

    /* 5. Comprobar existencia y tamano en disco */
    ifstream f(ruta, ios::binary | ios::ate);
    if (!f) {
        diag.tipo = TipoFalloEntrada::ARCHIVO_NO_EXISTE;
        diag.titulo = "Archivo no encontrado";
        diag.mensaje = "No se pudo acceder al archivo especificado en la ruta:\n" + ruta;
        diag.sugerencia = "Verifique que el archivo exista y no haya sido movido o eliminado.";
        return diag;
    }

    streamsize tamano = f.tellg();
    f.close();

    /* 6. Deteccion de archivo vacio (0 bytes / valor muerto) */
    if (tamano == 0) {
        diag.tipo = TipoFalloEntrada::ARCHIVO_VACIO;
        diag.titulo = "Archivo Vacio (Valor Muerto)";
        diag.mensaje = "El archivo seleccionado tiene 0 bytes de contenido.\n\n"
                       "No contiene ninguna linea de codigo ni caracteres para que el analizador lexico pueda operar.";
        diag.sugerencia = "Escriba codigo fuente LP en el archivo antes de analizarlo.";
        return diag;
    }

    /* 7. Deteccion de contenido binario corrupto en archivos de texto */
    if ((ext == ".lp" || ext == ".txt") && contieneBytesBinarios(ruta)) {
        diag.tipo = TipoFalloEntrada::CONTENIDO_BINARIO;
        diag.titulo = "Contenido Binario Invalido";
        diag.mensaje = "El archivo tiene extension de texto ('" + ext + "'), pero contiene caracteres nulos o secuencias binarias no legibles.\n\n"
                       "Esto ocurre cuando se renombra un archivo binario o imagen con extension .lp o .txt.";
        diag.sugerencia = "Asegurese de que el archivo contenga texto plano codificado en UTF-8 o ASCII.";
        return diag;
    }

    /* Archivo valido */
    diag.esValido = true;
    diag.tipo = TipoFalloEntrada::NINGUNO;
    return diag;
}

/* Valida el contenido de texto extraido o escrito en el editor */
DiagnosticoEntrada ErrorHandler::validarCodigoFuente(const string& codigo) {
    DiagnosticoEntrada diag;
    diag.esValido = false;

    if (codigo.empty()) {
        diag.tipo = TipoFalloEntrada::TEXTO_VACIO;
        diag.titulo = "Entrada Vacia (Valor Muerto)";
        diag.mensaje = "No se ha ingresado ningun codigo fuente en el editor ni se ha cargado ningun archivo.";
        diag.sugerencia = "Escriba o pegue codigo LP en la pestana de Entrada, o abra un archivo con [Abrir Archivo].";
        return diag;
    }

    /* Comprobar si solo contiene espacios en blanco o saltos de linea */
    bool tieneContenidoReal = false;
    for (char c : codigo) {
        if (!isspace(static_cast<unsigned char>(c))) {
            tieneContenidoReal = true;
            break;
        }
    }

    if (!tieneContenidoReal) {
        diag.tipo = TipoFalloEntrada::TEXTO_VACIO;
        diag.titulo = "Entrada en Blanco (Valor Muerto)";
        diag.mensaje = "El texto proporcionado solo contiene espacios en blanco, tabulaciones o saltos de linea.\n\n"
                       "No existe ninguna instruccion, numero o identificador para tokenizar.";
        diag.sugerencia = "Ingrese codigo del lenguaje LP (ej: int edad = 20;).";
        return diag;
    }

    diag.esValido = true;
    diag.tipo = TipoFalloEntrada::NINGUNO;
    return diag;
}

/* Genera el reporte formateado de errores y anomalias lexicas */
string ErrorHandler::generarReporteErrores(const vector<Token>& errores) {
    ostringstream ss;
    ss << "=== REPORTE DE ERRORES Y ANOMALIAS LEXICAS (LP) ===\r\n";
    ss << "Total de incidencias encontradas: " << errores.size() << "\r\n";
    ss << "-------------------------------------------------------------------------------------------\r\n";

    if (errores.empty()) {
        ss << "[ESTADO]: Codigo lexico valido al 100%. Sin errores detectados.\r\n";
        return ss.str();
    }

    ss << "LINEA      LEXEMA                        TIPO / DESCRIPCION DEL ERROR O ALERTA\r\n";
    ss << "-------------------------------------------------------------------------------------------\r\n";
    for (const auto& e : errores) {
        string lin = "Linea " + to_string(e.linea);
        lin.resize(11, ' ');
        string lex = "'" + e.lexema + "'";
        if (lex.size() < 30) lex.resize(30, ' ');
        else lex += "  ";
        string tipo = (e.mensaje.find("carece de sentido") != string::npos) ? "[ALERTA] " : "[ERROR]  ";
        string msg = e.mensaje.empty() ? "ERROR_LEXICO" : e.mensaje;
        ss << lin << lex << tipo << msg << "\r\n";
    }

    ss << "-------------------------------------------------------------------------------------------\r\n";
    ss << "Nota: Las alertas indican anomalias que carecen de sentido logico/lexico (ej. ';;;;;;;;;').\r\n";
    ss << "      Los errores lexicos impiden continuar con el analisis sintactico formal.\r\n";
    return ss.str();
}

/* Genera la salida detallada de tokens basada en la tabla del profesor */
string ErrorHandler::generarReporteTokens(const vector<Token>& tokens, const string& secuenciaLineas) {
    ostringstream ss;
    ss << "=== SECUENCIA DE TOKENS (FORMATO LP) ===\r\n";
    ss << secuenciaLineas << "\r\n\r\n";

    ss << "=== TABLA DETALLADA DE TOKENS (ESPECIFICACION LP) ===\r\n";
    ss << "Total: " << tokens.size() << " token(s) reconocidos\r\n";
    ss << "------------------------------------------------------------------------------------------------------\r\n";
    ss << "#      TOKEN            LEXEMA               CATEGORIA DEL TOKEN                    LINEA\r\n";
    ss << "------------------------------------------------------------------------------------------------------\r\n";

    if (tokens.empty()) {
        ss << "(Sin tokens reconocidos)\r\n";
    } else {
        int i = 1;
        for (const auto& t : tokens) {
            string num = to_string(i++) + ".";
            num.resize(7, ' ');
            ss << num;

            string tipo = t.toString();
            if (tipo.size() < 17) tipo.resize(17, ' ');
            else tipo += " ";
            ss << tipo;

            string lex = t.lexema;
            if (lex.size() < 21) lex.resize(21, ' ');
            else lex = lex.substr(0, 18) + "... ";
            ss << lex;

            string cat = t.getCategoria();
            if (cat.size() < 39) cat.resize(39, ' ');
            else cat += " ";
            ss << cat;

            ss << t.linea << "\r\n";
        }
    }
    return ss.str();
}

/* Genera la salida detallada de la tabla de simbolos */
string ErrorHandler::generarReporteTablaSimbolos(const SymbolTable& tabla) {
    ostringstream ss;
    ss << "=== TABLA DE SIMBOLOS (IDENTIFICADORES LP) ===\r\n";
    const auto& entradas = tabla.getEntradas();
    ss << "Total: " << entradas.size() << " identificador(es) unico(s)\r\n";
    ss << "-------------------------------------------------------------------------------------------\r\n";

    if (tabla.vacia()) {
        ss << "(Tabla de simbolos vacia: no se definieron identificadores en el programa)\r\n";
    } else {
        ss << "POSICION   IDENTIFICADOR         LINEA INICIAL   APARICIONES\r\n";
        ss << "-------------------------------------------------------------------------------------------\r\n";
        for (const auto& e : entradas) {
            string posStr = to_string(e.pos);
            posStr.resize(11, ' ');
            ss << posStr;

            string nom = e.nombre;
            if (nom.size() < 22) nom.resize(22, ' ');
            else nom += "  ";
            ss << nom;

            string lin = "Linea " + to_string(e.lineaIni);
            lin.resize(16, ' ');
            ss << lin;

            for (size_t k = 0; k < e.apariciones.size(); ++k) {
                if (k > 0) ss << ", ";
                ss << "L" << e.apariciones[k];
            }
            ss << "\r\n";
        }
    }
    return ss.str();
}
