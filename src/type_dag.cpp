#include <cassert>

#include "type_dag.hpp"
#include "typed_ast.hpp"
#include "typed_ast_type.hpp"

#include <iostream>
template<typename T>
void print(T value) {
	// print enum
	std::cout << static_cast<typename std::underlying_type<T>::type>(value) << std::endl;
} 

namespace TypeChecker {

void Dag::test(GraphComponent* dag) {
    for (auto& next : dag->m_next) {
        test(next.get());
    }
}

GraphComponent* Dag::create(TypedAST* root) {
    auto graph_root = create_graph(root);

    visit(graph_root);

    std::vector<GraphNode*> nodes_copy = m_nodes;

    transpose();

    std::vector<GraphNode*> transposed = m_nodes;
    m_nodes.clear();

    for (auto node : transposed) 
        node->m_visited = false;

    // find components
    std::vector<int> node_component(m_nodes.size(), -1);
    std::vector<GraphComponent*> components(m_nodes.size());
    int component_id = 0;

    for (int i = (int)transposed.size()-1; i > -1; i--) {
        if (transposed[i]->m_visited) {
            assert(node_component[i] >= 0);
            continue;
        }

        visit(transposed[i]);

        while(m_nodes.size()) {
            auto node = m_nodes.back();
            m_nodes.pop_back();

            node_component[node->m_id] = component_id;
            components[component_id]->m_body.push_back(node->m_value);
        }

        assert(node_component[i] >= 0);
        component_id++;
    }

    components.resize(component_id);

    // link components
    for (auto node : nodes_copy) {
        auto component = components[node_component[node->m_id]];

        for (auto next : node->m_next) {
            auto next_component = components[node_component[next->m_id]];

            if (node_component[node->m_id] != node_component[next->m_id]) {
                component->m_next.insert(
                    std::unique_ptr<GraphComponent>(next_component));
            }
        }
    }

    auto root_component = components[node_component[graph_root->m_id]];

    // need to delete nodes in nodes_copy and transposed
    for (int i = 0; i < (int)nodes_copy.size(); i++) {
        delete nodes_copy[i];
        delete transposed[i];
    }

    return root_component;
}

void Dag::transpose() {
    auto  normal = m_nodes;
    auto& transposed = m_nodes;

    transposed.clear();

    for (auto node : normal) {
        auto trans_node = new GraphNode {node->m_value};
        transposed.push_back(trans_node);
    }

    for (auto node : normal) {
        for (auto next : node->m_next) {
            auto trans_node = transposed[next->m_id];

            trans_node->m_next.push_back(node);
        }
    }
}

void Dag::visit(GraphNode* node) {
    node->m_visited = true;

    for (auto& next : node->m_next) {
        if (!next->m_visited) {
            visit(next);
        }
    }

    node->m_id = m_nodes.size();
    m_nodes.push_back(node);
}

GraphNode* Dag::create_graph(TypedASTDeclaration* ast) {
    auto node = new GraphNode {ast};
    m_declarations[ast] = node;

    auto value_node = create_graph(ast->m_value.get());
    node->m_next.push_back(value_node);
    
    return node;
}

GraphNode* Dag::create_graph(TypedASTDeclarationList* ast) {
    auto node = new GraphNode {ast};

    for (auto& decl : ast->m_declarations) {
        auto decl_node = create_graph(decl.get());
        node->m_next.push_back(decl_node);
    }

    return node;
}

GraphNode* Dag::create_graph(TypedASTBinaryExpression* ast) {
    auto node = new GraphNode {ast};

    auto lhs = create_graph(ast->m_lhs.get());
    auto rhs = create_graph(ast->m_rhs.get());

    node->m_next.push_back(lhs);
    node->m_next.push_back(rhs);

    return node;
}

GraphNode* Dag::create_graph(TypedASTFunctionLiteral* ast) {
    auto node = new GraphNode {ast};

    auto body = create_graph(ast->m_body.get());
    node->m_next.push_back(body);

    for (auto& arg : ast->m_args) {
        auto arg_node = create_graph(arg.get());
        node->m_next.push_back(arg_node);
    }

    return node;
}

GraphNode* Dag::create_graph(TypedASTCallExpression* ast) {
    auto node = new GraphNode {ast};

    auto callee_node = create_graph(ast->m_callee.get());
    node->m_next.push_back(callee_node);

    for (auto& arg : ast->m_args) {
        auto arg_node = create_graph(arg.get());
        node->m_next.push_back(arg_node);
    }

    return node;
}

GraphNode* Dag::create_graph(TypedASTBlock* ast) {
    auto node = new GraphNode {ast};

    for (auto& stmt : ast->m_body) {
        auto stmt_node = create_graph(stmt.get());
        node->m_next.push_back(stmt_node);
    }

    return node;
}

GraphNode* Dag::create_graph(TypedASTReturnStatement* ast) {
    auto node = new GraphNode {ast};

    auto value_node = create_graph(ast->m_value.get());
    node->m_next.push_back(value_node);

    return node;
}

GraphNode* Dag::create_graph(TypedASTIdentifier* ast) {
    auto node = new GraphNode {ast};

    assert(ast->m_declaration);

    auto decl_node = m_declarations[ast->m_declaration];
    node->m_next.push_back(decl_node);

    return node;
}

GraphNode* Dag::create_graph(TypedASTNumberLiteral* ast) {return new GraphNode {ast};}
GraphNode* Dag::create_graph(TypedASTBooleanLiteral* ast) {return new GraphNode {ast};}
GraphNode* Dag::create_graph(TypedASTStringLiteral* ast) {return new GraphNode {ast};}
GraphNode* Dag::create_graph(TypedASTNullLiteral* ast) {return new GraphNode {ast};}

GraphNode* Dag::create_graph(TypedAST* ast) {    
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
        return create_graph(static_cast<TypedASTIdentifier*>(ast));
    case ast_type::NumberLiteral:
        return create_graph(static_cast<TypedASTNumberLiteral*>(ast));
	case ast_type::StringLiteral:
        return create_graph(static_cast<TypedASTStringLiteral*>(ast));
	case ast_type::BooleanLiteral:
        return create_graph(static_cast<TypedASTBooleanLiteral*>(ast));
	case ast_type::NullLiteral:
        return create_graph(static_cast<TypedASTNullLiteral*>(ast));
	case ast_type::IfStatement:
	case ast_type::ForStatement:
	case ast_type::ObjectLiteral:
	case ast_type::ArrayLiteral:
	case ast_type::DictionaryLiteral:
        assert(0);
    }
}

} 