#include <vector>
#include <algorithm>
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
    mono_id[id] = Mono {mono_type::Mono, id};
    return mono_id[id++];
}

Mono new_param () {
    mono_id[id] = Param {mono_type::Mono, id};
    return mono_id[id++];
}

const Mono arrow = new_mono();
Mono ident = new_mono(); // identity

// Constants declaration
Mono integer = new_mono();
Mono string = new_mono();
Mono boolean = new_mono();
Mono array = new_mono();

struct Env {
    std::unordered_map<std::string, Poly> types;
};

bool Mono::operator<(Mono other) {
    return id < other.id;
}

Mono hm_var (Poly type, Env env) {
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
        // los variables de tipo cuantificadas se tienen que 
        // reemplazar por variables nuevas
        auto p_base = static_cast<Param*>(&base);

        mono_id[id] = Param {mono_type::Param,id};

        auto inst = static_cast<Param*>(&mono_id[id]);

        std::sort(p_base->params.begin(), p_base->params.end());
        std::sort(type.forall_ids.begin(), type.forall_ids.end());

        int i = 0;
        for (auto& var_id : type.forall_ids) {
            while (var_id > p_base->params[i].id && 
                   var_id > p_base->base.id) i++;
            
            if (p_base->base.id == var_id) {
                // could be param
                p_base->base = new_mono();
            }

            if (p_base->params[i].id == var_id) {
                // could be param
                p_base->params[i] = new_mono();
            }
        }
        
        inst->base   = p_base->base;
        inst->params = p_base->params;
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

Mono hm_abs (Mono callee, Mono args) {
    auto type = Param {
        mono_type::Param,
        id++,
        arrow,
        {callee, args}
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