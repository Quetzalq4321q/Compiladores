// Expone un control WinAPI que dibuja y desplaza el grafo del AST.
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ASTGraph.h"

namespace ASTGraphView {
HWND create(HWND parent, int controlId);
void setGraph(HWND view, const ASTGraph& graph);
void fit(HWND view);
}
