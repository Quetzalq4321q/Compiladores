#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <iomanip>

#include "Lexer.h"
#include "Token.h"
#include "SymbolTable.h"
#include "ErrorHandler.h"
#include "FileConverter.h"
#include "AST.h"
#include "Parser.h"
#include "Interpreter.h"

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
    cout << "\033[1;32m    [< / >] Lexico, Sintactico (Descenso Recursivo), AST e Interprete LP\033[0m" << endl;
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
    bool soloAnalisis = false;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--test" || arg == "-t") {
            modoPruebas = true;
        } else if (arg == "--no-run") {
            soloAnalisis = true;
        } else if (arg == "--help" || arg == "-h") {
            cout << "Uso: compilador_lp [opciones] [archivo_fuente]\n" << endl;
            cout << "Opciones:" << endl;
            cout << "  -h, --help    Muestra esta ayuda" << endl;
            cout << "  -t, --test    Ejecuta las pruebas unitarias incorporadas" << endl;
            cout << "  --no-run      Solo compila y valida sintaxis sin ejecutar" << endl;
            cout << "\nFormatos soportados: .lp, .txt, .docx, .pdf" << endl;
            cout << "Por defecto procesa 'input/ejemplo.lp' si no se especifica archivo.\n" << endl;
            return 0;
        } else if (rutaArchivo.empty()) {
            rutaArchivo = arg;
        }
    }

    if (modoPruebas) {
        cout << "\033[1;33m[*] Ejecutando suite de pruebas unitarias integradas...\033[0m\n" << endl;
        string codigoPrueba =
            "func int doble(int x) {\n"
            "    return x * 2;\n"
            "}\n"
            "void main() {\n"
            "    int edad;\n"
            "    edad = 20;\n"
            "    println(\"Edad:\", edad, \"Doble:\", func doble(edad));\n"
            "}";

        Lexer lex(codigoPrueba);
        lex.analizar();

        Parser parser(lex.getTokens());
        auto ast = parser.parsear();

        if (lex.getErrores().empty() && parser.esAceptado()) {
            cout << "\033[1;32m[PASS] Programa valido aceptado exitosamente por el Parser.\033[0m" << endl;
            Interpreter interp(ast);
            if (interp.ejecutar()) {
                cout << "\033[1;32m[PASS] Interprete ejecuto el programa correctamente:\033[0m" << endl;
                cout << interp.getSalida() << endl;
                return 0;
            }
        }
        cerr << "\033[1;31m[FAIL] Fallo en la prueba unitaria.\033[0m" << endl;
        return 1;
    }

    /* Si no se paso ningun archivo, buscar input/ejemplo.lp o usar codigo por defecto */
    string fuente = "";
    if (rutaArchivo.empty()) {
        ifstream fCheck("input/ejemplo.lp");
        if (fCheck.good()) {
            rutaArchivo = "input/ejemplo.lp";
            cout << "[INFO] Ningun archivo especificado. Usando archivo por defecto: " << rutaArchivo << "\n" << endl;
        } else {
            cout << "[INFO] Usando programa fuente de ejemplo oficial integrado:\n" << endl;
            fuente =
                "void main() {\n"
                "    int edad;\n"
                "    float promedio;\n"
                "    edad = 20;\n"
                "    promedio = 15.5;\n"
                "    if (edad >= 18 && edad <= 60) {\n"
                "        println(\"Edad valida:\", edad);\n"
                "    }\n"
                "    println(\"Promedio:\", promedio);\n"
                "    return 0;\n"
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

    cout << "\033[1;32m[OK] Entrada validada. Iniciando analisis lexico y sintactico...\033[0m\n" << endl;

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

    /* Ejecutar Analisis Sintactico por Descenso Recursivo */
    cout << "\033[1;35m[*] Ejecutando Analizador Sintactico (Parser) y construyendo AST...\033[0m\n" << endl;
    Parser parser(tokens);
    auto ast = parser.parsear();
    bool sintaxisValida = parser.esAceptado();
    const auto& errSintacticos = parser.getErrores();

    string strAST = ast ? ast->toTreeString() : "(AST no disponible debido a errores sintacticos criticos)\r\n";
    string strSintactico = ErrorHandler::generarReporteSintactico(sintaxisValida, errSintacticos);
    string strDiagCompleto = ErrorHandler::generarReporteDiagnosticoCompleto(errores, sintaxisValida, errSintacticos);

    /* Mostrar Arbol Sintactico (AST) en consola */
    cout << "\033[1;32m" << strAST << "\033[0m" << endl;

    /* Mostrar Diagnostico Sintactico */
    if (sintaxisValida && errores.empty()) {
        cout << "\033[1;32m[✓] PROGRAMA ACEPTADO: Sin errores lexicos ni sintacticos.\033[0m\n" << endl;
        cout << "\033[1;32m" << strSintactico << "\033[0m" << endl;
    } else {
        cout << "\033[1;31m[✗] PROGRAMA RECHAZADO: Se detectaron fallas estructurales.\033[0m\n" << endl;
        cout << "\033[1;31m" << strSintactico << "\033[0m" << endl;
    }

    string strEjecucion = "";
    if (sintaxisValida && errores.empty() && !soloAnalisis) {
        cout << "\033[1;36m===================================================================" << endl;
        cout << " EJECUCION EN TIEMPO REAL (INTERPRETE DEL LENGUAJE LP)" << endl;
        cout << "===================================================================\033[0m\n" << endl;

        Interpreter interprete(ast);
        bool exitoExec = interprete.ejecutar();
        strEjecucion = interprete.getReporteEjecucion();

        cout << "\033[1;32m[SALIDA POR CONSOLA]:\033[0m" << endl;
        string salidaPura = interprete.getSalida();
        if (salidaPura.empty()) {
            cout << "(Sin llamadas a println)\n" << endl;
        } else {
            cout << salidaPura << endl;
        }

        if (exitoExec) {
            cout << "\033[1;32m[✓] Fin de ejecucion exitoso (Retorno: " << interprete.getCodigoRetorno()
                 << ", Tiempo: " << fixed << setprecision(3) << interprete.getTiempoMs() << " ms)\033[0m\n" << endl;
        } else {
            cerr << "\033[1;31m[!] Errores en tiempo de ejecucion detectados:\033[0m" << endl;
            for (const auto& err : interprete.getErrores()) {
                cerr << "  * " << err << endl;
            }
            cout << endl;
        }
    }

    /* Guardar archivos de salida */
    cout << "[*] Guardando reportes en la carpeta output/..." << endl;
    guardarReporte("output/tokens.txt", strTokens);
    guardarReporte("output/tabla_simbolos.txt", strTabla);
    guardarReporte("output/ast.txt", strAST);
    guardarReporte("output/errores.txt", strDiagCompleto);
    if (!strEjecucion.empty()) {
        guardarReporte("output/ejecucion.txt", strEjecucion);
    }

    cout << "\n\033[1;32m===================================================================" << endl;
    cout << " Proceso finalizado con exito." << endl;
    cout << "===================================================================\033[0m\n" << endl;

    bool exitoTotal = errores.empty() && sintaxisValida;
    return exitoTotal ? 0 : 2;
}
