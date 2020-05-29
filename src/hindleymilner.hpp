#pragma once

#include <unordered_map>
#include <vector>
#include <string>

namespace HindleyMilner {
    
enum class expression_type {
    Variable,
    App,
    Abs,
    Let,
};

enum class mono_type {
    Mono,
    Param
};

struct Env {
    std::unordered_map<std::string, Poly> types;
    std::unordered_map<int, Poly> by_id;
};

struct Mono {
    mono_type type;
    int id;
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

Mono arrow;
Mono ident;
Mono integer;
Mono string;
Mono boolean;

void unify (Mono, Mono);

Mono hm_var (Poly type, Env env);
Mono hm_app (Mono t1, Mono t2);
Mono hm_abs (std::vector<Poly>, Mono t2);
Poly hm_let (Poly x, std::vector<std::string> e1, Env env);

}