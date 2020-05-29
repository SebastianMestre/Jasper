#pragma once

namespace HindleyMilner {
    
enum class expression_type {
    Variable,
    App,
    Abs,
    Let,
};

struct Mono {};

void unify (Mono, Mono);

}