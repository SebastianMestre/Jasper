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
    void gather_free_variables(TypedAST*, std::vector<int>&);
    void gather_free_variables(Param*, std::vector<int>&);
    bool is_bound(Mono*);
};

// TODO funcion check que tras deduce checkee los tipos
// TODO assert vtype after deduce calls

bool TypeChecker::is_bound(Mono* type) {
    return env.bounded_types.count(type->id);
}

void TypeChecker::gather_free_variables(
        Param* type, std::vector<int>& free_vars) {
    // the objective of the function is to recursionate over the
    // type tree and get the free variables
    auto base = type->base;
    auto params = type->params;

    assert(base->type == mono_type::Mono);
    if (!is_bound(base)) {
        free_vars.push_back(base->id);
    }

    for (auto param : params) {
        if (param->type == mono_type::Mono) {
            if (!is_bound(base)) {
                free_vars.push_back(base->id);
            }
        } else {
            gather_free_variables(static_cast<Param*>(param), free_vars);
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

        if (base->type == mono_type::Mono) {
            if (!is_bound(base)) {
                free_vars.push_back(base->id);
            }
        } else {
            gather_free_variables(static_cast<Param*>(base), free_vars);
        }
    }
}

void TypeChecker::deduce(TypedASTNumberLiteral* ast) {
    auto type = Poly {
        &integer, // new variable associated to integer
        {}
    };
    ast->m_vtype = type;
}

void TypeChecker::deduce(TypedASTStringLiteral* ast) {
    auto type = Poly {
        &string,
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
    auto& value = ast->m_value;
    deduce(value.get());

    auto identifier = ast->m_identifier_token->m_text;
    auto type = value->m_vtype; 

    assert(type.forall_ids.size() == 0); // value should be monotype

    std::vector<int> free_vars;
    
    gather_free_variables(value.get(), free_vars);

    ast->m_vtype = hm_let(value->m_vtype, free_vars);
    env.types[identifier] = ast->m_vtype;
}

void TypeChecker::deduce(TypedASTFunctionLiteral* ast) {
    auto  args = ast->m_args;
    auto& body = ast->m_body;

    Mono* args_type = new_param();
    auto p_args_type = static_cast<Param*>(args_type);

    // should deduce using particular rule
    // to get the return value
    Mono return_type;
    
    // the return statements must return a monotype
    // value

    p_args_type->base = array;

    for (auto& arg : args) {
        deduce(arg.get());

        // maybe should instantiate the base
        auto arg_type = arg->m_vtype.base;

        p_args_type->params.push_back(arg_type);
    }

    auto func_type = hm_abs(args_type, return_type);

    ast->m_vtype = Poly{
        func_type,
        {}
    };
}

void TypeChecker::deduce (TypedASTCallExpression* ast) {
    auto  args = ast->m_args;
    auto& callee = ast->m_callee;

    Mono* args_type = new_param();
    auto p_args_type = static_cast<Param*>(args_type);
    Poly callee_type;

    // what does callee really store
    // if it's not an identifier maybe it's not right
    // to use it right away
    deduce(callee.get());
    callee_type = callee->m_vtype;

    p_args_type->base = array;

    for (auto& arg : args) {
        deduce(arg.get());

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