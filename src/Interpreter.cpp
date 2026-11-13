#include "Interpreter.h"
#include <iostream>
#include <cmath>
#include <iomanip>

using namespace std;

/* ==========================================================
   Implementacion de RuntimeValue
   ========================================================== */

RuntimeValue::RuntimeValue()
    : tipo(RuntimeType::VAL_VOID), valorInt(0), valorFloat(0.0),
      valorChar('\0'), valorBool(false), valorStr("") {}

RuntimeValue RuntimeValue::makeInt(long long v) {
    RuntimeValue val;
    val.tipo = RuntimeType::VAL_INT;
    val.valorInt = v;
    val.valorFloat = static_cast<double>(v);
    return val;
}

RuntimeValue RuntimeValue::makeFloat(double v) {
    RuntimeValue val;
    val.tipo = RuntimeType::VAL_FLOAT;
    val.valorFloat = v;
    val.valorInt = static_cast<long long>(v);
    return val;
}

RuntimeValue RuntimeValue::makeChar(char v) {
    RuntimeValue val;
    val.tipo = RuntimeType::VAL_CHAR;
    val.valorChar = v;
    val.valorInt = static_cast<long long>(v);
    val.valorStr = string(1, v);
    return val;
}

RuntimeValue RuntimeValue::makeBool(bool v) {
    RuntimeValue val;
    val.tipo = RuntimeType::VAL_BOOLEAN;
    val.valorBool = v;
    val.valorInt = v ? 1 : 0;
    return val;
}

RuntimeValue RuntimeValue::makeString(const string& v) {
    RuntimeValue val;
    val.tipo = RuntimeType::VAL_STRING;
    val.valorStr = v;
    return val;
}

RuntimeValue RuntimeValue::makeArray(size_t tam, const RuntimeValue& defecto) {
    RuntimeValue val;
    val.tipo = RuntimeType::VAL_ARRAY;
    val.arreglo.assign(tam, defecto);
    return val;
}

RuntimeValue RuntimeValue::makeVoid() {
    return RuntimeValue();
}

string RuntimeValue::toString() const {
    switch (tipo) {
        case RuntimeType::VAL_INT:
            return to_string(valorInt);
        case RuntimeType::VAL_FLOAT: {
            ostringstream oss;
            oss << valorFloat;
            string s = oss.str();
            if (s.find('.') == string::npos && s.find('e') == string::npos) s += ".0";
            return s;
        }
        case RuntimeType::VAL_CHAR:
            return string(1, valorChar);
        case RuntimeType::VAL_BOOLEAN:
            return valorBool ? "true" : "false";
        case RuntimeType::VAL_STRING:
            return valorStr;
        case RuntimeType::VAL_ARRAY: {
            string res = "[";
            for (size_t i = 0; i < arreglo.size(); ++i) {
                if (i > 0) res += ", ";
                res += arreglo[i].toString();
            }
            res += "]";
            return res;
        }
        default:
            return "void";
    }
}

long long RuntimeValue::toInt() const {
    if (tipo == RuntimeType::VAL_INT) return valorInt;
    if (tipo == RuntimeType::VAL_FLOAT) return static_cast<long long>(valorFloat);
    if (tipo == RuntimeType::VAL_BOOLEAN) return valorBool ? 1 : 0;
    if (tipo == RuntimeType::VAL_CHAR) return static_cast<long long>(valorChar);
    return 0;
}

double RuntimeValue::toFloat() const {
    if (tipo == RuntimeType::VAL_FLOAT) return valorFloat;
    if (tipo == RuntimeType::VAL_INT) return static_cast<double>(valorInt);
    if (tipo == RuntimeType::VAL_BOOLEAN) return valorBool ? 1.0 : 0.0;
    return 0.0;
}

bool RuntimeValue::toBool() const {
    if (tipo == RuntimeType::VAL_BOOLEAN) return valorBool;
    if (tipo == RuntimeType::VAL_INT) return valorInt != 0;
    if (tipo == RuntimeType::VAL_FLOAT) return valorFloat != 0.0;
    if (tipo == RuntimeType::VAL_STRING) return !valorStr.empty();
    return false;
}

bool RuntimeValue::isNumber() const {
    return tipo == RuntimeType::VAL_INT || tipo == RuntimeType::VAL_FLOAT;
}

/* ==========================================================
   Implementacion de Environment
   ========================================================== */

Environment::Environment(shared_ptr<Environment> padre)
    : padre(padre) {}

void Environment::definir(const string& nombre, const RuntimeValue& valor) {
    variables[nombre] = valor;
}

bool Environment::asignar(const string& nombre, const RuntimeValue& valor) {
    auto it = variables.find(nombre);
    if (it != variables.end()) {
        it->second = valor;
        return true;
    }
    if (padre) {
        return padre->asignar(nombre, valor);
    }
    return false;
}

bool Environment::obtener(const string& nombre, RuntimeValue& out) const {
    auto it = variables.find(nombre);
    if (it != variables.end()) {
        out = it->second;
        return true;
    }
    if (padre) {
        return padre->obtener(nombre, out);
    }
    return false;
}

/* ==========================================================
   Implementacion de Interpreter
   ========================================================== */

Interpreter::Interpreter(shared_ptr<ProgramNode> ast)
    : ast(ast), tiempoMs(0.0), codigoRetorno(0), pasosEjecutados(0) {}

void Interpreter::reportarErrorRuntime(const string& msg, int linea) {
    string err = "[ERROR EN TIEMPO DE EJECUCION] Linea " + to_string(linea) + ": " + msg;
    erroresRuntime.push_back(err);
    salidaConsola << "\r\n" << err << "\r\n";
}

void Interpreter::verificarPasos(int linea) {
    pasosEjecutados++;
    if (pasosEjecutados > MAX_PASOS) {
        reportarErrorRuntime("Limite maximo de operaciones excedido (posible bucle infinito)", linea);
        throw runtime_error("Bucle infinito detectado");
    }
}

RuntimeValue Interpreter::valorPorDefecto(const string& tipo) {
    if (tipo.find("int") != string::npos) return RuntimeValue::makeInt(0);
    if (tipo.find("float") != string::npos) return RuntimeValue::makeFloat(0.0);
    if (tipo.find("char") != string::npos) return RuntimeValue::makeChar('\0');
    if (tipo.find("boolean") != string::npos) return RuntimeValue::makeBool(false);
    if (tipo.find("string") != string::npos) return RuntimeValue::makeString("");
    return RuntimeValue::makeVoid();
}

bool Interpreter::ejecutar() {
    salidaConsola.str("");
    salidaConsola.clear();
    erroresRuntime.clear();
    funciones.clear();
    codigoRetorno = 0;
    pasosEjecutados = 0;

    if (!ast || !ast->funcionPrincipal) {
        reportarErrorRuntime("No se encontro la funcion principal 'main()' para ejecutar", 1);
        return false;
    }

    /* 1. Indexar todas las funciones declaradas */
    for (const auto& fn : ast->funciones) {
        funciones[fn->nombre] = fn;
    }

    auto inicio = chrono::high_resolution_clock::now();

    try {
        /* 2. Crear ambito global */
        auto envGlobal = make_shared<Environment>();

        /* 3. Ejecutar bloque de main() */
        auto bloqueMain = ast->funcionPrincipal->bloque;
        if (bloqueMain) {
            RuntimeValue res = ejecutarBloque(bloqueMain, envGlobal);
            codigoRetorno = static_cast<int>(res.toInt());
        }
    } catch (const ReturnException& ret) {
        codigoRetorno = static_cast<int>(ret.valor.toInt());
    } catch (const exception& e) {
        /* Error critico ya registrado */
    }

    auto fin = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> dur = fin - inicio;
    tiempoMs = dur.count();

    return erroresRuntime.empty();
}

string Interpreter::getSalida() const {
    return salidaConsola.str();
}

const vector<string>& Interpreter::getErrores() const {
    return erroresRuntime;
}

double Interpreter::getTiempoMs() const {
    return tiempoMs;
}

int Interpreter::getCodigoRetorno() const {
    return codigoRetorno;
}

string Interpreter::getReporteEjecucion() const {
    ostringstream ss;
    ss << "===========================================================================================\r\n";
    ss << "                          CONSOLA DE EJECUCION DEL PROGRAMA LP                             \r\n";
    ss << "===========================================================================================\r\n\r\n";

    if (erroresRuntime.empty()) {
        ss << "[ESTADO]: EJECUCION EXITOSA\r\n";
    } else {
        ss << "[ESTADO]: EJECUCION CON ERRORES (" << erroresRuntime.size() << " incidencias)\r\n";
    }
    ss << "Tiempo de ejecucion: " << fixed << setprecision(3) << tiempoMs << " ms\r\n";
    ss << "Codigo de retorno  : " << codigoRetorno << "\r\n";
    ss << "-------------------------------------------------------------------------------------------\r\n";
    ss << "SALIDA DEL PROGRAMA (println):\r\n";
    ss << "-------------------------------------------------------------------------------------------\r\n";

    string sal = salidaConsola.str();
    if (sal.empty()) {
        ss << "(El programa no genero salidas por consola)\r\n";
    } else {
        ss << sal << "\r\n";
    }

    if (!erroresRuntime.empty()) {
        ss << "-------------------------------------------------------------------------------------------\r\n";
        ss << "DETALLE DE ERRORES EN TIEMPO DE EJECUCION:\r\n";
        for (const auto& err : erroresRuntime) {
            ss << " * " << err << "\r\n";
        }
    }
    ss << "===========================================================================================\r\n";
    return ss.str();
}

RuntimeValue Interpreter::ejecutarBloque(shared_ptr<ASTNode> nodo, shared_ptr<Environment> env) {
    if (!nodo) return RuntimeValue::makeVoid();

    auto bloque = dynamic_pointer_cast<BlockNode>(nodo);
    if (!bloque) {
        return ejecutarComando(nodo, env);
    }

    auto envBloque = make_shared<Environment>(env);

    /* 1. Declarar variables locales */
    ejecutarVariables(bloque->variables, envBloque);

    /* 2. Ejecutar comandos secuenciales */
    for (const auto& cmd : bloque->comandos) {
        if (!erroresRuntime.empty()) break;
        ejecutarComando(cmd, envBloque);
    }

    return RuntimeValue::makeVoid();
}

void Interpreter::ejecutarVariables(const vector<shared_ptr<VarDeclNode>>& vars, shared_ptr<Environment> env) {
    for (const auto& decl : vars) {
        for (const auto& nomConDim : decl->nombres) {
            /* Extraer nombre y posibles dimensiones (ej: arr[10][20]) */
            size_t corch = nomConDim.find('[');
            string nombreLimpio = (corch == string::npos) ? nomConDim : nomConDim.substr(0, corch);

            if (corch == string::npos) {
                /* Variable simple */
                env->definir(nombreLimpio, valorPorDefecto(decl->tipo));
            } else {
                /* Arreglo con dimensiones */
                vector<int> dims;
                size_t p = corch;
                while (p < nomConDim.size() && nomConDim[p] == '[') {
                    size_t fin = nomConDim.find(']', p);
                    if (fin != string::npos) {
                        string tamStr = nomConDim.substr(p + 1, fin - p - 1);
                        try {
                            dims.push_back(stoi(tamStr));
                        } catch (...) {
                            dims.push_back(10);
                        }
                        p = fin + 1;
                    } else break;
                }

                /* Construir estructura multidimensional */
                RuntimeValue defVal = valorPorDefecto(decl->tipo);
                if (dims.empty()) dims.push_back(10);

                RuntimeValue arrVal = RuntimeValue::makeArray(dims.back(), defVal);
                for (int d = static_cast<int>(dims.size()) - 2; d >= 0; --d) {
                    arrVal = RuntimeValue::makeArray(dims[d], arrVal);
                }
                env->definir(nombreLimpio, arrVal);
            }
        }
    }
}

RuntimeValue Interpreter::ejecutarComando(shared_ptr<ASTNode> cmd, shared_ptr<Environment> env) {
    if (!cmd) return RuntimeValue::makeVoid();
    verificarPasos(cmd->linea);

    if (auto asign = dynamic_pointer_cast<AssignNode>(cmd)) {
        ejecutarAsignacion(asign, env);
    } else if (auto ifNode = dynamic_pointer_cast<IfNode>(cmd)) {
        ejecutarIf(ifNode, env);
    } else if (auto whileNode = dynamic_pointer_cast<WhileNode>(cmd)) {
        ejecutarWhile(whileNode, env);
    } else if (auto printNode = dynamic_pointer_cast<PrintlnNode>(cmd)) {
        ejecutarPrintln(printNode, env);
    } else if (auto scanfNode = dynamic_pointer_cast<ScanfNode>(cmd)) {
        ejecutarScanf(scanfNode, env);
    } else if (auto retNode = dynamic_pointer_cast<ReturnNode>(cmd)) {
        RuntimeValue val = retNode->expresion ? evaluarExpresion(retNode->expresion, env) : RuntimeValue::makeInt(0);
        ReturnException ex;
        ex.valor = val;
        throw ex;
    } else if (auto fnCall = dynamic_pointer_cast<FuncCallNode>(cmd)) {
        vector<RuntimeValue> args;
        for (const auto& a : fnCall->argumentos) {
            args.push_back(evaluarExpresion(a, env));
        }
        invocarFuncion(fnCall->nombreFuncion, args, fnCall->linea);
    } else if (auto bloque = dynamic_pointer_cast<BlockNode>(cmd)) {
        ejecutarBloque(bloque, env);
    }

    return RuntimeValue::makeVoid();
}

void Interpreter::ejecutarAsignacion(shared_ptr<AssignNode> asign, shared_ptr<Environment> env) {
    RuntimeValue val = evaluarExpresion(asign->expresion, env);

    if (asign->indices.empty()) {
        /* Asignacion simple: id = expr */
        if (!env->asignar(asign->id, val)) {
            /* Si no estaba asignada, definirla */
            env->definir(asign->id, val);
        }
    } else {
        /* Asignacion en arreglo: arr[i][j] = val */
        RuntimeValue actualArr;
        if (!env->obtener(asign->id, actualArr)) {
            reportarErrorRuntime("Variable de arreglo no declarada: '" + asign->id + "'", asign->linea);
            return;
        }

        /* Evaluar indices */
        vector<long long> idxs;
        for (const auto& idxExpr : asign->indices) {
            RuntimeValue idxVal = evaluarExpresion(idxExpr, env);
            idxs.push_back(idxVal.toInt());
        }

        /* Navegar arreglo y asignar */
        RuntimeValue* cursor = &actualArr;
        for (size_t k = 0; k < idxs.size(); ++k) {
            long long i = idxs[k];
            if (cursor->tipo != RuntimeType::VAL_ARRAY || i < 0 || i >= static_cast<long long>(cursor->arreglo.size())) {
                reportarErrorRuntime("Indice de arreglo fuera de rango (" + to_string(i) + ")", asign->linea);
                return;
            }
            if (k + 1 == idxs.size()) {
                cursor->arreglo[i] = val;
            } else {
                cursor = &(cursor->arreglo[i]);
            }
        }
        env->asignar(asign->id, actualArr);
    }
}

void Interpreter::ejecutarIf(shared_ptr<IfNode> ifNode, shared_ptr<Environment> env) {
    RuntimeValue cond = evaluarExpresion(ifNode->condicion, env);
    if (cond.toBool()) {
        ejecutarBloque(ifNode->bloqueSi, env);
    } else if (ifNode->bloqueSino) {
        ejecutarBloque(ifNode->bloqueSino, env);
    }
}

void Interpreter::ejecutarWhile(shared_ptr<WhileNode> whileNode, shared_ptr<Environment> env) {
    while (evaluarExpresion(whileNode->condicion, env).toBool()) {
        verificarPasos(whileNode->linea);
        if (!erroresRuntime.empty()) break;
        ejecutarBloque(whileNode->bloque, env);
    }
}

void Interpreter::ejecutarPrintln(shared_ptr<PrintlnNode> printNode, shared_ptr<Environment> env) {
    for (size_t i = 0; i < printNode->terminos.size(); ++i) {
        RuntimeValue val = evaluarExpresion(printNode->terminos[i], env);
        if (i > 0) salidaConsola << " ";
        salidaConsola << val.toString();
    }
    salidaConsola << "\r\n";
}

void Interpreter::ejecutarScanf(shared_ptr<ScanfNode> scanfNode, shared_ptr<Environment> env) {
    for (const auto& term : scanfNode->terminos) {
        /* En ejecucion no interactiva se asigna un valor por defecto o 0 */
        RuntimeValue valDef = RuntimeValue::makeInt(0);
        env->asignar(term.id, valDef);
    }
}

RuntimeValue Interpreter::evaluarExpresion(shared_ptr<ExprNode> expr, shared_ptr<Environment> env) {
    if (!expr) return RuntimeValue::makeVoid();

    if (auto lit = dynamic_pointer_cast<LiteralNode>(expr)) {
        return evaluarLiteral(lit);
    }
    if (auto idNode = dynamic_pointer_cast<IdentifierNode>(expr)) {
        return evaluarIdentificador(idNode, env);
    }
    if (auto binOp = dynamic_pointer_cast<BinaryOpNode>(expr)) {
        return evaluarOpBinaria(binOp, env);
    }
    if (auto unOp = dynamic_pointer_cast<UnaryOpNode>(expr)) {
        return evaluarOpUnaria(unOp, env);
    }
    if (auto call = dynamic_pointer_cast<FuncCallNode>(expr)) {
        vector<RuntimeValue> args;
        for (const auto& a : call->argumentos) {
            args.push_back(evaluarExpresion(a, env));
        }
        return invocarFuncion(call->nombreFuncion, args, call->linea);
    }

    return RuntimeValue::makeVoid();
}

RuntimeValue Interpreter::evaluarLiteral(shared_ptr<LiteralNode> lit) {
    if (lit->tipoDato == "int") {
        try {
            return RuntimeValue::makeInt(stoll(lit->valor));
        } catch (...) {
            return RuntimeValue::makeInt(0);
        }
    }
    if (lit->tipoDato == "float") {
        try {
            return RuntimeValue::makeFloat(stod(lit->valor));
        } catch (...) {
            return RuntimeValue::makeFloat(0.0);
        }
    }
    if (lit->tipoDato == "texto" || lit->tipoDato == "string") {
        string s = lit->valor;
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
            s = s.substr(1, s.size() - 2);
        }
        return RuntimeValue::makeString(s);
    }
    if (lit->tipoDato == "char") {
        char c = lit->valor.empty() ? '\0' : lit->valor[0];
        return RuntimeValue::makeChar(c);
    }
    if (lit->tipoDato == "boolean") {
        return RuntimeValue::makeBool(lit->valor == "true");
    }
    return RuntimeValue::makeVoid();
}

RuntimeValue Interpreter::evaluarIdentificador(shared_ptr<IdentifierNode> idNode, shared_ptr<Environment> env) {
    RuntimeValue val;
    if (!env->obtener(idNode->nombre, val)) {
        reportarErrorRuntime("Variable no inicializada o no declarada: '" + idNode->nombre + "'", idNode->linea);
        return RuntimeValue::makeInt(0);
    }

    if (idNode->indices.empty()) {
        return val;
    }

    /* Acceso indexado a arreglo */
    RuntimeValue* cursor = &val;
    for (const auto& idxExpr : idNode->indices) {
        long long i = evaluarExpresion(idxExpr, env).toInt();
        if (cursor->tipo != RuntimeType::VAL_ARRAY || i < 0 || i >= static_cast<long long>(cursor->arreglo.size())) {
            reportarErrorRuntime("Indice fuera de limites (" + to_string(i) + ") en arreglo '" + idNode->nombre + "'", idNode->linea);
            return RuntimeValue::makeInt(0);
        }
        cursor = &(cursor->arreglo[i]);
    }

    return *cursor;
}

RuntimeValue Interpreter::evaluarOpBinaria(shared_ptr<BinaryOpNode> binOp, shared_ptr<Environment> env) {
    RuntimeValue izq = evaluarExpresion(binOp->izq, env);
    string op = binOp->op;

    /* Cortocircuito logico */
    if (op == "&&") {
        if (!izq.toBool()) return RuntimeValue::makeBool(false);
        return RuntimeValue::makeBool(evaluarExpresion(binOp->der, env).toBool());
    }
    if (op == "||") {
        if (izq.toBool()) return RuntimeValue::makeBool(true);
        return RuntimeValue::makeBool(evaluarExpresion(binOp->der, env).toBool());
    }

    RuntimeValue der = evaluarExpresion(binOp->der, env);

    /* Concatenacion de cadenas */
    if (op == "+" && (izq.tipo == RuntimeType::VAL_STRING || der.tipo == RuntimeType::VAL_STRING)) {
        return RuntimeValue::makeString(izq.toString() + der.toString());
    }

    /* Operaciones aritmeticas */
    bool esFloat = (izq.tipo == RuntimeType::VAL_FLOAT || der.tipo == RuntimeType::VAL_FLOAT);

    if (op == "+") {
        if (esFloat) return RuntimeValue::makeFloat(izq.toFloat() + der.toFloat());
        return RuntimeValue::makeInt(izq.toInt() + der.toInt());
    }
    if (op == "-") {
        if (esFloat) return RuntimeValue::makeFloat(izq.toFloat() - der.toFloat());
        return RuntimeValue::makeInt(izq.toInt() - der.toInt());
    }
    if (op == "*") {
        if (esFloat) return RuntimeValue::makeFloat(izq.toFloat() * der.toFloat());
        return RuntimeValue::makeInt(izq.toInt() * der.toInt());
    }
    if (op == "/") {
        double divisor = der.toFloat();
        if (abs(divisor) < 1e-12) {
            reportarErrorRuntime("Division por cero", binOp->linea);
            return RuntimeValue::makeInt(0);
        }
        if (esFloat) return RuntimeValue::makeFloat(izq.toFloat() / divisor);
        return RuntimeValue::makeInt(izq.toInt() / der.toInt());
    }
    if (op == "%") {
        long long divisor = der.toInt();
        if (divisor == 0) {
            reportarErrorRuntime("Operacion modulo (%) por cero", binOp->linea);
            return RuntimeValue::makeInt(0);
        }
        return RuntimeValue::makeInt(izq.toInt() % divisor);
    }

    /* Operaciones relacionales */
    if (op == "==") {
        if (izq.tipo == RuntimeType::VAL_STRING || der.tipo == RuntimeType::VAL_STRING) {
            return RuntimeValue::makeBool(izq.toString() == der.toString());
        }
        return RuntimeValue::makeBool(abs(izq.toFloat() - der.toFloat()) < 1e-9);
    }
    if (op == "!=") {
        if (izq.tipo == RuntimeType::VAL_STRING || der.tipo == RuntimeType::VAL_STRING) {
            return RuntimeValue::makeBool(izq.toString() != der.toString());
        }
        return RuntimeValue::makeBool(abs(izq.toFloat() - der.toFloat()) >= 1e-9);
    }
    if (op == "<")  return RuntimeValue::makeBool(izq.toFloat() < der.toFloat());
    if (op == "<=") return RuntimeValue::makeBool(izq.toFloat() <= der.toFloat());
    if (op == ">")  return RuntimeValue::makeBool(izq.toFloat() > der.toFloat());
    if (op == ">=") return RuntimeValue::makeBool(izq.toFloat() >= der.toFloat());

    return RuntimeValue::makeVoid();
}

RuntimeValue Interpreter::evaluarOpUnaria(shared_ptr<UnaryOpNode> unOp, shared_ptr<Environment> env) {
    RuntimeValue val = evaluarExpresion(unOp->operando, env);
    if (unOp->op == "-") {
        if (val.tipo == RuntimeType::VAL_FLOAT) return RuntimeValue::makeFloat(-val.valorFloat);
        return RuntimeValue::makeInt(-val.valorInt);
    }
    if (unOp->op == "!") {
        return RuntimeValue::makeBool(!val.toBool());
    }
    return val;
}

RuntimeValue Interpreter::invocarFuncion(const string& nombre, const vector<RuntimeValue>& args, int linea) {
    auto it = funciones.find(nombre);
    if (it == funciones.end()) {
        reportarErrorRuntime("Llamada a funcion no definida: '" + nombre + "'", linea);
        return RuntimeValue::makeInt(0);
    }

    auto declFn = it->second;
    auto envFn = make_shared<Environment>();

    /* Asignar parametros a los argumentos evaluados */
    for (size_t i = 0; i < declFn->parametros.size(); ++i) {
        string nomParam = declFn->parametros[i].nombre;
        RuntimeValue valArg = (i < args.size()) ? args[i] : valorPorDefecto(declFn->parametros[i].tipo);
        envFn->definir(nomParam, valArg);
    }

    try {
        if (declFn->bloque) {
            ejecutarBloque(declFn->bloque, envFn);
        }
    } catch (const ReturnException& ret) {
        return ret.valor;
    }

    return RuntimeValue::makeVoid();
}
