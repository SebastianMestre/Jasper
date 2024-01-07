#include "metacheck.hpp"

#include "../algorithms/tarjan_solver.hpp"
#include "../ast.hpp"
#include "../log/log.hpp"

#include <cassert>

/*

Jasper does not make a syntactic distinction between type-level declarations and
value-level declarations, but the the typechecker needs this distinction to do
its thing.

Metacheck infers, for each expression and declaration in a program, whether it
corresponds to a value, a variant constructor, a monomorphic type, or a
polymorphic type. We refer to this categorization as "meta types".

Most expressions are trivial to infer a meta type for, but it's a bit trickier
for identifiers (as they inherit the meta type from their declaration) and for
field access expressions (as their meta type must be computed from the meta type
of their subexpression).

We can (mostly) do meta type inference in a fully top-down manner if we first do
a shallow inference pass and order the declarations carefully after that for a
deep pass.

== How it works ================================================================

The main idea is that the meta type of most expressions can be inferred just
from the syntactic category (i.e. the ASTTag of the node). The only expressions
that can't be inferred this way are Identifiers and access expressions.

To ensure identifiers can be inferred, we must visit them after we infer a
meta type for their declaration, so we visit declarations that have "difficult"
expressions in dependency order.

If there ever is a cycle of declarations that can't be inferred shallowly, it's
either either broken or really weird code that we don't care about supporting at
the moment. In those cases we just error out.

During metatype inference, we sometimes assign MetaType::Undefined to some terms
and declarations, but these are always resolved by the end of the process. The
following passes may assume that no expression or declaration has
MetaType::Undefined.

Example 1 - wrong

    a := b;
    b := c;
    c := a;

Example 2 - wrong

    a := b.x;
    b := a.y;

Example 3 - wrong

    a := c(b).x
    b := d(a).y

Example 4 - ok but really weird, and probably wrong

    a := c(fn() => b).x
    b := d(fn() => a).y

*/

namespace TypeChecker {

using AST::ExprTag;
using AST::StmtTag;

static MetaType infer(AST::Expr*);
static void infer_stmt(AST::Stmt*);

static MetaType infer_shallow(AST::Expr* ast) {
	switch (ast->type()) {
	case ExprTag::NumberLiteral:
	case ExprTag::IntegerLiteral:
	case ExprTag::StringLiteral:
	case ExprTag::BooleanLiteral:
	case ExprTag::ArrayLiteral:
	case ExprTag::NullLiteral:
	case ExprTag::FunctionLiteral:
	case ExprTag::CallExpression:
	case ExprTag::IndexExpression:
	case ExprTag::MatchExpression:
	case ExprTag::TernaryExpression:
	case ExprTag::ConstructorExpression:
	case ExprTag::SequenceExpression:
		return MetaType::Term;

	case ExprTag::UnionExpression:
	case ExprTag::StructExpression:
		return MetaType::TypeFunction;

	case ExprTag::TypeTerm:
		return MetaType::Type;

	case ExprTag::AccessExpression:
	case ExprTag::Identifier:
		return MetaType::Undefined;

	case ExprTag::BuiltinTypeFunction:
		Log::fatal() << "unexpected AST type in infer_shallow ("
		             << AST::expr_string[int(ast->type())] << ") during infer";
	}
}

static bool can_infer_shallow(AST::Expr* ast) {
	return infer_shallow(ast) != MetaType::Undefined;
}

static void check(AST::Expr* ast, MetaType meta_type) {
	if (infer(ast) != meta_type) Log::fatal() << "failed metacheck";
}


static MetaType infer(AST::NumberLiteral* ast) {
	return ast->m_meta_type = infer_shallow(ast);
}

static MetaType infer(AST::IntegerLiteral* ast) {
	return ast->m_meta_type = infer_shallow(ast);
}

static MetaType infer(AST::StringLiteral* ast) {
	return ast->m_meta_type = infer_shallow(ast);
}

static MetaType infer(AST::BooleanLiteral* ast) {
	return ast->m_meta_type = infer_shallow(ast);
}

static MetaType infer(AST::NullLiteral* ast) {
	return ast->m_meta_type = infer_shallow(ast);
}

static MetaType infer(AST::ArrayLiteral* ast) {
	for (auto element : ast->m_elements) {
		check(element, MetaType::Term);
	}
	return ast->m_meta_type = infer_shallow(ast);
}

static MetaType infer(AST::FunctionLiteral* ast) {
	for (auto& decl : ast->m_args) {
		decl.m_meta_type = MetaType::Term;
		if (decl.m_type_hint) {
			check(decl.m_type_hint, MetaType::Type);
		}
	}
	check(ast->m_body, MetaType::Term);
	return ast->m_meta_type = infer_shallow(ast);
}

static MetaType infer(AST::Identifier* ast) {
	MetaType meta_type = ast->m_declaration->m_meta_type;
	if (meta_type == MetaType::Undefined) Log::fatal() << "Unresolved meta type on variable '" << ast->m_text << "'";
	return ast->m_meta_type = meta_type;
}

static MetaType infer(AST::CallExpression* ast) {
	check(ast->m_callee, MetaType::Term);
	for (auto arg : ast->m_args) {
		check(arg, MetaType::Term);
	}
	return ast->m_meta_type = infer_shallow(ast);
}

static MetaType infer(AST::IndexExpression* ast) {
	check(ast->m_callee, MetaType::Term);
	check(ast->m_index, MetaType::Term);
	return ast->m_meta_type = infer_shallow(ast);
}

static MetaType infer(AST::AccessExpression* ast) {
	MetaType target_meta_type = infer(ast->m_target);
	if (target_meta_type == MetaType::Term) return ast->m_meta_type = MetaType::Term;
	if (target_meta_type == MetaType::Type) return ast->m_meta_type = MetaType::Constructor;
	Log::fatal() << "failed metacheck (" << __LINE__ << ")";
}

static MetaType infer(AST::MatchExpression* ast) {
	if (ast->m_type_hint) {
		check(ast->m_type_hint, MetaType::Type);
	}
	for (auto& kv : ast->m_cases) {
		kv.second.m_declaration.m_meta_type = MetaType::Term;
		if (kv.second.m_declaration.m_type_hint) {
			check(kv.second.m_declaration.m_type_hint, MetaType::Type);
		}
		check(kv.second.m_expression, MetaType::Term);
	}
	return ast->m_meta_type = infer_shallow(ast);
}

static MetaType infer(AST::TernaryExpression* ast) {
	check(ast->m_condition, MetaType::Term);
	check(ast->m_then_expr, MetaType::Term);
	check(ast->m_else_expr, MetaType::Term);
	return ast->m_meta_type = infer_shallow(ast);
}

static MetaType infer(AST::ConstructorExpression* ast) {
	MetaType constructor_meta_type = infer(ast->m_constructor);
	if (constructor_meta_type != MetaType::Constructor && constructor_meta_type != MetaType::Type) Log::fatal() << "failed metacheck (" << __LINE__ << ")";

	for (auto arg : ast->m_args) {
		check(arg, MetaType::Term);
	}

	return ast->m_meta_type = infer_shallow(ast);
}

static void infer_stmt(AST::Block* ast) {
	for (auto stmt : ast->m_body) {
		infer_stmt(stmt);
	}
}

static void infer_stmt(AST::ReturnStatement* ast) {
	check(ast->m_value, MetaType::Term);
}

static void infer_stmt(AST::IfElseStatement* ast) {
	check(ast->m_condition, MetaType::Term);
	infer_stmt(ast->m_body);
	if (ast->m_else_body) infer_stmt(ast->m_else_body);
}

static void infer_stmt(AST::WhileStatement* ast) {
	check(ast->m_condition, MetaType::Term);
	infer_stmt(ast->m_body);
}

static void infer_stmt(AST::ExpressionStatement* ast) {
	check(ast->m_expression, MetaType::Term);
}

static void infer_stmt(AST::Declaration* ast) {
	ast->m_meta_type = infer_shallow(ast->m_value);
	MetaType value_meta_type = infer(ast->m_value);
	if (value_meta_type == MetaType::Term) {
		if (ast->m_type_hint) {
			check(ast->m_type_hint, MetaType::Type);
		}
	} else {
		if (ast->m_type_hint) Log::fatal() << "failed metacheck (" << __LINE__ << ")";
	}
	ast->m_meta_type = value_meta_type;
}

static void infer_stmt(AST::Stmt* ast) {
	switch (ast->tag()) {
	case StmtTag::Block: return infer_stmt(static_cast<AST::Block*>(ast));
	case StmtTag::ReturnStatement: return infer_stmt(static_cast<AST::ReturnStatement*>(ast));
	case StmtTag::IfElseStatement: return infer_stmt(static_cast<AST::IfElseStatement*>(ast));
	case StmtTag::WhileStatement: return infer_stmt(static_cast<AST::WhileStatement*>(ast));
	case StmtTag::ExpressionStatement: return infer_stmt(static_cast<AST::ExpressionStatement*>(ast));
	case StmtTag::Declaration: return infer_stmt(static_cast<AST::Declaration*>(ast));
	}
}

static MetaType infer(AST::SequenceExpression* ast) {
	infer_stmt(ast->m_body);
	return ast->m_meta_type = infer_shallow(ast);
}

static MetaType infer(AST::UnionExpression* ast) {
	for (auto type : ast->m_types) {
		check(type, MetaType::Type);
	}
	return ast->m_meta_type = infer_shallow(ast);
}

static MetaType infer(AST::StructExpression* ast) {
	for (auto type : ast->m_types) {
		check(type, MetaType::Type);
	}
	return ast->m_meta_type = infer_shallow(ast);
}

static MetaType infer(AST::TypeTerm* ast) {
	check(ast->m_callee, MetaType::TypeFunction);
	for (auto arg : ast->m_args) {
		check(arg, MetaType::Type);
	}
	return ast->m_meta_type = infer_shallow(ast);
}

static MetaType infer(AST::Expr* ast) {
	switch (ast->type()) {
	case ExprTag::NumberLiteral: return infer(static_cast<AST::NumberLiteral*>(ast));
	case ExprTag::IntegerLiteral: return infer(static_cast<AST::IntegerLiteral*>(ast));
	case ExprTag::StringLiteral: return infer(static_cast<AST::StringLiteral*>(ast));
	case ExprTag::BooleanLiteral: return infer(static_cast<AST::BooleanLiteral*>(ast));
	case ExprTag::ArrayLiteral: return infer(static_cast<AST::ArrayLiteral*>(ast));
	case ExprTag::NullLiteral: return infer(static_cast<AST::NullLiteral*>(ast));
	case ExprTag::FunctionLiteral: return infer(static_cast<AST::FunctionLiteral*>(ast));
	case ExprTag::CallExpression: return infer(static_cast<AST::CallExpression*>(ast));
	case ExprTag::IndexExpression: return infer(static_cast<AST::IndexExpression*>(ast));
	case ExprTag::MatchExpression: return infer(static_cast<AST::MatchExpression*>(ast));
	case ExprTag::TernaryExpression: return infer(static_cast<AST::TernaryExpression*>(ast));
	case ExprTag::ConstructorExpression: return infer(static_cast<AST::ConstructorExpression*>(ast));
	case ExprTag::SequenceExpression: return infer(static_cast<AST::SequenceExpression*>(ast));
	case ExprTag::UnionExpression: return infer(static_cast<AST::UnionExpression*>(ast));
	case ExprTag::StructExpression: return infer(static_cast<AST::StructExpression*>(ast));
	case ExprTag::TypeTerm: return infer(static_cast<AST::TypeTerm*>(ast));
	case ExprTag::AccessExpression: return infer(static_cast<AST::AccessExpression*>(ast));
	case ExprTag::Identifier: return infer(static_cast<AST::Identifier*>(ast));
	case ExprTag::BuiltinTypeFunction:
		Log::fatal() << "unexpected AST type in infer_shallow ("
		             << AST::expr_string[int(ast->type())] << ") during infer";
	}
}

void metacheck_program(AST::Program* ast) {

	int counter = 0;
	std::unordered_map<AST::Declaration*, int> decl_to_index;
	std::vector<AST::Declaration*> index_to_decl;

	for (auto& decl : ast->m_declarations) {
		decl.m_meta_type = infer_shallow(decl.m_value);

		index_to_decl.push_back(&decl);
		decl_to_index.insert({&decl, counter});
		counter += 1;
	}

	TarjanSolver metacheck_dependency_solver(counter);
	TarjanSolver full_dependency_solver(counter);

	for (auto kv : decl_to_index) {
		auto decl = kv.first;
		auto u = kv.second;
		for (auto other : decl->m_references) {
			auto it = decl_to_index.find(other);
			if (it != decl_to_index.end()) {
				int v = it->second;
				if (decl->m_meta_type == MetaType::Undefined) {
					metacheck_dependency_solver.add_edge(u, v);
				}
				full_dependency_solver.add_edge(u, v);
			}
		}
	}

	full_dependency_solver.solve();
	metacheck_dependency_solver.solve();

	for (auto const& comp : metacheck_dependency_solver.vertices_of_components()) {

		if (comp.size() > 1) {
			Log::error() << "Unresolved dependency cycle involving the global variables:";
			for (int index : comp) {
				Log::info() << index_to_decl[index]->m_identifier;
			}
			Log::fatal() << "Terminating.";
		}

		auto* decl = index_to_decl[comp[0]];
		if (decl->m_meta_type == MetaType::Undefined) {
			decl->m_meta_type = infer(decl->m_value);
		}
	}

	for (auto& decl : ast->m_declarations) {

		// we haven't looked inside the declarations whose top-level meta type
		// could be determined shallowly. Do that now.
		if (can_infer_shallow(decl.m_value)) {
			infer(decl.m_value);
		}

		// check typehints on global variables
		if (decl.m_type_hint) {
			if (decl.m_meta_type != MetaType::Term) Log::fatal() << "failed metacheck (" << __LINE__ << ")";
			check(decl.m_type_hint, MetaType::Type);
		}
	}

	for (auto const& comp : full_dependency_solver.vertices_of_components()) {

		bool type_in_component = false;
		bool term_in_component = false;
		bool ctor_in_component = false;

		for (auto index : comp) {
			auto decl = index_to_decl[index];
			switch (decl->m_meta_type) {
			case MetaType::Type:
			case MetaType::TypeFunction:
				type_in_component = true;
				break;
			case MetaType::Term:
				term_in_component = true;
				break;
			case MetaType::Constructor:
				ctor_in_component = true;
				break;
			}
		}

		if (ctor_in_component) {
			Log::fatal() << "found a constructor stored as a global variable";
		}

		// We don't deal with types and terms in the same component.
		if (type_in_component && term_in_component) {
			Log::fatal() << "found reference cycle with types and values";
		}
	}

}

} // namespace TypeChecker
