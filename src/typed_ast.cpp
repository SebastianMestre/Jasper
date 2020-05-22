#include <cassert>

#include "typed_ast.hpp"
#include "ast.hpp"
#include "ast_type.hpp"

// la funcion convertast hace una pasada configurando los tipos

TypedAST* TypedASTDeclaration::convertAST(AST* ast) {
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

TypedAST* TypedASTDeclarationList::convertAST (AST* ast) {
    ASTDeclarationList* declarations = static_cast<ASTDeclarationList*>(ast);

    for (auto& declaration : declarations->m_declarations) {
        TypedASTDeclaration* typed_declaration = new TypedASTDeclaration();
        typed_declaration->convertAST(declaration.get());
        m_declarations.push_back(std::unique_ptr<TypedAST>(typed_declaration));
    }

    return this;
}

TypedAST* TypedAST::convertAST (AST* ast) {
    switch (ast->type()) {
    case ast_type::DeclarationList:
        return static_cast<TypedASTDeclarationList*>(ast)->convertAST(ast);
    }
}

