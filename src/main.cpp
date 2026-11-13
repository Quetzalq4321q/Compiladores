#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iomanip>

#include "Lexer.h"
#include "Token.h"
#include "FileConverter.h"
#include "ErrorHandler.h"
#include "AST.h"
#include "Parser.h"
#include "Interpreter.h"

using namespace std;

/* Identificadores de controles */
#define IDC_TAB             100
#define IDC_BTN_ABRIR       101
#define IDC_BTN_ANALIZAR    102
#define IDC_LABEL_FILE      103
#define IDC_LABEL_STATUS    104
#define IDC_BTN_EJECUTAR    105
#define IDC_BTN_EJ_VALIDO   106
#define IDC_BTN_EJ_ERROR    107
#define IDC_BTN_LIMPIAR     108
#define IDC_EDIT_ENTRADA    109
#define IDC_EDIT_TOKENS     110
#define IDC_EDIT_TABLA      111
#define IDC_EDIT_ERRORES    112
#define IDC_EDIT_AST        113
#define IDC_EDIT_CONSOLA    114

/* Handles de controles de la ventana */
HWND hTab;
HWND hEditEntrada;
HWND hEditConsola;
HWND hEditTokens;
HWND hEditTabla;
HWND hEditAST;
HWND hEditErrores;
HWND hBtnAbrir;
HWND hBtnAnalizar;
HWND hBtnEjecutar;
HWND hBtnEjValido;
HWND hBtnEjError;
HWND hBtnLimpiar;
HWND hLabelFile;
HWND hLabelStatus;
HBRUSH hBrushConsole;

string rutaArchivo;
string dirProyecto;

/* Obtiene la ruta del directorio del ejecutable */
static string obtenerDirEjecutable() {
    char buf[MAX_PATH];
    GetModuleFileNameA(NULL, buf, MAX_PATH);
    string ruta(buf);
    size_t sep = ruta.find_last_of("\\/");
    if (sep != string::npos) return ruta.substr(0, sep + 1);
    return "";
}

/* Setea texto en un control EDIT convirtiendo saltos de linea a formato Windows */
static void setEditText(HWND hEdit, const string& texto) {
    string conv;
    conv.reserve(texto.size() * 2);
    for (char c : texto) {
        if (c == '\n') conv += "\r\n";
        else conv += c;
    }
    SetWindowTextA(hEdit, conv.c_str());
}

/* Obtiene el texto actual del editor de entrada */
static string obtenerTextoEditor() {
    int len = GetWindowTextLengthA(hEditEntrada);
    if (len > 0) {
        vector<char> buf(len + 1);
        GetWindowTextA(hEditEntrada, buf.data(), len + 1);
        return string(buf.data());
    }
    if (!rutaArchivo.empty()) {
        string f = FileConverter::leer(rutaArchivo);
        if (!f.empty()) {
            setEditText(hEditEntrada, f);
            return f;
        }
    }
    return "";
}

/* Alterna la visibilidad de los paneles segun la pestana activa (6 pestanas) */
static void mostrarPestana(int idx) {
    ShowWindow(hEditEntrada, SW_HIDE);
    ShowWindow(hEditConsola, SW_HIDE);
    ShowWindow(hEditTokens,  SW_HIDE);
    ShowWindow(hEditTabla,   SW_HIDE);
    ShowWindow(hEditAST,     SW_HIDE);
    ShowWindow(hEditErrores, SW_HIDE);
    switch (idx) {
        case 0: ShowWindow(hEditEntrada, SW_SHOW); break;
        case 1: ShowWindow(hEditConsola, SW_SHOW); break;
        case 2: ShowWindow(hEditTokens,  SW_SHOW); break;
        case 3: ShowWindow(hEditTabla,   SW_SHOW); break;
        case 4: ShowWindow(hEditAST,     SW_SHOW); break;
        case 5: ShowWindow(hEditErrores, SW_SHOW); break;
    }
}

/* Guarda texto en disco creando el directorio si no existe */
static void guardarArchivo(const string& ruta, const string& contenido) {
    string dir = ruta.substr(0, ruta.find_last_of("\\/"));
    CreateDirectoryA(dir.c_str(), NULL);
    ofstream f(ruta);
    if (f) f << contenido;
}

/* Ejecuta el analisis lexico y sintactico del codigo */
static void ejecutarAnalisis(HWND hWnd) {
    string fuente = obtenerTextoEditor();
    if (fuente.empty()) {
        MessageBoxA(hWnd, "El codigo fuente esta vacio. Escriba codigo antes de analizar.", "Aviso", MB_ICONWARNING);
        return;
    }

    DiagnosticoEntrada diag = ErrorHandler::validarCodigoFuente(fuente);
    if (!diag.esValido) {
        string msg = diag.mensaje + "\n\nSugerencia: " + diag.sugerencia;
        MessageBoxA(hWnd, msg.c_str(), diag.titulo.c_str(), MB_ICONWARNING);
        return;
    }

    /* 1. Analisis lexico */
    Lexer lexer(fuente);
    lexer.analizar();

    const vector<Token>& tokens  = lexer.getTokens();
    const SymbolTable&   tabla   = lexer.getTabla();
    const vector<Token>& errLexicos = lexer.getErrores();

    /* 2. Analisis sintactico y construccion del AST */
    Parser parser(tokens);
    auto ast = parser.parsear();
    bool sintaxisValida = parser.esAceptado();
    const auto& errSintacticos = parser.getErrores();

    /* 3. Reportes */
    string strTokens     = ErrorHandler::generarReporteTokens(tokens, lexer.getSalidaPorLineas());
    string strTabla      = ErrorHandler::generarReporteTablaSimbolos(tabla);
    string strAST        = ast ? ast->toTreeString() : "(AST no generado debido a errores sintacticos criticos)\r\n";
    string strDiagnostico = ErrorHandler::generarReporteDiagnosticoCompleto(errLexicos, sintaxisValida, errSintacticos);

    /* 4. Actualizar vistas */
    setEditText(hEditTokens,  strTokens);
    setEditText(hEditTabla,   strTabla);
    setEditText(hEditAST,     strAST);
    setEditText(hEditErrores, strDiagnostico);

    bool programaAceptado = errLexicos.empty() && sintaxisValida;

    /* Actualizar banner de estado */
    if (programaAceptado) {
        SetWindowTextA(hLabelStatus, "[✓] PROGRAMA ACEPTADO: 0 errores lexicos / 0 errores sintacticos. Listo para ejecutar con [▶ Ejecutar LP].");
        TabCtrl_SetCurSel(hTab, 4); /* Mostrar AST */
        mostrarPestana(4);
    } else {
        string st = "[✗] PROGRAMA RECHAZADO (" + to_string(errLexicos.size()) + " err. lexicos, " +
                    to_string(errSintacticos.size()) + " err. sintacticos)";
        SetWindowTextA(hLabelStatus, st.c_str());
        TabCtrl_SetCurSel(hTab, 5); /* Mostrar Diagnostico */
        mostrarPestana(5);
    }

    /* 5. Exportar a output/ */
    string outDir = dirProyecto + "..\\output\\";
    guardarArchivo(outDir + "tokens.txt",        strTokens);
    guardarArchivo(outDir + "tabla_simbolos.txt", strTabla);
    guardarArchivo(outDir + "ast.txt",            strAST);
    guardarArchivo(outDir + "errores.txt",        strDiagnostico);

    if (programaAceptado) {
        MessageBoxA(hWnd,
            "[✓] PROGRAMA ACEPTADO\n\n"
            "El programa fuente es 100% valido segun la gramatica formal LP.\n"
            "El arbol sintactico (AST) se genero correctamente.\n\n"
            "Puedes presionar [▶ Ejecutar LP] para ejecutar el programa.",
            "Compilador LP - Analisis Exitoso", MB_ICONINFORMATION);
    } else {
        string aviso = "[✗] PROGRAMA RECHAZADO\n\n"
                       "Fallas detectadas:\n"
                       "  - Errores Lexicos: " + to_string(errLexicos.size()) + "\n"
                       "  - Errores Sintacticos: " + to_string(errSintacticos.size()) + "\n\n"
                       "Revise la pestana 'Diagnostico y Errores' para ver la linea exacta y sugerencia de correccion.";
        MessageBoxA(hWnd, aviso.c_str(), "Compilador LP - Errores Detectados", MB_ICONWARNING);
    }
}

/* Ejecuta el analisis y la interpretacion en tiempo real del programa LP */
static void ejecutarProgramaLP(HWND hWnd) {
    string fuente = obtenerTextoEditor();
    if (fuente.empty()) {
        MessageBoxA(hWnd, "El codigo fuente esta vacio. Escriba codigo antes de ejecutar.", "Aviso", MB_ICONWARNING);
        return;
    }

    DiagnosticoEntrada diag = ErrorHandler::validarCodigoFuente(fuente);
    if (!diag.esValido) {
        string msg = diag.mensaje + "\n\nSugerencia: " + diag.sugerencia;
        MessageBoxA(hWnd, msg.c_str(), diag.titulo.c_str(), MB_ICONWARNING);
        return;
    }

    /* 1. Analisis lexico */
    Lexer lexer(fuente);
    lexer.analizar();
    const auto& tokens = lexer.getTokens();
    const auto& tabla = lexer.getTabla();
    const auto& errLexicos = lexer.getErrores();

    /* 2. Analisis sintactico */
    Parser parser(tokens);
    auto ast = parser.parsear();
    bool sintaxisValida = parser.esAceptado();
    const auto& errSintacticos = parser.getErrores();

    string strTokens = ErrorHandler::generarReporteTokens(tokens, lexer.getSalidaPorLineas());
    string strTabla = ErrorHandler::generarReporteTablaSimbolos(tabla);
    string strAST = ast ? ast->toTreeString() : "(AST no disponible)\r\n";
    string strDiag = ErrorHandler::generarReporteDiagnosticoCompleto(errLexicos, sintaxisValida, errSintacticos);

    setEditText(hEditTokens, strTokens);
    setEditText(hEditTabla, strTabla);
    setEditText(hEditAST, strAST);
    setEditText(hEditErrores, strDiag);

    bool programaAceptado = errLexicos.empty() && sintaxisValida;
    if (!programaAceptado) {
        SetWindowTextA(hLabelStatus, "[✗] ERROR DE COMPILACION: El programa no puede ejecutarse debido a errores.");
        TabCtrl_SetCurSel(hTab, 5);
        mostrarPestana(5);
        MessageBoxA(hWnd, "No se puede ejecutar el programa.\n\nContiene errores de compilacion. Revise la pestana 'Diagnostico y Errores'.", "Error al Ejecutar", MB_ICONWARNING);
        return;
    }

    /* 3. Ejecucion con el Interprete */
    Interpreter interprete(ast);
    bool exitoExec = interprete.ejecutar();
    string reporteExec = interprete.getReporteEjecucion();

    setEditText(hEditConsola, reporteExec);

    /* Exportar a disco */
    string outDir = dirProyecto + "..\\output\\";
    guardarArchivo(outDir + "ejecucion.txt", reporteExec);
    guardarArchivo(outDir + "tokens.txt", strTokens);
    guardarArchivo(outDir + "tabla_simbolos.txt", strTabla);
    guardarArchivo(outDir + "ast.txt", strAST);
    guardarArchivo(outDir + "errores.txt", strDiag);

    if (exitoExec) {
        ostringstream st;
        st << "[✓] EJECUTADO EXITOSAMENTE | Retorno: " << interprete.getCodigoRetorno()
           << " | Tiempo: " << fixed << setprecision(3) << interprete.getTiempoMs() << " ms";
        SetWindowTextA(hLabelStatus, st.str().c_str());
    } else {
        SetWindowTextA(hLabelStatus, "[!] EJECUTADO CON ERRORES DE RUNTIME (Ver Consola)");
    }

    /* Conmutar a la Consola de Ejecucion */
    TabCtrl_SetCurSel(hTab, 1);
    mostrarPestana(1);
}

/* Ajusta los controles al redimensionar la ventana */
static void redimensionarPaneles(HWND hWnd) {
    RECT rc;
    GetClientRect(hWnd, &rc);
    int ancho = rc.right;
    int alto  = rc.bottom;
    int m     = 10;

    /* Fila 1: Botonera superior */
    int btnY = 8;
    int btnH = 30;

    SetWindowPos(hBtnAbrir,     NULL, m,             btnY, 110, btnH, SWP_NOZORDER);
    SetWindowPos(hBtnAnalizar,  NULL, m + 115,       btnY, 100, btnH, SWP_NOZORDER);
    SetWindowPos(hBtnEjecutar,  NULL, m + 220,       btnY, 120, btnH, SWP_NOZORDER);
    SetWindowPos(hBtnEjValido,  NULL, m + 345,       btnY, 110, btnH, SWP_NOZORDER);
    SetWindowPos(hBtnEjError,   NULL, m + 460,       btnY, 110, btnH, SWP_NOZORDER);
    SetWindowPos(hBtnLimpiar,   NULL, m + 575,       btnY, 80,  btnH, SWP_NOZORDER);
    SetWindowPos(hLabelFile,    NULL, m + 665,       btnY + 5, ancho - (m + 675), 20, SWP_NOZORDER);

    /* Fila 2: Banner de estado */
    int statusY = 44;
    int statusH = 22;
    SetWindowPos(hLabelStatus,  NULL, m + 2, statusY, ancho - (m * 2 + 4), statusH, SWP_NOZORDER);

    /* Fila 3: Pestanas y Paneles */
    int tabY = 70;
    int tabW = ancho - m * 2;
    int tabH = alto - tabY - m;
    SetWindowPos(hTab, NULL, m, tabY, tabW, tabH, SWP_NOZORDER);

    RECT rcTab = {m, tabY, m + tabW, tabY + tabH};
    TabCtrl_AdjustRect(hTab, FALSE, &rcTab);
    int eX = rcTab.left - m + 4;
    int eY = rcTab.top  - tabY + 4;
    int eW = rcTab.right  - rcTab.left - 8;
    int eH = rcTab.bottom - rcTab.top  - 8;

    SetWindowPos(hEditEntrada, NULL, m + eX, tabY + eY, eW, eH, SWP_NOZORDER);
    SetWindowPos(hEditConsola, NULL, m + eX, tabY + eY, eW, eH, SWP_NOZORDER);
    SetWindowPos(hEditTokens,  NULL, m + eX, tabY + eY, eW, eH, SWP_NOZORDER);
    SetWindowPos(hEditTabla,   NULL, m + eX, tabY + eY, eW, eH, SWP_NOZORDER);
    SetWindowPos(hEditAST,     NULL, m + eX, tabY + eY, eW, eH, SWP_NOZORDER);
    SetWindowPos(hEditErrores, NULL, m + eX, tabY + eY, eW, eH, SWP_NOZORDER);
}

/* Manejador de eventos de la ventana */
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_CREATE: {
        HFONT hFont = CreateFontA(
            15, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

        hBtnAbrir = CreateWindowA("BUTTON", "Abrir Archivo",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0,0,0,0, hWnd, (HMENU)IDC_BTN_ABRIR, NULL, NULL);
        SendMessage(hBtnAbrir, WM_SETFONT, (WPARAM)hFont, TRUE);

        hBtnAnalizar = CreateWindowA("BUTTON", "Analizar",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0,0,0,0, hWnd, (HMENU)IDC_BTN_ANALIZAR, NULL, NULL);
        SendMessage(hBtnAnalizar, WM_SETFONT, (WPARAM)hFont, TRUE);

        hBtnEjecutar = CreateWindowA("BUTTON", "Ejecutar LP",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0,0,0,0, hWnd, (HMENU)IDC_BTN_EJECUTAR, NULL, NULL);
        SendMessage(hBtnEjecutar, WM_SETFONT, (WPARAM)hFont, TRUE);

        hBtnEjValido = CreateWindowA("BUTTON", "Ejemplo OK",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0,0,0,0, hWnd, (HMENU)IDC_BTN_EJ_VALIDO, NULL, NULL);
        SendMessage(hBtnEjValido, WM_SETFONT, (WPARAM)hFont, TRUE);

        hBtnEjError = CreateWindowA("BUTTON", "Ejemplo Error",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0,0,0,0, hWnd, (HMENU)IDC_BTN_EJ_ERROR, NULL, NULL);
        SendMessage(hBtnEjError, WM_SETFONT, (WPARAM)hFont, TRUE);

        hBtnLimpiar = CreateWindowA("BUTTON", "Limpiar",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0,0,0,0, hWnd, (HMENU)IDC_BTN_LIMPIAR, NULL, NULL);
        SendMessage(hBtnLimpiar, WM_SETFONT, (WPARAM)hFont, TRUE);

        hLabelFile = CreateWindowA("STATIC", "ejemplo.lp (codigo cargado)",
            WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
            0,0,0,0, hWnd, (HMENU)IDC_LABEL_FILE, NULL, NULL);
        SendMessage(hLabelFile, WM_SETFONT, (WPARAM)hFont, TRUE);

        hLabelStatus = CreateWindowA("STATIC", "[LISTO]: Presiona [Ejecutar LP] para compilar y ejecutar, o [Analizar] para validar.",
            WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
            0,0,0,0, hWnd, (HMENU)IDC_LABEL_STATUS, NULL, NULL);
        SendMessage(hLabelStatus, WM_SETFONT, (WPARAM)hFont, TRUE);

        hTab = CreateWindowA(WC_TABCONTROLA, "",
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
            0,0,0,0, hWnd, (HMENU)IDC_TAB, NULL, NULL);
        SendMessage(hTab, WM_SETFONT, (WPARAM)hFont, TRUE);

        TCITEMA ti = {};
        ti.mask = TCIF_TEXT;
        ti.pszText = const_cast<char*>("Entrada (Codigo Fuente)");
        TabCtrl_InsertItem(hTab, 0, &ti);
        ti.pszText = const_cast<char*>("Consola de Ejecucion");
        TabCtrl_InsertItem(hTab, 1, &ti);
        ti.pszText = const_cast<char*>("Secuencia de Tokens");
        TabCtrl_InsertItem(hTab, 2, &ti);
        ti.pszText = const_cast<char*>("Tabla de Simbolos");
        TabCtrl_InsertItem(hTab, 3, &ti);
        ti.pszText = const_cast<char*>("Arbol Sintactico (AST)");
        TabCtrl_InsertItem(hTab, 4, &ti);
        ti.pszText = const_cast<char*>("Diagnostico y Errores");
        TabCtrl_InsertItem(hTab, 5, &ti);

        DWORD estiloInput = WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
                            ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN;

        DWORD estiloEdit = WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
                           ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL;

        HFONT hMono = CreateFontA(
            16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

        /* Control de Entrada editable */
        hEditEntrada = CreateWindowA("EDIT", "",
            estiloInput, 0,0,0,0, hWnd, (HMENU)IDC_EDIT_ENTRADA, NULL, NULL);
        SendMessage(hEditEntrada, WM_SETFONT, (WPARAM)hMono, TRUE);

        /* Consola de Ejecucion con apariencia de terminal */
        hBrushConsole = CreateSolidBrush(RGB(20, 24, 30));
        hEditConsola = CreateWindowA("EDIT", "",
            estiloEdit, 0,0,0,0, hWnd, (HMENU)IDC_EDIT_CONSOLA, NULL, NULL);
        SendMessage(hEditConsola, WM_SETFONT, (WPARAM)hMono, TRUE);
        ShowWindow(hEditConsola, SW_HIDE);

        /* Controles de salida solo lectura */
        hEditTokens = CreateWindowA("EDIT", "",
            estiloEdit, 0,0,0,0, hWnd, (HMENU)IDC_EDIT_TOKENS, NULL, NULL);
        SendMessage(hEditTokens, WM_SETFONT, (WPARAM)hMono, TRUE);
        ShowWindow(hEditTokens, SW_HIDE);

        hEditTabla = CreateWindowA("EDIT", "",
            estiloEdit, 0,0,0,0, hWnd, (HMENU)IDC_EDIT_TABLA, NULL, NULL);
        SendMessage(hEditTabla, WM_SETFONT, (WPARAM)hMono, TRUE);
        ShowWindow(hEditTabla, SW_HIDE);

        hEditAST = CreateWindowA("EDIT", "",
            estiloEdit, 0,0,0,0, hWnd, (HMENU)IDC_EDIT_AST, NULL, NULL);
        SendMessage(hEditAST, WM_SETFONT, (WPARAM)hMono, TRUE);
        ShowWindow(hEditAST, SW_HIDE);

        hEditErrores = CreateWindowA("EDIT", "",
            estiloEdit, 0,0,0,0, hWnd, (HMENU)IDC_EDIT_ERRORES, NULL, NULL);
        SendMessage(hEditErrores, WM_SETFONT, (WPARAM)hMono, TRUE);
        ShowWindow(hEditErrores, SW_HIDE);

        /* Codigo de ejemplo por defecto */
        setEditText(hEditEntrada,
            "func int sumar(int a, int b) {\r\n"
            "    return a + b;\r\n"
            "}\r\n"
            "\r\n"
            "void main() {\r\n"
            "    int edad;\r\n"
            "    float promedio;\r\n"
            "    int total;\r\n"
            "    edad = 20;\r\n"
            "    promedio = 15.5;\r\n"
            "    total = func sumar(edad, 10);\r\n"
            "    if (edad >= 18 && edad <= 60) {\r\n"
            "        println(\"Edad valida:\", edad);\r\n"
            "    }\r\n"
            "    println(\"Total calculado con funcion:\", total);\r\n"
            "    println(\"Promedio general:\", promedio);\r\n"
            "    return 0;\r\n"
            "}");

        setEditText(hEditConsola,
            "===========================================================================================\r\n"
            "                          CONSOLA DE EJECUCION DEL PROGRAMA LP                             \r\n"
            "===========================================================================================\r\n\r\n"
            "Presiona [Ejecutar LP] para compilar e interpretar el programa en tiempo real.\r\n"
            "Las llamadas a 'println(...)' mostraran su salida en este panel terminal.");

        setEditText(hEditTokens,
            "=== SECUENCIA DE TOKENS ===\r\n"
            "Presiona [Analizar] o [Ejecutar LP] para generar los tokens.");
        setEditText(hEditTabla,
            "=== TABLA DE SIMBOLOS ===\r\n"
            "Presiona [Analizar] o [Ejecutar LP] para visualizar los identificadores.");
        setEditText(hEditAST,
            "=== ARBOL SINTACTICO ABSTRACTO (AST) ===\r\n"
            "Presiona [Analizar] o [Ejecutar LP] para visualizar el arbol sintactico.");
        setEditText(hEditErrores,
            "=== DIAGNOSTICO INTEGRAL (LEXICO Y SINTACTICO) ===\r\n"
            "Presiona [Analizar] para verificar el diagnostico completo.");
        break;
    }

    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT: {
        if ((HWND)lParam == hEditConsola) {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(50, 255, 140)); /* Verde terminal moderno */
            SetBkColor(hdc, RGB(20, 24, 30));
            return (LRESULT)hBrushConsole;
        }
        break;
    }

    case WM_SIZE:
        redimensionarPaneles(hWnd);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDC_BTN_ABRIR: {
            char szFile[MAX_PATH] = {};
            OPENFILENAMEA ofn = {};
            ofn.lStructSize  = sizeof(ofn);
            ofn.hwndOwner    = hWnd;
            ofn.lpstrFile    = szFile;
            ofn.nMaxFile     = sizeof(szFile);
            ofn.lpstrFilter  =
                "Archivos LP (*.lp)\0*.lp\0"
                "Archivos de Texto (*.txt)\0*.txt\0"
                "Documentos Word (*.docx)\0*.docx\0"
                "Archivos PDF (*.pdf)\0*.pdf\0"
                "Todos los archivos (*.*)\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileNameA(&ofn)) {
                DiagnosticoEntrada diag = ErrorHandler::validarArchivo(szFile);
                if (!diag.esValido) {
                    string msg = diag.mensaje + "\n\nSugerencia: " + diag.sugerencia;
                    MessageBoxA(hWnd, msg.c_str(), diag.titulo.c_str(), MB_ICONERROR);
                    break;
                }

                rutaArchivo = szFile;
                string nombre = rutaArchivo;
                size_t sep = nombre.find_last_of("\\/");
                if (sep != string::npos) nombre = nombre.substr(sep + 1);
                SetWindowTextA(hLabelFile, nombre.c_str());

                string contenido = FileConverter::leer(rutaArchivo);
                if (!contenido.empty()) {
                    setEditText(hEditEntrada, contenido);
                    TabCtrl_SetCurSel(hTab, 0);
                    mostrarPestana(0);
                    SetWindowTextA(hLabelStatus, ("[ARCHIVO CARGADO]: " + nombre).c_str());
                } else {
                    string msg = "No se pudo leer el archivo.\n\n" + FileConverter::ultimoError();
                    MessageBoxA(hWnd, msg.c_str(), "Error al Procesar Archivo", MB_ICONERROR);
                }
            }
            break;
        }

        case IDC_BTN_ANALIZAR:
            ejecutarAnalisis(hWnd);
            break;

        case IDC_BTN_EJECUTAR:
            ejecutarProgramaLP(hWnd);
            break;

        case IDC_BTN_EJ_VALIDO:
            setEditText(hEditEntrada,
                "func int multiplicar(int a, int b) {\r\n"
                "    return a * b;\r\n"
                "}\r\n"
                "\r\n"
                "void main() {\r\n"
                "    int edad;\r\n"
                "    float promedio;\r\n"
                "    int calculo;\r\n"
                "    edad = 20;\r\n"
                "    promedio = 15.5;\r\n"
                "    calculo = func multiplicar(edad, 2);\r\n"
                "    if (edad >= 18 && edad <= 60) {\r\n"
                "        println(\"Programa valido. Edad:\", edad);\r\n"
                "        println(\"Calculo con funcion:\", calculo);\r\n"
                "    }\r\n"
                "    return 0;\r\n"
                "}");
            SetWindowTextA(hLabelFile, "ejemplo_valido.lp");
            SetWindowTextA(hLabelStatus, "[INFO]: Codigo valido cargado. Presiona [Ejecutar LP] para correr.");
            TabCtrl_SetCurSel(hTab, 0);
            mostrarPestana(0);
            break;

        case IDC_BTN_EJ_ERROR:
            setEditText(hEditEntrada,
                "void main() {\r\n"
                "    int edad\r\n"
                "    edad = 20;\r\n"
                "}");
            SetWindowTextA(hLabelFile, "ejemplo_error.lp");
            SetWindowTextA(hLabelStatus, "[INFO]: Codigo con error (falta punto y coma) cargado. Presiona [Analizar] para ver el diagnostico.");
            TabCtrl_SetCurSel(hTab, 0);
            mostrarPestana(0);
            break;

        case IDC_BTN_LIMPIAR:
            SetWindowTextA(hEditEntrada, "");
            SetWindowTextA(hEditConsola, "");
            SetWindowTextA(hEditTokens, "");
            SetWindowTextA(hEditTabla, "");
            SetWindowTextA(hEditAST, "");
            SetWindowTextA(hEditErrores, "");
            SetWindowTextA(hLabelFile, "(editor vacio)");
            SetWindowTextA(hLabelStatus, "[LISTO]: Editor despejado.");
            TabCtrl_SetCurSel(hTab, 0);
            mostrarPestana(0);
            break;
        }
        break;

    case WM_NOTIFY: {
        NMHDR* pNM = reinterpret_cast<NMHDR*>(lParam);
        if (pNM->hwndFrom == hTab && pNM->code == TCN_SELCHANGE) {
            mostrarPestana(TabCtrl_GetCurSel(hTab));
        }
        break;
    }

    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hWnd, &rc);
        FillRect(hdc, &rc, (HBRUSH)(COLOR_BTNFACE + 1));
        return 1;
    }

    case WM_DESTROY:
        if (hBrushConsole) DeleteObject(hBrushConsole);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcA(hWnd, msg, wParam, lParam);
    }
    return 0;
}

/* Punto de entrada WinMain para aplicacion de ventana */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    dirProyecto = obtenerDirEjecutable();

    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_TAB_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);

    HICON hAppIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(101));
    if (!hAppIcon) {
        hAppIcon = (HICON)LoadImageA(NULL, "res\\compiler.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
    }
    if (!hAppIcon) {
        hAppIcon = LoadIcon(NULL, IDI_APPLICATION);
    }

    WNDCLASSEXA wc   = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "LexLP_Clase";
    wc.hIcon         = hAppIcon;
    wc.hIconSm       = hAppIcon;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    RegisterClassExA(&wc);

    HWND hWnd = CreateWindowExA(0, "LexLP_Clase",
        "Compilador LP - Entorno Integrado: Lexico, Sintactico (Parser), AST y Ejecucion",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1080, 700,
        NULL, NULL, hInstance, NULL);

    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hAppIcon);
    SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hAppIcon);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg = {};
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return static_cast<int>(msg.wParam);
}
