#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cassert>
#include <string>

#include "hindleymilner.hpp"

namespace HindleyMilner {

// the monotypes' id shouldn't start before the native
// types id
int id = NATIVE_TYPES;

// map from id to monotype
std::unordered_map<int, Mono*> mono_id;

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

Mono* instanciate(Mono* base, std::unordered_set<int>& forall) {
    // this function should recursionate over the tree type
    // changing the quantified types in the forall specifier
    // for new fresh types
    
    if (base->type == mono_type::Mono) {
        if (forall.count(base->id)) {
            mono_id[id] = new Mono {mono_type::Mono, id};
        } else {
            return base;
        }
    } else {
        auto p_base = static_cast<Param*>(base);

        mono_id[id] = new Param {id};

        Mono* fresh_base = instanciate(p_base->base, forall);

        std::vector<Mono*> fresh_params;
        for (auto param : p_base->params) {
            fresh_params.push_back(instanciate(param, forall));
        }
        
        auto param = static_cast<Param*>(mono_id[id]);
        param->base   = fresh_base;
        param->params = fresh_params;
    }

    return mono_id[id++];
}

Mono* instanciate(Poly type) {
    std::unordered_set<int> forall {type.forall_ids.begin(), type.forall_ids.end()};
    return instanciate(type.base, forall);
}

Mono* hm_var (Poly type) {
    mono_id[id] = instanciate(type);
    
    return mono_id[id++]; 
}

Mono* representative(Mono* t) {
    Mono* rep = mono_id[t->id];
    if (rep->id == t->id) {
        return rep;
    } else {
        mono_id[id] = representative(rep);
        return mono_id[id];
    }
}

void connect(Mono* t1, Mono* t2) {
    Mono* r2 = representative(t2);
    mono_id[t1->id]->id = r2->id;
}

void unify(Mono* t1, Mono* t2, Env env) {
    Mono* r1 = representative(t1);
    Mono* r2 = representative(t2);

    if (t1->type == mono_type::Mono) {
        if (t2->type == mono_type::Mono) {
            if (!env.is_bound(t1))
                connect(t1, t2);
            else if (!env.is_bound(t2))
                connect(t2, t1);
            else
                assert(r1 == r2);
        } else {
            assert(!env.is_bound(t1));
            connect(t1, t2);
        }
    } else if (t2->type == mono_type::Mono) {
        assert(!env.is_bound(t2));
        connect(t2, t1);
    } else {
        assert(r1 == r2);

        Param* p1 = static_cast<Param*>(t1);
        Param* p2 = static_cast<Param*>(t2);

        assert(p1->params.size() == p2->params.size());

        for (int i = 0; i < (int)p1->params.size(); i++) {
            unify(p1->params[i], p2->params[i], env);
        }
    }
}

Mono* hm_app (Mono* t1, Mono* t2, Env env) {
    // maybe should use Param instead of mono
    Mono* t3 = new_mono();

    mono_id[id] = new Param {id};

    Param* param = static_cast<Param*>(mono_id[id]);
    param->base = new_instance(Arrow);
    param->params = std::vector<Mono*>{t2, t3};

    unify(t1, mono_id[id++], env);

    return t3;
}

Mono* hm_abs (Mono* args, Mono* ret) { // ret(urn)
    mono_id[id] = new Param {id};

    Param* param = static_cast<Param*>(mono_id[id]);
    param->base = new_instance(Arrow);
    param->params = std::vector<Mono*>{args, ret};

    return mono_id[id++];
}

Poly hm_let (Poly value, std::vector<int> free_vars) {
    // las variables libres deben asignarse al
    // tipo poly en los valores forall

    for (auto id : free_vars) {
        value.forall_ids.push_back(id);
    }

    return value;
}

}