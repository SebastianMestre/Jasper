#include <cassert>

#include "typed_ast.hpp"
#include "ast.hpp"
#include "ast_type.hpp"

std::unique_ptr<TypedAST> get_unique(std::unique_ptr<AST>& ast) {
    return std::unique_ptr<TypedAST>(convertAST(ast.get()));
}

TypedAST* convertAST(ASTNumberLiteral* ast) {
    auto typed_number = new TypedASTNumberLiteral;

    // desambiguar el tipo en float
    // por defecto es int
    // chequeo si es float:
    //      typed_number->m_vtype = value_type::Float

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
    }
}

