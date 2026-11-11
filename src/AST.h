#pragma once

#include <string>
#include <vector>
#include <memory>
#include <sstream>

using namespace std;

/* Clase base para todos los nodos del Arbol de Sintaxis Abstracta (AST) */
class ASTNode {
public:
    int linea;
    string tipoNodo;

    explicit ASTNode(const string& tipoNodo, int linea = 1)
        : linea(linea), tipoNodo(tipoNodo) {}

    virtual ~ASTNode() = default;

    /* Genera una representacion visual jerarquica en formato arbol con ramas */
    virtual string toTreeString(const string& prefix = "", bool isLast = true) const = 0;
};

/* Nodo de Expresion base para expresiones evaluables */
class ExprNode : public ASTNode {
public:
    explicit ExprNode(const string& tipoNodo, int linea = 1)
        : ASTNode(tipoNodo, linea) {}
};

/* Nodo para Literales (enteros, decimales, texto) */
class LiteralNode : public ExprNode {
public:
    string valor;
    string tipoDato; /* "int", "float", "texto" */

    LiteralNode(const string& valor, const string& tipoDato, int linea = 1);
    string toTreeString(const string& prefix = "", bool isLast = true) const override;
};

/* Nodo para Identificadores y accesos a arreglos: id o id[dim] */
class IdentifierNode : public ExprNode {
public:
    string nombre;
    vector<shared_ptr<ExprNode>> indices; /* Para dimension2: [expr][expr] */

    IdentifierNode(const string& nombre, int linea = 1);
    string toTreeString(const string& prefix = "", bool isLast = true) const override;
};

/* Nodo para Operaciones Binarias (+, -, *, /, %, ==, !=, <, <=, >, >=, &&, ||) */
class BinaryOpNode : public ExprNode {
public:
    string op;
    shared_ptr<ExprNode> izq;
    shared_ptr<ExprNode> der;

    BinaryOpNode(const string& op, shared_ptr<ExprNode> izq, shared_ptr<ExprNode> der, int linea = 1);
    string toTreeString(const string& prefix = "", bool isLast = true) const override;
};

/* Nodo para Operaciones Unarias (+, -, !) */
class UnaryOpNode : public ExprNode {
public:
    string op;
    shared_ptr<ExprNode> operando;

    UnaryOpNode(const string& op, shared_ptr<ExprNode> operando, int linea = 1);
    string toTreeString(const string& prefix = "", bool isLast = true) const override;
};

/* Nodo para Llamada a Funcion: func id ( argumentos ) */
class FuncCallNode : public ExprNode {
public:
    string nombreFuncion;
    vector<shared_ptr<ExprNode>> argumentos;

    FuncCallNode(const string& nombreFuncion, int linea = 1);
    string toTreeString(const string& prefix = "", bool isLast = true) const override;
};

/* Nodo para Declaracion de Variable: int edad, nota; float[10] promedio; */
class VarDeclNode : public ASTNode {
public:
    string tipo;                 /* ej: "int", "float[10]" */
    vector<string> nombres;      /* ej: "edad", "nota" */

    VarDeclNode(const string& tipo, const vector<string>& nombres, int linea = 1);
    string toTreeString(const string& prefix = "", bool isLast = true) const override;
};

/* Nodo para Asignacion: id = COMPLEMENTO ; */
class AssignNode : public ASTNode {
public:
    string id;
    vector<shared_ptr<ExprNode>> indices; /* Para id[dim] = complemento si aplica */
    shared_ptr<ExprNode> expresion;      /* Expresion o llamada func */

    AssignNode(const string& id, shared_ptr<ExprNode> expresion, int linea = 1);
    string toTreeString(const string& prefix = "", bool isLast = true) const override;
};

/* Nodo para Comando de Lectura: scanf ( termino, ... ) ; */
class ScanfNode : public ASTNode {
public:
    struct TerminoLectura {
        string id;
        vector<shared_ptr<ExprNode>> dimensiones;
    };
    vector<TerminoLectura> terminos;

    explicit ScanfNode(int linea = 1);
    string toTreeString(const string& prefix = "", bool isLast = true) const override;
};

/* Nodo para Comando de Escritura: println ( termino, ... ) ; */
class PrintlnNode : public ASTNode {
public:
    vector<shared_ptr<ExprNode>> terminos; /* Cadenas, identificadores o expresiones */

    explicit PrintlnNode(int linea = 1);
    string toTreeString(const string& prefix = "", bool isLast = true) const override;
};

/* Nodo para Seleccion: if ( expr ) BLOQUE [else BLOQUE] */
class IfNode : public ASTNode {
public:
    shared_ptr<ExprNode> condicion;
    shared_ptr<ASTNode> bloqueSi;
    shared_ptr<ASTNode> bloqueSino; /* nullptr si no hay else */

    IfNode(shared_ptr<ExprNode> condicion, shared_ptr<ASTNode> bloqueSi, shared_ptr<ASTNode> bloqueSino = nullptr, int linea = 1);
    string toTreeString(const string& prefix = "", bool isLast = true) const override;
};

/* Nodo para Repeticion: while ( expr ) BLOQUE */
class WhileNode : public ASTNode {
public:
    shared_ptr<ExprNode> condicion;
    shared_ptr<ASTNode> bloque;

    WhileNode(shared_ptr<ExprNode> condicion, shared_ptr<ASTNode> bloque, int linea = 1);
    string toTreeString(const string& prefix = "", bool isLast = true) const override;
};

/* Nodo para Retorno: return EXPRESION ; */
class ReturnNode : public ASTNode {
public:
    shared_ptr<ExprNode> expresion;

    ReturnNode(shared_ptr<ExprNode> expresion, int linea = 1);
    string toTreeString(const string& prefix = "", bool isLast = true) const override;
};

/* Nodo para Bloque: { LISTA_VARIABLES COMANDOS } */
class BlockNode : public ASTNode {
public:
    vector<shared_ptr<VarDeclNode>> variables;
    vector<shared_ptr<ASTNode>> comandos;

    explicit BlockNode(int linea = 1);
    string toTreeString(const string& prefix = "", bool isLast = true) const override;
};

/* Nodo para Declaracion de Funcion: TIPO_RETORNO id ( PARAMETROS ) BLOQUE */
class FunctionDeclNode : public ASTNode {
public:
    string tipoRetorno;
    string nombre;
    struct Parametro {
        string tipo;
        string nombre;
    };
    vector<Parametro> parametros;
    shared_ptr<BlockNode> bloque;

    FunctionDeclNode(const string& tipoRetorno, const string& nombre, int linea = 1);
    string toTreeString(const string& prefix = "", bool isLast = true) const override;
};

/* Nodo Raiz del Programa: PROGRAMA -> LISTA_FUNCIONES PRINCIPAL */
class ProgramNode : public ASTNode {
public:
    vector<shared_ptr<FunctionDeclNode>> funciones;
    shared_ptr<FunctionDeclNode> funcionPrincipal; /* main() BLOQUE */

    explicit ProgramNode(int linea = 1);
    string toTreeString(const string& prefix = "", bool isLast = true) const override;
};
