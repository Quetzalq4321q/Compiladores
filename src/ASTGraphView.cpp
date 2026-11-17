// Dibuja nodos y conexiones del AST con GDI, desplazamiento y ampliacion.
#include "ASTGraphView.h"

#include <algorithm>
#include <cmath>
#include <string>

using namespace std;

namespace {
constexpr wchar_t className[] = L"LP_AST_GRAPH_VIEW";

struct ViewState {
    ASTGraph graph;
    int scrollX = 0;
    int scrollY = 0;
    double zoom = 1.0;
};

ViewState* state(HWND view) {
    return reinterpret_cast<ViewState*>(GetWindowLongPtrW(view, GWLP_USERDATA));
}

wstring unicode(const string& source) {
    if (source.empty()) return L"";
    UINT page = CP_UTF8;
    int len = MultiByteToWideChar(page, MB_ERR_INVALID_CHARS, source.data(), static_cast<int>(source.size()), nullptr, 0);
    if (len == 0) {
        page = CP_ACP;
        len = MultiByteToWideChar(page, 0, source.data(), static_cast<int>(source.size()), nullptr, 0);
    }
    if (len == 0) return L"";
    wstring result(len, L'\0');
    MultiByteToWideChar(page, 0, source.data(), static_cast<int>(source.size()), result.data(), len);
    return result;
}

COLORREF fill(GraphKind kind) {
    switch (kind) {
        case GraphKind::Program: return RGB(220, 233, 255);
        case GraphKind::Function: return RGB(217, 241, 232);
        case GraphKind::Block: return RGB(232, 233, 242);
        case GraphKind::Statement: return RGB(255, 240, 213);
        case GraphKind::Expression: return RGB(240, 229, 255);
        case GraphKind::Value: return RGB(232, 244, 252);
        case GraphKind::Group: return RGB(242, 244, 247);
    }
    return RGB(255, 255, 255);
}

int scaled(double zoom, int value) {
    return static_cast<int>(lround(zoom * value));
}

void updateScroll(HWND view, ViewState& s) {
    RECT client;
    GetClientRect(view, &client);
    int contentW = scaled(s.zoom, s.graph.width());
    int contentH = scaled(s.zoom, s.graph.height());
    int pageW = max(1L, client.right - client.left);
    int pageH = max(1L, client.bottom - client.top);
    s.scrollX = clamp(s.scrollX, 0, max(0, contentW - pageW));
    s.scrollY = clamp(s.scrollY, 0, max(0, contentH - pageH));
    SCROLLINFO info = {};
    info.cbSize = sizeof(info);
    info.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    info.nMin = 0;
    info.nMax = max(0, contentW - 1);
    info.nPage = pageW;
    info.nPos = s.scrollX;
    SetScrollInfo(view, SB_HORZ, &info, TRUE);
    info.nMax = max(0, contentH - 1);
    info.nPage = pageH;
    info.nPos = s.scrollY;
    SetScrollInfo(view, SB_VERT, &info, TRUE);
}

void scroll(HWND view, ViewState& s, int bar, WORD action) {
    SCROLLINFO info = {};
    info.cbSize = sizeof(info);
    info.fMask = SIF_TRACKPOS | SIF_PAGE;
    GetScrollInfo(view, bar, &info);
    int& position = bar == SB_HORZ ? s.scrollX : s.scrollY;
    switch (action) {
        case SB_LINELEFT: position -= 48; break;
        case SB_LINERIGHT: position += 48; break;
        case SB_PAGELEFT: position -= static_cast<int>(info.nPage); break;
        case SB_PAGERIGHT: position += static_cast<int>(info.nPage); break;
        case SB_THUMBTRACK: position = info.nTrackPos; break;
        default: return;
    }
    updateScroll(view, s);
    InvalidateRect(view, nullptr, FALSE);
}

void paintGraph(HDC dc, const RECT& client, const ViewState& s) {
    FillRect(dc, &client, reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
    if (s.graph.vertices().empty()) {
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(60, 72, 88));
        RECT textArea = client;
        DrawTextW(dc, L"Analice un programa para mostrar el AST grafico.", -1,
                  &textArea, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        return;
    }

    const auto& vertices = s.graph.vertices();
    HPEN edge = CreatePen(PS_SOLID, max(1, scaled(s.zoom, 2)), RGB(104, 117, 138));
    HGDIOBJ oldPen = SelectObject(dc, edge);
    for (const auto& node : vertices) {
        int x1 = scaled(s.zoom, node.x + ASTGraph::nodeWidth / 2) - s.scrollX;
        int y1 = scaled(s.zoom, node.y + ASTGraph::nodeHeight / 2) - s.scrollY;
        for (size_t child : node.children) {
            const auto& target = vertices[child];
            int x2 = scaled(s.zoom, target.x - ASTGraph::nodeWidth / 2) - s.scrollX;
            int y2 = scaled(s.zoom, target.y + ASTGraph::nodeHeight / 2) - s.scrollY;
            int middle = (x1 + x2) / 2;
            MoveToEx(dc, x1, y1, nullptr);
            LineTo(dc, middle, y1);
            LineTo(dc, middle, y2);
            LineTo(dc, x2, y2);
        }
    }
    SelectObject(dc, oldPen);
    DeleteObject(edge);

    HPEN border = CreatePen(PS_SOLID, 1, RGB(84, 101, 121));
    oldPen = SelectObject(dc, border);
    HFONT labelFont = CreateFontW(max(10, scaled(s.zoom, 14)), 0, 0, 0, FW_SEMIBOLD,
        FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HFONT lineFont = CreateFontW(max(9, scaled(s.zoom, 11)), 0, 0, 0, FW_NORMAL,
        FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    SetBkMode(dc, TRANSPARENT);
    for (const auto& node : vertices) {
        int left = scaled(s.zoom, node.x - ASTGraph::nodeWidth / 2) - s.scrollX;
        int top = scaled(s.zoom, node.y) - s.scrollY;
        int width = scaled(s.zoom, ASTGraph::nodeWidth);
        int height = scaled(s.zoom, ASTGraph::nodeHeight);
        if (left + width < client.left || left > client.right || top + height < client.top || top > client.bottom) continue;
        HBRUSH brush = CreateSolidBrush(fill(node.kind));
        HGDIOBJ oldBrush = SelectObject(dc, brush);
        RoundRect(dc, left, top, left + width, top + height, 12, 12);
        SelectObject(dc, oldBrush);
        DeleteObject(brush);
        RECT label = {left + 5, top + 4, left + width - 5, top + scaled(s.zoom, 34)};
        HGDIOBJ oldFont = SelectObject(dc, labelFont);
        SetTextColor(dc, RGB(20, 35, 52));
        wstring name = unicode(node.label);
        DrawTextW(dc, name.c_str(), static_cast<int>(name.size()), &label,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        SelectObject(dc, lineFont);
        SetTextColor(dc, RGB(83, 98, 115));
        wstring line = L"Linea " + to_wstring(node.line);
        RECT lineArea = {left + 4, top + scaled(s.zoom, 32), left + width - 4, top + height - 2};
        DrawTextW(dc, line.c_str(), static_cast<int>(line.size()), &lineArea,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(dc, oldFont);
    }
    SelectObject(dc, oldPen);
    DeleteObject(border);
    DeleteObject(labelFont);
    DeleteObject(lineFont);
}

LRESULT CALLBACK viewProc(HWND view, UINT message, WPARAM wParam, LPARAM lParam) {
    ViewState* s = state(view);
    switch (message) {
        case WM_NCCREATE:
            SetWindowLongPtrW(view, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(new ViewState));
            return TRUE;
        case WM_NCDESTROY:
            delete s;
            SetWindowLongPtrW(view, GWLP_USERDATA, 0);
            return 0;
        case WM_SIZE:
            if (s) updateScroll(view, *s);
            InvalidateRect(view, nullptr, FALSE);
            return 0;
        case WM_HSCROLL:
            if (s) scroll(view, *s, SB_HORZ, LOWORD(wParam));
            return 0;
        case WM_VSCROLL:
            if (s) scroll(view, *s, SB_VERT, LOWORD(wParam));
            return 0;
        case WM_MOUSEWHEEL:
            if (s) {
                if (GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) {
                    s->zoom = clamp(s->zoom + (GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? 0.1 : -0.1), 0.5, 2.0);
                    updateScroll(view, *s);
                    InvalidateRect(view, nullptr, FALSE);
                } else {
                    s->scrollY -= GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA * 60;
                    updateScroll(view, *s);
                    InvalidateRect(view, nullptr, FALSE);
                }
            }
            return 0;
        case WM_ERASEBKGND: return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(view, &ps);
            RECT client;
            GetClientRect(view, &client);
            int w = max(1L, client.right - client.left), h = max(1L, client.bottom - client.top);
            HDC memory = CreateCompatibleDC(dc);
            HBITMAP bitmap = CreateCompatibleBitmap(dc, w, h);
            HGDIOBJ old = SelectObject(memory, bitmap);
            if (s) paintGraph(memory, client, *s);
            BitBlt(dc, 0, 0, w, h, memory, 0, 0, SRCCOPY);
            SelectObject(memory, old);
            DeleteObject(bitmap);
            DeleteDC(memory);
            EndPaint(view, &ps);
            return 0;
        }
    }
    return DefWindowProcW(view, message, wParam, lParam);
}
}

HWND ASTGraphView::create(HWND parent, int controlId) {
    HINSTANCE instance = reinterpret_cast<HINSTANCE>(GetModuleHandleW(nullptr));
    WNDCLASSEXW cls = {};
    cls.cbSize = sizeof(cls);
    cls.lpfnWndProc = viewProc;
    cls.hInstance = instance;
    cls.lpszClassName = className;
    cls.hCursor = LoadCursorA(nullptr, IDC_ARROW);
    cls.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    RegisterClassExW(&cls);
    return CreateWindowExW(0, className, L"", WS_CHILD | WS_HSCROLL | WS_VSCROLL |
        WS_CLIPSIBLINGS, 0, 0, 0, 0, parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(controlId)),
        instance, nullptr);
}

void ASTGraphView::setGraph(HWND view, const ASTGraph& graph) {
    if (!view) return;
    ViewState* s = state(view);
    if (!s) return;
    s->graph = graph;
    s->scrollX = 0;
    s->scrollY = 0;
    s->zoom = 1.0;
    updateScroll(view, *s);
    InvalidateRect(view, nullptr, FALSE);
}
