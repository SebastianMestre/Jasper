#include <vector>
#include <cassert>

#include "hindleymilner.hpp"
#include "typed_ast.hpp"

namespace HindleyMilner {

class TypeChecker {
    Env env;
    void deduce(TypedAST*);
};

std::vector<std::string> gather_free_variables(TypedAST* ast) {
    return std::move(std::vector<std::string>{});
}

void TypeChecker::deduce (TypedAST* ast) {
    // ValueLiteral -> aplicar [VAR]
    // Declaration -> aplicar [LET]
    // Identifier -> aplicar [VAR]
    // FunctionLiteral -> aplicar [ABS]
    // CallExpression -> aplicar [APP]

    // Todas las llamadas a VAR requieren que
    // antes se genere un tipo Poly
    if (ast->type() == ast_type::NumberLiteral) {
        auto type = Poly {
            integer,
            {}
        };
        ast->m_vtype = type;
    }

    if (ast->type() == ast_type::StringLiteral) {
        auto type = Poly {
            string,
            {}
        };
        ast->m_vtype = type;
    }

    if (ast->type() == ast_type::BooleanLiteral) {
        auto type = Poly {
            boolean,
            {}
        };
        ast->m_vtype = type;
    }

    if (ast->type() == ast_type::Identifier) {
        auto identifier = static_cast<TypedASTIdentifier*>(ast)->m_token->m_text;

        assert(env.types.count(identifier));

        auto type = env.types[identifier];
        ast->m_vtype = type;
    }

    if (ast->type() == ast_type::Declaration) {
        // TODO appendear al environment los hints de tipos

        auto& value = static_cast<TypedASTDeclaration*>(ast)->m_value;
        deduce(value.get());

        auto type = value->m_vtype;

        auto free_vars = gather_free_variables(value.get());

        ast->m_vtype = hm_let(type, free_vars, env);
    }

    // Antes de llamar a ABS se deben tipar los argumentos
    // y el cuerpo
    if (ast->type() == ast_type::FunctionLiteral) {
        auto  args = static_cast<TypedASTFunctionLiteral*>(ast)->m_args;
        auto& body = static_cast<TypedASTFunctionLiteral*>(ast)->m_body;

        std::vector<Poly> args_types;
        Poly body_type;

        deduce(body.get());
        body_type = body->m_vtype;

        for (auto& arg : args) {
            deduce(arg.get());
            args_types.push_back(arg->m_vtype);
        }

        auto func_type = hm_abs(args_types, hm_var(body_type, env));

        ast->m_vtype = Poly{
            func_type,
            {}
        };
    }

    // Antes de llamar a APP se deben tipar los valores y
    // verificar que la funcion este tipada
    if (ast->type() == ast_type::CallExpression) {
        auto  args = static_cast<TypedASTCallExpression*>(ast)->m_args;
        auto& callee = static_cast<TypedASTCallExpression*>(ast)->m_callee;

        Param args_type;
        Poly callee_type;

        deduce(callee.get());
        callee_type = callee->m_vtype;

        // not sure about this
        for (auto& arg : args) {
            args_type.params.push_back(arg->m_vtype.base);
        }

        // maybe an extra configuration step like
        // args_type.base = array
        //
        // would be required

        ast->m_vtype = Poly{
            hm_app(hm_var(callee_type, env), args_type),
            {}
        };
    }
}

}