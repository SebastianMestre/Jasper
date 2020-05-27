#pragma once

#include <vector>
#include <unordered_set>

#include "typed_ast.hpp"

namespace TypeChecker {

class GraphNode {
public:
    bool m_visited {false};
    TypedAST* m_value;
    std::vector<GraphNode*> m_next;
    int m_id;

    GraphNode(TypedAST* value) : m_value{value} {}
};

class GraphComponent {
public:
    std::vector<TypedAST*> m_body;
    std::unordered_set<GraphComponent*> m_next;
};

class TC {
public:
    GraphComponent m_root;
    std::vector<GraphNode*> m_nodes;
    GraphComponent* createDag(TypedAST*);
    void visit(GraphNode*);
};

GraphNode* create_graph(TypedAST*);

}