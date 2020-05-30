#pragma once

#include <unordered_map>
#include <vector>
#include <string>

namespace HindleyMilner {

enum class mono_type {
    Mono,
    Param
};

struct Mono {
    mono_type type;
    int id;

    bool operator<(Mono);
};

// las funciones son un subconjuntos de los
// tipos parametricos
struct Param : Mono {
    Mono base;
    std::vector<Mono> params;
}; 

struct Poly {
    Mono base;
    std::vector<int> forall_ids;
};

struct Env {
    std::unordered_map<std::string, Poly> types;
};

// Mono arrow;
// Mono ident;
// Mono integer;
// Mono string;
// Mono boolean;
// Mono array;

void unify (Mono, Mono);

Mono new_mono();
Mono new_param();

Mono hm_var (Poly, Env);
Mono hm_app (Mono, Mono);
Mono hm_abs (Mono, Mono);
Poly hm_let (Poly, std::vector<std::string>, Env);

}