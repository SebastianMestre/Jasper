#include "type_checker.hpp"
#include "typed_ast.hpp"
#include "typed_ast_type.hpp"

namespace TypeChecker {

void TC::createDag(TypedAST* root) {
    createGraph(root);

    // traverse graph
    for (auto& node : m_graph) {
        
    }

    // dfs on traversed graph
    for (auto& node : m_graph) {
        // visit(node);
    }
}

GraphNode* createGraph(TypedASTDeclaration* ast) {
    auto node_representation = new GraphNode {ast};
    auto value = createGraph(ast->m_value.get());
    node_representation->m_next.push_back(value);
    return node_representation;
}

GraphNode* createGraph(TypedASTDeclarationList* ast) {
    auto node = new GraphNode {ast};
    for (auto& decl : ast->m_declarations) {
        auto decl_node = createGraph(decl.get());
        node->m_next.push_back(decl_node);
    }
    return node;
}

GraphNode* createGraph(TypedASTBinaryExpression* ast) {
    auto node = new GraphNode {ast};

    auto lhs = createGraph(ast->m_lhs.get());
    auto rhs = createGraph(ast->m_rhs.get());

    node->m_next.push_back(lhs);
    node->m_next.push_back(rhs);

    return node;
}

GraphNode* createGraph(TypedASTFunctionLiteral* ast) {
    auto node = new GraphNode {ast};

    auto body = createGraph(ast->m_body.get());
    node->m_next.push_back(body);

    for (auto& arg : ast->m_args) {
        auto arg_node = createGraph(arg.get());
        node->m_next.push_back(arg_node);
    }

    return node;
}

GraphNode* createGraph(TypedAST* node) {
    switch(node->type()) {
    case ast_type::Declaration:
        return createGraph(static_cast<TypedASTDeclaration*>(node));
    }
}

} 