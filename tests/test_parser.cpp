// Verifica producciones, recuperacion de errores, AST e interpretacion.
#include <iostream>
#include <cassert>
#include <string>
#include "../src/Lexer.h"
#include "../src/Parser.h"
#include "../src/AST.h"
#include "../src/ErrorHandler.h"
#include "../src/Interpreter.h"

using namespace std;

/* Test 1: Programa valido minimo de la Seccion 4.2 de la guia del profesor */
void testProgramaValidoMinimo() {
    cout << "=== Test 1: Programa Valido Minimo (Seccion 4.2) ===" << endl;
    string codigo =
        "void main() {\n"
        "    int edad;\n"
        "    float promedio;\n"
        "    edad = 20;\n"
        "    promedio = 15.5;\n"
        "}";

    Lexer lexer(codigo);
    lexer.analizar();
    assert(lexer.getErrores().empty());

    Parser parser(lexer.getTokens());
    auto ast = parser.parsear();

    assert(parser.esAceptado());
    assert(parser.getErrores().empty());
    assert(ast != nullptr);
    assert(ast->funcionPrincipal != nullptr);
    assert(ast->funcionPrincipal->bloque != nullptr);
    assert(ast->funcionPrincipal->bloque->variables.size() == 2);
    assert(ast->funcionPrincipal->bloque->comandos.size() == 2);

    cout << "[PASS] Programa valido aceptado exitosamente por el Parser." << endl;
    cout << "[INFO] Arbol Sintactico generado:\n" << ast->toTreeString() << endl;
}

/* Test 2: Programa invalido por falta de ';' (Seccion 4.3 de la guia del profesor) */
void testProgramaInvalidoFaltaPuntoYComa() {
    cout << "=== Test 2: Programa Invalido por Falta de ';' (Seccion 4.3) ===" << endl;
    string codigo =
        "void main() {\n"
        "    int edad\n"
        "    edad = 20;\n"
        "}";

    Lexer lexer(codigo);
    lexer.analizar();

    Parser parser(lexer.getTokens());
    auto ast = parser.parsear();

    assert(!parser.esAceptado());
    assert(!parser.getErrores().empty());

    const auto& errs = parser.getErrores();
    bool errorPuntoYComa = false;
    for (const auto& e : errs) {
        if (e.mensaje.find(";") != string::npos || e.construccionEsperada == ";") {
            errorPuntoYComa = true;
            break;
        }
    }
    assert(errorPuntoYComa);
    cout << "[PASS] Error sintactico de falta de ';' detectado correctamente en linea " << errs[0].linea << "." << endl;
    cout << parser.getReporteErrores() << endl;
}

/* Test 3: Programa invalido por bloque o parentesis sin cerrar */
void testProgramaInvalidoBloqueAbierto() {
    cout << "=== Test 3: Programa Invalido por Llave de Cierre Faltante ===" << endl;
    string codigo =
        "void main() {\n"
        "    int edad;\n"
        "    edad = 20;\n";

    Lexer lexer(codigo);
    lexer.analizar();

    Parser parser(lexer.getTokens());
    parser.parsear();

    assert(!parser.esAceptado());
    assert(!parser.getErrores().empty());
    cout << "[PASS] Rechazo correcto de bloque no cerrado." << endl;
}

/* Test 4: Palabra reservada 'func', declaracion e invocacion de funciones */
void testPalabraReservadaFunc() {
    cout << "=== Test 4: Palabra Reservada 'func' y Declaracion/Llamada de Funciones ===" << endl;
    string codigo =
        "func int sumar(int a, int b) {\n"
        "    return a + b;\n"
        "}\n"
        "void main() {\n"
        "    int res;\n"
        "    res = func sumar(10, 20);\n"
        "    println(\"Resultado:\", res);\n"
        "}";

    Lexer lexer(codigo);
    lexer.analizar();
    assert(lexer.getErrores().empty());

    /* Comprobar que 'func' fue reconocido como KW_FUNC */
    bool tieneKwFunc = false;
    for (const auto& t : lexer.getTokens()) {
        if (t.tipo == LexTokenType::KW_FUNC) {
            tieneKwFunc = true;
            break;
        }
    }
    assert(tieneKwFunc);

    Parser parser(lexer.getTokens());
    auto ast = parser.parsear();

    assert(parser.esAceptado());
    assert(ast != nullptr);
    assert(ast->funciones.size() == 1);
    assert(ast->funciones[0]->nombre == "sumar");
    assert(ast->funciones[0]->parametros.size() == 2);
    assert(ast->funcionPrincipal != nullptr);

    cout << "[PASS] Declaracion e invocacion con 'func' analizadas correctamente en AST." << endl;
}

/* Test 5: Estructuras de control (while, if-else) y operadores relacionales/logicos */
void testEstructurasControl() {
    cout << "=== Test 5: Estructuras de Control (while, if, else, println, scanf) ===" << endl;
    string codigo =
        "void main() {\n"
        "    int i, limite;\n"
        "    scanf(limite);\n"
        "    i = 0;\n"
        "    while (i < limite) {\n"
        "        if (i == 5) {\n"
        "            println(\"Mitad alcanzada\");\n"
        "        } else {\n"
        "            println(\"Iteracion:\", i);\n"
        "        }\n"
        "        i = i + 1;\n"
        "    }\n"
        "    return i;\n"
        "}";

    Lexer lexer(codigo);
    lexer.analizar();
    assert(lexer.getErrores().empty());

    Parser parser(lexer.getTokens());
    auto ast = parser.parsear();

    assert(parser.esAceptado());
    assert(ast != nullptr);
    assert(ast->funcionPrincipal->bloque->comandos.size() == 4); /* scanf, i=0, while, return */

    cout << "[PASS] Sentencias de control, lectura y escritura validadas 100%." << endl;
}

/* Test 6: Declaracion de arreglos con dimensiones y acceso indexado */
void testArreglosYDimensiones() {
    cout << "=== Test 6: Arreglos y Dimensiones ===" << endl;
    string codigo =
        "void main() {\n"
        "    int matriz[10][20];\n"
        "    matriz[0][1] = 99;\n"
        "}";

    Lexer lexer(codigo);
    lexer.analizar();
    assert(lexer.getErrores().empty());

    Parser parser(lexer.getTokens());
    auto ast = parser.parsear();

    assert(parser.esAceptado());
    assert(ast != nullptr);
    cout << "[PASS] Arreglos multidimensionales analizados correctamente." << endl;
}

/* Test 7: Recuperacion de errores en modo panico (Panic Mode Recovery) */
void testRecuperacionModoPanico() {
    cout << "=== Test 7: Recuperacion de Errores (Modo Panico) ===" << endl;
    string codigo =
        "void main() {\n"
        "    int x, y;\n"
        "    x = 1\n"
        "    y = 2;\n"
        "}";

    Lexer lexer(codigo);
    lexer.analizar();

    Parser parser(lexer.getTokens());
    auto ast = parser.parsear();

    assert(!parser.esAceptado());
    assert(parser.getErrores().size() >= 1);
    assert(ast != nullptr);
    assert(ast->funcionPrincipal != nullptr);

    cout << "[PASS] Modo panico sincronizo con exito y continuo el analisis sintactico." << endl;
}

/* Test 8: Ejecucion real con el Interprete de LP */
void testEjecucionInterprete() {
    cout << "=== Test 8: Ejecucion Completa con Interprete LP ===" << endl;
    string codigo =
        "func int duplicar(int n) {\n"
        "    return n * 2;\n"
        "}\n"
        "void main() {\n"
        "    int a, b, res;\n"
        "    a = 15;\n"
        "    b = 5;\n"
        "    res = func duplicar(a + b);\n"
        "    println(\"Resultado duplicado:\", res);\n"
        "    return res;\n"
        "}";

    Lexer lexer(codigo);
    lexer.analizar();
    assert(lexer.getErrores().empty());

    Parser parser(lexer.getTokens());
    auto ast = parser.parsear();
    assert(parser.esAceptado());

    const string arbol = ast->toTreeString();
    assert(arbol.find("+-- Funcion: int duplicar") != string::npos);
    assert(arbol.find("\\-- Funcion: void main") != string::npos);
    assert(arbol.find("|   \\--") != string::npos);
    for (unsigned char c : arbol) assert(c < 128); // Apto para consola y controles ANSI.

    Interpreter interprete(ast);
    bool ok = interprete.ejecutar();

    assert(ok);
    assert(interprete.getCodigoRetorno() == 40);
    string salida = interprete.getSalida();
    assert(salida.find("40") != string::npos);

    cout << "[PASS] Interprete ejecuto el programa correctamente: " << salida;
    cout << "[INFO] Retorno: " << interprete.getCodigoRetorno() << " en " << interprete.getTiempoMs() << " ms." << endl;
}

/* Test 9: Manejo seguro de division por cero en runtime */
void testErrorRuntimeDivisionPorCero() {
    cout << "=== Test 9: Manejo Seguro de Division por Cero en Runtime ===" << endl;
    string codigo =
        "void main() {\n"
        "    int a, b, c;\n"
        "    a = 10;\n"
        "    b = 0;\n"
        "    c = a / b;\n"
        "}";

    Lexer lexer(codigo);
    lexer.analizar();
    Parser parser(lexer.getTokens());
    auto ast = parser.parsear();
    assert(parser.esAceptado());

    Interpreter interprete(ast);
    bool ok = interprete.ejecutar();

    assert(!ok);
    assert(!interprete.getErrores().empty());
    cout << "[PASS] Division por cero detectada y controlada de forma segura: " << interprete.getErrores()[0] << endl;
}

/* Cubre precedencia, arreglos, E/S, control de flujo, funciones y retorno. */
void testEntregable2Integral() {
    const string codigo = R"(func int doble(int n) {
    return n * 2;
}
void main() {
    int matriz[2][3], valor, i;
    scanf(valor);
    valor = func doble(1 + 2 * 3);
    matriz[1][2] = valor;
    i = 0;
    if (!(valor < 10) && valor >= 14) {
        println("Valor:", matriz[1][2] + 1);
    } else {
        println("No valido");
    }
    while (i < 2) {
        i = i + 1;
    }
    return -(valor + i);
})";
    Lexer lexer(codigo);
    lexer.analizar();
    assert(lexer.getErrores().empty());
    Parser parser(lexer.getTokens());
    auto ast = parser.parsear();
    assert(parser.esAceptado());
    assert(ast && ast->funciones.size() == 1);
    auto bloque = ast->funcionPrincipal->bloque;
    assert(bloque->comandos.size() == 7);
    auto asignacion = dynamic_pointer_cast<AssignNode>(bloque->comandos[1]);
    assert(asignacion);
    auto llamada = dynamic_pointer_cast<FuncCallNode>(asignacion->expresion);
    assert(llamada && llamada->argumentos.size() == 1);
    auto suma = dynamic_pointer_cast<BinaryOpNode>(llamada->argumentos[0]);
    assert(suma && suma->op == "+");
    auto producto = dynamic_pointer_cast<BinaryOpNode>(suma->der);
    assert(producto && producto->op == "*");
    Interpreter interprete(ast);
    assert(interprete.ejecutar());
    assert(interprete.getCodigoRetorno() == -16);
    assert(interprete.getSalida().find("Valor: 15") != string::npos);
}

void testIndicesDeLecturaEnAST() {
    Lexer lexer("void main() { int matriz[2]; scanf(matriz[1]); return 0; }");
    lexer.analizar();
    Parser parser(lexer.getTokens());
    auto ast = parser.parsear();
    assert(parser.esAceptado());
    assert(ast->toTreeString().find("Indice [0]:") != string::npos);
}

void testPrecedenciaLogicaYAsociatividad() {
    Lexer lexer("void main() { int a; a = 10 - 3 - 2; if (a > 0 || a < 0 && a > -2) { println(a); } return a; }");
    lexer.analizar();
    Parser parser(lexer.getTokens());
    auto ast = parser.parsear();
    assert(parser.esAceptado());
    auto bloque = ast->funcionPrincipal->bloque;
    auto asignacion = dynamic_pointer_cast<AssignNode>(bloque->comandos[0]);
    auto resta = dynamic_pointer_cast<BinaryOpNode>(asignacion->expresion);
    assert(resta && resta->op == "-");
    auto izquierda = dynamic_pointer_cast<BinaryOpNode>(resta->izq);
    assert(izquierda && izquierda->op == "-");
    auto seleccion = dynamic_pointer_cast<IfNode>(bloque->comandos[1]);
    auto disyuncion = dynamic_pointer_cast<BinaryOpNode>(seleccion->condicion);
    assert(disyuncion && disyuncion->op == "||");
    auto conjuncion = dynamic_pointer_cast<BinaryOpNode>(disyuncion->der);
    assert(conjuncion && conjuncion->op == "&&");
}

void testLlamadaCombinadaEnExpresion() {
    Lexer lexer("func int doble(int n) { return n * 2; } void main() { int a; a = -(func doble(2) + 3); println(a); return a; }");
    lexer.analizar();
    Parser parser(lexer.getTokens());
    auto ast = parser.parsear();
    assert(parser.esAceptado());
    auto asignacion = dynamic_pointer_cast<AssignNode>(ast->funcionPrincipal->bloque->comandos[0]);
    auto negativo = dynamic_pointer_cast<UnaryOpNode>(asignacion->expresion);
    assert(negativo && negativo->op == "-");
    auto suma = dynamic_pointer_cast<BinaryOpNode>(negativo->operando);
    assert(suma && suma->op == "+" && dynamic_pointer_cast<FuncCallNode>(suma->izq));
    Interpreter interprete(ast);
    assert(interprete.ejecutar());
    assert(interprete.getCodigoRetorno() == -7);
}

void testMultiplesErroresSintacticos() {
    Lexer lexer("void main() { int a\n a = 1; println(a + ); return; }");
    lexer.analizar();
    Parser parser(lexer.getTokens());
    auto ast = parser.parsear();
    assert(ast != nullptr);
    assert(!parser.esAceptado());
    assert(parser.getErrores().size() >= 3);
    for (const auto& e : parser.getErrores()) {
        assert(e.linea > 0 && !e.tokenEncontrado.empty() && !e.construccionEsperada.empty());
    }
    assert(parser.getReporteErrores().find("ESPERADO") != string::npos);
}

int main() {
    cout << "\n=======================================================" << endl;
    cout << " SUITE DE PRUEBAS DEL ANALIZADOR SINTACTICO E INTERPRETE LP" << endl;
    cout << "=======================================================\n" << endl;

    testProgramaValidoMinimo();
    testProgramaInvalidoFaltaPuntoYComa();
    testProgramaInvalidoBloqueAbierto();
    testPalabraReservadaFunc();
    testEstructurasControl();
    testArreglosYDimensiones();
    testRecuperacionModoPanico();
    testEjecucionInterprete();
    testErrorRuntimeDivisionPorCero();
    testEntregable2Integral();
    testIndicesDeLecturaEnAST();
    testPrecedenciaLogicaYAsociatividad();
    testLlamadaCombinadaEnExpresion();
    testMultiplesErroresSintacticos();

    cout << "\n=======================================================" << endl;
    cout << " [PASS] TODAS LAS PRUEBAS SINTACTICAS Y DE EJECUCION EXITOSAS" << endl;
    cout << "=======================================================\n" << endl;

    return 0;
}
