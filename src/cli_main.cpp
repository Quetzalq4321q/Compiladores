#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>

#include "Lexer.h"
#include "Token.h"
#include "SymbolTable.h"
#include "ErrorHandler.h"
#include "FileConverter.h"

using namespace std;

/* Muestra el banner e icono del compilador LP en la consola */
void mostrarBanner() {
    cout << "\033[1;36m";
    cout << R"(
    ===================================================================
      _____                      _ _           _                     _     ____  
     / ____|                    (_) |         | |                   | |   |  _ \
    | |     ___  _ __ ___  _ __  _| | __ _  __| | ___  _ __ ___  ___| |   | |_) |
    | |    / _ \| '_ ` _ \| '_ \| | |/ _` |/ _` |/ _ \| '__/ _ \/ __| |   |  __/ 
    | |___| (_) | | | | | | |_) | | | (_| | (_| | (_) | | |  __/\__ \ |___| |    
     \_____\___/|_| |_| |_| .__/|_|_|\__,_|\__,_|\___/|_|  \___||___/_____|_|    
                          | |                                                    
                          |_|       [ Lenguaje de Programacion - C++20 ]         
    ===================================================================
    )" << "\033[0m" << endl;
    cout << "\033[1;32m    [< / >] Analizador Lexico, Tabla de Simbolos y Contencion de Errores\033[0m" << endl;
    cout << "    -------------------------------------------------------------------\n" << endl;
}

/* Guarda un archivo en disco asegurando creacion basica */
void guardarReporte(const string& ruta, const string& contenido) {
    ofstream f(ruta);
    if (f) {
        f << contenido;
        cout << "\033[1;32m  [OK] Exportado: \033[0m" << ruta << endl;
    } else {
        cerr << "\033[1;31m  [ERROR] No se pudo guardar: \033[0m" << ruta << endl;
    }
}

int main(int argc, char* argv[]) {
    mostrarBanner();

    string rutaArchivo = "";
    bool modoPruebas = false;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--test" || arg == "-t") {
            modoPruebas = true;
        } else if (arg == "--help" || arg == "-h") {
            cout << "Uso: compilador_lp [opciones] [archivo_fuente]\n" << endl;
            cout << "Opciones:" << endl;
            cout << "  -h, --help    Muestra esta ayuda" << endl;
            cout << "  -t, --test    Ejecuta las pruebas unitarias incorporadas" << endl;
            cout << "\nFormatos soportados: .lp, .txt, .docx, .pdf" << endl;
            cout << "Por defecto analiza 'input/ejemplo.lp' si no se especifica archivo.\n" << endl;
            return 0;
        } else if (rutaArchivo.empty()) {
            rutaArchivo = arg;
        }
    }

    if (modoPruebas) {
        cout << "\033[1;33m[*] Ejecutando suite de pruebas unitarias...\033[0m\n" << endl;
        string codigoPrueba =
            "void main() {\n"
            "int edad = 20;\n"
            "float promedio = 15.5;\n"
            "if (edad >= 18 && edad <= 60)\n"
            "{ println(\"Edad valida\"); }\n"
            "return; }";

        Lexer lex(codigoPrueba);
        lex.analizar();

        cout << lex.getSalidaPorLineas() << endl;
        if (lex.getErrores().empty()) {
            cout << "\033[1;32m[PASS] Prueba basica completada exitosamente sin errores.\033[0m" << endl;
            return 0;
        } else {
            cerr << "\033[1;31m[FAIL] Se detectaron errores inesperados.\033[0m" << endl;
            return 1;
        }
    }

    /* Si no se paso ningun archivo, buscar input/ejemplo.lp o usar codigo por defecto */
    string fuente = "";
    if (rutaArchivo.empty()) {
        ifstream fCheck("input/ejemplo.lp");
        if (fCheck.good()) {
            rutaArchivo = "input/ejemplo.lp";
            cout << "[INFO] Ningun archivo especificado. Usando archivo por defecto: " << rutaArchivo << "\n" << endl;
        } else {
            cout << "[INFO] Usando programa fuente de ejemplo oficial integrado (Seccion 12):\n" << endl;
            fuente =
                "void main() {\n"
                "int edad = 20;\n"
                "float promedio = 15.5;\n"
                "if (edad >= 18 && edad <= 60) {\n"
                "println(\"Edad valida\");\n"
                "}\n"
                "return;\n"
                "}";
        }
    }

    if (!rutaArchivo.empty()) {
        cout << "[*] Validando archivo de entrada: " << rutaArchivo << "..." << endl;
        DiagnosticoEntrada diagArchivo = ErrorHandler::validarArchivo(rutaArchivo);
        if (!diagArchivo.esValido) {
            cerr << "\n\033[1;31m[ERROR DE ENTRADA]: " << diagArchivo.titulo << "\033[0m" << endl;
            cerr << diagArchivo.mensaje << "\n" << endl;
            cerr << "\033[1;33mSugerencia: " << diagArchivo.sugerencia << "\033[0m\n" << endl;
            return 1;
        }

        fuente = FileConverter::leer(rutaArchivo);
        if (fuente.empty()) {
            cerr << "\033[1;31m[ERROR]: No se pudo leer el contenido del archivo.\033[0m" << endl;
            cerr << FileConverter::ultimoError() << endl;
            return 1;
        }
    }

    /* Validar codigo fuente para prevenir valores muertos o vacios */
    DiagnosticoEntrada diagCodigo = ErrorHandler::validarCodigoFuente(fuente);
    if (!diagCodigo.esValido) {
        cerr << "\n\033[1;31m[ERROR EN CODIGO]: " << diagCodigo.titulo << "\033[0m" << endl;
        cerr << diagCodigo.mensaje << "\n" << endl;
        cerr << "\033[1;33mSugerencia: " << diagCodigo.sugerencia << "\033[0m\n" << endl;
        return 1;
    }

    cout << "\033[1;32m[OK] Entrada validada. Iniciando analisis lexico...\033[0m\n" << endl;

    Lexer lexer(fuente);
    lexer.analizar();

    const auto& tokens  = lexer.getTokens();
    const auto& tabla   = lexer.getTabla();
    const auto& errores = lexer.getErrores();

    string strTokens  = ErrorHandler::generarReporteTokens(tokens, lexer.getSalidaPorLineas());
    string strTabla   = ErrorHandler::generarReporteTablaSimbolos(tabla);
    string strErrores = ErrorHandler::generarReporteErrores(errores);

    /* Mostrar secuencia de tokens en consola */
    cout << "\033[1;36m" << strTokens << "\033[0m" << endl;
    cout << "\033[1;33m" << strTabla << "\033[0m" << endl;

    if (errores.empty()) {
        cout << "\033[1;32m" << strErrores << "\033[0m" << endl;
    } else {
        cout << "\033[1;31m" << strErrores << "\033[0m" << endl;
    }

    /* Guardar archivos de salida */
    cout << "\n[*] Guardando reportes en la carpeta output/..." << endl;
    guardarReporte("output/tokens.txt", strTokens);
    guardarReporte("output/tabla_simbolos.txt", strTabla);
    guardarReporte("output/errores.txt", strErrores);

    cout << "\n\033[1;32m===================================================================" << endl;
    cout << " Analisis lexico finalizado con exito." << endl;
    cout << "===================================================================\033[0m\n" << endl;

    return errores.empty() ? 0 : 2;
}
