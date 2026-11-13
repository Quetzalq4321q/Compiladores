#include "AST.h"

using namespace std;

/* Funciones auxiliares para formato de arbol */
static string getRama(bool isLast) {
    return isLast ? "\\-- " : "+-- ";
}

static string getSubPrefix(const string& prefix, bool isLast) {
    return prefix + (isLast ? "    " : "|   ");
}

/* LiteralNode */
LiteralNode::LiteralNode(const string& valor, const string& tipoDato, int linea)
    : ExprNode("Literal (" + tipoDato + ")", linea), valor(valor), tipoDato(tipoDato) {}

string LiteralNode::toTreeString(const string& prefix, bool isLast) const {
    ostringstream ss;
    ss << prefix << getRama(isLast) << "Literal [" << tipoDato << "]: " << valor
       << " (Linea " << linea << ")\n";
    return ss.str();
}

/* IdentifierNode */
IdentifierNode::IdentifierNode(const string& nombre, int linea)
    : ExprNode("Identificador", linea), nombre(nombre) {}

string IdentifierNode::toTreeString(const string& prefix, bool isLast) const {
    ostringstream ss;
    ss << prefix << getRama(isLast) << "Identificador: " << nombre
       << " (Linea " << linea << ")\n";
    string subPrefix = getSubPrefix(prefix, isLast);
    for (size_t i = 0; i < indices.size(); ++i) {
        bool lastIndex = (i + 1 == indices.size());
        ss << subPrefix << getRama(lastIndex) << "Indice [" << i << "]:\n";
        ss << indices[i]->toTreeString(getSubPrefix(subPrefix, lastIndex), true);
    }
    return ss.str();
}

/* BinaryOpNode */
BinaryOpNode::BinaryOpNode(const string& op, shared_ptr<ExprNode> izq, shared_ptr<ExprNode> der, int linea)
    : ExprNode("Operacion Binaria", linea), op(op), izq(izq), der(der) {}

string BinaryOpNode::toTreeString(const string& prefix, bool isLast) const {
    ostringstream ss;
    ss << prefix << getRama(isLast) << "Operador Binario: '" << op
       << "' (Linea " << linea << ")\n";
    string subPrefix = getSubPrefix(prefix, isLast);
    if (izq) {
        ss << izq->toTreeString(subPrefix, der == nullptr);
    }
    if (der) {
        ss << der->toTreeString(subPrefix, true);
    }
    return ss.str();
}

/* UnaryOpNode */
UnaryOpNode::UnaryOpNode(const string& op, shared_ptr<ExprNode> operando, int linea)
    : ExprNode("Operacion Unaria", linea), op(op), operando(operando) {}

string UnaryOpNode::toTreeString(const string& prefix, bool isLast) const {
    ostringstream ss;
    ss << prefix << getRama(isLast) << "Operador Unario: '" << op
       << "' (Linea " << linea << ")\n";
    if (operando) {
        ss << operando->toTreeString(getSubPrefix(prefix, isLast), true);
    }
    return ss.str();
}

/* FuncCallNode */
FuncCallNode::FuncCallNode(const string& nombreFuncion, int linea)
    : ExprNode("Llamada Funcion", linea), nombreFuncion(nombreFuncion) {}

string FuncCallNode::toTreeString(const string& prefix, bool isLast) const {
    ostringstream ss;
    ss << prefix << getRama(isLast) << "Llamada: func " << nombreFuncion
       << "() (Linea " << linea << ")\n";
    string subPrefix = getSubPrefix(prefix, isLast);
    for (size_t i = 0; i < argumentos.size(); ++i) {
        bool lastArg = (i + 1 == argumentos.size());
        ss << subPrefix << getRama(lastArg) << "Arg [" << i << "]:\n";
        ss << argumentos[i]->toTreeString(getSubPrefix(subPrefix, lastArg), true);
    }
    return ss.str();
}

/* VarDeclNode */
VarDeclNode::VarDeclNode(const string& tipo, const vector<string>& nombres, int linea)
    : ASTNode("Declaracion Variable", linea), tipo(tipo), nombres(nombres) {}

string VarDeclNode::toTreeString(const string& prefix, bool isLast) const {
    ostringstream ss;
    ss << prefix << getRama(isLast) << "Decl Variable [" << tipo << "]: ";
    for (size_t i = 0; i < nombres.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << nombres[i];
    }
    ss << " (Linea " << linea << ")\n";
    return ss.str();
}

/* AssignNode */
AssignNode::AssignNode(const string& id, shared_ptr<ExprNode> expresion, int linea)
    : ASTNode("Asignacion", linea), id(id), expresion(expresion) {}

string AssignNode::toTreeString(const string& prefix, bool isLast) const {
    ostringstream ss;
    ss << prefix << getRama(isLast) << "Asignacion: " << id;
    for (size_t i = 0; i < indices.size(); ++i) {
        ss << "[]";
    }
    ss << " = ... (Linea " << linea << ")\n";
    string subPrefix = getSubPrefix(prefix, isLast);
    for (size_t i = 0; i < indices.size(); ++i) {
        bool lastIndex = (i + 1 == indices.size() && !expresion);
        ss << subPrefix << getRama(lastIndex) << "Indice destino [" << i << "]:\n";
        ss << indices[i]->toTreeString(getSubPrefix(subPrefix, lastIndex), true);
    }
    if (expresion) {
        ss << expresion->toTreeString(subPrefix, true);
    }
    return ss.str();
}

/* ScanfNode */
ScanfNode::ScanfNode(int linea)
    : ASTNode("Lectura (scanf)", linea) {}

string ScanfNode::toTreeString(const string& prefix, bool isLast) const {
    ostringstream ss;
    ss << prefix << getRama(isLast) << "Comando: scanf() (Linea " << linea << ")\n";
    string subPrefix = getSubPrefix(prefix, isLast);
    for (size_t i = 0; i < terminos.size(); ++i) {
        bool lastT = (i + 1 == terminos.size());
        ss << subPrefix << getRama(lastT) << "Leer: " << terminos[i].id << "\n";
    }
    return ss.str();
}

/* PrintlnNode */
PrintlnNode::PrintlnNode(int linea)
    : ASTNode("Escritura (println)", linea) {}

string PrintlnNode::toTreeString(const string& prefix, bool isLast) const {
    ostringstream ss;
    ss << prefix << getRama(isLast) << "Comando: println() (Linea " << linea << ")\n";
    string subPrefix = getSubPrefix(prefix, isLast);
    for (size_t i = 0; i < terminos.size(); ++i) {
        bool lastT = (i + 1 == terminos.size());
        ss << terminos[i]->toTreeString(subPrefix, lastT);
    }
    return ss.str();
}

/* IfNode */
IfNode::IfNode(shared_ptr<ExprNode> condicion, shared_ptr<ASTNode> bloqueSi, shared_ptr<ASTNode> bloqueSino, int linea)
    : ASTNode("Seleccion (if)", linea), condicion(condicion), bloqueSi(bloqueSi), bloqueSino(bloqueSino) {}

string IfNode::toTreeString(const string& prefix, bool isLast) const {
    ostringstream ss;
    ss << prefix << getRama(isLast) << "Estructura: if (Linea " << linea << ")\n";
    string subPrefix = getSubPrefix(prefix, isLast);
    if (condicion) {
        ss << subPrefix << getRama(false) << "Condicion:\n";
        ss << condicion->toTreeString(getSubPrefix(subPrefix, false), true);
    }
    if (bloqueSi) {
        ss << subPrefix << getRama(bloqueSino == nullptr) << "Bloque Entonces:\n";
        ss << bloqueSi->toTreeString(getSubPrefix(subPrefix, bloqueSino == nullptr), true);
    }
    if (bloqueSino) {
        ss << subPrefix << getRama(true) << "Bloque Sino (else):\n";
        ss << bloqueSino->toTreeString(getSubPrefix(subPrefix, true), true);
    }
    return ss.str();
}

/* WhileNode */
WhileNode::WhileNode(shared_ptr<ExprNode> condicion, shared_ptr<ASTNode> bloque, int linea)
    : ASTNode("Repeticion (while)", linea), condicion(condicion), bloque(bloque) {}

string WhileNode::toTreeString(const string& prefix, bool isLast) const {
    ostringstream ss;
    ss << prefix << getRama(isLast) << "Estructura: while (Linea " << linea << ")\n";
    string subPrefix = getSubPrefix(prefix, isLast);
    if (condicion) {
        ss << subPrefix << getRama(false) << "Condicion:\n";
        ss << condicion->toTreeString(getSubPrefix(subPrefix, false), true);
    }
    if (bloque) {
        ss << subPrefix << getRama(true) << "Cuerpo del bucle:\n";
        ss << bloque->toTreeString(getSubPrefix(subPrefix, true), true);
    }
    return ss.str();
}

/* ReturnNode */
ReturnNode::ReturnNode(shared_ptr<ExprNode> expresion, int linea)
    : ASTNode("Retorno (return)", linea), expresion(expresion) {}

string ReturnNode::toTreeString(const string& prefix, bool isLast) const {
    ostringstream ss;
    ss << prefix << getRama(isLast) << "Comando: return (Linea " << linea << ")\n";
    if (expresion) {
        ss << expresion->toTreeString(getSubPrefix(prefix, isLast), true);
    }
    return ss.str();
}

/* BlockNode */
BlockNode::BlockNode(int linea)
    : ASTNode("Bloque", linea) {}

string BlockNode::toTreeString(const string& prefix, bool isLast) const {
    ostringstream ss;
    ss << prefix << getRama(isLast) << "Bloque { ... } (Linea " << linea << ")\n";
    string subPrefix = getSubPrefix(prefix, isLast);

    size_t total = variables.size() + comandos.size();
    size_t contador = 0;

    for (const auto& var : variables) {
        contador++;
        ss << var->toTreeString(subPrefix, contador == total);
    }
    for (const auto& cmd : comandos) {
        contador++;
        ss << cmd->toTreeString(subPrefix, contador == total);
    }
    return ss.str();
}

/* FunctionDeclNode */
FunctionDeclNode::FunctionDeclNode(const string& tipoRetorno, const string& nombre, int linea)
    : ASTNode("Declaracion Funcion", linea), tipoRetorno(tipoRetorno), nombre(nombre) {}

string FunctionDeclNode::toTreeString(const string& prefix, bool isLast) const {
    ostringstream ss;
    ss << prefix << getRama(isLast) << "Funcion: " << tipoRetorno << " " << nombre << "(";
    for (size_t i = 0; i < parametros.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << parametros[i].tipo << " " << parametros[i].nombre;
    }
    ss << ") (Linea " << linea << ")\n";
    if (bloque) {
        ss << bloque->toTreeString(getSubPrefix(prefix, isLast), true);
    }
    return ss.str();
}

/* ProgramNode */
ProgramNode::ProgramNode(int linea)
    : ASTNode("PROGRAMA LP", linea) {}

string ProgramNode::toTreeString(const string& prefix, bool isLast) const {
    ostringstream ss;
    ss << prefix << "PROGRAMA LP\n";
    size_t total = funciones.size() + (funcionPrincipal ? 1 : 0);
    size_t contador = 0;

    for (const auto& fn : funciones) {
        contador++;
        ss << fn->toTreeString(prefix, contador == total);
    }
    if (funcionPrincipal) {
        ss << funcionPrincipal->toTreeString(prefix, true);
    }
    return ss.str();
}
