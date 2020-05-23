#include <cassert>

#include "typed_ast.hpp"
#include "ast.hpp"
#include "ast_type.hpp"

// la funcion convertast hace una pasada configurando los tipos

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
        auto* typed_element = convertAST(element.get());
        typed_object->m_body.push_back(
            std::unique_ptr<TypedAST>(typed_element));
    }

    return typed_object;
}

TypedAST* convertAST(ASTArrayLiteral* ast) {
    auto typed_array = new TypedASTArrayLiteral;

    for (auto& element : ast->m_elements) {
        auto* typed_element = convertAST(element.get());
        typed_array->m_elements.push_back(
            std::unique_ptr<TypedAST>(typed_element)); 
    }

    // checkeo para determinar que los tipos sean iguales
    // a lo largo del array
    for (int i = 0; i < ast->m_elements.size()-1; i++) {
        assert(
            typed_array->m_elements[i]->m_vtype == 
            typed_array->m_elements[i+1]->m_vtype);
    }

    return typed_array;
}

TypedAST* convertAST(ASTDictionaryLiteral* ast) {
    auto typed_dict = new TypedASTDictionaryLiteral;

    for (auto& element : ast->m_body) {
        auto* typed_element = convertAST(element.get());
        typed_dict->m_body.push_back(
            std::unique_ptr<TypedAST>(typed_element));
    }

    return typed_dict;
}

TypedAST* convertAST(ASTDeclaration* ast) {
    auto typed_dec = new TypedASTDeclaration;
    typed_dec->m_identifier_token = ast->m_identifier_token;
    typed_dec->m_typename_token = ast->m_typename_token;

    TypedAST* typed_value = convertAST(ast->m_value.get());
    typed_dec->m_value = std::unique_ptr<TypedAST>(typed_value);

    // if (ast->m_typename_token != nullptr) {
    //    assert(typed_value->m_vtype == ast->m_typename_token->type());
    // }

    typed_dec->m_vtype = typed_value->m_vtype;

    return typed_dec;
}

TypedAST* convertAST (ASTDeclarationList* ast) {
    auto typed_declist = new TypedASTDeclarationList;

    for (auto& declaration : ast->m_declarations) {
        auto typed_declaration = convertAST(declaration.get());
        typed_declist->m_declarations.push_back(
            std::unique_ptr<TypedAST>(typed_declaration));
    }

    return typed_declist;
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
    case ast_type::Declaration:
        return convertAST(static_cast<ASTDeclaration*>(ast));
    case ast_type::DeclarationList:
        return convertAST(static_cast<ASTDeclarationList*>(ast));
    }
}

