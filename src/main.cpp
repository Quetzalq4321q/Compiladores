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

using namespace std;

/* Identificadores de controles */
#define IDC_TAB          100
#define IDC_BTN_ABRIR    101
#define IDC_BTN_ANALIZAR 102
#define IDC_LABEL_FILE   103
#define IDC_EDIT_TOKENS  110
#define IDC_EDIT_TABLA   111
#define IDC_EDIT_ERRORES 112

/* Handles de controles de la ventana */
HWND hTab;
HWND hEditTokens;
HWND hEditTabla;
HWND hEditErrores;
HWND hBtnAbrir;
HWND hBtnAnalizar;
HWND hLabelFile;

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
    ShowWindow(hEditTokens,  SW_HIDE);
    ShowWindow(hEditTabla,   SW_HIDE);
    ShowWindow(hEditErrores, SW_HIDE);
    switch (idx) {
        case 0: ShowWindow(hEditTokens,  SW_SHOW); break;
        case 1: ShowWindow(hEditTabla,   SW_SHOW); break;
        case 2: ShowWindow(hEditErrores, SW_SHOW); break;
    }
}

/* Guarda texto en disco creando el directorio si no existe */
static void guardarArchivo(const string& ruta, const string& contenido) {
    string dir = ruta.substr(0, ruta.find_last_of("\\/"));
    CreateDirectoryA(dir.c_str(), NULL);

    ofstream f(ruta);
    if (f) f << contenido;
}

/* Ejecuta el analisis lexico del archivo actual y actualiza la UI */
static void ejecutarAnalisis(HWND hWnd) {
    if (rutaArchivo.empty()) {
        MessageBoxA(hWnd, "Primero abre un archivo.", "Aviso", MB_ICONWARNING);
        return;
    }

    SetWindowTextA(hLabelFile, "Leyendo archivo...");
    UpdateWindow(hWnd);

    string fuente = FileConverter::leer(rutaArchivo);
    if (fuente.empty()) {
        string msg = "No se pudo leer el archivo.\n\n" + FileConverter::ultimoError();
        MessageBoxA(hWnd, msg.c_str(), "Error", MB_ICONERROR);

        string nombre = rutaArchivo;
        size_t sep = nombre.find_last_of("\\/");
        if (sep != string::npos) nombre = nombre.substr(sep + 1);
        SetWindowTextA(hLabelFile, nombre.c_str());
        return;
    }

    /* Restaura nombre del archivo en la barra */
    {
        string nombre = rutaArchivo;
        size_t sep = nombre.find_last_of("\\/");
        if (sep != string::npos) nombre = nombre.substr(sep + 1);
        SetWindowTextA(hLabelFile, nombre.c_str());
    }

    /* Analisis lexico */
    Lexer lexer(fuente);
    lexer.analizar();

    const vector<Token>& tokens  = lexer.getTokens();
    const SymbolTable&   tabla   = lexer.getTabla();
    const vector<Token>& errores = lexer.getErrores();

    /* Pestana 0: Lista de tokens */
    string strTokens;
    {
        ostringstream ss;
        ss << "=== LISTA DE TOKENS ===\n";
        ss << "Total: " << tokens.size() << " token(s)\n";
        ss << "---------------------------------------\n";
        if (tokens.empty()) {
            ss << "(Sin tokens reconocidos)\n";
        } else {
            ss << "#     TOKEN        LEXEMA         LINEA\n";
            ss << "---------------------------------------\n";
            int i = 1;
            for (const auto& t : tokens) {
                string num = to_string(i++) + ".";
                num.resize(6, ' ');
                ss << num;

                string tipo = t.toString();
                tipo.resize(13, ' ');
                ss << tipo;

                string lex = t.lexema;
                if (lex.size() > 14) lex = lex.substr(0, 14);
                lex.resize(15, ' ');
                ss << lex;

                ss << t.linea << "\n";
            }
        }
        strTokens = ss.str();
    }

    /* Pestana 1: Tabla de simbolos */
    string strTabla;
    {
        ostringstream ss;
        ss << "=== TABLA DE SIMBOLOS ===\n";
        ss << "---------------------------------\n";
        if (tabla.vacia()) {
            ss << "(Vacia en Semana 1)\n\n";
            ss << "Los identificadores (ID) se\n";
            ss << "agregaran en la Semana 2.\n";
        } else {
            ss << "POS   IDENTIFICADOR\n";
            ss << "---------------------------------\n";
            int p = 0;
            for (const auto& s : tabla.getSimbolos()) {
                string idx = to_string(p++);
                idx.resize(6, ' ');
                ss << idx << s << "\n";
            }
        }
        strTabla = ss.str();
    }

    /* Pestana 2: Errores lexicos */
    string strErrores;
    {
        ostringstream ss;
        ss << "=== ERRORES LEXICOS ===\n";
        ss << "Total: " << errores.size() << " error(es)\n";
        ss << "---------------------------------------\n";
        if (errores.empty()) {
            ss << "(Sin errores lexicos)\n";
        } else {
            ss << "LINEA  LEXEMA         RESULTADO\n";
            ss << "---------------------------------------\n";
            for (const auto& e : errores) {
                string lin = to_string(e.linea);
                lin.resize(7, ' ');
                string lex = "'" + e.lexema + "'";
                lex.resize(15, ' ');
                ss << lin << lex << "ERROR_LEXICO\n";
            }
        }
        strErrores = ss.str();
    }

    /* Actualiza la interfaz */
    setEditText(hEditTokens,  strTokens);
    setEditText(hEditTabla,   strTabla);
    setEditText(hEditErrores, strErrores);
    TabCtrl_SetCurSel(hTab, 0);
    mostrarPestana(0);

    /* Exporta resultados a la carpeta output/ */
    string outDir = dirProyecto + "..\\output\\";
    guardarArchivo(outDir + "tokens.txt",          strTokens);
    guardarArchivo(outDir + "tabla_simbolos.txt",   strTabla);
    guardarArchivo(outDir + "errores.txt",          strErrores);

    MessageBoxA(hWnd,
        "Analisis completado.\n\n"
        "Resultados exportados a la carpeta output/:\n"
        "  - tokens.txt\n"
        "  - tabla_simbolos.txt\n"
        "  - errores.txt",
        "Listo", MB_ICONINFORMATION);
}

/* Ajusta los controles al redimensionar la ventana */
static void redimensionarPaneles(HWND hWnd) {
    RECT rc;
    GetClientRect(hWnd, &rc);
    int ancho = rc.right;
    int alto  = rc.bottom;
    int barra = 40;
    int m     = 5;

    SetWindowPos(hBtnAbrir,    NULL, m,       7, 120, 26, SWP_NOZORDER);
    SetWindowPos(hBtnAnalizar, NULL, m + 130, 7, 100, 26, SWP_NOZORDER);
    SetWindowPos(hLabelFile,   NULL, m + 240, 11, ancho - 250, 20, SWP_NOZORDER);

    int tabX = m, tabY = barra, tabW = ancho - m * 2, tabH = alto - barra - m;
    SetWindowPos(hTab, NULL, tabX, tabY, tabW, tabH, SWP_NOZORDER);

    RECT rcTab = {tabX, tabY, tabX + tabW, tabY + tabH};
    TabCtrl_AdjustRect(hTab, FALSE, &rcTab);
    int eX = rcTab.left - tabX + 2;
    int eY = rcTab.top  - tabY + 2;
    int eW = rcTab.right  - rcTab.left - 4;
    int eH = rcTab.bottom - rcTab.top  - 4;

    SetWindowPos(hEditTokens,  NULL, tabX + eX, tabY + eY, eW, eH, SWP_NOZORDER);
    SetWindowPos(hEditTabla,   NULL, tabX + eX, tabY + eY, eW, eH, SWP_NOZORDER);
    SetWindowPos(hEditErrores, NULL, tabX + eX, tabY + eY, eW, eH, SWP_NOZORDER);
}

/* Manejador de eventos de la ventana */
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_CREATE: {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        hBtnAbrir = CreateWindowA("BUTTON", "Abrir Archivo",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0,0,0,0, hWnd, (HMENU)IDC_BTN_ABRIR, NULL, NULL);
        SendMessage(hBtnAbrir, WM_SETFONT, (WPARAM)hFont, TRUE);

        hBtnAnalizar = CreateWindowA("BUTTON", "Analizar",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0,0,0,0, hWnd, (HMENU)IDC_BTN_ANALIZAR, NULL, NULL);
        SendMessage(hBtnAnalizar, WM_SETFONT, (WPARAM)hFont, TRUE);

        hLabelFile = CreateWindowA("STATIC", "(sin archivo)",
            WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
            0,0,0,0, hWnd, (HMENU)IDC_LABEL_FILE, NULL, NULL);
        SendMessage(hLabelFile, WM_SETFONT, (WPARAM)hFont, TRUE);

        hTab = CreateWindowA(WC_TABCONTROLA, "",
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_FIXEDWIDTH,
            0,0,0,0, hWnd, (HMENU)IDC_TAB, NULL, NULL);
        SendMessage(hTab, WM_SETFONT, (WPARAM)hFont, TRUE);

        TCITEMA ti = {};
        ti.mask = TCIF_TEXT;
        ti.pszText = const_cast<char*>("Lista de Tokens");
        TabCtrl_InsertItem(hTab, 0, &ti);
        ti.pszText = const_cast<char*>("Tabla de Simbolos");
        TabCtrl_InsertItem(hTab, 1, &ti);
        ti.pszText = const_cast<char*>("Errores Lexicos");
        TabCtrl_InsertItem(hTab, 2, &ti);

        DWORD estiloEdit = WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
                           ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL;

        HFONT hMono = CreateFontA(
            16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

        hEditTokens = CreateWindowA("EDIT", "",
            estiloEdit, 0,0,0,0, hWnd, (HMENU)IDC_EDIT_TOKENS, NULL, NULL);
        SendMessage(hEditTokens, WM_SETFONT, (WPARAM)hMono, TRUE);

        hEditTabla = CreateWindowA("EDIT", "",
            estiloEdit, 0,0,0,0, hWnd, (HMENU)IDC_EDIT_TABLA, NULL, NULL);
        SendMessage(hEditTabla, WM_SETFONT, (WPARAM)hMono, TRUE);
        ShowWindow(hEditTabla, SW_HIDE);

        hEditErrores = CreateWindowA("EDIT", "",
            estiloEdit, 0,0,0,0, hWnd, (HMENU)IDC_EDIT_ERRORES, NULL, NULL);
        SendMessage(hEditErrores, WM_SETFONT, (WPARAM)hMono, TRUE);
        ShowWindow(hEditErrores, SW_HIDE);

        setEditText(hEditTokens,
            "=== LISTA DE TOKENS ===\n"
            "Abre un archivo y presiona [Analizar].\n\n"
            "Formatos aceptados: .lp  .txt  .docx  .pdf");
        setEditText(hEditTabla,
            "=== TABLA DE SIMBOLOS ===\n"
            "Abre un archivo y presiona [Analizar].");
        setEditText(hEditErrores,
            "=== ERRORES LEXICOS ===\n"
            "Abre un archivo y presiona [Analizar].");
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
                rutaArchivo = szFile;
                string nombre = rutaArchivo;
                size_t sep = nombre.find_last_of("\\/");
                if (sep != string::npos) nombre = nombre.substr(sep + 1);
                SetWindowTextA(hLabelFile, nombre.c_str());
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

    WNDCLASSEXA wc   = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "LexLP_Clase";
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    RegisterClassExA(&wc);

    HWND hWnd = CreateWindowExA(0, "LexLP_Clase",
        "Analizador Lexico LP  -  Semana 1: NUM_INT y NUM_DEC",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 860, 600,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg = {};
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return static_cast<int>(msg.wParam);
}
