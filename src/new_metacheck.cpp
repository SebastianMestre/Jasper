#include "ast.hpp"

#include "meta_unifier.hpp"

namespace TypeChecker {

// TODO: pass the unifier around

static void process_declaration_type_hint(MetaUnifier& uf, AST::Declaration* ast) {
	auto typehint = ast->m_typehint;
	if (!typehint) return;
	uf.turn_into(ast->m_metatype, Tag::Term);
	metacheck(uf, typehint);
	uf.turn_into(typehint->m_metatype, Tag::Mono);
}

void metacheck(MetaUnifier& uf, AST::Declaration* ast) {
	ast->m_metatype = uf.create_var_node();
	metacheck(uf, ast->m_value);
}

void metacheck(MetaUnifier& uf, AST::AccessExpression* ast) {
	metacheck(uf, ast->m_target);
	ast->m_metatype = uf.create_dot_node(ast->m_target->m_metatype);
}

void metacheck(MetaUnifier& uf, AST::MatchExpression* ast) {
	ast->m_metatype = uf.create_const_node(Tag::Term);

	metacheck(uf, ast->m_target);
	uf.turn_into(ast->m_target.m_metatype, Tag::Term);

	if (ast->m_type_hint) {
		metacheck(uf, ast->m_type_hint);
		uf.turn_into(ast->m_type_hint->m_metatype, Tag::Mono);
	}

	for (auto& kv : ast->m_cases) {
		auto& case_data = kv.second;

		case_data.m_declaration.m_metatype = Tag::Term;
		process_declaration_type_hint(&case_data.m_declaration, tc);

		metacheck(uf, case_data.m_expression);
		uf.turn_into(case_data.m_expression->m_metatype, Tag::Term);
	}
}

void metacheck(MetaUnifier& uf, AST::AST* ast) {
#define DISPATCH(type)                                                         \
	case ASTTag::type:                                                         \
	return metacheck(uf, static_cast<AST::type*>(ast));

	switch (ast->type()) {
		DISPATCH(AccessExpression)
	}
}

} // namespace TypeChecker
