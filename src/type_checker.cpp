#include <vector>
#include <cassert>

#include "hindleymilner.hpp"
#include "type_checker.hpp"
#include "typed_ast.hpp"

namespace HindleyMilner {

// TODO assert vtype after deduce calls

bool Env::is_bound(Mono* type) {
    return bounded_types.count(type->id);
}

void TypeChecker::gather_free_variables(
        Mono* type, std::vector<int>& free_vars) {
    // the objective of the function is to recursionate over the
    // type tree and get the free variables

    if (type->type == mono_type::Mono) {
        if (!env.is_bound(type)) {
            free_vars.push_back(type->id);
        }
    } else {
        auto p_type = static_cast<Param*>(type);

        gather_free_variables(p_type->base, free_vars);

        for (auto param : p_type->params) {
            gather_free_variables(param, free_vars);
        }
    }
}

void TypeChecker::gather_free_variables(
        TypedAST* ast, std::vector<int>& free_vars) {
    // for the moment the only way there's free variables on a rvalue
    // of a declaration is that ast is a function because it's not
    // allowed to have not declared variables
    //
    // in the future there could be rvalues such as parametrized objects
    if (ast->type() != ast_type::FunctionLiteral) {
        return;
    }

    auto function = static_cast<TypedASTFunctionLiteral*>(ast);

    for (auto& arg : function->m_args) {
        assert(arg->type() == ast_type::Declaration);

        auto decl = static_cast<TypedASTDeclaration*>(arg.get());
        auto base = decl->m_vtype.base;

        gather_free_variables(base, free_vars);
    }
}

void TypeChecker::deduce(TypedASTNumberLiteral* ast) {
    auto type = Poly {
        new_instance(Integer),
        {}
    };
    ast->m_vtype = type;
}

void TypeChecker::deduce(TypedASTStringLiteral* ast) {
    auto type = Poly {
        new_instance(String),
        {}
    };
    ast->m_vtype = type;
}

void TypeChecker::deduce(TypedASTBooleanLiteral* ast) {
    auto type = Poly {
        new_instance(Boolean),
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
    auto& value = ast->m_value;
    deduce(value.get());

    auto identifier = ast->m_identifier_token->m_text;
    auto type = value->m_vtype; 

    assert(type.forall_ids.size() == 0); // value should be monotype

    std::vector<int> free_vars;
    
    gather_free_variables(value.get(), free_vars);

    ast->m_vtype = hm_let(type, free_vars);
    env.types[identifier] = ast->m_vtype;
}

void TypeChecker::deduce(TypedASTFunctionLiteral* ast) {
    Mono* return_type = nullptr;

    deduce(ast->m_body.get());
    assert(ast->m_body->type() == ast_type::Block); // TODO check
    auto body = static_cast<TypedASTBlock*>(ast->m_body.get());

    for (auto& stmt : body->m_body) {
        if (stmt->type() == ast_type::ReturnStatement) {
            auto type = stmt->m_vtype;

            // the return statements must return a monotype
            assert(type.forall_ids.size() == 0);
            return_type = type.base;

            break;
        }
    }

    if (!return_type) {
        return_type = new_instance(Void);
    }

    auto& args = ast->m_args;

    Param* args_type = static_cast<Param*>(new_param());

    args_type->base = new_instance(Array);

    for (auto& arg : args) {
        deduce(arg.get());

        auto arg_type = arg->m_vtype.base;

        // arg should be monotype
        assert(arg->m_vtype.forall_ids.size() == 0);

        args_type->params.push_back(arg_type);
    }

    auto func_type = hm_abs(args_type, return_type);

    ast->m_vtype = Poly{
        func_type,
        {}
    };
}

void TypeChecker::deduce (TypedASTCallExpression* ast) {
    auto& args = ast->m_args;
    auto& callee = ast->m_callee;

    Param* args_type = static_cast<Param*>(new_param());
    Poly callee_type;

    assert(callee->type() == ast_type::Identifier);

    deduce(callee.get());
    callee_type = callee->m_vtype;

    args_type->base = new_instance(Array);

    for (auto& arg : args) {
        deduce(arg.get());

        auto arg_type = arg->m_vtype.base;

        // arg should be monotype
        assert(arg->m_vtype.forall_ids.size() == 0);

        args_type->params.push_back(arg_type);
    }

    auto result_type = hm_app(hm_var(callee_type), args_type);

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