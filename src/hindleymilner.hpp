#pragma once

#include <vector>

namespace HindleyMilner {

// fwd declarations
class Representative;
class Env;

enum class mono_type {
    Mono,
    Param
};

class Mono {
private:
    int id;
public:
    mono_type type;

    void const set_id(int _id) {id = _id;}

    friend Representative representative(Mono*);
    friend Mono* new_instance(const Mono);

    Mono(int _id): id(_id), type(mono_type::Mono) {}
    Mono(mono_type _type, int _id): id(_id), type(_type) {}
};

// tipo parametrico puede ser desde
// Ej: funcion a -> b (infija)
//     lista   list a
//     mapa    map  a b
//     lista de funcion list (a -> b)
class Param : public Mono {
public:
    // responsabilidad del programador no poner de base una variable
    const Mono* base; 
    std::vector<Mono*> params {};

    Param(int _id, const Mono* _base): 
        Mono(mono_type::Param, _id), base(_base) {}
}; 

class Representative {
public:
    int id;
    Mono* subject;
};

class Poly {
public:
    Mono* base;
    std::vector<int> forall_ids;
};

#define NATIVE_TYPES 6
const Mono Arrow   {mono_type::Mono, 0};
const Mono Void    {mono_type::Mono, 1};
const Mono Integer {mono_type::Mono, 2};
const Mono String  {mono_type::Mono, 3};
const Mono Boolean {mono_type::Mono, 4};
const Mono Array   {mono_type::Mono, 5};

Mono* new_mono();
Mono* new_param(const Mono);
Mono* new_instance(const Mono);

// representative devuelve la instancia mas
// refinada del tipo que se pasa
Representative representative(Mono*);

Mono* hm_var (Poly);
Mono* hm_app (Mono*, Mono*, Env);
Mono* hm_abs (Mono*, Mono*);
Poly hm_let (Poly, std::vector<int>);

}