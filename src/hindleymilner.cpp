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

Mono arrow = new_mono();
Mono ident = new_mono(); // identity

// Constants declaration
Mono integer = new_mono();
Mono string = new_mono();
Mono boolean = new_mono();

struct Env {
    std::unordered_map<std::string, Poly> types;
    std::unordered_map<int, Poly> by_id;
};

// hm_var no deberia tirar errores o realizar validaciones
// solo deberia devolver el tipo mas general
Mono hm_var (Poly type, Env env) {
    // por otro lado externamente se tendria que pasar un polytipe
    // ya parametrizado para que de esa forma la base del polytipe
    // sea un tipo sin variables libres y var devuelve esa base
    //
    // es todo lo que puede hacer var desde un aspecto practico, ya
    // que si se esperase que var hiciese las sustituciones la funcion
    // ya no seria fiel a su respectiva regla
    //
    // aun asi puede realizar la validacion de que ninguna tipo este
    // libre
    
    auto is_hinted = [env](int var)->bool{
        return env.by_id.count(var);
    };
    
    for (auto& id : type.forall_ids) {
        if (!is_hinted(id)) {
            assert(0);
        }
    }
    
    return type.base; 
}

void unify(Mono t1, Mono t2) {}

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

Poly hm_let (Poly x, std::vector<std::string> free_vars, Env env) {
    // el tipo x debe castearse para ser el mas generico
    x.base = ident;
    // el env serían los hints de tipos por lo que hay
    // que atravesar todos los tipos de t1 viendo cuales
    // no estan hinteados
    
    // std::vector<std::string> free_vars;
    // free(&e1, free_vars);

    auto is_hinted = [env](std::string var)->bool{
        return env.types.count(var);
    };

    for (auto& var : free_vars) {
        if (!is_hinted(var)) {
            // TODO marcar que variables tienen el tipo nuevo
            x.forall_ids.push_back(new_mono().id);
        }
    }

    return x;
}

}