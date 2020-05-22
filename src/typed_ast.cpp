#include <cassert>

#include "typed_ast.hpp"
#include "ast.hpp"
#include "ast_type.hpp"

// la funcion convertast hace una pasada configurando los tipos

TypedAST* TypedASTDeclaration::convertAST(ASTDeclaration* ast) {
    ASTDeclaration* declaration = static_cast<ASTDeclaration*>(ast);
    m_identifier_token = declaration->m_identifier_token;
    m_typename_token = declaration->m_typename_token;

    TypedAST* typed_value = TypedAST::convertAST(declaration->m_value.get());
    m_value = std::unique_ptr<TypedAST>(typed_value);

    if (m_typename_token != nullptr) {
        assert(m_value->m_vtype == m_typename_token->type());
        m_vtype = m_typename_token->type();
    } else {
        m_vtype = m_value->m_vtype;
    }

    return this;
}

TypedAST* TypedASTDeclarationList::convertAST (ASTDeclarationList* ast) {
    for (auto& declaration : ast->m_declarations) {
        auto typed_declaration = TypedAST::convertAST(declaration.get());
        m_declarations.push_back(std::unique_ptr<TypedAST>(typed_declaration));
    }

    return this;
}

TypedAST* TypedAST::convertAST (AST* ast) {
    switch (ast->type()) {
    case ast_type::Declaration:
        auto decl = new TypedASTDeclaration;
        return decl->convertAST(static_cast<ASTDeclaration*>(ast));
    case ast_type::DeclarationList:
        auto decl_list = new TypedASTDeclarationList;
        return decl_list->convertAST(static_cast<ASTDeclarationList*>(ast));
    }
}

