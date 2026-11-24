// Convierte el AST en un grafo con posiciones y lo exporta como SVG.
#pragma once

#include "AST.h"
#include <cstddef>
#include <string>
#include <vector>

enum class GraphKind { Program, Function, Block, Statement, Expression, Value, Group };

struct GraphVertex {
    std::string label;
    GraphKind kind;
    int line;
    int x = 0;
    int y = 0;
    std::vector<std::size_t> children;
};

class ASTGraph {
public:
    static constexpr int nodeWidth = 170;
    static constexpr int nodeHeight = 54;

    static ASTGraph fromProgram(const std::shared_ptr<ProgramNode>& program);
    std::string toSVG() const;

    const std::vector<GraphVertex>& vertices() const { return nodes; }
    int width() const { return canvasWidth; }
    int height() const { return canvasHeight; }

private:
    std::vector<GraphVertex> nodes;
    int canvasWidth = 0;
    int canvasHeight = 0;

    std::size_t add(const std::string& label, GraphKind kind, int line, std::size_t parent);
    void append(const std::shared_ptr<ASTNode>& node, std::size_t parent);
    int position(std::size_t index, int depth, int& leafIndex, int& maxDepth);
};
