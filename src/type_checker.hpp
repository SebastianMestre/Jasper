#pragma once;

#include <unordered_map>
#include <unordered_set>
#include <string>

#include "hindleymilner.hpp"

namespace HindleyMilner {

class Env {
public:
    std::unordered_map<std::string, HindleyMilner::Poly> types;
    std::unordered_set<int> bounded_types;
};

}