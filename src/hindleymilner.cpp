#include <vector>
#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <string>

#include "hindleymilner.hpp"

namespace HindleyMilner {

// Los tipos van a estar limitados a las ids predefinidas
// al inicio del programa mas los wildcards que se vayan 
// definiendo

int id = NATIVE_TYPES;

// mapa de identificadores a tipos
std::unordered_map<int, Mono*> mono_id{};

Mono* new_mono () {
    mono_id[id] = new Mono {id};
    return mono_id[id++];
}

Mono* new_param () {
    mono_id[id] = new Param {id};
    return mono_id[id++];
}

Mono* new_instance (const Mono base) {
    mono_id[id] = new Mono {base.id};
    return mono_id[id++];
}

Mono* instanciate(Mono* base) {
    // TODO
    if (base->type == mono_type::Mono) {
        mono_id[id] = new Mono {mono_type::Mono, id};
    } else {
        // los variables cuantificadas se tienen que 
        // reemplazar por variables nuevas
        auto p_base = static_cast<Param*>(base);

        mono_id[id] = new Param {id};

        auto inst = static_cast<Param*>(mono_id[id]);

        
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
}

Mono* hm_var (Poly* type) {
    // es todo lo que puede hacer var desde un aspecto practico, ya
    // que si se esperase que var hiciese las sustituciones la funcion
    // ya no seria fiel a su respectiva regla
    //
    // aun asi puede realizar la validacion de que ninguna tipo este
    // libre
    
    mono_id[id] = instanciate(type->base);
    
    return mono_id[id++]; 
}

void unify(Mono* t1, Mono* t2) {}

Mono* hm_app (Mono* t1, Mono* t2) {
    // maybe should use Param instead of mono
    Mono* t3 = new_mono();

    mono_id[id] = new Param {id};

    Param* param = static_cast<Param*>(mono_id[id]);
    param->base = new_instance(arrow);
    param->params = std::vector<Mono*>{t2, t3};

    unify(t1, mono_id[id++]);

    return t3;
}

Mono* hm_abs (Mono* args, Mono* ret) { // ret(urn)
    mono_id[id] = new Param {id};

    Param* param = static_cast<Param*>(mono_id[id]);
    param->base = new_instance(arrow);
    param->params = std::vector<Mono*>{args, ret};

    return mono_id[id++];
}

Poly* hm_let (Poly* value, std::vector<int> free_vars) {
    // las variables libres deben asignarse al
    // tipo poly en los valores forall

    for (auto id : free_vars) {
        value->forall_ids.push_back(id);
    }

    return value;
}

}