#include <cassert>
#include <iostream>

#include "./log/log.hpp"
#include "cst.hpp"
#include "ast.hpp"
#include "ast_allocator.hpp"

namespace AST {

InternedString const& Declaration::identifier_text() const {
	if (m_identifier.is_null()) {
		if (!m_cst)
			Log::fatal() << "No identifier string or fallback on declaration";

		auto cst = static_cast<CST::Declaration*>(m_cst);
		auto token = cst->m_identifier_token;

		Log::warning() << "No identifier on declaration " << token->m_text << ": using token data as fallback";

		return token->m_text;
	}

	return m_identifier;
}

Token const* Identifier::token() const {
	return static_cast<CST::Identifier*>(m_cst)->m_token;
}

AST* convert_ast(CST::IntegerLiteral* cst, Allocator& alloc) {
	auto ast = alloc.make<IntegerLiteral>();
	ast->m_value = std::stoi(cst->text());
	if (cst->m_negative)
		ast->m_value = -ast->m_value;
	return ast;
}

AST* convert_ast(CST::NumberLiteral* cst, Allocator& alloc) {
	auto ast = alloc.make<NumberLiteral>();
	ast->m_value = std::stof(cst->text());
	if (cst->m_negative)
		ast->m_value = -ast->m_value;
	return ast;
}

AST* convert_ast(CST::StringLiteral* cst, Allocator& alloc) {
	auto ast = alloc.make<StringLiteral>();
	ast->m_text = cst->m_token->m_text;
	return ast;
}

AST* convert_ast(CST::BooleanLiteral* cst, Allocator& alloc) {
	auto ast = alloc.make<BooleanLiteral>();
	ast->m_value = cst->m_token->m_type == TokenTag::KEYWORD_TRUE;
	return ast;
}

AST* convert_ast(CST::NullLiteral* cst, Allocator& alloc) {
	return alloc.make<NullLiteral>();
}

AST* convert_ast(CST::ArrayLiteral* cst, Allocator& alloc) {
	auto ast = alloc.make<ArrayLiteral>();

	for (auto element : cst->m_elements) {
		ast->m_elements.push_back(convert_ast(element, alloc));
	}

	return ast;
}

AST* convert_ast(CST::DictionaryLiteral* cst, Allocator& alloc) {
	auto ast = alloc.make<DictionaryLiteral>();

	for (auto& element : cst->m_body) {
		auto decl = static_cast<Declaration*>(convert_ast(&element, alloc));
		ast->m_body.push_back(std::move(*decl));
	}

	return ast;
}

AST* convert_ast(CST::FunctionLiteral* cst, Allocator& alloc) {
	auto ast = alloc.make<FunctionLiteral>();

	for (auto& arg : cst->m_args) {
		Declaration decl;

		decl.m_cst = &arg;
		decl.m_identifier = arg.m_identifier_token->m_text;
		if (arg.m_type_hint)
			decl.m_type_hint = convert_ast(arg.m_type_hint, alloc);
		decl.m_surrounding_function = ast;

		ast->m_args.push_back(std::move(decl));
	}

	ast->m_body = convert_ast(cst->m_body, alloc);

	return ast;
}

AST* convert_ast(CST::Declaration* cst, Allocator& alloc) {
	auto ast = alloc.make<Declaration>();

	ast->m_cst = cst;
	ast->m_identifier = cst->m_identifier_token->m_text;

	if (cst->m_type_hint)
		ast->m_type_hint = convert_ast(cst->m_type_hint, alloc);

	if (cst->m_value)
		ast->m_value = convert_ast(cst->m_value, alloc);

	return ast;
}

AST* convert_ast(CST::DeclarationList* cst, Allocator& alloc) {
	auto ast = alloc.make<DeclarationList>();

	for (auto& declaration : cst->m_declarations) {
		auto decl = static_cast<Declaration*>(convert_ast(&declaration, alloc));
		ast->m_declarations.push_back(std::move(*decl));
	}

	return ast;
}

AST* convert_ast(CST::Identifier* cst, Allocator& alloc) {
	auto ast = alloc.make<Identifier>();
	ast->m_cst = cst;
	ast->m_text = cst->m_token->m_text;
	return ast;
}

AST* convert_ast(CST::CallExpression* cst, Allocator& alloc) {
	auto ast = alloc.make<CallExpression>();

	for (auto arg : cst->m_args) {
		ast->m_args.push_back(convert_ast(arg, alloc));
	}

	ast->m_callee = convert_ast(cst->m_callee, alloc);

	return ast;
}

AST* convert_ast(CST::IndexExpression* cst, Allocator& alloc) {
	auto ast = alloc.make<IndexExpression>();

	ast->m_callee = convert_ast(cst->m_callee, alloc);
	ast->m_index = convert_ast(cst->m_index, alloc);

	return ast;
}

AST* convert_ast(CST::TernaryExpression* cst, Allocator& alloc) {
	auto ast = alloc.make<TernaryExpression>();

	ast->m_condition = convert_ast(cst->m_condition, alloc);
	ast->m_then_expr = convert_ast(cst->m_then_expr, alloc);
	ast->m_else_expr = convert_ast(cst->m_else_expr, alloc);

	return ast;
}

AST* convert_ast(CST::AccessExpression* cst, Allocator& alloc) {
	auto ast = alloc.make<AccessExpression>();

	ast->m_member = cst->m_member->m_text;
	ast->m_record = convert_ast(cst->m_record, alloc);

	return ast;
}

AST* convert_ast(CST::MatchExpression* cst, Allocator& alloc) {
	auto ast = alloc.make<MatchExpression>();

	auto matchee = static_cast<Identifier*>(convert_ast(&cst->m_matchee, alloc));
	ast->m_matchee = std::move(*matchee);

	if (cst->m_type_hint)
		ast->m_type_hint = convert_ast(cst->m_type_hint, alloc);

	std::unordered_map<InternedString, MatchExpression::CaseData> cases;
	for (auto& case_data : cst->m_cases) {
		auto case_name = case_data.m_name->m_text;

		Declaration declaration;
		// TODO: store match expression cst in declarations?
		declaration.m_identifier = case_data.m_identifier->m_text;

		if (case_data.m_type_hint)
			declaration.m_type_hint = convert_ast(case_data.m_type_hint, alloc);

		auto expression = convert_ast(case_data.m_expression, alloc);

		auto insertion_result = cases.insert(
		    {case_name,
		     MatchExpression::CaseData {std::move(declaration), expression}});

		assert(insertion_result.second);
	}

	ast->m_cases = std::move(cases);

	return ast;
}

AST* convert_ast(CST::ConstructorExpression* cst, Allocator& alloc) {
	auto ast = alloc.make<ConstructorExpression>();

	ast->m_constructor = convert_ast(cst->m_constructor, alloc);

	for (auto& arg : cst->m_args)
		ast->m_args.push_back(convert_ast(arg, alloc));

	return ast;
}

AST* convert_ast(CST::SequenceExpression* cst, Allocator& alloc) {
	auto result = alloc.make<SequenceExpression>();
	result->m_body = static_cast<Block*>(convert_ast(cst->m_body, alloc));
	return result;
}

AST* convert_ast(CST::Block* cst, Allocator& alloc) {
	auto ast = alloc.make<Block>();

	for (auto element : cst->m_body) {
		ast->m_body.push_back(convert_ast(element, alloc));
	}

	return ast;
}

AST* convert_ast(CST::ReturnStatement* cst, Allocator& alloc) {
	auto ast = alloc.make<ReturnStatement>();

	ast->m_value = convert_ast(cst->m_value, alloc);

	return ast;
}

AST* convert_ast(CST::IfElseStatement* cst, Allocator& alloc) {
	auto ast = alloc.make<IfElseStatement>();

	ast->m_condition = convert_ast(cst->m_condition, alloc);
	ast->m_body = convert_ast(cst->m_body, alloc);

	if (cst->m_else_body)
		ast->m_else_body = convert_ast(cst->m_else_body, alloc);

	return ast;
}

AST* convert_ast(CST::ForStatement* cst, Allocator& alloc) {
	auto ast = alloc.make<ForStatement>();

	auto decl = static_cast<Declaration*>(convert_ast(&cst->m_declaration, alloc));
	ast->m_declaration = std::move(*decl);
	ast->m_condition = convert_ast(cst->m_condition, alloc);
	ast->m_action = convert_ast(cst->m_action, alloc);
	ast->m_body = convert_ast(cst->m_body, alloc);

	return ast;
}

AST* convert_ast(CST::WhileStatement* cst, Allocator& alloc) {
	auto ast = alloc.make<WhileStatement>();

	ast->m_condition = convert_ast(cst->m_condition, alloc);
	ast->m_body = convert_ast(cst->m_body, alloc);

	return ast;
}

AST* convert_ast(CST::UnionExpression* cst, Allocator& alloc) {
	auto ast = alloc.make<UnionExpression>();

	for (auto& constructor : cst->m_constructors) {
		auto field = static_cast<Identifier*>(convert_ast(&constructor, alloc));
		ast->m_constructors.push_back(std::move(*field));
	}

	for (auto type : cst->m_types) {
		ast->m_types.push_back(convert_ast(type, alloc));
	}

	return ast;
};

AST* convert_ast(CST::StructExpression* cst, Allocator& alloc) {
	auto ast = alloc.make<StructExpression>();

	for (auto& cst_field : cst->m_fields) {
		auto field = static_cast<Identifier*>(convert_ast(&cst_field, alloc));
		ast->m_fields.push_back(std::move(*field));
	}

	for (auto type : cst->m_types) {
		ast->m_types.push_back(convert_ast(type, alloc));
	}

	return ast;
};

AST* convert_ast(CST::TypeTerm* cst, Allocator& alloc) {
	auto ast = alloc.make<TypeTerm>();

	ast->m_callee = convert_ast(cst->m_callee, alloc);
	for (auto arg : cst->m_args){
		ast->m_args.push_back(convert_ast(arg, alloc));
	}

	return ast;
}

AST* convert_ast(CST::CST* cst, Allocator& alloc) {
#define DISPATCH(type)                                                         \
	case CSTTag::type:                                                         \
		return convert_ast(static_cast<CST::type*>(cst), alloc)

#define REJECT(type)                                                           \
	case CSTTag::type:                                                         \
		Log::fatal("use of " #type " is forbidden in convert")

	switch (cst->type()) {
		DISPATCH(NumberLiteral);
		DISPATCH(IntegerLiteral);
		DISPATCH(StringLiteral);
		DISPATCH(BooleanLiteral);
		DISPATCH(NullLiteral);
		DISPATCH(ArrayLiteral);
		DISPATCH(DictionaryLiteral);
		DISPATCH(FunctionLiteral);
		REJECT(BlockFunctionLiteral);

		DISPATCH(Identifier);
		DISPATCH(CallExpression);
		DISPATCH(IndexExpression);
		DISPATCH(TernaryExpression);
		DISPATCH(AccessExpression);
		DISPATCH(MatchExpression);
		DISPATCH(ConstructorExpression);
		DISPATCH(SequenceExpression);
		REJECT(BinaryExpression);

		DISPATCH(Block);
		DISPATCH(ReturnStatement);
		DISPATCH(IfElseStatement);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);

		DISPATCH(DeclarationList);
		DISPATCH(Declaration);

		DISPATCH(UnionExpression);
		DISPATCH(StructExpression);
		DISPATCH(TypeTerm);
	}

	Log::fatal() << "(internal) CST type not handled in convert_ast: "
	             << cst_string[(int)cst->type()];

#undef REJECT
#undef DISPATCH
}

} // namespace AST
