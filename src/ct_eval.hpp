#include "typedefs.hpp"

namespace TypedAST {
struct TypedAST;
}

namespace TypeChecker {

struct TypeChecker;

Own<TypedAST::TypedAST> ct_eval(Own<TypedAST::TypedAST>, TypeChecker& tc);

} // namespace TypeChecker
