#include "Lexer.h"
#include <cctype>
#include <sstream>
#include <map>

using namespace std;

static const map<string, LexTokenType> PALABRAS_RESERVADAS = {
    {"int",    LexTokenType::KW_INT},
    {"float",  LexTokenType::KW_FLOAT},
    {"string", LexTokenType::KW_STRING},
    {"bool",   LexTokenType::KW_BOOL},
    {"char",   LexTokenType::KW_CHAR},
    {"if",     LexTokenType::KW_IF},
    {"else",   LexTokenType::KW_ELSE},
    {"while",  LexTokenType::KW_WHILE},
    {"for",    LexTokenType::KW_FOR},
    {"return", LexTokenType::KW_RETURN},
    {"void",   LexTokenType::KW_VOID},
    {"true",   LexTokenType::KW_TRUE},
    {"false",  LexTokenType::KW_FALSE},
    {"print",  LexTokenType::KW_PRINT}
};

/* Constructor: inicia en la primera linea y caracter */
Lexer::Lexer(const string& fuente)
    : fuente(fuente), pos(0), linea(1), columna(1), ultimoTipoLeido(""), ultimoToken() {}

/* Caracter actual en la posicion de lectura */
char Lexer::actual() const {
    if (pos >= fuente.size()) return '\0';
    return fuente[pos];
}

/* Look-ahead de 1 caracter */
char Lexer::siguiente() const {
    if (pos + 1 >= fuente.size()) return '\0';
    return fuente[pos + 1];
}

/* Busca el siguiente caracter no vacio sin alterar la posicion */
char Lexer::mirarSiguienteNoEspacio() const {
    size_t i = pos + 1;
    while (i < fuente.size()) {
        char c = fuente[i];
        if (isspace(static_cast<unsigned char>(c))) {
            i++;
            continue;
        }
        return c;
    }
    return '\0';
}

/* Registra un token reconocido y actualiza el ultimoToken procesado */
void Lexer::agregarToken(const Token& t) {
    tokens.push_back(t);
    ultimoToken = t;
}

/* Avanza una posicion y controla el conteo de lineas y columnas */
void Lexer::avanzar() {
    if (pos < fuente.size()) {
        if (fuente[pos] == '\n') {
            linea++;
            columna = 1;
        } else {
            columna++;
        }
        pos++;
    }
}

/* Consume espacios, tabulaciones y comentarios */
void Lexer::saltarEspaciosYComentarios() {
    while (pos < fuente.size()) {
        char c = actual();
        if (isspace(static_cast<unsigned char>(c))) {
            avanzar();
            continue;
        }

        // Comentario con '#'
        if (c == '#') {
            while (pos < fuente.size() && actual() != '\n') {
                avanzar();
            }
            continue;
        }

        // Comentario multilineal '/* ... */'
        if (c == '/' && siguiente() == '*') {
            avanzar(); // '/'
            avanzar(); // '*'
            while (pos < fuente.size()) {
                if (actual() == '*' && siguiente() == '/') {
                    avanzar(); // '*'
                    avanzar(); // '/'
                    break;
                }
                avanzar();
            }
            continue;
        }

        break;
    }
}

/* Procesa cadenas de texto entre comillas */
Token Lexer::lexTexto(int linIni, int colIni, char delim) {
    avanzar(); // Consume comilla inicial
    size_t inicio = pos;
    bool cerrado = false;

    while (pos < fuente.size()) {
        char c = actual();
        if (c == '\\') {
            avanzar();
            if (pos < fuente.size()) avanzar();
            continue;
        }
        if (c == delim) {
            cerrado = true;
            break;
        }
        if (c == '\n') break; // No multilinea
        avanzar();
    }

    if (cerrado) {
        string contenido = fuente.substr(inicio, pos - inicio);
        avanzar(); // Consume comilla final
        string lexema = string(1, delim) + contenido + string(1, delim);
        return Token(LexTokenType::TEXTO, lexema, linIni, colIni);
    }

    string incompleto = string(1, delim) + fuente.substr(inicio, pos - inicio);
    errores.push_back(Token(LexTokenType::LEX_ERROR, incompleto, linIni, colIni));
    return Token(LexTokenType::LEX_ERROR, incompleto, linIni, colIni);
}

/* Reconoce NUM_INT o NUM_DEC con validacion de multiples puntos y sufijos alfanumericos */
Token Lexer::lexNumero(int linIni, int colIni) {
    size_t inicio = pos;
    int puntos = 0;

    while (pos < fuente.size()) {
        char c = actual();
        if (isdigit(static_cast<unsigned char>(c))) {
            avanzar();
        } else if (c == '.') {
            // Verificar si hay doble punto consecutivo '..'
            if (siguiente() == '.') {
                avanzar(); // primer .
                avanzar(); // segundo .
                while (pos < fuente.size() && (isalnum(static_cast<unsigned char>(actual())) || actual() == '.')) {
                    avanzar();
                }
                string lexErr = fuente.substr(inicio, pos - inicio);
                errores.push_back(Token(LexTokenType::LEX_ERROR, lexErr, linIni, colIni, -1, "Duplicidad de puntos consecutivos en numero ('..')"));
                return Token(LexTokenType::LEX_ERROR, lexErr, linIni, colIni, -1, "Duplicidad de puntos consecutivos en numero ('..')");
            }

            puntos+ .0+;
            if (puntos > 1) {
                // Multiples puntos decimales (ej. 12.3.4)
                avanzar();
                while (pos < fuente.size() && (isalnum(static_cast<unsigned char>(actual())) || actual() == '.')) {
                    avanzar();
                }
                string lexErr = fuente.substr(inicio, pos - inicio);
                errores.push_back(Token(LexTokenType::LEX_ERROR, lexErr, linIni, colIni, -1, "Numero con multiples puntos decimales ('" + lexErr + "')"));
                return Token(LexTokenType::LEX_ERROR, lexErr, linIni, colIni, -1, "Numero con multiples puntos decimales ('" + lexErr + "')");
            }

            // Punto decimal: debe venir un digito
            if (!isdigit(static_cast<unsigned char>(siguiente()))) {
                avanzar();
                string lexErr = fuente.substr(inicio, pos - inicio);
                errores.push_back(Token(LexTokenType::LEX_ERROR, lexErr, linIni, colIni, -1, "Numero decimal mal formado (falta digito despues del punto: '" + lexErr + "')"));
                return Token(LexTokenType::LEX_ERROR, lexErr, linIni, colIni, -1, "Numero decimal mal formado (falta digito despues del punto: '" + lexErr + "')");
            }
            avanzar();
        } else {
            break;
        }
    }

    // Identificador invalido que empieza con digitos (ej. 20edad, 20var)
    if (pos < fuente.size() && (isalpha(static_cast<unsigned char>(actual())) || actual() == '_')) {
        while (pos < fuente.size() && (isalnum(static_cast<unsigned char>(actual())) || actual() == '_')) {
            avanzar();
        }
        string lexErr = fuente.substr(inicio, pos - inicio);
        errores.push_back(Token(LexTokenType::LEX_ERROR, lexErr, linIni, colIni, -1, "Identificador invalido: no puede comenzar con digitos ('" + lexErr + "')"));
        return Token(LexTokenType::LEX_ERROR, lexErr, linIni, colIni, -1, "Identificador invalido: no puede comenzar con digitos ('" + lexErr + "')");
    }

    string lexema = fuente.substr(inicio, pos - inicio);
    if (puntos == 1) {
        return Token(LexTokenType::NUM_DEC, lexema, linIni, colIni);
    }
    return Token(LexTokenType::NUM_INT, lexema, linIni, colIni);
}

/* Reconoce identificadores y palabras reservadas */
Token Lexer::lexIdentificadorOPalabra(int linIni, int colIni) {
    size_t inicio = pos;

    while (pos < fuente.size()) {
        char c = actual();
        if (isalnum(static_cast<unsigned char>(c)) || c == '_') {
            avanzar();
        } else {
            break;
        }
    }

    string lexema = fuente.substr(inicio, pos - inicio);

    // Comprobar palabra reservada
    auto it = PALABRAS_RESERVADAS.find(lexema);
    if (it != PALABRAS_RESERVADAS.end()) {
        if (lexema == "int" || lexema == "float" || lexema == "string" || lexema == "bool" || lexema == "char") {
            ultimoTipoLeido = lexema;
        }
        return Token(it->second, lexema, linIni, colIni);
    }

    // Es identificador (ID): registrar en la tabla de simbolos
    int idx = tabla.agregar(lexema, ultimoTipoLeido, linIni);
    ultimoTipoLeido = ""; // Reiniciar tipo

    return Token(LexTokenType::ID, lexema, linIni, colIni, idx);
}

/* Proceso principal de analisis lexico */
void Lexer::analizar() {
    tokens.clear();
    errores.clear();
    tabla.limpiar();
    pos = 0;
    linea = 1;
    columna = 1;
    ultimoTipoLeido = "";
    ultimoToken = Token();

    while (pos < fuente.size()) {
        saltarEspaciosYComentarios();
        if (pos >= fuente.size()) break;

        int linIni = linea;
        int colIni = columna;
        char c = actual();
        char sig = siguiente();

        // 1. Cadenas de texto
        if (c == '"' || c == '\'') {
            Token t = lexTexto(linIni, colIni, c);
            if (t.tipo != LexTokenType::LEX_ERROR) agregarToken(t);
            continue;
        }

        // 2. Numeros
        if (isdigit(static_cast<unsigned char>(c))) {
            Token t = lexNumero(linIni, colIni);
            if (t.tipo != LexTokenType::LEX_ERROR) agregarToken(t);
            continue;
        }

        // 3. Puntos sueltos o dobles puntos '..' aislados
        if (c == '.') {
            if (sig == '.') {
                size_t inicioErr = pos;
                while (pos < fuente.size() && actual() == '.') {
                    avanzar();
                }
                string lexErr = fuente.substr(inicioErr, pos - inicioErr);
                errores.push_back(Token(LexTokenType::LEX_ERROR, lexErr, linIni, colIni, -1, "Duplicidad de puntos ('" + lexErr + "') carece de sentido"));
                continue;
            }
            avanzar();
            errores.push_back(Token(LexTokenType::LEX_ERROR, ".", linIni, colIni, -1, "Punto flotante aislado '.' carece de sentido"));
            continue;
        }

        // 4. Doble coma repetida (,, o , ,)
        if (c == ',') {
            char sigNoEsp = mirarSiguienteNoEspacio();
            if (sigNoEsp == ',') {
                avanzar();
                saltarEspaciosYComentarios();
                avanzar();
                errores.push_back(Token(LexTokenType::LEX_ERROR, ",,", linIni, colIni, -1, "Duplicidad de comas (',,') carece de sentido"));
                continue;
            }
            if (sigNoEsp == ';') {
                avanzar();
                errores.push_back(Token(LexTokenType::LEX_ERROR, ",;", linIni, colIni, -1, "Coma huerfana antes del punto y coma (',;')"));
                continue;
            }
            avanzar();
            agregarToken(Token(LexTokenType::COMMA, ",", linIni, colIni));
            continue;
        }

        // 5. Doble punto y coma repetido (;; o ; ;)
        if (c == ';') {
            char sigNoEsp = mirarSiguienteNoEspacio();
            if (sigNoEsp == ';') {
                avanzar();
                saltarEspaciosYComentarios();
                avanzar();
                errores.push_back(Token(LexTokenType::LEX_ERROR, ";;", linIni, colIni, -1, "Duplicidad de punto y coma (';;') carece de sentido"));
                agregarToken(Token(LexTokenType::SEMICOLON, ";", linIni, colIni));
                continue;
            }

            // Validar operador huerfano antes de punto y coma (ej. edad + ;)
            if (ultimoToken.tipo == LexTokenType::OP_ASSIGN  ||
                ultimoToken.tipo == LexTokenType::OP_SUM     ||
                ultimoToken.tipo == LexTokenType::OP_SUB     ||
                ultimoToken.tipo == LexTokenType::OP_MUL     ||
                ultimoToken.tipo == LexTokenType::OP_DIV     ||
                ultimoToken.tipo == LexTokenType::OP_DIV_INT ||
                ultimoToken.tipo == LexTokenType::OP_MOD     ||
                ultimoToken.tipo == LexTokenType::OP_EQ      ||
                ultimoToken.tipo == LexTokenType::OP_NE      ||
                ultimoToken.tipo == LexTokenType::OP_LT      ||
                ultimoToken.tipo == LexTokenType::OP_GT      ||
                ultimoToken.tipo == LexTokenType::OP_LE      ||
                ultimoToken.tipo == LexTokenType::OP_GE) {
                errores.push_back(Token(LexTokenType::LEX_ERROR, ultimoToken.lexema + " ;", ultimoToken.linea, ultimoToken.columna, -1,
                    "Operador '" + ultimoToken.lexema + "' sin operando derecho antes de ';'"));
            }

            avanzar();
            ultimoTipoLeido = "";
            agregarToken(Token(LexTokenType::SEMICOLON, ";", linIni, colIni));
            continue;
        }

        // 6. Identificadores o palabras reservadas
        if (isalpha(static_cast<unsigned char>(c)) || c == '_') {
            agregarToken(lexIdentificadorOPalabra(linIni, colIni));
            continue;
        }

        // 7. Barras y division entera: validar /// o mas
        if (c == '/' && sig == '/') {
            if (pos + 2 < fuente.size() && fuente[pos + 2] == '/') {
                size_t inicioErr = pos;
                while (pos < fuente.size() && actual() == '/') {
                    avanzar();
                }
                string lexErr = fuente.substr(inicioErr, pos - inicioErr);
                errores.push_back(Token(LexTokenType::LEX_ERROR, lexErr, linIni, colIni, -1, "Secuencia de barras '" + lexErr + "' no valida"));
                continue;
            }
            avanzar(); avanzar();
            agregarToken(Token(LexTokenType::OP_DIV_INT, "//", linIni, colIni));
            continue;
        }

        // 8. Operadores compuestos de 2 caracteres
        if (c == '=' && sig == '=') {
            avanzar(); avanzar();
            agregarToken(Token(LexTokenType::OP_EQ, "==", linIni, colIni));
            continue;
        }
        if (c == '!' && sig == '=') {
            avanzar(); avanzar();
            agregarToken(Token(LexTokenType::OP_NE, "!=", linIni, colIni));
            continue;
        }
        if (c == '<' && sig == '=') {
            avanzar(); avanzar();
            agregarToken(Token(LexTokenType::OP_LE, "<=", linIni, colIni));
            continue;
        }
        if (c == '>' && sig == '=') {
            avanzar(); avanzar();
            agregarToken(Token(LexTokenType::OP_GE, ">=", linIni, colIni));
            continue;
        }

        // 9. Operadores y delimitadores simples
        switch (c) {
            case '=': avanzar(); agregarToken(Token(LexTokenType::OP_ASSIGN, "=", linIni, colIni)); break;
            case '+': avanzar(); agregarToken(Token(LexTokenType::OP_SUM, "+", linIni, colIni)); break;
            case '-': avanzar(); agregarToken(Token(LexTokenType::OP_SUB, "-", linIni, colIni)); break;
            case '*': avanzar(); agregarToken(Token(LexTokenType::OP_MUL, "*", linIni, colIni)); break;
            case '/': avanzar(); agregarToken(Token(LexTokenType::OP_DIV, "/", linIni, colIni)); break;
            case '%': avanzar(); agregarToken(Token(LexTokenType::OP_MOD, "%", linIni, colIni)); break;
            case '<': avanzar(); agregarToken(Token(LexTokenType::OP_LT, "<", linIni, colIni)); break;
            case '>': avanzar(); agregarToken(Token(LexTokenType::OP_GT, ">", linIni, colIni)); break;
            case '(': avanzar(); agregarToken(Token(LexTokenType::LPAREN, "(", linIni, colIni)); break;
            case ')': avanzar(); agregarToken(Token(LexTokenType::RPAREN, ")", linIni, colIni)); break;
            case '{': avanzar(); agregarToken(Token(LexTokenType::LBRACE, "{", linIni, colIni)); break;
            case '}': avanzar(); agregarToken(Token(LexTokenType::RBRACE, "}", linIni, colIni)); break;
            default: {
                string lex(1, c);
                errores.push_back(Token(LexTokenType::LEX_ERROR, lex, linIni, colIni, -1, "Caracter no reconocido '" + lex + "'"));
                avanzar();
                break;
            }
        }
    }
}

/* Genera la salida agrupada por linea:
   <INT> <ID,0> <=> <NUM_INT> <;>
   <FLOAT> <ID,1> <=> <ID,0> <//> <NUM_INT> <;> */
string Lexer::getSalidaPorLineas() const {
    if (tokens.empty()) return "(Sin tokens)";

    map<int, vector<Token>> porLinea;
    for (const auto& t : tokens) {
        porLinea[t.linea].push_back(t);
    }

    ostringstream ss;
    bool primera = true;|
    for (const auto& p : porLinea) {
        if (!primera) ss << "\n";
        primera = false;
        bool primerToken = true;
        for (const auto& t : p.second) {
            if (!primerToken) ss << " ";
            primerToken = false;
            ss << t.toString();
        }
    }
    return ss.str();
}

/* Accesores */
const vector<Token>& Lexer::getTokens()  const { return tokens;  }
const SymbolTable&   Lexer::getTabla()   const { return tabla;   }
const vector<Token>& Lexer::getErrores() const { return errores; }
