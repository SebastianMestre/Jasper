#include "utils/typedefs.hpp"
#include "typed_ast_allocator.hpp"

namespace TypedAST {
struct TypedAST;
}

namespace TypeChecker {

struct TypeChecker;

TypedAST::TypedAST* ct_eval(TypedAST::TypedAST*, TypeChecker& tc, TypedAST::Allocator& alloc);

} // namespace TypeChecker
