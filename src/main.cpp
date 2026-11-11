#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>

#include <string>
#include <sstream>
#include <fstream>
#include <vector>

#include "Lexer.h"
#include "Token.h"
#include "FileConverter.h"
#include "ErrorHandler.h"
#include "AST.h"
#include "Parser.h"

using namespace std;

/* Identificadores de controles */
#define IDC_TAB          100
#define IDC_BTN_ABRIR    101
#define IDC_BTN_ANALIZAR 102
#define IDC_LABEL_FILE   103
#define IDC_LABEL_STATUS 104
#define IDC_EDIT_ENTRADA 109
#define IDC_EDIT_TOKENS  110
#define IDC_EDIT_TABLA   111
#define IDC_EDIT_ERRORES 112
#define IDC_EDIT_AST     113

/* Handles de controles de la ventana */
HWND hTab;
HWND hEditEntrada;
HWND hEditTokens;
HWND hEditTabla;
HWND hEditAST;
HWND hEditErrores;
HWND hBtnAbrir;
HWND hBtnAnalizar;
HWND hLabelFile;
HWND hLabelStatus;

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

/* Alterna la visibilidad de los paneles segun la pestana activa */
static void mostrarPestana(int idx) {
    ShowWindow(hEditEntrada, SW_HIDE);
    ShowWindow(hEditTokens,  SW_HIDE);
    ShowWindow(hEditTabla,   SW_HIDE);
    ShowWindow(hEditAST,     SW_HIDE);
    ShowWindow(hEditErrores, SW_HIDE);
    switch (idx) {
        case 0: ShowWindow(hEditEntrada, SW_SHOW); break;
        case 1: ShowWindow(hEditTokens,  SW_SHOW); break;
        case 2: ShowWindow(hEditTabla,   SW_SHOW); break;
        case 3: ShowWindow(hEditAST,     SW_SHOW); break;
        case 4: ShowWindow(hEditErrores, SW_SHOW); break;
    }
}

/* Guarda texto en disco creando el directorio si no existe */
static void guardarArchivo(const string& ruta, const string& contenido) {
    string dir = ruta.substr(0, ruta.find_last_of("\\/"));
    CreateDirectoryA(dir.c_str(), NULL);

    ofstream f(ruta);
    if (f) f << contenido;
}

/* Ejecuta el analisis lexico y sintactico del texto en el editor o archivo actual y actualiza la UI */
static void ejecutarAnalisis(HWND hWnd) {
    string fuente;
    int len = GetWindowTextLengthA(hEditEntrada);
    if (len > 0) {
        vector<char> buf(len + 1);
        GetWindowTextA(hEditEntrada, buf.data(), len + 1);
        fuente = buf.data();
    } else if (!rutaArchivo.empty()) {
        fuente = FileConverter::leer(rutaArchivo);
        if (!fuente.empty()) {
            setEditText(hEditEntrada, fuente);
        }
    }

    /* Contencion de errores: Validar entrada para evitar valores muertos o entradas vacias */
    DiagnosticoEntrada diag = ErrorHandler::validarCodigoFuente(fuente);
    if (!diag.esValido) {
        string msg = diag.mensaje + "\n\nSugerencia: " + diag.sugerencia;
        MessageBoxA(hWnd, msg.c_str(), diag.titulo.c_str(), MB_ICONWARNING);
        return;
    }

    /* 1. Analisis lexico completo */
    Lexer lexer(fuente);
    lexer.analizar();

    const vector<Token>& tokens  = lexer.getTokens();
    const SymbolTable&   tabla   = lexer.getTabla();
    const vector<Token>& errLexicos = lexer.getErrores();

    /* 2. Analisis sintactico (Parser por Descenso Recursivo) y construccion del AST */
    Parser parser(tokens);
    auto ast = parser.parsear();
    bool sintaxisValida = parser.esAceptado();
    const auto& errSintacticos = parser.getErrores();

    /* 3. Generar reportes unificados a traves del modulo ErrorHandler */
    string strTokens     = ErrorHandler::generarReporteTokens(tokens, lexer.getSalidaPorLineas());
    string strTabla      = ErrorHandler::generarReporteTablaSimbolos(tabla);
    string strAST        = ast ? ast->toTreeString() : "(AST no generado debido a errores sintacticos criticos)\r\n";
    string strDiagnostico = ErrorHandler::generarReporteDiagnosticoCompleto(errLexicos, sintaxisValida, errSintacticos);

    /* 4. Actualizar la interfaz con los resultados */
    setEditText(hEditTokens,  strTokens);
    setEditText(hEditTabla,   strTabla);
    setEditText(hEditAST,     strAST);
    setEditText(hEditErrores, strDiagnostico);

    bool programaAceptado = errLexicos.empty() && sintaxisValida;

    /* Actualizar banner de estado */
    if (programaAceptado) {
        SetWindowTextA(hLabelStatus, "[✓] PROGRAMA ACEPTADO: 0 errores lexicos / 0 errores sintacticos");
        TabCtrl_SetCurSel(hTab, 3); /* Mostrar AST */
        mostrarPestana(3);
    } else {
        string st = "[✗] PROGRAMA RECHAZADO (" + to_string(errLexicos.size()) + " err. lexicos, " +
                    to_string(errSintacticos.size()) + " err. sintacticos)";
        SetWindowTextA(hLabelStatus, st.c_str());
        TabCtrl_SetCurSel(hTab, 4); /* Mostrar Diagnostico */
        mostrarPestana(4);
    }

    /* 5. Exportar resultados a la carpeta output/ */
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
            "Archivos exportados en output/:\n"
            "  - tokens.txt\n"
            "  - tabla_simbolos.txt\n"
            "  - ast.txt\n"
            "  - errores.txt",
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

/* Ajusta los controles al redimensionar la ventana */
static void redimensionarPaneles(HWND hWnd) {
    RECT rc;
    GetClientRect(hWnd, &rc);
    int ancho = rc.right;
    int alto  = rc.bottom;
    int barra = 52;
    int m     = 10;

    SetWindowPos(hBtnAbrir,    NULL, m,        10, 130, 32, SWP_NOZORDER);
    SetWindowPos(hBtnAnalizar, NULL, m + 140,  10, 120, 32, SWP_NOZORDER);
    SetWindowPos(hLabelFile,   NULL, m + 270,  17, 240, 22, SWP_NOZORDER);
    SetWindowPos(hLabelStatus, NULL, m + 520,  17, ancho - (m + 530), 22, SWP_NOZORDER);

    int tabX = m, tabY = barra, tabW = ancho - m * 2, tabH = alto - barra - m;
    SetWindowPos(hTab, NULL, tabX, tabY, tabW, tabH, SWP_NOZORDER);

    RECT rcTab = {tabX, tabY, tabX + tabW, tabY + tabH};
    TabCtrl_AdjustRect(hTab, FALSE, &rcTab);
    int eX = rcTab.left - tabX + 4;
    int eY = rcTab.top  - tabY + 4;
    int eW = rcTab.right  - rcTab.left - 8;
    int eH = rcTab.bottom - rcTab.top  - 8;

    SetWindowPos(hEditEntrada, NULL, tabX + eX, tabY + eY, eW, eH, SWP_NOZORDER);
    SetWindowPos(hEditTokens,  NULL, tabX + eX, tabY + eY, eW, eH, SWP_NOZORDER);
    SetWindowPos(hEditTabla,   NULL, tabX + eX, tabY + eY, eW, eH, SWP_NOZORDER);
    SetWindowPos(hEditAST,     NULL, tabX + eX, tabY + eY, eW, eH, SWP_NOZORDER);
    SetWindowPos(hEditErrores, NULL, tabX + eX, tabY + eY, eW, eH, SWP_NOZORDER);
}

/* Manejador de eventos de la ventana */
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_CREATE: {
        HFONT hFont = CreateFontA(
            16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
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

        hLabelFile = CreateWindowA("STATIC", "ejemplo.lp (codigo cargado)",
            WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
            0,0,0,0, hWnd, (HMENU)IDC_LABEL_FILE, NULL, NULL);
        SendMessage(hLabelFile, WM_SETFONT, (WPARAM)hFont, TRUE);

        hLabelStatus = CreateWindowA("STATIC", "[ESTADO]: Listo para compilar",
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
        ti.pszText = const_cast<char*>("Secuencia de Tokens");
        TabCtrl_InsertItem(hTab, 1, &ti);
        ti.pszText = const_cast<char*>("Tabla de Simbolos");
        TabCtrl_InsertItem(hTab, 2, &ti);
        ti.pszText = const_cast<char*>("Arbol Sintactico (AST)");
        TabCtrl_InsertItem(hTab, 3, &ti);
        ti.pszText = const_cast<char*>("Diagnostico y Errores");
        TabCtrl_InsertItem(hTab, 4, &ti);

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

        /* Codigo de ejemplo por defecto (ejemplo oficial del proyecto LP) */
        setEditText(hEditEntrada,
            "void main() {\r\n"
            "int edad;\r\n"
            "float promedio;\r\n"
            "edad = 20;\r\n"
            "promedio = 15.5;\r\n"
            "if (edad >= 18 && edad <= 60) {\r\n"
            "println(\"Edad valida\");\r\n"
            "}\r\n"
            "return 0;\r\n"
            "}");

        setEditText(hEditTokens,
            "=== SECUENCIA DE TOKENS ===\r\n"
            "Presiona [Analizar] para generar los tokens del programa.");
        setEditText(hEditTabla,
            "=== TABLA DE SIMBOLOS ===\r\n"
            "Presiona [Analizar] para visualizar los identificadores y sus posiciones.");
        setEditText(hEditAST,
            "=== ARBOL SINTACTICO ABSTRACTO (AST) ===\r\n"
            "Presiona [Analizar] para generar y visualizar el arbol sintactico.");
        setEditText(hEditErrores,
            "=== DIAGNOSTICO INTEGRAL (LEXICO Y SINTACTICO) ===\r\n"
            "Presiona [Analizar] para verificar el diagnostico completo.");
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
                /* Contencion de errores: Validar archivo antes de intentar cargarlo */
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
    icc.dwICC  = ICC_TAB_CLASSES;
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
        "Compilador LP - Analizador Lexico, Sintactico (Parser Descendente Recursivo) y AST",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1050, 680,
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
