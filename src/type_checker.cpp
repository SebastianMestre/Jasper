#include <vector>
#include <cassert>

#include "hindleymilner.hpp"
#include "typed_ast.hpp"

namespace HindleyMilner {

class TypeChecker {
    Env env;
    void deduce(TypedAST*);
    void deduce(TypedASTNumberLiteral*);
    void deduce(TypedASTStringLiteral*);
    void deduce(TypedASTBooleanLiteral*);
    void deduce(TypedASTDeclaration*);
    void deduce(TypedASTIdentifier*);
    void deduce(TypedASTFunctionLiteral*);
    void deduce(TypedASTCallExpression*);
};

std::vector<std::string> gather_free_variables(TypedAST* ast) {
    // for the moment the only way there's free variables on a rvalue
    // of a declaration is that ast is a function because it's not
    // allowed to have not declared variables
    //
    // in the future there could be rvalues such as parametrized objects
    if (ast->type() != ast_type::FunctionLiteral) {
        return std::move(std::vector<std::string>{});
    }

    auto function = static_cast<TypedASTFunctionLiteral*>(ast);

    std::vector<std::string> args_ids;

    for (auto& arg : function->m_args) {
        assert(arg->type() == ast_type::Declaration);

        auto decl = static_cast<TypedASTDeclaration*>(arg.get());

        args_ids.push_back(decl->m_identifier_token->m_text);
    }

    return std::move(args_ids);
}

void TypeChecker::deduce(TypedASTNumberLiteral* ast) {
    auto type = Poly {
        integer,
        {}
    };
    ast->m_vtype = type;
}

void TypeChecker::deduce(TypedASTStringLiteral* ast) {
    auto type = Poly {
        string,
        {}
    };
    ast->m_vtype = type;
}

void TypeChecker::deduce(TypedASTBooleanLiteral* ast) {
    auto type = Poly {
        boolean,
        {}
    };
    ast->m_vtype = type;
}

void TypeChecker::deduce(TypedASTIdentifier* ast) {
    auto identifier = ast->m_token->m_text;

    assert(env.types.count(identifier));

    ast->m_vtype = env.types[identifier];
}

void TypeChecker::deduce(TypedASTDeclaration* ast) {
    // TODO appendear al environment los hints de tipos
    auto& value = ast->m_value;
    deduce(value.get());

    auto type = value->m_vtype;
    auto identifier = ast->m_identifier_token->m_text;

    auto free_vars = gather_free_variables(value.get());

    ast->m_vtype = hm_let(type, free_vars, env);
    env.types[identifier] = ast->m_vtype;
}

void TypeChecker::deduce(TypedASTFunctionLiteral* ast) {
    auto  args = ast->m_args;
    auto& body = ast->m_body;

    Mono args_type = new_param();
    auto p_args_type = static_cast<Param*>(&args_type);
    Poly body_type;

    deduce(body.get());
    body_type = body->m_vtype;

    p_args_type->base = array;

    for (auto& arg : args) {
        deduce(arg.get());

        // maybe should instantiate the base
        auto arg_type = arg->m_vtype.base;

        p_args_type->params.push_back(arg_type);
    }

    auto func_type = hm_abs(hm_var(body_type, env), args_type);

    ast->m_vtype = Poly{
        func_type,
        {}
    };
}

void TypeChecker::deduce (TypedASTCallExpression* ast) {
    auto  args = ast->m_args;
    auto& callee = ast->m_callee;

    Mono args_type = new_param();
    auto p_args_type = static_cast<Param*>(&args_type);
    Poly callee_type;

    deduce(callee.get());
    callee_type = callee->m_vtype;

    p_args_type->base = array;

    for (auto& arg : args) {
        deduce(arg.get());

        // maybe should instantiate the base
        auto arg_type = arg->m_vtype.base;

        p_args_type->params.push_back(arg_type);
    }

    auto result_type = hm_app(hm_var(callee_type, env), args_type);

    ast->m_vtype = Poly{
        result_type,
        {}
    };
}

void TypeChecker::deduce (TypedAST* ast) {
    switch (ast->type()) {
    case ast_type::NumberLiteral:
        return deduce(static_cast<TypedASTNumberLiteral*>(ast));
    case ast_type::StringLiteral:
        return deduce(static_cast<TypedASTStringLiteral*>(ast));
    case ast_type::BooleanLiteral:
        return deduce(static_cast<TypedASTBooleanLiteral*>(ast));
    case ast_type::Identifier:
        return deduce(static_cast<TypedASTIdentifier*>(ast));
    case ast_type::Declaration:
        return deduce(static_cast<TypedASTDeclaration*>(ast));
    case ast_type::FunctionLiteral:
        return deduce(static_cast<TypedASTFunctionLiteral*>(ast));
    case ast_type::CallExpression:
        return deduce(static_cast<TypedASTCallExpression*>(ast));
    default:
        assert(0);
    }
}

}