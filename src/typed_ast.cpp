#include <cassert>

#include "typed_ast.hpp"
#include "ast.hpp"
#include "ast_type.hpp"

// la funcion convertast hace una pasada configurando los tipos

TypedAST* convertAST(ASTDeclaration* ast) {
    auto typed_dec = new TypedASTDeclaration;
    typed_dec->m_identifier_token = ast->m_identifier_token;
    typed_dec->m_typename_token = ast->m_typename_token;

    TypedAST* typed_value = convertAST(ast->m_value.get());
    typed_dec->m_value = std::unique_ptr<TypedAST>(typed_value);

    if (ast->m_typename_token != nullptr) {
        assert(typed_value->m_vtype == ast->m_typename_token->type());
    }

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
    case ast_type::Declaration:
        return convertAST(static_cast<ASTDeclaration*>(ast));
    case ast_type::DeclarationList:
        return convertAST(static_cast<ASTDeclarationList*>(ast));
    }
}

