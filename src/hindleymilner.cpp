#include <vector>
#include <cassert>
#include <unordered_map>
#include <string>

#include "hindleymilner.hpp"

namespace HindleyMilner {

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
    
    Mono base = type.base;
    if (base.type == mono_type::Mono) {
        mono_id[id] = Mono {mono_type::Mono, id};
    } else {
        auto p_base = static_cast<Param*>(&base);
        mono_id[id] = Param {
            mono_type::Param,
            id,
            p_base->base,
            p_base->params
        };
    }
    
    return mono_id[id++]; 
}

void unify(Mono t1, Mono t2) {}

Mono hm_app (Mono t1, Mono t2) {
    // maybe should use Param instead of mono
    Mono t3 = new_mono();

    mono_id[id] = Param {
        mono_type::Param,
        id,
        arrow,
        {t2, t3}
    };

    unify(t1, mono_id[id++]);

    return t3;
}

Mono hm_abs (std::vector<Poly> args, Mono callee) {
    // maybe a base tipe like array could be passed
    Param args_type {mono_type::Param, id};

    for (Poly arg : args) {
        // the type of the args can be overwritten without
        // problems for the global state of the types
        //
        // that means we don't have to create an instation
        // of the arg before we use it
        args_type.params.push_back(arg.base);
    }

    mono_id[id++] = args_type;
    
    auto type = Param {
        mono_type::Param,
        id++,
        arrow,
        {callee, args_type}
    };

    return type;
}

Poly hm_let (Poly value, std::vector<std::string> free_vars, Env env) {
    // el env serían los hints de tipos por lo que hay
    // que atravesar todos los tipos de t1 viendo cuales
    // no estan hinteados

    auto is_hinted = [env](std::string var)->bool{
        return env.types.count(var);
    };

    for (auto& var : free_vars) {
        if (!is_hinted(var)) {
            value.forall_ids.push_back(new_mono().id);
        }
    }

    return value;
}

}