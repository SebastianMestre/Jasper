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

struct Env {
    std::unordered_map<std::string, Poly> types;
    std::unordered_map<int, Poly> by_id;
};

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
    Mono base;
    std::vector<int> forall_ids;
};

struct Expression {
    expression_type type; 
};

struct Variable : Expression {
    std::string name;

    Variable (std::string _name) : name{_name}, Expression{expression_type::Variable} {}
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

Mono arrow;
Mono ident;
Mono integer;
Mono string;
Mono boolean;

void unify (Mono, Mono);

Mono hm_var (Poly type, Env env);
Mono hm_app (Mono t1, Mono t2);
Mono hm_abs (Mono t1, Mono t2);
Poly hm_let (Poly x, std::vector<std::string> e1, Env env);

}