#include <cassert>
#include <iostream>

#include "ast.hpp"
#include "typed_ast.hpp"
#include "typed_ast_type.hpp"

std::unique_ptr<TypedAST> get_unique(std::unique_ptr<AST>& ast) {
    return std::unique_ptr<TypedAST>(convertAST(ast.get()));
}

TypedAST* convertAST(ASTNumberLiteral* ast) {
    auto typed_number = new TypedASTNumberLiteral;

    // desambiguar el tipo en float
    // por defecto es int
    // chequeo si es float:
    //      typed_number->m_vtype = ast_vtype::Float

    typed_number->m_token = ast->m_token;
    return typed_number;
}

TypedAST* convertAST(ASTStringLiteral* ast) {
    auto typed_string = new TypedASTStringLiteral;
    typed_string->m_token = ast->m_token;
    return typed_string;
}

TypedAST* convertAST(ASTBooleanLiteral* ast) {
    auto typed_boolean = new TypedASTBooleanLiteral;
    typed_boolean->m_token = ast->m_token;
    return typed_boolean;
}

TypedAST* convertAST(ASTNullLiteral* ast) {
    return new TypedASTNullLiteral;
}

TypedAST* convertAST(ASTObjectLiteral* ast) {
    auto typed_object = new TypedASTObjectLiteral;
    // al tipo de los objetos se les debe sumar una identificacion
    // de clase

    for (auto& element : ast->m_body) {
        typed_object->m_body.push_back(get_unique(element));
    }

    return typed_object;
}

TypedAST* convertAST(ASTArrayLiteral* ast) {
    auto typed_array = new TypedASTArrayLiteral;

    for (auto& element : ast->m_elements) {
        typed_array->m_elements.push_back(get_unique(element)); 
    }

    return typed_array;
}

TypedAST* convertAST(ASTDictionaryLiteral* ast) {
    auto typed_dict = new TypedASTDictionaryLiteral;

    for (auto& element : ast->m_body) {
        typed_dict->m_body.push_back(get_unique(element));
    }

    return typed_dict;
}

TypedAST* convertAST(ASTFunctionLiteral* ast) {
    auto typed_function = new TypedASTFunctionLiteral;

    for (auto& arg : ast->m_args) {
        typed_function->m_args.push_back(get_unique(arg));
    }

    typed_function->m_body = get_unique(ast->m_body);

    return typed_function;
}

TypedAST* convertAST(ASTDeclarationList* ast) {
    auto typed_declist = new TypedASTDeclarationList;

    for (auto& declaration : ast->m_declarations) {
        typed_declist->m_declarations.push_back(get_unique(declaration));
    }

    return typed_declist;
}

TypedAST* convertAST(ASTDeclaration* ast) {
    auto typed_dec = new TypedASTDeclaration;

    typed_dec->m_identifier_token = ast->m_identifier_token;
    typed_dec->m_typename_token   = ast->m_typename_token;
    if(ast->m_value)
		typed_dec->m_value = get_unique(ast->m_value);

    return typed_dec;
}

TypedAST* convertAST(ASTIdentifier* ast) {
    auto typed_id = new TypedASTIdentifier;
    typed_id->m_token = ast->m_token;
    return typed_id;
}

TypedAST* convertASTPizza(ASTBinaryExpression* ast) {
	// TODO: error handling
	assert(ast->m_rhs->type() == ast_type::CallExpression);
	auto call = static_cast<ASTCallExpression*>(ast->m_rhs.get());

	auto result = new TypedASTCallExpression;

	result->m_args.push_back(get_unique(ast->m_lhs));
	for (auto& arg : call->m_args)
		result->m_args.push_back(get_unique(arg));

	result->m_callee = get_unique(call->m_callee);

	return result;
}

// This function desugars binary operators into function calls
TypedAST* convertAST(ASTBinaryExpression* ast) {

	if (ast->m_op_token->m_type == token_type::PIZZA)
		return convertASTPizza(ast);

	auto identifier = std::make_unique<TypedASTIdentifier>();
	identifier->m_token = ast->m_op_token;

	auto typed_ast = new TypedASTCallExpression;
	typed_ast->m_callee = std::move(identifier);

	typed_ast->m_args.push_back(get_unique(ast->m_lhs));
	typed_ast->m_args.push_back(get_unique(ast->m_rhs));

	return typed_ast;
}

TypedAST* convertAST(ASTCallExpression* ast) {
    auto typed_ce = new TypedASTCallExpression;

    for (auto& arg : ast->m_args) {
        typed_ce->m_args.push_back(get_unique(arg));
    }

    typed_ce->m_callee = get_unique(ast->m_callee);

    return typed_ce;
}

TypedAST* convertAST(ASTIndexExpression* ast) {
    auto typed_index = new TypedASTIndexExpression;

    typed_index->m_callee = get_unique(ast->m_callee);
    typed_index->m_index  = get_unique(ast->m_index);

    return typed_index;
}

TypedAST* convertAST(ASTBlock* ast) {
    auto typed_block = new TypedASTBlock;

    for (auto& element : ast->m_body) {
        typed_block->m_body.push_back(get_unique(element));
    }

    return typed_block;
}

TypedAST* convertAST(ASTReturnStatement* ast) {
    auto typed_rs = new TypedASTReturnStatement;

    typed_rs->m_value = get_unique(ast->m_value);

    return typed_rs;
}

TypedAST* convertAST(ASTIfStatement* ast) {
    auto typed_if = new TypedASTIfStatement;

    typed_if->m_condition = get_unique(ast->m_condition);
    typed_if->m_body      = get_unique(ast->m_body);

    return typed_if;
}

TypedAST* convertAST(ASTForStatement* ast) {
    auto typed_for = new TypedASTForStatement;

    typed_for->m_declaration = get_unique(ast->m_declaration);
    typed_for->m_condition   = get_unique(ast->m_condition);
    typed_for->m_action      = get_unique(ast->m_action);
    typed_for->m_body        = get_unique(ast->m_body);

    return typed_for;
}

TypedAST* convertAST (AST* ast) {
    switch (ast->type()) {
    case ast_type::NumberLiteral:
        return convertAST(static_cast<ASTNumberLiteral*>(ast));
    case ast_type::StringLiteral:
        return convertAST(static_cast<ASTStringLiteral*>(ast));
    case ast_type::BooleanLiteral:
        return convertAST(static_cast<ASTBooleanLiteral*>(ast));
    case ast_type::NullLiteral:
        return convertAST(static_cast<ASTNullLiteral*>(ast));
    case ast_type::ObjectLiteral:
        return convertAST(static_cast<ASTObjectLiteral*>(ast));
    case ast_type::ArrayLiteral:
        return convertAST(static_cast<ASTArrayLiteral*>(ast));
    case ast_type::DictionaryLiteral:
        return convertAST(static_cast<ASTDictionaryLiteral*>(ast));
    case ast_type::FunctionLiteral:
        return convertAST(static_cast<ASTFunctionLiteral*>(ast));
    case ast_type::DeclarationList:
        return convertAST(static_cast<ASTDeclarationList*>(ast));
    case ast_type::Declaration:
        return convertAST(static_cast<ASTDeclaration*>(ast));
    case ast_type::Identifier:
        return convertAST(static_cast<ASTIdentifier*>(ast));
    case ast_type::BinaryExpression:
        return convertAST(static_cast<ASTBinaryExpression*>(ast));
    case ast_type::CallExpression:
        return convertAST(static_cast<ASTCallExpression*>(ast));
    case ast_type::IndexExpression:
        return convertAST(static_cast<ASTIndexExpression*>(ast));
    case ast_type::Block:
        return convertAST(static_cast<ASTBlock*>(ast));
    case ast_type::ReturnStatement:
        return convertAST(static_cast<ASTReturnStatement*>(ast));
    case ast_type::IfStatement:
        return convertAST(static_cast<ASTIfStatement*>(ast));
    case ast_type::ForStatement:
        return convertAST(static_cast<ASTForStatement*>(ast));
    default:
        std::cerr << "Error: AST type not handled in convertAST" << std::endl;
        assert(0);
    }
}

// --- --- --- --- -- typeAST -- --- --- --- --- ---

bool valid_vtype(TypedAST* ast) {
	// maybe its not needed
	return (
		ast->m_vtype != ast_vtype::Void &&
		ast->m_vtype != ast_vtype::TypeError);
}

void typeAST(TypedASTArrayLiteral* ast) {
	ast->m_vtype = ast_vtype::Array;
	
	for (auto& element : ast->m_elements) {
		if (element->m_vtype == ast_vtype::Undefined) {
			ast->m_vtype = ast_vtype::Undefined;
		}

		if (!valid_vtype(element.get())) {
			ast->m_vtype = ast_vtype::TypeError;
			break;
		}
	}

	if (ast->m_vtype == ast_vtype::Array) {
		for (int i = 0; i < (int)ast->m_elements.size()-1; i++) {
			if (ast->m_elements[i]->m_vtype != 
				ast->m_elements[i+1]->m_vtype) {
				ast->m_vtype = ast_vtype::TypeError;
				break;
			}
		}
	}
}

void typeAST(TypedASTFunctionLiteral* ast) {
    // TODO
}

void typeAST(TypedASTDeclaration* ast) {
	auto& value = ast->m_value;
    ast->m_vtype = ast_vtype::Void;

    if (value) {
	    ast->m_vtype = value->m_vtype;

        if (!valid_vtype(value.get())) {
            ast->m_vtype = ast_vtype::TypeError;
        }
    }
}

void typeAST(TypedASTDeclarationList* ast) {
	ast->m_vtype = ast_vtype::Void;

	for (auto& decl : ast->m_declarations) {
		ast_vtype vtype = decl->m_vtype;

		if (vtype == ast_vtype::TypeError) {
			ast->m_vtype = ast_vtype::TypeError;
			break;
		}
	}
}

void typeAST(TypedAST* ast) {
	switch(ast->type()) {
	case ast_type::ArrayLiteral:
		typeAST(static_cast<TypedASTArrayLiteral*>(ast));
		break;
	case ast_type::Declaration:
		typeAST(static_cast<TypedASTDeclaration*>(ast));
		break;
	case ast_type::DeclarationList:
		typeAST(static_cast<TypedASTDeclarationList*>(ast));
		break;
	}
}
