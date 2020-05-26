#include "type_checker.hpp"
#include "typed_ast.hpp"
#include "typed_ast_type.hpp"

namespace TypeChecker {

void TC::type(TypedAST* root) {
    traverse(root);
    m_root = root;
}

void TC::traverse(TypedAST* node) {
    switch(node->type()) {
    case ast_type::Declaration:
        return 
    }
}

} 