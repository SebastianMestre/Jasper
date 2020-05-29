#include <vector>
#include <cassert>
#include <unordered_map>
#include <string>

#include "hindleymilner.hpp"

namespace HindleyMilner {

struct Mono {
    int id;
};

// las funciones son un subconjuntos de los
// tipos parametricos
struct Param : Mono {
    const Mono* base;
    const std::vector<Mono*> params;
}; 

struct Poly {
    std::vector<int> forall_ids;
    Mono base;
};

struct Expression {
    expression_type type; 
};

struct Variable : Expression {
    std::string name;
};

struct App : Expression {
    Expression* e1;
    Expression* e2;
};

struct Abs : Expression {
    Variable* x;
    Expression* e;
};

struct Let : Expression {
    Variable* x;
    Expression* e1;
    Expression* e2;
};

// Los tipos van a estar limitados a las ids predefinidas
// al inicio del programa mas los wildcards que se vayan 
// definiendo

int id = 0;

// mapa de identificadores a tipos
std::unordered_map<int, Mono> mono_id;

Mono new_mono () {
    mono_id[id].id = id;
    return mono_id[id++];
}

const Mono arrow = new_mono();
const Mono ident = new_mono(); // identity

Mono hm_var (Poly type, Mono target) {
    // esta regla se aplica en todas las pruebas de manera previa a
    // la aplicacion de otras reglas para constatar que un politipo
    // puede trasnformarse en el tipo deseado
    //
    // para una aplicacion practica esta regla ha de ejecutarse de
    // antemano y su resultado sera idealmente target o de otra forma
    // tirara error
    for (auto& id : type.forall_ids) {
        auto type = mono_id[id];

        if (type.id == target.id) {
            return target;
        }
    }

    assert(0);
}

Mono hm_app (Mono t1, Mono t2) {
    Mono t3;

    mono_id[id] = Param {
        id,
        &arrow,
        {&t2, &t3}
    };

    unify(t1, mono_id[id++]);

    return t3;
}

Mono hm_abs (Mono t1, Mono t2) {
    // se debe verificar que t1 es _newvar_
    mono_id[id] = Param {
        id++,
        &arrow,
        {&t1, &t2}
    };

    return mono_id[id];
}

struct Env {
    std::unordered_map<std::string, Poly> types;
};

void free(Expression* e, std::vector<std::string>& vars) {
    switch (e->type) {
    case expression_type::Variable:
        vars.push_back(static_cast<Variable*>(e)->name);
        break;
    case expression_type::Abs:
        free(static_cast<Abs*>(e)->e, vars);
        free(static_cast<Abs*>(e)->x, vars);
        break;
    case expression_type::App:
        free(static_cast<App*>(e)->e1, vars);
        free(static_cast<App*>(e)->e2, vars);
        break;
    case expression_type::Let:
        free(static_cast<Let*>(e)->e1, vars);
        free(static_cast<Let*>(e)->e2, vars);
        free(static_cast<Let*>(e)->x, vars);
        break;
    }
}

void hm_let (Poly& x, Expression e1, Env env) {
    // el tipo x debe castearse para ser el mas generico
    x.base = ident;
    // el env serían los hints de tipos por lo que hay
    // que atravesar todos los tipos de t1 viendo cuales
    // no estan hinteados
    
    std::vector<std::string> free_vars;
    free(&e1, free_vars);

    auto is_hinted = [env](std::string var)->bool{
        return env.types.count(var);
    };

    for (auto& var : free_vars) {
        if (!is_hinted(var)) {
            // TODO marcar que variables tienen el tipo nuevo
            x.forall_ids.push_back(new_mono().id);
        }
    }
}

}