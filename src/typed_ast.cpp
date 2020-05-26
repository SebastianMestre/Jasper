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

TypedAST* convertAST(ASTBinaryExpression* ast) {
    auto typed_be = new TypedASTBinaryExpression;

    typed_be->m_op  = ast->m_op;
    typed_be->m_lhs = get_unique(ast->m_lhs);
    typed_be->m_rhs = get_unique(ast->m_rhs);

    return typed_be;
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

void typeAST(TypedASTArrayLiteral* ast) {
	ast->m_vtype = ast_vtype::Array;
	
	for (auto& element : ast->m_elements) {
		if (element->m_vtype == ast_vtype::Undefined) {
			ast->m_vtype = ast_vtype::Undefined;
		}

        // cannot hold void
		if (ast->m_vtype == ast_vtype::Void ||
		    ast->m_vtype == ast_vtype::TypeError) {
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
        
        // cannot hold void
        if (ast->m_vtype == ast_vtype::Void ||
            ast->m_vtype == ast_vtype::TypeError) {
            ast->m_vtype = ast_vtype::TypeError;
        }
    }
}

void typeAST(TypedASTDeclarationList* ast) {
	ast->m_vtype = ast_vtype::Void;

	for (auto& decl : ast->m_declarations) {
		ast_vtype vtype = decl->m_vtype;

        // TODO if any of the decl is Undefined, decl_list has to be Undefined too
        // TODO gotta fix the tests too

		if (vtype == ast_vtype::TypeError) {
			ast->m_vtype = ast_vtype::TypeError;
			break;
		}
	}
}

void typeAST(TypedASTBinaryExpression* ast) {
    auto& lhs = ast->m_lhs;
    auto& rhs = ast->m_rhs;

    // suppose the operators are closed in the variables
    // types
    ast->m_vtype = lhs->m_vtype;

    if (lhs->m_vtype != rhs->m_vtype) {
        ast->m_vtype = ast_vtype::TypeError;
    }

    if (lhs->m_vtype == ast_vtype::Undefined || 
        rhs->m_vtype == ast_vtype::Undefined) {
        ast->m_vtype = ast_vtype::Undefined;
    }

    if (lhs->m_vtype == ast_vtype::TypeError ||
        rhs->m_vtype == ast_vtype::TypeError) {
        ast->m_vtype = ast_vtype::TypeError;
    }
}

void typeAST(TypedASTCallExpression* ast) {
    // TODO
}

void typeAST(TypedASTIndexExpression* ast) {
    // supposing that callee is an array
    auto array = static_cast<TypedASTArrayLiteral*>(ast->m_callee.get());
    auto& index = ast->m_index;

    ast->m_vtype = array->m_elements[0]->m_vtype;

    if (index->m_vtype == ast_vtype::Undefined || 
        array->m_vtype == ast_vtype::Undefined) {
        ast->m_vtype = ast_vtype::Undefined;
    }

    if (array->m_vtype == ast_vtype::TypeError ||
        index->m_vtype != ast_vtype::Integer) {
        ast->m_vtype = ast_vtype::TypeError;
    }
}

void typeAST(TypedASTBlock* ast) {
    ast->m_vtype = ast_vtype::Void;

    for (auto& elem : ast->m_body) {
        if (elem->m_vtype == ast_vtype::Undefined) {
            ast->m_vtype = ast_vtype::Undefined;
        }

        if (elem->m_vtype == ast_vtype::TypeError) {
            ast->m_vtype = ast_vtype::TypeError;
            break;
        }
    }
}

void typeAST(TypedASTReturnStatement* ast) {
    ast->m_vtype = ast->m_value->m_vtype;
}

void typeAST(TypedASTIfStatement* ast) {
    auto& condition = ast->m_condition;
    auto& body = ast->m_body;

    ast->m_vtype = ast_vtype::Void;

    if (body->m_vtype == ast_vtype::Undefined ||
        condition->m_vtype == ast_vtype::Undefined) {
        ast->m_vtype = ast_vtype::Undefined;
    }

    if (body->m_vtype == ast_vtype::TypeError ||
        condition->m_vtype == ast_vtype::TypeError) {
        ast->m_vtype = ast_vtype::TypeError;
    }
}

void typeAST(TypedASTForStatement* ast) {
    auto& declaration = ast->m_declaration;
    auto& condition = ast->m_condition;
    auto& action = ast->m_action;
    auto& body = ast->m_body;

    ast->m_vtype = ast_vtype::Void;

    if (body->m_vtype == ast_vtype::Undefined ||
        action->m_vtype == ast_vtype::Undefined ||
        condition->m_vtype == ast_vtype::Undefined ||
        declaration->m_vtype == ast_vtype::Undefined) {
        ast->m_vtype = ast_vtype::Undefined;
    }

    if (body->m_vtype == ast_vtype::TypeError ||
        action->m_vtype == ast_vtype::TypeError ||
        condition->m_vtype == ast_vtype::TypeError ||
        declaration->m_vtype == ast_vtype::TypeError) {
        ast->m_vtype = ast_vtype::TypeError;
    }
}

void typeAST(TypedAST* ast) {
	switch(ast->type()) {
	case ast_type::ArrayLiteral:
		return typeAST(static_cast<TypedASTArrayLiteral*>(ast));
	case ast_type::Declaration:
		return typeAST(static_cast<TypedASTDeclaration*>(ast));
	case ast_type::DeclarationList:
		return typeAST(static_cast<TypedASTDeclarationList*>(ast));
    case ast_type::FunctionLiteral:
        return typeAST(static_cast<TypedASTFunctionLiteral*>(ast));
    case ast_type::BinaryExpression:
        return typeAST(static_cast<TypedASTBinaryExpression*>(ast));
    case ast_type::CallExpression:
        return typeAST(static_cast<TypedASTCallExpression*>(ast));
    case ast_type::IndexExpression:
        return typeAST(static_cast<TypedASTIndexExpression*>(ast));
    case ast_type::Block:
        return typeAST(static_cast<TypedASTBlock*>(ast));
    case ast_type::ReturnStatement:
        return typeAST(static_cast<TypedASTReturnStatement*>(ast));
    case ast_type::IfStatement:
        return typeAST(static_cast<TypedASTIfStatement*>(ast));
    case ast_type::ForStatement:
        return typeAST(static_cast<TypedASTForStatement*>(ast));
    case ast_type::Identifier:
    case ast_type::BooleanLiteral:
    case ast_type::DictionaryLiteral:
    case ast_type::NullLiteral:
    case ast_type::NumberLiteral:
    case ast_type::ObjectLiteral:
    case ast_type::StringLiteral:
        break;
	}
}