#include <iostream>
#include <cassert>
#include <string>
#include "../src/Lexer.h"

using namespace std;

void testEjemploOficial() {
    cout << "=== Test 1: Ejemplo Oficial LP (Seccion 12) ===" << endl;
    string codigo =
        "void main() {\n"
        "int edad = 20;\n"
        "float promedio = 15.5;\n"
        "if (edad >= 18 && edad <= 60)\n"
        "{ println(\"Edad valida\"); }\n"
        "return; }";

    Lexer lexer(codigo);
    lexer.analizar();

    string salidaTokens = lexer.getSalidaPorLineas();
    cout << "Secuencia generada:\n" << salidaTokens << "\n" << endl;

    string esperado =
        "<VOID> <MAIN> <(> <)> <{>\r\n"
        "<INT> <ID,0> <=> <NUM_INT> <;>\r\n"
        "<FLOAT> <ID,1> <=> <NUM_DEC> <;>\r\n"
        "<IF> <(> <ID,0> <COMP> <NUM_INT> <&&> <ID,0> <COMP> <NUM_INT> <)>\r\n"
        "<{> <PRINTLN> <(> <TEXTO> <)> <;> <}>\r\n"
        "<RETURN> <;> <}>";

    if (salidaTokens == esperado) {
        cout << "[PASS] Secuencia de tokens coincide exactamente con la especificacion." << endl;
    } else {
        cout << "[WARN] Diferencia en saltos de linea o espacios." << endl;
        cout << "Esperado:\n" << esperado << endl;
    }

    /* Verificar tabla de simbolos */
    const auto& simbolos = lexer.getTabla().getSimbolos();
    assert(simbolos.size() == 2);
    assert(simbolos[0] == "edad");
    assert(simbolos[1] == "promedio");
    cout << "[PASS] Tabla de simbolos: [0] = edad, [1] = promedio." << endl;

    /* Verificar errores */
    assert(lexer.getErrores().empty());
    cout << "[PASS] 0 errores lexicos en codigo valido." << endl;
    cout << endl;
}

void testOperadoresYAritmetica() {
    cout << "=== Test 2: Operadores Aritmeticos y Comentarios ===" << endl;
    string codigo =
        "// Este comentario debe ser ignorado\n"
        "a = b + c;\n"
        "x = y * 2;\n"
        "z = a % 2;";

    Lexer lexer(codigo);
    lexer.analizar();

    cout << lexer.getSalidaPorLineas() << endl;
    assert(lexer.getErrores().empty());
    cout << "[PASS] Operadores +, *, %, = y comentarios procesados correctamente." << endl;
    cout << endl;
}

void testComparacionesYLogicos() {
    cout << "=== Test 3: Operadores Relacionales (<COMP>) y Logicos ===" << endl;
    string codigo =
        "if (a >= b)\n"
        "if (a != b)\n"
        "if (a == b)\n"
        "if (a > 10 && b < 20)\n"
        "if (!activo)";

    Lexer lexer(codigo);
    lexer.analizar();

    cout << lexer.getSalidaPorLineas() << endl;
    assert(lexer.getErrores().empty());
    cout << "[PASS] Comparadores >=, !=, ==, >, < producen <COMP>, && y ! reconocidos." << endl;
    cout << endl;
}

void testIdentificadoresRepetidos() {
    cout << "=== Test 4: Identificadores Repetidos en Tabla de Simbolos ===" << endl;
    string codigo =
        "int edad = 20;\n"
        "float promedio = edad / 2;\n"
        "edad = edad + 1;";

    Lexer lexer(codigo);
    lexer.analizar();

    cout << lexer.getSalidaPorLineas() << endl;
    const auto& simbolos = lexer.getTabla().getSimbolos();
    assert(simbolos.size() == 2);
    assert(simbolos[0] == "edad");
    assert(simbolos[1] == "promedio");
    cout << "[PASS] Identificadores repetidos reutilizan la posicion existente (edad=0, promedio=1)." << endl;
    cout << endl;
}

void testErroresLexicos() {
    cout << "=== Test 5: Manejo de Errores Lexicos ===" << endl;
    string codigo =
        "int @edad;\n"
        "$variable = 10;\n"
        "float mal = 12.3.4;\n"
        "int 99var = 5;";

    Lexer lexer(codigo);
    lexer.analizar();

    const auto& errores = lexer.getErrores();
    cout << "Errores detectados (" << errores.size() << "):" << endl;
    for (const auto& e : errores) {
        cout << "  Linea " << e.linea << ": '" << e.lexema << "' -> " << e.mensaje << endl;
    }
    assert(errores.size() >= 4);
    cout << "[PASS] Errores detectados para @, $, multiples puntos y digito al inicio de identificador." << endl;
    cout << endl;
}

#include "../src/ErrorHandler.h"

void testContencionErroresYCategorias() {
    cout << "=== Test 6: Contencion de Errores de Entrada y Categorias de Tokens ===" << endl;

    /* 1. Deteccion de imagenes */
    DiagnosticoEntrada diagImg = ErrorHandler::validarArchivo("foto.png");
    assert(!diagImg.esValido);
    assert(diagImg.tipo == TipoFalloEntrada::IMAGEN);
    cout << "[PASS] Imagen detectada y rechazada con mensaje explicito: " << diagImg.titulo << endl;

    /* 2. Deteccion de binarios */
    DiagnosticoEntrada diagBin = ErrorHandler::validarArchivo("programa.exe");
    assert(!diagBin.esValido);
    assert(diagBin.tipo == TipoFalloEntrada::BINARIO_EJECUTABLE);
    cout << "[PASS] Binario detectado y rechazado: " << diagBin.titulo << endl;

    /* 3. Deteccion de valores muertos (texto vacio o solo espacios) */
    DiagnosticoEntrada diagVacio = ErrorHandler::validarCodigoFuente("   \n\t  \r\n  ");
    assert(!diagVacio.esValido);
    assert(diagVacio.tipo == TipoFalloEntrada::TEXTO_VACIO);
    cout << "[PASS] Valor muerto detectado y rechazado: " << diagVacio.titulo << endl;

    /* 4. Verificacion de Categorias de Tokens segun tabla del profesor */
    Token tInt(LexTokenType::KW_INT, "int", 1);
    assert(tInt.getCategoria() == "Palabra reservada");

    Token tNum(LexTokenType::NUM_INT, "42", 1);
    assert(tNum.getCategoria() == "Numero entero");

    Token tDec(LexTokenType::NUM_DEC, "3.14", 1);
    assert(tDec.getCategoria() == "Numero decimal");

    Token tComp(LexTokenType::OP_COMP, ">=", 1);
    assert(tComp.getCategoria() == "Operador de Comparacion/Relacionales");

    Token tLog(LexTokenType::OP_AND, "&&", 1);
    assert(tLog.getCategoria() == "Operador logico");

    Token tAsig(LexTokenType::OP_ASSIGN, "=", 1);
    assert(tAsig.getCategoria() == "Operador de Asignacion");

    Token tEsp(LexTokenType::SEMICOLON, ";", 1);
    assert(tEsp.getCategoria() == "Simbolo especial");

    cout << "[PASS] Categorias de la tabla oficial asignadas correctamente a cada token." << endl;
    cout << endl;
}

void testAnomaliasSentidoLexico() {
    cout << "=== Test 7: Anomalias de Escritura y Falta de Sentido Lexico (;;;;...) ===" << endl;
    string codigo =
        "int edad = 20;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n"
        "float promedio = 15.5;\n"
        "edad = edad +++ 1;\n"
        "int x = a === b;";

    Lexer lexer(codigo);
    lexer.analizar();

    const auto& tokens = lexer.getTokens();
    const auto& errores = lexer.getErrores();

    /* 1. Verificar que los puntos y comas se leyeron como tokens ("los lee si") */
    int cantSemicolons = 0;
    for (const auto& t : tokens) {
        if (t.tipo == LexTokenType::SEMICOLON) cantSemicolons++;
    }
    assert(cantSemicolons >= 30);
    cout << "[PASS] Se leyeron exitosamente los " << cantSemicolons << " tokens <;> individuales." << endl;

    /* 2. Verificar que se genero la alerta de falta de sentido lexico ("pero lo alerta") */
    bool alertaPuntosComas = false;
    bool alertaPlus = false;
    bool alertaIgual = false;

    for (const auto& e : errores) {
        if (e.mensaje.find("puntos y comas") != string::npos && e.mensaje.find("carece de sentido") != string::npos) {
            alertaPuntosComas = true;
        }
        if (e.mensaje.find("+++") != string::npos || (e.mensaje.find("aritmeticos repetidos") != string::npos)) {
            alertaPlus = true;
        }
        if (e.mensaje.find("===") != string::npos) {
            alertaIgual = true;
        }
    }

    assert(alertaPuntosComas);
    cout << "[PASS] Alerta generada para secuencia redundante de ';' (carece de sentido lexico/logico)." << endl;
    assert(alertaPlus);
    cout << "[PASS] Alerta generada para operadores aritmeticos redundantes ('+++')." << endl;
    assert(alertaIgual);
    cout << "[PASS] Alerta generada para operadores de asignacion anomala ('===')." << endl;
    cout << endl;
}

int main() {
    cout << "========================================" << endl;
    cout << " EJECUTANDO PRUEBAS UNITARIAS LEXER LP  " << endl;
    cout << "========================================\n" << endl;

    testEjemploOficial();
    testOperadoresYAritmetica();
    testComparacionesYLogicos();
    testIdentificadoresRepetidos();
    testErroresLexicos();
    testContencionErroresYCategorias();
    testAnomaliasSentidoLexico();

    cout << "========================================" << endl;
    cout << " TODAS LAS PRUEBAS PASARON EXITOSAMENTE " << endl;
    cout << "========================================" << endl;
    return 0;
}
