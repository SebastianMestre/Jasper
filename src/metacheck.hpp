#pragma once

namespace AST {
struct AST;
}

struct MetaUnifier;

namespace TypeChecker {


void metacheck(MetaUnifier&, AST::AST*);

}
