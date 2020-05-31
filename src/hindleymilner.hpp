#pragma once

#include <vector>

namespace HindleyMilner {

enum class mono_type {
    Mono,
    Param
};

class Mono {
public:
    mono_type type;
    int id;

    Mono(int _id): type(mono_type::Mono), id(_id) {}
    Mono(mono_type _type, int _id): type(_type), id(_id) {}
};

// TODO hacer unique_ptr todos los punteros a mono
// tipo parametrico puede ser desde
// Ej: funcion a -> b (infija)
//     lista   list a
//     mapa    map  a b
//     lista de funcion list (a -> b)
class Param : public Mono {
public:
    const Mono* base; // responsabilidad del programador no poner variables de base
    std::vector<Mono*> params {};

    Param(int _id, const Mono* _base): 
        Mono(mono_type::Param, _id), base(_base) {}
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
// Maybe it's a good idea to have a class named
// representative and limit the operations of
// mono
Mono* representative(Mono*);

class Env;

Mono* hm_var (Poly);
Mono* hm_app (Mono*, Mono*, Env);
Mono* hm_abs (Mono*, Mono*);
Poly hm_let (Poly, std::vector<int>);

}