#include "type_checker.hpp"
#include "typed_ast.hpp"
#include "typed_ast_type.hpp"

namespace TypeChecker {

void TC::createDag(TypedAST* root) {
    auto graph_root = create_graph(root);

    // dfs to list the nodes

    // traspose graph
    /* for (auto& node : m_node_list) {
        
    } */

    // dfs on trasposed graph
    /* for (auto& node : m_node_list) {
        // visit(node);
    } */
}

GraphNode* create_graph(TypedASTDeclaration* ast) {
    auto node_representation = new GraphNode {ast};
    auto value = create_graph(ast->m_value.get());
    node_representation->m_next.push_back(value);
    return node_representation;
}

GraphNode* create_graph(TypedASTDeclarationList* ast) {
    auto node = new GraphNode {ast};
    for (auto& decl : ast->m_declarations) {
        auto decl_node = create_graph(decl.get());
        node->m_next.push_back(decl_node);
    }
    return node;
}

GraphNode* create_graph(TypedASTBinaryExpression* ast) {
    auto node = new GraphNode {ast};

    auto lhs = create_graph(ast->m_lhs.get());
    auto rhs = create_graph(ast->m_rhs.get());

    node->m_next.push_back(lhs);
    node->m_next.push_back(rhs);

    return node;
}

GraphNode* create_graph(TypedASTFunctionLiteral* ast) {
    auto node = new GraphNode {ast};

    auto body = create_graph(ast->m_body.get());
    node->m_next.push_back(body);

    for (auto& arg : ast->m_args) {
        auto arg_node = create_graph(arg.get());
        node->m_next.push_back(arg_node);
    }

    return node;
}

GraphNode* create_graph(TypedASTCallExpression* ast) {
    auto node = new GraphNode {ast};

    auto callee_node = create_graph(ast->m_callee.get());
    node->m_next.push_back(callee_node);

    for (auto& arg : ast->m_args) {
        auto arg_node = create_graph(arg.get());
        node->m_next.push_back(arg_node);
    }

    return node;
}

GraphNode* create_graph(TypedASTBlock* ast) {
    auto node = new GraphNode {ast};

    for (auto& stmt : ast->m_body) {
        auto stmt_node = create_graph(stmt.get());
        node->m_next.push_back(stmt_node);
    }

    return node;
}

GraphNode* create_graph(TypedASTReturnStatement* ast) {
    auto node = new GraphNode {ast};

    auto value_node = create_graph(ast->m_value.get());
    node->m_next.push_back(value_node);

    return node;
}

GraphNode* create_graph(TypedAST* ast) {
    switch(ast->type()) {
    case ast_type::Declaration:
        return create_graph(static_cast<TypedASTDeclaration*>(ast));
	case ast_type::FunctionLiteral:
        return create_graph(static_cast<TypedASTFunctionLiteral*>(ast));
	case ast_type::DeclarationList:
        return create_graph(static_cast<TypedASTDeclarationList*>(ast));
	case ast_type::BinaryExpression:
        return create_graph(static_cast<TypedASTBinaryExpression*>(ast));
	case ast_type::CallExpression:
        return create_graph(static_cast<TypedASTCallExpression*>(ast));
	case ast_type::IndexExpression:
        return create_graph(static_cast<TypedASTIndexExpression*>(ast));
	case ast_type::Block:
        return create_graph(static_cast<TypedASTBlock*>(ast));
	case ast_type::ReturnStatement:
        return create_graph(static_cast<TypedASTReturnStatement*>(ast));
	case ast_type::Identifier:
	case ast_type::IfStatement:
	case ast_type::ForStatement:
    case ast_type::NumberLiteral:
	case ast_type::StringLiteral:
	case ast_type::BooleanLiteral:
	case ast_type::NullLiteral:
	case ast_type::ObjectLiteral:
	case ast_type::ArrayLiteral:
	case ast_type::DictionaryLiteral:
        break;
    }
}

} 