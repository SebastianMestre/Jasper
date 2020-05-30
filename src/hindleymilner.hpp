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
// las funciones son un subconjuntos de los
// tipos parametricos
class Param : public Mono {
public:
    Mono* base {nullptr};
    std::vector<Mono*> params {};

    Param(int _id): Mono(mono_type::Param, _id) {}
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
Mono* new_param();
Mono* new_instance(const Mono);

Mono* hm_var (Poly);
Mono* hm_app (Mono*, Mono*);
Mono* hm_abs (Mono*, Mono*);
Poly hm_let (Poly, std::vector<int>);

}