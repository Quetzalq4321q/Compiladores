#pragma once

#include <vector>
#include <string>
#include <memory>
#include "Token.h"
#include "AST.h"

using namespace std;

/* Estructura para registrar errores sintacticos con su ubicacion y detalle */
struct SyntaxError {
    int linea;
    string tokenEncontrado;
    string construccionEsperada;
    string mensaje;
};

/* Analizador Sintactico por Descenso Recursivo para el Lenguaje LP */
class Parser {
public:
    /* Inicializa con la lista de tokens producida por el lexer */
    explicit Parser(const vector<Token>& tokens);

    /* Ejecuta el analisis sintactico completo y construye el AST */
    shared_ptr<ProgramNode> parsear();

    /* Indica si el programa fue aceptado sintacticamente (0 errores) */
    bool esAceptado() const;

    /* Devuelve la lista de errores sintacticos detectados */
    const vector<SyntaxError>& getErrores() const;

    /* Genera el reporte textual de errores sintacticos */
    string getReporteErrores() const;

private:
    const vector<Token>& tokens;
    size_t pos;
    vector<SyntaxError> errores;
    shared_ptr<ProgramNode> astRaiz;

    /* ==========================================================
       Metodos correspondientes a los No Terminales de la Gramatica
       ========================================================== */

    /* PROGRAMA -> LISTA_FUNCIONES PRINCIPAL */
    shared_ptr<ProgramNode> parsePrograma();

    /* LISTA_FUNCIONES -> DECLARACION_FUNCION LISTA_FUNCIONES | ε */
    void parseListaFunciones(vector<shared_ptr<FunctionDeclNode>>& lista);

    /* DECLARACION_FUNCION -> TIPO_RETORNO id ( PARAMETROS ) BLOQUE */
    shared_ptr<FunctionDeclNode> parseDeclaracionFuncion();

    /* TIPO_RETORNO -> TIPO | void */
    string parseTipoRetorno();

    /* TIPO -> TIPO_BASE DIMENSION */
    string parseTipo();

    /* TIPO_BASE -> char | float | int | boolean */
    string parseTipoBase();

    /* DIMENSION -> [ num_int ] DIMENSION | ε */
    string parseDimension();

    /* PARAMETROS -> TIPO id LISTA_PARAMETROS | ε */
    void parseParametros(vector<FunctionDeclNode::Parametro>& params);

    /* LISTA_PARAMETROS -> , TIPO id LISTA_PARAMETROS | ε */
    void parseListaParametros(vector<FunctionDeclNode::Parametro>& params);

    /* PRINCIPAL -> [TIPO_RETORNO] main ( ) BLOQUE */
    shared_ptr<FunctionDeclNode> parsePrincipal();

    /* BLOQUE -> { LISTA_VARIABLES COMANDOS } */
    shared_ptr<BlockNode> parseBloque();

    /* LISTA_VARIABLES -> TIPO id LISTA_ID ; LISTA_VARIABLES | ε */
    void parseListaVariables(vector<shared_ptr<VarDeclNode>>& vars);

    /* LISTA_ID -> , id LISTA_ID | ε */
    void parseListaId(vector<string>& ids);

    /* COMANDOS -> COMANDO COMANDOS | ε */
    void parseComandos(vector<shared_ptr<ASTNode>>& cmds);

    /* COMANDO -> LECTURA | ESCRITURA | ASIGNACION | FUNCION | SELECCION | MIENTRAS | RETORNO */
    shared_ptr<ASTNode> parseComando();

    /* LECTURA -> scanf ( TERMINO_LECTURA NUEVO_TERMINO_LECTURA ) ; */
    shared_ptr<ScanfNode> parseLectura();

    /* TERMINO_LECTURA -> id DIMENSION2 */
    void parseTerminoLectura(ScanfNode::TerminoLectura& term);

    /* NUEVO_TERMINO_LECTURA -> , TERMINO_LECTURA NUEVO_TERMINO_LECTURA | ε */
    void parseNuevoTerminoLectura(vector<ScanfNode::TerminoLectura>& terminos);

    /* DIMENSION2 -> [ EXPRESION_ADITIVA ] DIMENSION2 | ε */
    vector<shared_ptr<ExprNode>> parseDimension2();

    /* ESCRITURA -> println ( TERMINO_ESCRITURA NUEVO_TERMINO_ESCRITURA ) ; */
    shared_ptr<PrintlnNode> parseEscritura();

    /* TERMINO_ESCRITURA -> id DIMENSION2 | CONSTANTE | texto */
    shared_ptr<ExprNode> parseTerminoEscritura();

    /* NUEVO_TERMINO_ESCRITURA -> , TERMINO_ESCRITURA NUEVO_TERMINO_ESCRITURA | ε */
    void parseNuevoTerminoEscritura(vector<shared_ptr<ExprNode>>& terminos);

    /* SELECCION -> if ( EXPRESION ) BLOQUE SINO */
    shared_ptr<IfNode> parseSeleccion();

    /* SINO -> else BLOQUE | ε */
    shared_ptr<ASTNode> parseSino();

    /* MIENTRAS -> while ( EXPRESION ) BLOQUE */
    shared_ptr<WhileNode> parseMientras();

    /* ASIGNACION -> id [DIMENSION2] = COMPLEMENTO ; */
    shared_ptr<AssignNode> parseAsignacion();

    /* COMPLEMENTO -> EXPRESION | FUNCION */
    shared_ptr<ExprNode> parseComplemento();

    /* FUNCION -> func id ( ARGUMENTOS ) */
    shared_ptr<FuncCallNode> parseFuncion();

    /* ARGUMENTOS -> EXPRESION NUEVO_ARGUMENTO | ε */
    void parseArgumentos(vector<shared_ptr<ExprNode>>& args);

    /* NUEVO_ARGUMENTO -> , EXPRESION NUEVO_ARGUMENTO | ε */
    void parseNuevoArgumento(vector<shared_ptr<ExprNode>>& args);

    /* RETORNO -> return EXPRESION ; */
    shared_ptr<ReturnNode> parseRetorno();

    /* Jerarquia de Expresiones segun la especificacion */
    shared_ptr<ExprNode> parseExpresion();
    shared_ptr<ExprNode> parseExpresionO();
    shared_ptr<ExprNode> parseExpresionO2(shared_ptr<ExprNode> izq);
    shared_ptr<ExprNode> parseExpresionY();
    shared_ptr<ExprNode> parseExpresionY2(shared_ptr<ExprNode> izq);
    shared_ptr<ExprNode> parseExpresionRelacional();
    shared_ptr<ExprNode> parseExpresionRelacional2(shared_ptr<ExprNode> izq);
    shared_ptr<ExprNode> parseExpresionAditiva();
    shared_ptr<ExprNode> parseExpresionAditiva2(shared_ptr<ExprNode> izq);
    shared_ptr<ExprNode> parseExpresionMultiplicativa();
    shared_ptr<ExprNode> parseExpresionMultiplicativa2(shared_ptr<ExprNode> izq);
    shared_ptr<ExprNode> parseFactor();
    shared_ptr<ExprNode> parseTermino();
    string parseSigno();
    shared_ptr<LiteralNode> parseConstante();

    /* ==========================================================
       Funciones auxiliares del Parser
       ========================================================== */
    Token actual() const;
    Token anterior() const;
    Token siguiente() const;
    bool esFin() const;
    bool verificar(LexTokenType tipo) const;
    bool coincidir(LexTokenType tipo);
    Token consumir(LexTokenType tipo, const string& mensajeEsperado);
    void reportarError(const string& mensaje, const string& esperado = "");
    void sincronizar();
    bool esTipoBase(LexTokenType t) const;
    bool esInicioComando(LexTokenType t) const;
};
