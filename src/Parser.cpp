#include "Parser.h"
#include <iostream>

using namespace std;

Parser::Parser(const vector<Token>& tokens)
    : tokens(tokens), pos(0), astRaiz(nullptr) {}

/* Token actual o EOF si se llego al final */
Token Parser::actual() const {
    if (pos < tokens.size()) return tokens[pos];
    int lin = tokens.empty() ? 1 : tokens.back().linea;
    return Token(LexTokenType::LEX_EOF, "EOF", lin);
}

Token Parser::anterior() const {
    if (pos > 0 && pos - 1 < tokens.size()) return tokens[pos - 1];
    return Token(LexTokenType::LEX_EOF, "INICIO", 1);
}

Token Parser::siguiente() const {
    if (pos + 1 < tokens.size()) return tokens[pos + 1];
    int lin = tokens.empty() ? 1 : tokens.back().linea;
    return Token(LexTokenType::LEX_EOF, "EOF", lin);
}

bool Parser::esFin() const {
    return pos >= tokens.size() || actual().tipo == LexTokenType::LEX_EOF;
}

bool Parser::verificar(LexTokenType tipo) const {
    if (esFin()) return tipo == LexTokenType::LEX_EOF;
    return actual().tipo == tipo;
}

bool Parser::coincidir(LexTokenType tipo) {
    if (verificar(tipo)) {
        pos++;
        return true;
    }
    return false;
}

Token Parser::consumir(LexTokenType tipo, const string& mensajeEsperado) {
    if (verificar(tipo)) {
        Token t = actual();
        pos++;
        return t;
    }
    reportarError("Se esperaba " + mensajeEsperado + ", pero se encontro '" + actual().lexema + "'",
                  tokenTypeToString(tipo));
    return actual();
}

void Parser::reportarError(const string& mensaje, const string& esperado) {
    Token t = actual();
    SyntaxError err;
    err.linea = t.linea;
    err.tokenEncontrado = t.lexema.empty() ? tokenTypeToString(t.tipo) : t.lexema;
    err.construccionEsperada = esperado;
    err.mensaje = mensaje;
    errores.push_back(err);
}

/* Recuperacion de errores: salta tokens hasta encontrar un punto seguro de sincronizacion */
void Parser::sincronizar() {
    if (pos < tokens.size()) pos++;

    while (!esFin()) {
        if (anterior().tipo == LexTokenType::SEMICOLON) return;
        if (anterior().tipo == LexTokenType::RBRACE) return;

        switch (actual().tipo) {
            case LexTokenType::KW_IF:
            case LexTokenType::KW_WHILE:
            case LexTokenType::KW_RETURN:
            case LexTokenType::KW_SCANF:
            case LexTokenType::KW_PRINTLN:
            case LexTokenType::KW_INT:
            case LexTokenType::KW_FLOAT:
            case LexTokenType::KW_CHAR:
            case LexTokenType::KW_BOOLEAN:
            case LexTokenType::KW_VOID:
            case LexTokenType::KW_MAIN:
            case LexTokenType::KW_FUNC:
            case LexTokenType::RBRACE:
                return;
            default:
                pos++;
        }
    }
}

bool Parser::esTipoBase(LexTokenType t) const {
    return t == LexTokenType::KW_INT ||
           t == LexTokenType::KW_FLOAT ||
           t == LexTokenType::KW_CHAR ||
           t == LexTokenType::KW_BOOLEAN ||
           t == LexTokenType::KW_STRING;
}

bool Parser::esInicioComando(LexTokenType t) const {
    return t == LexTokenType::ID ||
           t == LexTokenType::KW_SCANF ||
           t == LexTokenType::KW_PRINTLN ||
           t == LexTokenType::KW_IF ||
           t == LexTokenType::KW_WHILE ||
           t == LexTokenType::KW_RETURN ||
           t == LexTokenType::KW_FUNC;
}

/* ==========================================================
   Metodos del Analizador Sintactico (Descenso Recursivo)
   ========================================================== */

shared_ptr<ProgramNode> Parser::parsear() {
    errores.clear();
    pos = 0;
    astRaiz = parsePrograma();
    return astRaiz;
}

bool Parser::esAceptado() const {
    return errores.empty();
}

const vector<SyntaxError>& Parser::getErrores() const {
    return errores;
}

string Parser::getReporteErrores() const {
    ostringstream ss;
    ss << "=== REPORTE DE ANALISIS SINTACTICO (LP) ===\r\n";
    if (errores.empty()) {
        ss << "[ESTADO]: PROGRAMA ACEPTADO. Estructura sintactica 100% valida.\r\n";
        return ss.str();
    }

    ss << "[ESTADO]: PROGRAMA RECHAZADO. Total de errores sintacticos: " << errores.size() << "\r\n";
    ss << "-------------------------------------------------------------------------------------------\r\n";
    ss << "LINEA      TOKEN ENCONTRADO              DESCRIPCION DEL ERROR SINTACTICO\r\n";
    ss << "-------------------------------------------------------------------------------------------\r\n";

    for (const auto& e : errores) {
        string lin = "Linea " + to_string(e.linea);
        lin.resize(11, ' ');
        string tok = "'" + e.tokenEncontrado + "'";
        if (tok.size() < 30) tok.resize(30, ' ');
        else tok += " ";
        ss << lin << tok << e.mensaje << "\r\n";
    }
    ss << "-------------------------------------------------------------------------------------------\r\n";
    return ss.str();
}

/* PROGRAMA -> LISTA_FUNCIONES PRINCIPAL */
shared_ptr<ProgramNode> Parser::parsePrograma() {
    auto prog = make_shared<ProgramNode>(actual().linea);

    /* Parsear declaraciones de funciones previas a main si existen */
    parseListaFunciones(prog->funciones);

    /* Parsear la funcion principal main() */
    prog->funcionPrincipal = parsePrincipal();

    if (!esFin()) {
        reportarError("Tokens adicionales inesperados al final del archivo", "EOF");
    }

    return prog;
}

/* LISTA_FUNCIONES -> DECLARACION_FUNCION LISTA_FUNCIONES | ε */
void Parser::parseListaFunciones(vector<shared_ptr<FunctionDeclNode>>& lista) {
    while (!esFin()) {
        /* Si empieza con 'func' es una declaracion de funcion con prefijo */
        if (verificar(LexTokenType::KW_FUNC)) {
            auto fn = parseDeclaracionFuncion();
            if (fn) lista.push_back(fn);
            continue;
        }

        /* Comprobar si estamos en main():
           - "void main" o "int main" o "main (" */
        bool esRetornoVoid = verificar(LexTokenType::KW_VOID);
        bool esRetornoTipo = esTipoBase(actual().tipo);

        if (esRetornoVoid || esRetornoTipo) {
            Token sig = siguiente();
            if (sig.tipo == LexTokenType::KW_MAIN || sig.lexema == "main") {
                /* Es la funcion principal */
                break;
            }
            if (sig.tipo == LexTokenType::ID) {
                /* Es una declaracion de funcion */
                auto fn = parseDeclaracionFuncion();
                if (fn) lista.push_back(fn);
                continue;
            }
        } else if (verificar(LexTokenType::KW_MAIN) || actual().lexema == "main") {
            /* main sin tipo explícito */
            break;
        }

        /* Ninguna de las anteriores: romper */
        break;
    }
}

/* DECLARACION_FUNCION -> TIPO_RETORNO id ( PARAMETROS ) BLOQUE */
shared_ptr<FunctionDeclNode> Parser::parseDeclaracionFuncion() {
    int lin = actual().linea;

    /* Admite prefijo opcional 'func' para compatibilidad extendida */
    coincidir(LexTokenType::KW_FUNC);

    string tipoRet = "void";
    if (verificar(LexTokenType::KW_VOID) || esTipoBase(actual().tipo)) {
        tipoRet = parseTipoRetorno();
    }

    Token idTok = consumir(LexTokenType::ID, "nombre de la funcion");
    auto fnNode = make_shared<FunctionDeclNode>(tipoRet, idTok.lexema, lin);

    consumir(LexTokenType::LPAREN, "'(' tras el nombre de la funcion");
    parseParametros(fnNode->parametros);
    consumir(LexTokenType::RPAREN, "')' tras los parametros de la funcion");

    fnNode->bloque = parseBloque();
    return fnNode;
}

/* TIPO_RETORNO -> TIPO | void */
string Parser::parseTipoRetorno() {
    if (coincidir(LexTokenType::KW_VOID)) {
        return "void";
    }
    return parseTipo();
}

/* TIPO -> TIPO_BASE DIMENSION */
string Parser::parseTipo() {
    string base = parseTipoBase();
    string dim = parseDimension();
    return base + dim;
}

/* TIPO_BASE -> char | float | int | boolean */
string Parser::parseTipoBase() {
    if (coincidir(LexTokenType::KW_INT))     return "int";
    if (coincidir(LexTokenType::KW_FLOAT))   return "float";
    if (coincidir(LexTokenType::KW_CHAR))    return "char";
    if (coincidir(LexTokenType::KW_BOOLEAN)) return "boolean";
    if (coincidir(LexTokenType::KW_STRING))  return "string";

    reportarError("tipo de dato base ('int', 'float', 'char', 'boolean')", "TIPO_BASE");
    return "desconocido";
}

/* DIMENSION -> [ num_int ] DIMENSION | ε */
string Parser::parseDimension() {
    string res = "";
    while (verificar(LexTokenType::LBRACKET)) {
        consumir(LexTokenType::LBRACKET, "'[' para dimension de arreglo");
        Token num = consumir(LexTokenType::NUM_INT, "tamano entero del arreglo");
        consumir(LexTokenType::RBRACKET, "']' al cerrar dimension de arreglo");
        res += "[" + num.lexema + "]";
    }
    return res;
}

/* PARAMETROS -> TIPO id LISTA_PARAMETROS | ε */
void Parser::parseParametros(vector<FunctionDeclNode::Parametro>& params) {
    if (esTipoBase(actual().tipo)) {
        string t = parseTipo();
        Token idTok = consumir(LexTokenType::ID, "identificador de parametro");
        params.push_back({t, idTok.lexema});
        parseListaParametros(params);
    }
}

/* LISTA_PARAMETROS -> , TIPO id LISTA_PARAMETROS | ε */
void Parser::parseListaParametros(vector<FunctionDeclNode::Parametro>& params) {
    while (coincidir(LexTokenType::COMMA)) {
        string t = parseTipo();
        Token idTok = consumir(LexTokenType::ID, "identificador de parametro");
        params.push_back({t, idTok.lexema});
    }
}

/* PRINCIPAL -> [TIPO_RETORNO] main ( ) BLOQUE */
shared_ptr<FunctionDeclNode> Parser::parsePrincipal() {
    int lin = actual().linea;
    string tipoRet = "void";

    if (verificar(LexTokenType::KW_VOID)) {
        consumir(LexTokenType::KW_VOID, "'void'");
    } else if (esTipoBase(actual().tipo)) {
        tipoRet = parseTipo();
    }

    if (verificar(LexTokenType::KW_MAIN)) {
        consumir(LexTokenType::KW_MAIN, "'main'");
    } else if (actual().lexema == "main") {
        pos++;
    } else {
        reportarError("declaracion de la funcion principal 'main'", "main");
    }

    consumir(LexTokenType::LPAREN, "'(' despues de 'main'");
    consumir(LexTokenType::RPAREN, "')' despues de 'main('");

    auto fnPrincipal = make_shared<FunctionDeclNode>(tipoRet, "main", lin);
    fnPrincipal->bloque = parseBloque();
    return fnPrincipal;
}

/* BLOQUE -> { LISTA_VARIABLES COMANDOS } */
shared_ptr<BlockNode> Parser::parseBloque() {
    int lin = actual().linea;
    auto bloque = make_shared<BlockNode>(lin);

    consumir(LexTokenType::LBRACE, "'{' para abrir el bloque");

    /* 1. Declaraciones de variables al inicio del bloque */
    parseListaVariables(bloque->variables);

    /* 2. Comandos ejecutables */
    parseComandos(bloque->comandos);

    consumir(LexTokenType::RBRACE, "'}' para cerrar el bloque");
    return bloque;
}

/* LISTA_VARIABLES -> TIPO id LISTA_ID ; LISTA_VARIABLES | ε */
void Parser::parseListaVariables(vector<shared_ptr<VarDeclNode>>& vars) {
    while (!esFin() && esTipoBase(actual().tipo)) {
        int lin = actual().linea;
        string t = parseTipo();

        Token idTok = consumir(LexTokenType::ID, "identificador en la declaracion de variable");
        string dimId = parseDimension();
        vector<string> nombres;
        nombres.push_back(idTok.lexema + dimId);

        parseListaId(nombres);

        if (verificar(LexTokenType::SEMICOLON)) {
            consumir(LexTokenType::SEMICOLON, "';'");
        } else {
            reportarError("Se esperaba ';' al finalizar la declaracion de la variable, pero se encontro '" +
                          actual().lexema + "'", ";");
            sincronizar();
        }

        vars.push_back(make_shared<VarDeclNode>(t, nombres, lin));
    }
}

/* LISTA_ID -> , id DIMENSION LISTA_ID | ε */
void Parser::parseListaId(vector<string>& ids) {
    while (coincidir(LexTokenType::COMMA)) {
        Token idTok = consumir(LexTokenType::ID, "identificador tras coma en declaracion");
        string dimId = parseDimension();
        ids.push_back(idTok.lexema + dimId);
    }
}

/* COMANDOS -> COMANDO COMANDOS | ε */
void Parser::parseComandos(vector<shared_ptr<ASTNode>>& cmds) {
    while (!esFin() && !verificar(LexTokenType::RBRACE)) {
        /* Advertencia si declaran variables despues de comandos */
        if (esTipoBase(actual().tipo)) {
            reportarError("Las declaraciones de variables deben ubicarse al inicio del bloque antes de los comandos", "COMANDO");
            /* Parsear para recuperar */
            vector<shared_ptr<VarDeclNode>> vAux;
            parseListaVariables(vAux);
            continue;
        }

        auto cmd = parseComando();
        if (cmd) {
            cmds.push_back(cmd);
        } else {
            sincronizar();
        }
    }
}

/* COMANDO -> LECTURA | ESCRITURA | ASIGNACION | FUNCION | SELECCION | MIENTRAS | RETORNO */
shared_ptr<ASTNode> Parser::parseComando() {
    LexTokenType t = actual().tipo;

    if (t == LexTokenType::KW_SCANF) {
        return parseLectura();
    }
    if (t == LexTokenType::KW_PRINTLN) {
        return parseEscritura();
    }
    if (t == LexTokenType::KW_IF) {
        return parseSeleccion();
    }
    if (t == LexTokenType::KW_WHILE) {
        return parseMientras();
    }
    if (t == LexTokenType::KW_RETURN) {
        return parseRetorno();
    }
    if (t == LexTokenType::KW_FUNC) {
        auto fn = parseFuncion();
        coincidir(LexTokenType::SEMICOLON); /* Tolerar ';' opcional en llamada como comando */
        return fn;
    }
    if (t == LexTokenType::ID) {
        return parseAsignacion();
    }

    reportarError("comando valido (asignacion, if, while, scanf, println, return o func)", "COMANDO");
    return nullptr;
}

/* LECTURA -> scanf ( TERMINO_LECTURA NUEVO_TERMINO_LECTURA ) ; */
shared_ptr<ScanfNode> Parser::parseLectura() {
    int lin = actual().linea;
    auto node = make_shared<ScanfNode>(lin);

    consumir(LexTokenType::KW_SCANF, "'scanf'");
    consumir(LexTokenType::LPAREN, "'(' tras 'scanf'");

    ScanfNode::TerminoLectura term;
    parseTerminoLectura(term);
    node->terminos.push_back(term);

    parseNuevoTerminoLectura(node->terminos);

    consumir(LexTokenType::RPAREN, "')' al cerrar 'scanf'");
    consumir(LexTokenType::SEMICOLON, "';' al finalizar la sentencia 'scanf'");
    return node;
}

/* TERMINO_LECTURA -> id DIMENSION2 */
void Parser::parseTerminoLectura(ScanfNode::TerminoLectura& term) {
    Token idTok = consumir(LexTokenType::ID, "identificador en termino de lectura");
    term.id = idTok.lexema;
    term.dimensiones = parseDimension2();
}

/* NUEVO_TERMINO_LECTURA -> , TERMINO_LECTURA NUEVO_TERMINO_LECTURA | ε */
void Parser::parseNuevoTerminoLectura(vector<ScanfNode::TerminoLectura>& terminos) {
    while (coincidir(LexTokenType::COMMA)) {
        ScanfNode::TerminoLectura term;
        parseTerminoLectura(term);
        terminos.push_back(term);
    }
}

/* DIMENSION2 -> [ EXPRESION_ADITIVA ] DIMENSION2 | ε */
vector<shared_ptr<ExprNode>> Parser::parseDimension2() {
    vector<shared_ptr<ExprNode>> indices;
    while (coincidir(LexTokenType::LBRACKET)) {
        auto expr = parseExpresionAditiva();
        indices.push_back(expr);
        consumir(LexTokenType::RBRACKET, "']' al cerrar dimension de acceso");
    }
    return indices;
}

/* ESCRITURA -> println ( TERMINO_ESCRITURA NUEVO_TERMINO_ESCRITURA ) ; */
shared_ptr<PrintlnNode> Parser::parseEscritura() {
    int lin = actual().linea;
    auto node = make_shared<PrintlnNode>(lin);

    consumir(LexTokenType::KW_PRINTLN, "'println'");
    consumir(LexTokenType::LPAREN, "'(' tras 'println'");

    auto t1 = parseTerminoEscritura();
    if (t1) node->terminos.push_back(t1);

    parseNuevoTerminoEscritura(node->terminos);

    consumir(LexTokenType::RPAREN, "')' al cerrar 'println'");
    consumir(LexTokenType::SEMICOLON, "';' al finalizar la sentencia 'println'");
    return node;
}

/* TERMINO_ESCRITURA -> id DIMENSION2 | CONSTANTE | texto */
shared_ptr<ExprNode> Parser::parseTerminoEscritura() {
    int lin = actual().linea;

    if (verificar(LexTokenType::TEXTO)) {
        Token tok = consumir(LexTokenType::TEXTO, "cadena de texto");
        return make_shared<LiteralNode>(tok.lexema, "texto", lin);
    }
    if (verificar(LexTokenType::NUM_INT) || verificar(LexTokenType::NUM_DEC)) {
        return parseConstante();
    }
    if (verificar(LexTokenType::ID)) {
        Token idTok = consumir(LexTokenType::ID, "identificador");
        auto idNode = make_shared<IdentifierNode>(idTok.lexema, lin);
        idNode->indices = parseDimension2();
        return idNode;
    }

    /* O expresion general */
    return parseExpresion();
}

/* NUEVO_TERMINO_ESCRITURA -> , TERMINO_ESCRITURA NUEVO_TERMINO_ESCRITURA | ε */
void Parser::parseNuevoTerminoEscritura(vector<shared_ptr<ExprNode>>& terminos) {
    while (coincidir(LexTokenType::COMMA)) {
        auto t = parseTerminoEscritura();
        if (t) terminos.push_back(t);
    }
}

/* SELECCION -> if ( EXPRESION ) BLOQUE SINO */
shared_ptr<IfNode> Parser::parseSeleccion() {
    int lin = actual().linea;
    consumir(LexTokenType::KW_IF, "'if'");
    consumir(LexTokenType::LPAREN, "'(' tras 'if'");

    auto cond = parseExpresion();

    consumir(LexTokenType::RPAREN, "')' tras la condicion del 'if'");
    auto bloqueSi = parseBloque();
    auto bloqueSino = parseSino();

    return make_shared<IfNode>(cond, bloqueSi, bloqueSino, lin);
}

/* SINO -> else BLOQUE | ε */
shared_ptr<ASTNode> Parser::parseSino() {
    if (coincidir(LexTokenType::KW_ELSE)) {
        return parseBloque();
    }
    return nullptr;
}

/* MIENTRAS -> while ( EXPRESION ) BLOQUE */
shared_ptr<WhileNode> Parser::parseMientras() {
    int lin = actual().linea;
    consumir(LexTokenType::KW_WHILE, "'while'");
    consumir(LexTokenType::LPAREN, "'(' tras 'while'");

    auto cond = parseExpresion();

    consumir(LexTokenType::RPAREN, "')' tras la condicion del 'while'");
    auto bloque = parseBloque();

    return make_shared<WhileNode>(cond, bloque, lin);
}

/* ASIGNACION -> id [DIMENSION2] = COMPLEMENTO ; */
shared_ptr<AssignNode> Parser::parseAsignacion() {
    int lin = actual().linea;
    Token idTok = consumir(LexTokenType::ID, "identificador de variable a asignar");

    auto node = make_shared<AssignNode>(idTok.lexema, nullptr, lin);
    if (verificar(LexTokenType::LBRACKET)) {
        node->indices = parseDimension2();
    }

    consumir(LexTokenType::OP_ASSIGN, "'=' en sentencia de asignacion");
    node->expresion = parseComplemento();

    if (verificar(LexTokenType::SEMICOLON)) {
        consumir(LexTokenType::SEMICOLON, "';'");
    } else {
        reportarError("Se esperaba ';' al final de la asignacion de variable, pero se encontro '" +
                      actual().lexema + "'", ";");
        sincronizar();
    }

    return node;
}

/* COMPLEMENTO -> EXPRESION | FUNCION */
shared_ptr<ExprNode> Parser::parseComplemento() {
    if (verificar(LexTokenType::KW_FUNC)) {
        return parseFuncion();
    }
    return parseExpresion();
}

/* FUNCION -> func id ( ARGUMENTOS ) */
shared_ptr<FuncCallNode> Parser::parseFuncion() {
    int lin = actual().linea;
    consumir(LexTokenType::KW_FUNC, "'func'");
    Token idTok = consumir(LexTokenType::ID, "nombre de la funcion a invocar");

    auto callNode = make_shared<FuncCallNode>(idTok.lexema, lin);
    consumir(LexTokenType::LPAREN, "'(' tras el nombre de la funcion");
    parseArgumentos(callNode->argumentos);
    consumir(LexTokenType::RPAREN, "')' al cerrar los argumentos de la funcion");

    return callNode;
}

/* ARGUMENTOS -> EXPRESION NUEVO_ARGUMENTO | ε */
void Parser::parseArgumentos(vector<shared_ptr<ExprNode>>& args) {
    if (!verificar(LexTokenType::RPAREN) && !esFin()) {
        auto expr = parseExpresion();
        if (expr) args.push_back(expr);
        parseNuevoArgumento(args);
    }
}

/* NUEVO_ARGUMENTO -> , EXPRESION NUEVO_ARGUMENTO | ε */
void Parser::parseNuevoArgumento(vector<shared_ptr<ExprNode>>& args) {
    while (coincidir(LexTokenType::COMMA)) {
        auto expr = parseExpresion();
        if (expr) args.push_back(expr);
    }
}

/* RETORNO -> return EXPRESION ; */
shared_ptr<ReturnNode> Parser::parseRetorno() {
    int lin = actual().linea;
    consumir(LexTokenType::KW_RETURN, "'return'");

    if (verificar(LexTokenType::SEMICOLON)) {
        reportarError("La sentencia 'return' exige una expresion de retorno (ej: 'return edad;'). La gramatica de LP no contempla 'return;' vacio.",
                      "EXPRESION");
        consumir(LexTokenType::SEMICOLON, "';'");
        return make_shared<ReturnNode>(nullptr, lin);
    }

    auto expr = parseExpresion();

    if (verificar(LexTokenType::SEMICOLON)) {
        consumir(LexTokenType::SEMICOLON, "';'");
    } else {
        reportarError("Se esperaba ';' al final de la sentencia return", ";");
        sincronizar();
    }

    return make_shared<ReturnNode>(expr, lin);
}

/* ==========================================================
   Jerarquia de Expresiones
   ========================================================== */

/* EXPRESION -> EXPRESION_O */
shared_ptr<ExprNode> Parser::parseExpresion() {
    return parseExpresionO();
}

/* EXPRESION_O -> EXPRESION_Y EXPRESION_O2 */
shared_ptr<ExprNode> Parser::parseExpresionO() {
    auto izq = parseExpresionY();
    return parseExpresionO2(izq);
}

/* EXPRESION_O2 -> || EXPRESION_Y EXPRESION_O2 | ε */
shared_ptr<ExprNode> Parser::parseExpresionO2(shared_ptr<ExprNode> izq) {
    while (coincidir(LexTokenType::OP_OR)) {
        int lin = anterior().linea;
        auto der = parseExpresionY();
        izq = make_shared<BinaryOpNode>("||", izq, der, lin);
    }
    return izq;
}

/* EXPRESION_Y -> EXPRESION_RELACIONAL EXPRESION_Y2 */
shared_ptr<ExprNode> Parser::parseExpresionY() {
    auto izq = parseExpresionRelacional();
    return parseExpresionY2(izq);
}

/* EXPRESION_Y2 -> && EXPRESION_RELACIONAL EXPRESION_Y2 | ε */
shared_ptr<ExprNode> Parser::parseExpresionY2(shared_ptr<ExprNode> izq) {
    while (coincidir(LexTokenType::OP_AND)) {
        int lin = anterior().linea;
        auto der = parseExpresionRelacional();
        izq = make_shared<BinaryOpNode>("&&", izq, der, lin);
    }
    return izq;
}

/* EXPRESION_RELACIONAL -> EXPRESION_ADITIVA EXPRESION_RELACIONAL2 */
shared_ptr<ExprNode> Parser::parseExpresionRelacional() {
    auto izq = parseExpresionAditiva();
    return parseExpresionRelacional2(izq);
}

/* EXPRESION_RELACIONAL2 -> comp EXPRESION_ADITIVA | ε */
shared_ptr<ExprNode> Parser::parseExpresionRelacional2(shared_ptr<ExprNode> izq) {
    if (verificar(LexTokenType::OP_COMP)) {
        Token compTok = actual();
        pos++;
        int lin = compTok.linea;
        auto der = parseExpresionAditiva();
        return make_shared<BinaryOpNode>(compTok.lexema, izq, der, lin);
    }
    return izq;
}

/* EXPRESION_ADITIVA -> EXPRESION_MULTIPLICATIVA EXPRESION_ADITIVA2 */
shared_ptr<ExprNode> Parser::parseExpresionAditiva() {
    auto izq = parseExpresionMultiplicativa();
    return parseExpresionAditiva2(izq);
}

/* EXPRESION_ADITIVA2 -> OPERADOR_ADITIVO EXPRESION_MULTIPLICATIVA EXPRESION_ADITIVA2 | ε */
shared_ptr<ExprNode> Parser::parseExpresionAditiva2(shared_ptr<ExprNode> izq) {
    while (verificar(LexTokenType::OP_SUM) || verificar(LexTokenType::OP_SUB)) {
        string op = actual().lexema;
        int lin = actual().linea;
        pos++;
        auto der = parseExpresionMultiplicativa();
        izq = make_shared<BinaryOpNode>(op, izq, der, lin);
    }
    return izq;
}

/* EXPRESION_MULTIPLICATIVA -> FACTOR EXPRESION_MULTIPLICATIVA2 */
shared_ptr<ExprNode> Parser::parseExpresionMultiplicativa() {
    auto izq = parseFactor();
    return parseExpresionMultiplicativa2(izq);
}

/* EXPRESION_MULTIPLICATIVA2 -> OPERADOR_MULTIPLICATIVO FACTOR EXPRESION_MULTIPLICATIVA2 | ε */
shared_ptr<ExprNode> Parser::parseExpresionMultiplicativa2(shared_ptr<ExprNode> izq) {
    while (verificar(LexTokenType::OP_MUL) || verificar(LexTokenType::OP_DIV) || verificar(LexTokenType::OP_MOD)) {
        string op = actual().lexema;
        int lin = actual().linea;
        pos++;
        auto der = parseFactor();
        izq = make_shared<BinaryOpNode>(op, izq, der, lin);
    }
    return izq;
}

/* FACTOR -> SIGNO TERMINO | texto | ! FACTOR | ( EXPRESION ) */
shared_ptr<ExprNode> Parser::parseFactor() {
    int lin = actual().linea;

    /* Negacion logica: ! FACTOR */
    if (coincidir(LexTokenType::OP_NOT)) {
        auto f = parseFactor();
        return make_shared<UnaryOpNode>("!", f, lin);
    }

    /* Cadena de texto */
    if (verificar(LexTokenType::TEXTO)) {
        Token tok = consumir(LexTokenType::TEXTO, "cadena de texto");
        return make_shared<LiteralNode>(tok.lexema, "texto", lin);
    }

    /* Expresion entre parentesis: ( EXPRESION ) */
    if (coincidir(LexTokenType::LPAREN)) {
        auto expr = parseExpresion();
        consumir(LexTokenType::RPAREN, "')' para cerrar expresion entre parentesis");
        return expr;
    }

    /* Llamada a funcion con func dentro de una expresion */
    if (verificar(LexTokenType::KW_FUNC)) {
        return parseFuncion();
    }

    /* SIGNO TERMINO */
    string signo = parseSigno();
    auto term = parseTermino();

    if (!signo.empty() && term) {
        return make_shared<UnaryOpNode>(signo, term, lin);
    }
    return term;
}

/* TERMINO -> id DIMENSION2 | CONSTANTE */
shared_ptr<ExprNode> Parser::parseTermino() {
    int lin = actual().linea;

    if (verificar(LexTokenType::ID)) {
        Token idTok = consumir(LexTokenType::ID, "identificador");
        auto idNode = make_shared<IdentifierNode>(idTok.lexema, lin);
        idNode->indices = parseDimension2();
        return idNode;
    }

    if (verificar(LexTokenType::NUM_INT) || verificar(LexTokenType::NUM_DEC)) {
        return parseConstante();
    }

    reportarError("termino valido (identificador, numero o expresion)", "TERMINO");
    return nullptr;
}

/* SIGNO -> + | - | ε */
string Parser::parseSigno() {
    if (coincidir(LexTokenType::OP_SUM)) return "+";
    if (coincidir(LexTokenType::OP_SUB)) return "-";
    return "";
}

/* CONSTANTE -> num_int | num_dec */
shared_ptr<LiteralNode> Parser::parseConstante() {
    int lin = actual().linea;
    if (verificar(LexTokenType::NUM_INT)) {
        Token t = consumir(LexTokenType::NUM_INT, "numero entero");
        return make_shared<LiteralNode>(t.lexema, "int", lin);
    }
    if (verificar(LexTokenType::NUM_DEC)) {
        Token t = consumir(LexTokenType::NUM_DEC, "numero decimal");
        return make_shared<LiteralNode>(t.lexema, "float", lin);
    }
    reportarError("constante numerica entera o decimal", "CONSTANTE");
    return nullptr;
}
