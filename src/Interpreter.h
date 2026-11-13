#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <chrono>
#include "AST.h"

using namespace std;

/* Tipos de valores en tiempo de ejecucion */
enum class RuntimeType {
    VAL_VOID,
    VAL_INT,
    VAL_FLOAT,
    VAL_CHAR,
    VAL_BOOLEAN,
    VAL_STRING,
    VAL_ARRAY
};

/* Representacion de un valor evaluado en tiempo de ejecucion */
struct RuntimeValue {
    RuntimeType tipo;
    long long valorInt;
    double valorFloat;
    char valorChar;
    bool valorBool;
    string valorStr;
    vector<RuntimeValue> arreglo;

    RuntimeValue();
    static RuntimeValue makeInt(long long v);
    static RuntimeValue makeFloat(double v);
    static RuntimeValue makeChar(char v);
    static RuntimeValue makeBool(bool v);
    static RuntimeValue makeString(const string& v);
    static RuntimeValue makeArray(size_t tam, const RuntimeValue& defecto);
    static RuntimeValue makeVoid();

    string toString() const;
    long long toInt() const;
    double toFloat() const;
    bool toBool() const;
    bool isNumber() const;
};

/* Excepcion de control de flujo para sentencias 'return' */
struct ReturnException {
    RuntimeValue valor;
};

/* Entorno de ejecucion (Scope de variables) */
class Environment {
public:
    shared_ptr<Environment> padre;
    unordered_map<string, RuntimeValue> variables;

    explicit Environment(shared_ptr<Environment> padre = nullptr);

    /* Declara una variable en el ambito actual */
    void definir(const string& nombre, const RuntimeValue& valor);

    /* Asigna un nuevo valor a una variable existente (sube en los ambitos) */
    bool asignar(const string& nombre, const RuntimeValue& valor);

    /* Obtiene el valor de una variable */
    bool obtener(const string& nombre, RuntimeValue& out) const;
};

/* Motor de Ejecucion e Interpretacion del Lenguaje LP */
class Interpreter {
public:
    explicit Interpreter(shared_ptr<ProgramNode> ast);

    /* Ejecuta el programa comenzando en la funcion principal main() */
    bool ejecutar();

    /* Devuelve la salida en consola capturada durante la ejecucion */
    string getSalida() const;

    /* Devuelve la lista de errores en tiempo de ejecucion si existieron */
    const vector<string>& getErrores() const;

    /* Devuelve el tiempo de ejecucion en milisegundos */
    double getTiempoMs() const;

    /* Devuelve el codigo de retorno del programa */
    int getCodigoRetorno() const;

    /* Genera el reporte formateado de ejecucion */
    string getReporteEjecucion() const;

private:
    shared_ptr<ProgramNode> ast;
    ostringstream salidaConsola;
    vector<string> erroresRuntime;
    double tiempoMs;
    int codigoRetorno;
    unordered_map<string, shared_ptr<FunctionDeclNode>> funciones;
    long long pasosEjecutados;
    static const long long MAX_PASOS = 2000000; /* Previene bucles infinitos accidentales */

    /* Metodos recursivos de ejecucion */
    RuntimeValue ejecutarBloque(shared_ptr<ASTNode> nodo, shared_ptr<Environment> env);
    void ejecutarVariables(const vector<shared_ptr<VarDeclNode>>& vars, shared_ptr<Environment> env);
    RuntimeValue ejecutarComando(shared_ptr<ASTNode> cmd, shared_ptr<Environment> env);
    void ejecutarAsignacion(shared_ptr<AssignNode> asign, shared_ptr<Environment> env);
    void ejecutarIf(shared_ptr<IfNode> ifNode, shared_ptr<Environment> env);
    void ejecutarWhile(shared_ptr<WhileNode> whileNode, shared_ptr<Environment> env);
    void ejecutarPrintln(shared_ptr<PrintlnNode> printNode, shared_ptr<Environment> env);
    void ejecutarScanf(shared_ptr<ScanfNode> scanfNode, shared_ptr<Environment> env);

    /* Evaluacion de expresiones */
    RuntimeValue evaluarExpresion(shared_ptr<ExprNode> expr, shared_ptr<Environment> env);
    RuntimeValue evaluarLiteral(shared_ptr<LiteralNode> lit);
    RuntimeValue evaluarIdentificador(shared_ptr<IdentifierNode> idNode, shared_ptr<Environment> env);
    RuntimeValue evaluarOpBinaria(shared_ptr<BinaryOpNode> binOp, shared_ptr<Environment> env);
    RuntimeValue evaluarOpUnaria(shared_ptr<UnaryOpNode> unOp, shared_ptr<Environment> env);
    RuntimeValue invocarFuncion(const string& nombre, const vector<RuntimeValue>& args, int linea);

    /* Utilidades */
    RuntimeValue valorPorDefecto(const string& tipo);
    void reportarErrorRuntime(const string& msg, int linea);
    void verificarPasos(int linea);
};
