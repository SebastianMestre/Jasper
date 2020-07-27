
namespace TypedAST {
struct TypedAST;
}

namespace Frontend {
struct CompileTimeEnvironment;
}

namespace TypeChecker {

void match_identifiers(TypedAST::TypedAST* ast, Frontend::CompileTimeEnvironment&);

}
