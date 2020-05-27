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

class Dag {
protected:
    std::vector<GraphNode*> m_nodes;

    void transpose();
    void visit(GraphNode*);

public:
    GraphComponent* create(TypedAST*);
    void test(GraphComponent*);
};

GraphNode* create_graph(TypedAST*);

}