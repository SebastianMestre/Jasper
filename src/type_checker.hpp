#pragma once

#include <vector>

#include "typed_ast.hpp"

namespace TypeChecker {

class TypeGraph {
};

class TC {
protected:
    TypeGraph m_root;
public:
    std::vector<TypeGraph> m_traverse;
    void type(TypedAST*);
    void traverse(TypedAST*);
};


}