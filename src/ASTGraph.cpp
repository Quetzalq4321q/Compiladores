// Construye un grafo real del AST y escribe una imagen vectorial independiente.
#include "ASTGraph.h"

#include <algorithm>
#include <sstream>

using namespace std;

namespace {
constexpr size_t noParent = static_cast<size_t>(-1);
constexpr int leafStep = 196;
constexpr int levelStep = 105;
constexpr int margin = 36;

string xml(const string& value) {
    string result;
    for (unsigned char c : value) {
        switch (c) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&apos;"; break;
            default: if (c >= 32 || c == '\t') result += static_cast<char>(c);
        }
    }
    return result;
}

string shortLabel(const string& value) {
    if (value.size() <= 29) return value;
    size_t end = 26;
    while (end > 0 && (static_cast<unsigned char>(value[end]) & 0xc0) == 0x80) --end;
    return value.substr(0, end) + "...";
}

const char* fillFor(GraphKind kind) {
    switch (kind) {
        case GraphKind::Program: return "#dce9ff";
        case GraphKind::Function: return "#d9f1e8";
        case GraphKind::Block: return "#e8e9f2";
        case GraphKind::Statement: return "#fff0d5";
        case GraphKind::Expression: return "#f0e5ff";
        case GraphKind::Value: return "#e8f4fc";
        case GraphKind::Group: return "#f2f4f7";
    }
    return "#ffffff";
}
}

size_t ASTGraph::add(const string& label, GraphKind kind, int line, size_t parent) {
    size_t index = nodes.size();
    nodes.push_back({label, kind, line});
    if (parent != noParent) nodes[parent].children.push_back(index);
    return index;
}

void ASTGraph::append(const shared_ptr<ASTNode>& node, size_t parent) {
    if (!node) return;
    if (auto program = dynamic_pointer_cast<ProgramNode>(node)) {
        size_t p = add("Programa LP", GraphKind::Program, program->linea, parent);
        for (const auto& fn : program->funciones) append(fn, p);
        append(program->funcionPrincipal, p);
    } else if (auto fn = dynamic_pointer_cast<FunctionDeclNode>(node)) {
        size_t p = add("Funcion " + fn->tipoRetorno + " " + fn->nombre, GraphKind::Function, fn->linea, parent);
        for (const auto& arg : fn->parametros) add("Parametro " + arg.tipo + " " + arg.nombre, GraphKind::Value, fn->linea, p);
        append(fn->bloque, p);
    } else if (auto block = dynamic_pointer_cast<BlockNode>(node)) {
        size_t p = add("Bloque", GraphKind::Block, block->linea, parent);
        for (const auto& var : block->variables) append(var, p);
        for (const auto& cmd : block->comandos) append(cmd, p);
    } else if (auto var = dynamic_pointer_cast<VarDeclNode>(node)) {
        size_t p = add("Declaracion " + var->tipo, GraphKind::Statement, var->linea, parent);
        for (const auto& name : var->nombres) add(name, GraphKind::Value, var->linea, p);
    } else if (auto assign = dynamic_pointer_cast<AssignNode>(node)) {
        size_t p = add("Asignacion " + assign->id, GraphKind::Statement, assign->linea, parent);
        for (size_t i = 0; i < assign->indices.size(); ++i) {
            size_t group = add("Indice destino " + to_string(i + 1), GraphKind::Group, assign->linea, p);
            append(assign->indices[i], group);
        }
        append(assign->expresion, p);
    } else if (auto input = dynamic_pointer_cast<ScanfNode>(node)) {
        size_t p = add("scanf", GraphKind::Statement, input->linea, parent);
        for (const auto& term : input->terminos) {
            size_t target = add("Leer " + term.id, GraphKind::Value, input->linea, p);
            for (size_t i = 0; i < term.dimensiones.size(); ++i) {
                size_t group = add("Indice " + to_string(i + 1), GraphKind::Group, input->linea, target);
                append(term.dimensiones[i], group);
            }
        }
    } else if (auto output = dynamic_pointer_cast<PrintlnNode>(node)) {
        size_t p = add("println", GraphKind::Statement, output->linea, parent);
        for (const auto& term : output->terminos) append(term, p);
    } else if (auto select = dynamic_pointer_cast<IfNode>(node)) {
        size_t p = add("if", GraphKind::Statement, select->linea, parent);
        size_t cond = add("Condicion", GraphKind::Group, select->linea, p);
        append(select->condicion, cond);
        size_t thenNode = add("Entonces", GraphKind::Group, select->linea, p);
        append(select->bloqueSi, thenNode);
        if (select->bloqueSino) {
            size_t elseNode = add("Sino", GraphKind::Group, select->linea, p);
            append(select->bloqueSino, elseNode);
        }
    } else if (auto loop = dynamic_pointer_cast<WhileNode>(node)) {
        size_t p = add("while", GraphKind::Statement, loop->linea, parent);
        size_t cond = add("Condicion", GraphKind::Group, loop->linea, p);
        append(loop->condicion, cond);
        size_t body = add("Cuerpo", GraphKind::Group, loop->linea, p);
        append(loop->bloque, body);
    } else if (auto ret = dynamic_pointer_cast<ReturnNode>(node)) {
        size_t p = add("return", GraphKind::Statement, ret->linea, parent);
        append(ret->expresion, p);
    } else if (auto binary = dynamic_pointer_cast<BinaryOpNode>(node)) {
        size_t p = add("Operador " + binary->op, GraphKind::Expression, binary->linea, parent);
        append(binary->izq, p);
        append(binary->der, p);
    } else if (auto unary = dynamic_pointer_cast<UnaryOpNode>(node)) {
        size_t p = add("Unario " + unary->op, GraphKind::Expression, unary->linea, parent);
        append(unary->operando, p);
    } else if (auto call = dynamic_pointer_cast<FuncCallNode>(node)) {
        size_t p = add("Llamada " + call->nombreFuncion, GraphKind::Expression, call->linea, parent);
        for (size_t i = 0; i < call->argumentos.size(); ++i) {
            size_t arg = add("Argumento " + to_string(i + 1), GraphKind::Group, call->linea, p);
            append(call->argumentos[i], arg);
        }
    } else if (auto id = dynamic_pointer_cast<IdentifierNode>(node)) {
        size_t p = add("Identificador " + id->nombre, GraphKind::Value, id->linea, parent);
        for (size_t i = 0; i < id->indices.size(); ++i) {
            size_t group = add("Indice " + to_string(i + 1), GraphKind::Group, id->linea, p);
            append(id->indices[i], group);
        }
    } else if (auto literal = dynamic_pointer_cast<LiteralNode>(node)) {
        add("Literal " + literal->valor, GraphKind::Value, literal->linea, parent);
    }
}

int ASTGraph::position(size_t index, int depth, int& leafIndex, int& maxDepth) {
    maxDepth = max(maxDepth, depth);
    const auto& children = nodes[index].children;
    nodes[index].y = margin + depth * levelStep;
    if (children.empty()) {
        nodes[index].x = margin + nodeWidth / 2 + leafIndex * leafStep;
        ++leafIndex;
    } else {
        for (size_t child : children) position(child, depth + 1, leafIndex, maxDepth);
        nodes[index].x = (nodes[children.front()].x + nodes[children.back()].x) / 2;
    }
    return nodes[index].x;
}

ASTGraph ASTGraph::fromProgram(const shared_ptr<ProgramNode>& program) {
    ASTGraph graph;
    if (!program) return graph;
    graph.append(program, noParent);
    if (graph.nodes.empty()) return graph;
    int leaves = 0, depth = 0;
    graph.position(0, 0, leaves, depth);
    graph.canvasWidth = margin * 2 + nodeWidth + max(0, leaves - 1) * leafStep;
    graph.canvasHeight = margin * 2 + nodeHeight + depth * levelStep;
    return graph;
}

string ASTGraph::toSVG() const {
    ostringstream out;
    int w = max(canvasWidth, 320), h = max(canvasHeight, 120);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << w
        << "\" height=\"" << h << "\" viewBox=\"0 0 " << w << ' ' << h << "\" role=\"img\">\n";
    out << "<title>Arbol sintactico abstracto de LP</title>\n";
    out << "<rect width=\"100%\" height=\"100%\" fill=\"#ffffff\"/>\n";
    for (const auto& node : nodes) {
        for (size_t child : node.children) {
            int x1 = node.x;
            int y1 = node.y + nodeHeight;
            int x2 = nodes[child].x;
            int y2 = nodes[child].y;
            int middle = (y1 + y2) / 2;
            out << "<path d=\"M " << x1 << ' ' << y1 << " V " << middle << " H " << x2
                << " V " << y2 << "\" fill=\"none\" stroke=\"#68758a\" stroke-width=\"2\"/>\n";
        }
    }
    for (const auto& node : nodes) {
        int left = node.x - nodeWidth / 2;
        out << "<g><title>" << xml(node.label) << " (linea " << node.line << ")</title>";
        out << "<rect x=\"" << left << "\" y=\"" << node.y << "\" width=\"" << nodeWidth
            << "\" height=\"" << nodeHeight << "\" rx=\"10\" fill=\"" << fillFor(node.kind)
            << "\" stroke=\"#546579\" stroke-width=\"1.5\"/>";
        out << "<text x=\"" << node.x << "\" y=\"" << node.y + 25
            << "\" text-anchor=\"middle\" fill=\"#142334\" font-family=\"Segoe UI,Arial,sans-serif\" font-size=\"14\">"
            << xml(shortLabel(node.label)) << "</text>";
        out << "<text x=\"" << node.x << "\" y=\"" << node.y + 43
            << "\" text-anchor=\"middle\" fill=\"#536273\" font-family=\"Segoe UI,Arial,sans-serif\" font-size=\"11\">L"
            << node.line << "</text></g>\n";
    }
    out << "</svg>\n";
    return out.str();
}
