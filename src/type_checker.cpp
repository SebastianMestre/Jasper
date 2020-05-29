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
    // Todas las llamadas a VAR requieren que
    // antes se genere un tipo Poly
    if (ast->type() == ast_type::NumberLiteral) {
        auto type = Poly {
            integer,
            {}
        };
        ast->m_vtype = hm_var(type, env);
    }

    if (ast->type() == ast_type::StringLiteral) {
        auto type = Poly {
            string,
            {}
        };
        ast->m_vtype = hm_var(type, env);
    }

    if (ast->type() == ast_type::BooleanLiteral) {
        auto type = Poly {
            boolean,
            {}
        };
        ast->m_vtype = hm_var(type, env);
    }

    if (ast->type() == ast_type::Identifier) {
        auto identifier = static_cast<TypedASTIdentifier*>(ast)->m_token->m_text;

        assert(env.types.count(identifier));

        auto type = env.types[identifier];
        ast->m_vtype = hm_var(type, env);
    }

    if (ast->type() == ast_type::Declaration) {
        // TODO appendear al environment los hints de tipos
        auto type = Poly {
            ident,
            {}
        };

        // expression es una representacion de el valor que
        // se asigna a la variable
        //
        // se deberia pasar una estructura representativa del
        // valor asignado para que hm_let detecte que variables
        // estan siendo usadas
        //
        // podria reemplazarse con un vector de los tipos usados
        // en el valor asignados. sea
        // valores_utilizados = gather_free_variables(ast->m_value)

        auto& value = static_cast<TypedASTDeclaration*>(ast)->m_value;
        auto  free_vars = gather_free_variables(value->get());

        ast->m_vtype = hm_let(type, free_vars, env);
    }
}

}