#pragma once

#include <vector>

#include "typed_ast.hpp"

namespace TypeChecker {

class GraphNode {
public:
    bool m_visited {false};
    TypedAST* m_value;
    std::vector<GraphNode*> m_next;

    GraphNode(TypedAST* value) : m_value{value} {}
};

class GraphComponent {
public:
    std::vector<TypedAST*> m_body;
};

class TC {
public:
    GraphComponent m_root;
    void createDag(TypedAST*);
};

GraphNode* create_graph(TypedAST*);

}