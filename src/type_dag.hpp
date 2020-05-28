#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>

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
    std::unordered_set<std::unique_ptr<GraphComponent>> m_next;
};

class Dag {
protected:
    std::vector<GraphNode*> m_nodes;
    std::unordered_map<TypedAST*, GraphNode*> m_declarations;

    void transpose();
    void visit(GraphNode*);

public:
    GraphComponent* create(TypedAST*);
    void test(GraphComponent*);

    GraphNode* create_graph(TypedAST*);
    GraphNode* create_graph(TypedASTDeclaration* ast);
    GraphNode* create_graph(TypedASTDeclarationList* ast);
    GraphNode* create_graph(TypedASTBinaryExpression* ast);
    GraphNode* create_graph(TypedASTFunctionLiteral* ast);
    GraphNode* create_graph(TypedASTCallExpression* ast);
    GraphNode* create_graph(TypedASTBlock* ast);
    GraphNode* create_graph(TypedASTReturnStatement* ast);
    GraphNode* create_graph(TypedASTIdentifier* ast);
    GraphNode* create_graph(TypedASTNumberLiteral* ast);
    GraphNode* create_graph(TypedASTBooleanLiteral* ast);
    GraphNode* create_graph(TypedASTStringLiteral* ast);
    GraphNode* create_graph(TypedASTNullLiteral* ast);
};

}