#include <cassert>
#include <iostream>

#include "ast.hpp"
#include "typed_ast.hpp"
#include "typed_ast_type.hpp"

namespace TypedAST {

std::unique_ptr<TypedAST> get_unique(std::unique_ptr<AST::AST>& ast) {
    return std::unique_ptr<TypedAST>(convertAST(ast.get()));
}

TypedAST* convertAST(AST::NumberLiteral* ast) {
    auto typed_number = new NumberLiteral;

    // desambiguar el tipo en float
    // por defecto es int
    // chequeo si es float:
    //      typed_number->m_vtype = ast_vtype::Float

    typed_number->m_token = ast->m_token;
    return typed_number;
}

TypedAST* convertAST(AST::StringLiteral* ast) {
    auto typed_string = new StringLiteral;
    typed_string->m_token = ast->m_token;
    return typed_string;
}

TypedAST* convertAST(AST::BooleanLiteral* ast) {
    auto typed_boolean = new BooleanLiteral;
    typed_boolean->m_token = ast->m_token;
    return typed_boolean;
}

TypedAST* convertAST(AST::NullLiteral* ast) {
    return new NullLiteral;
}

TypedAST* convertAST(AST::ObjectLiteral* ast) {
    auto typed_object = new ObjectLiteral;
    // al tipo de los objetos se les debe sumar una identificacion
    // de clase

    for (auto& element : ast->m_body) {
        typed_object->m_body.push_back(get_unique(element));
    }

    return typed_object;
}

TypedAST* convertAST(AST::ArrayLiteral* ast) {
    auto typed_array = new ArrayLiteral;

    for (auto& element : ast->m_elements) {
        typed_array->m_elements.push_back(get_unique(element)); 
    }

    return typed_array;
}

TypedAST* convertAST(AST::DictionaryLiteral* ast) {
    auto typed_dict = new DictionaryLiteral;

    for (auto& element : ast->m_body) {
        typed_dict->m_body.push_back(get_unique(element));
    }

    return typed_dict;
}

TypedAST* convertAST(AST::FunctionLiteral* ast) {
    auto typed_function = new FunctionLiteral;

    for (auto& arg : ast->m_args) {
        typed_function->m_args.push_back(get_unique(arg));
    }

    typed_function->m_body = get_unique(ast->m_body);

    return typed_function;
}

TypedAST* convertAST(AST::DeclarationList* ast) {
    auto typed_declist = new DeclarationList;

    for (auto& declaration : ast->m_declarations) {
        typed_declist->m_declarations.push_back(get_unique(declaration));
    }

    return typed_declist;
}

TypedAST* convertAST(AST::Declaration* ast) {
    auto typed_dec = new Declaration;

	typed_dec->m_identifier_token = ast->m_identifier_token;
	// TODO: handle type hint
	if (ast->m_value)
		typed_dec->m_value = get_unique(ast->m_value);

	return typed_dec;
}

TypedAST* convertAST(AST::Identifier* ast) {
    auto typed_id = new Identifier;
    typed_id->m_token = ast->m_token;
    return typed_id;
}

TypedAST* convertASTPizza(AST::BinaryExpression* ast) {
	// TODO: error handling
	assert(ast->m_rhs->type() == ast_type::CallExpression);
	auto call = static_cast<AST::CallExpression*>(ast->m_rhs.get());

	auto result = new CallExpression;

	result->m_args.push_back(get_unique(ast->m_lhs));
	for (auto& arg : call->m_args)
		result->m_args.push_back(get_unique(arg));

	result->m_callee = get_unique(call->m_callee);

	return result;
}

TypedAST* convertASTDot(AST::BinaryExpression* ast) {
	// TODO: error handling

	// TODO: move this check to the parser
	assert(ast->m_rhs->type() == ast_type::Identifier);
	auto ident = static_cast<AST::Identifier*>(ast->m_rhs.get());

	auto tok = ast->m_op_token;
	std::cerr << "Error: @" << tok->m_line0+1 << ":" << tok->m_col0 << " | Dot (.) operator not implemented yet\n";

	return new NullLiteral;
}

// This function desugars binary operators into function calls
TypedAST* convertAST(AST::BinaryExpression* ast) {

	if (ast->m_op_token->m_type == token_type::PIZZA)
		return convertASTPizza(ast);

	if (ast->m_op_token->m_type == token_type::DOT)
		return convertASTDot(ast);

	auto identifier = std::make_unique<Identifier>();
	identifier->m_token = ast->m_op_token;

	auto typed_ast = new CallExpression;
	typed_ast->m_callee = std::move(identifier);

	typed_ast->m_args.push_back(get_unique(ast->m_lhs));
	typed_ast->m_args.push_back(get_unique(ast->m_rhs));

	return typed_ast;
}

TypedAST* convertAST(AST::CallExpression* ast) {
    auto typed_ce = new CallExpression;

    for (auto& arg : ast->m_args) {
        typed_ce->m_args.push_back(get_unique(arg));
    }

    typed_ce->m_callee = get_unique(ast->m_callee);

    return typed_ce;
}

TypedAST* convertAST(AST::IndexExpression* ast) {
    auto typed_index = new IndexExpression;

    typed_index->m_callee = get_unique(ast->m_callee);
    typed_index->m_index  = get_unique(ast->m_index);

    return typed_index;
}

TypedAST* convertAST(AST::Block* ast) {
    auto typed_block = new Block;

    for (auto& element : ast->m_body) {
        typed_block->m_body.push_back(get_unique(element));
    }

    return typed_block;
}

TypedAST* convertAST(AST::ReturnStatement* ast) {
    auto typed_rs = new ReturnStatement;

    typed_rs->m_value = get_unique(ast->m_value);

    return typed_rs;
}

TypedAST* convertAST(AST::IfStatement* ast) {
    auto typed_if = new IfStatement;

    typed_if->m_condition = get_unique(ast->m_condition);
    typed_if->m_body      = get_unique(ast->m_body);

    return typed_if;
}

TypedAST* convertAST(AST::ForStatement* ast) {
    auto typed_for = new ForStatement;

    typed_for->m_declaration = get_unique(ast->m_declaration);
    typed_for->m_condition   = get_unique(ast->m_condition);
    typed_for->m_action      = get_unique(ast->m_action);
    typed_for->m_body        = get_unique(ast->m_body);

    return typed_for;
}

TypedAST* convertAST (AST::AST* ast) {
    switch (ast->type()) {
    case ast_type::NumberLiteral:
        return convertAST(static_cast<AST::NumberLiteral*>(ast));
    case ast_type::StringLiteral:
        return convertAST(static_cast<AST::StringLiteral*>(ast));
    case ast_type::BooleanLiteral:
        return convertAST(static_cast<AST::BooleanLiteral*>(ast));
    case ast_type::NullLiteral:
        return convertAST(static_cast<AST::NullLiteral*>(ast));
    case ast_type::ObjectLiteral:
        return convertAST(static_cast<AST::ObjectLiteral*>(ast));
    case ast_type::ArrayLiteral:
        return convertAST(static_cast<AST::ArrayLiteral*>(ast));
    case ast_type::DictionaryLiteral:
        return convertAST(static_cast<AST::DictionaryLiteral*>(ast));
    case ast_type::FunctionLiteral:
        return convertAST(static_cast<AST::FunctionLiteral*>(ast));
    case ast_type::DeclarationList:
        return convertAST(static_cast<AST::DeclarationList*>(ast));
    case ast_type::Declaration:
        return convertAST(static_cast<AST::Declaration*>(ast));
    case ast_type::Identifier:
        return convertAST(static_cast<AST::Identifier*>(ast));
    case ast_type::BinaryExpression:
        return convertAST(static_cast<AST::BinaryExpression*>(ast));
    case ast_type::CallExpression:
        return convertAST(static_cast<AST::CallExpression*>(ast));
    case ast_type::IndexExpression:
        return convertAST(static_cast<AST::IndexExpression*>(ast));
    case ast_type::Block:
        return convertAST(static_cast<AST::Block*>(ast));
    case ast_type::ReturnStatement:
        return convertAST(static_cast<AST::ReturnStatement*>(ast));
    case ast_type::IfStatement:
        return convertAST(static_cast<AST::IfStatement*>(ast));
    case ast_type::ForStatement:
        return convertAST(static_cast<AST::ForStatement*>(ast));
    default:
        std::cerr << "Error: AST type not handled in convertAST" << std::endl;
        assert(0);
    }
}

} // namespace TypedAST
