#include <cassert>
#include <iostream>

#include "./log/log.hpp"
#include "cst.hpp"
#include "typed_ast.hpp"
#include "typed_ast_allocator.hpp"

namespace TypedAST {

InternedString const& Declaration::identifier_text() const {
	if (m_identifier.is_null()) {
		if (!m_identifier_token) {
			Log::fatal() << "No identifier or fallback on declaration of '" << m_identifier_token->m_text << "'";
		}

		Log::warning() << "No identifier on declaration " << m_identifier_token->m_text << ": using token data as fallback";

		return m_identifier_token->m_text;
	}
	return m_identifier;
}

TypedAST* convert_ast(CST::IntegerLiteral* cst, Allocator& alloc) {
	auto typed_integer = alloc.make<IntegerLiteral>();
	typed_integer->m_value = std::stoi(cst->text());
	if (cst->m_negative)
		typed_integer->m_value = -typed_integer->m_value;
	typed_integer->m_token = cst->m_token;
	return typed_integer;
}

TypedAST* convert_ast(CST::NumberLiteral* cst, Allocator& alloc) {
	auto typed_number = alloc.make<NumberLiteral>();
	typed_number->m_value = std::stof(cst->text());
	if (cst->m_negative)
		typed_number->m_value = -typed_number->m_value;
	typed_number->m_token = cst->m_token;
	return typed_number;
}

TypedAST* convert_ast(CST::StringLiteral* cst, Allocator& alloc) {
	auto typed_string = alloc.make<StringLiteral>();
	typed_string->m_token = cst->m_token;
	return typed_string;
}

TypedAST* convert_ast(CST::BooleanLiteral* cst, Allocator& alloc) {
	auto typed_boolean = alloc.make<BooleanLiteral>();
	typed_boolean->m_token = cst->m_token;
	return typed_boolean;
}

TypedAST* convert_ast(CST::NullLiteral* cst, Allocator& alloc) {
	return alloc.make<NullLiteral>();
}

TypedAST* convert_ast(CST::ArrayLiteral* cst, Allocator& alloc) {
	auto typed_array = alloc.make<ArrayLiteral>();

	for (auto element : cst->m_elements) {
		typed_array->m_elements.push_back(convert_ast(element, alloc));
	}

	return typed_array;
}

TypedAST* convert_ast(CST::DictionaryLiteral* cst, Allocator& alloc) {
	auto typed_dict = alloc.make<DictionaryLiteral>();

	for (auto& element : cst->m_body) {
		auto decl = static_cast<Declaration*>(convert_ast(&element, alloc));
		typed_dict->m_body.push_back(std::move(*decl));
	}

	return typed_dict;
}

TypedAST* convert_ast(CST::FunctionLiteral* cst, Allocator& alloc) {
	auto typed_function = alloc.make<FunctionLiteral>();

	for (auto& arg : cst->m_args) {
		Declaration typed_decl;

		typed_decl.m_identifier_token = arg.m_identifier_token;
		typed_decl.m_identifier = arg.m_identifier_token->m_text;
		if (arg.m_type_hint)
			typed_decl.m_type_hint = convert_ast(arg.m_type_hint, alloc);
		typed_decl.m_surrounding_function = typed_function;

		typed_function->m_args.push_back(std::move(typed_decl));
	}

	typed_function->m_body = convert_ast(cst->m_body, alloc);

	return typed_function;
}

TypedAST* convert_ast(CST::Declaration* cst, Allocator& alloc) {
	auto typed_dec = alloc.make<Declaration>();

	typed_dec->m_identifier_token = cst->m_identifier_token;
	typed_dec->m_identifier = cst->m_identifier_token->m_text;

	if (cst->m_type_hint)
		typed_dec->m_type_hint = convert_ast(cst->m_type_hint, alloc);

	if (cst->m_value)
		typed_dec->m_value = convert_ast(cst->m_value, alloc);

	return typed_dec;
}

TypedAST* convert_ast(CST::DeclarationList* cst, Allocator& alloc) {
	auto typed_declist = alloc.make<DeclarationList>();

	for (auto& declaration : cst->m_declarations) {
		auto decl = static_cast<Declaration*>(convert_ast(&declaration, alloc));
		typed_declist->m_declarations.push_back(std::move(*decl));
	}

	return typed_declist;
}

TypedAST* convert_ast(CST::Identifier* cst, Allocator& alloc) {
	auto typed_id = alloc.make<Identifier>();
	typed_id->m_token = cst->m_token;
	return typed_id;
}

TypedAST* convert_ast(CST::CallExpression* cst, Allocator& alloc) {
	auto typed_ce = alloc.make<CallExpression>();

	for (auto arg : cst->m_args) {
		typed_ce->m_args.push_back(convert_ast(arg, alloc));
	}

	typed_ce->m_callee = convert_ast(cst->m_callee, alloc);

	return typed_ce;
}

TypedAST* convert_ast(CST::IndexExpression* cst, Allocator& alloc) {
	auto typed_index = alloc.make<IndexExpression>();

	typed_index->m_callee = convert_ast(cst->m_callee, alloc);
	typed_index->m_index = convert_ast(cst->m_index, alloc);

	return typed_index;
}

TypedAST* convert_ast(CST::TernaryExpression* cst, Allocator& alloc) {
	auto typed_ternary = alloc.make<TernaryExpression>();

	typed_ternary->m_condition = convert_ast(cst->m_condition, alloc);
	typed_ternary->m_then_expr = convert_ast(cst->m_then_expr, alloc);
	typed_ternary->m_else_expr = convert_ast(cst->m_else_expr, alloc);

	return typed_ternary;
}

TypedAST* convert_ast(CST::AccessExpression* cst, Allocator& alloc) {
	auto typed_ast = alloc.make<AccessExpression>();

	typed_ast->m_member = cst->m_member;
	typed_ast->m_record = convert_ast(cst->m_record, alloc);

	return typed_ast;
}

TypedAST* convert_ast(CST::MatchExpression* cst, Allocator& alloc) {
	auto typed_ast = alloc.make<MatchExpression>();

	auto matchee = static_cast<Identifier*>(convert_ast(&cst->m_matchee, alloc));
	typed_ast->m_matchee = std::move(*matchee);

	if (cst->m_type_hint)
		typed_ast->m_type_hint = convert_ast(cst->m_type_hint, alloc);

	std::unordered_map<InternedString, MatchExpression::CaseData> cases;
	for (auto& case_data : cst->m_cases) {
		auto case_name = case_data.m_name->m_text;

		Declaration declaration;
		declaration.m_identifier_token = case_data.m_identifier;
		declaration.m_identifier = case_data.m_identifier->m_text;

		if (case_data.m_type_hint)
			declaration.m_type_hint = convert_ast(case_data.m_type_hint, alloc);

		auto expression = convert_ast(case_data.m_expression, alloc);

		auto insertion_result = cases.insert(
		    {case_name,
		     MatchExpression::CaseData {std::move(declaration), expression}});

		assert(insertion_result.second);
	}

	typed_ast->m_cases = std::move(cases);

	return typed_ast;
}

TypedAST* convert_ast(CST::ConstructorExpression* cst, Allocator& alloc) {
	auto typed_ast = alloc.make<ConstructorExpression>();

	typed_ast->m_constructor = convert_ast(cst->m_constructor, alloc);

	for (auto& arg : cst->m_args)
		typed_ast->m_args.push_back(convert_ast(arg, alloc));

	return typed_ast;
}

TypedAST* convert_ast(CST::SequenceExpression* cst, Allocator& alloc) {
	auto result = alloc.make<SequenceExpression>();
	result->m_body = static_cast<Block*>(convert_ast(cst->m_body, alloc));
	return result;
}

TypedAST* convert_ast(CST::Block* cst, Allocator& alloc) {
	auto typed_block = alloc.make<Block>();

	for (auto element : cst->m_body) {
		typed_block->m_body.push_back(convert_ast(element, alloc));
	}

	return typed_block;
}

TypedAST* convert_ast(CST::ReturnStatement* cst, Allocator& alloc) {
	auto typed_rs = alloc.make<ReturnStatement>();

	typed_rs->m_value = convert_ast(cst->m_value, alloc);

	return typed_rs;
}

TypedAST* convert_ast(CST::IfElseStatement* cst, Allocator& alloc) {
	auto typed_if_else = alloc.make<IfElseStatement>();

	typed_if_else->m_condition = convert_ast(cst->m_condition, alloc);
	typed_if_else->m_body = convert_ast(cst->m_body, alloc);

	if (cst->m_else_body)
		typed_if_else->m_else_body = convert_ast(cst->m_else_body, alloc);

	return typed_if_else;
}

TypedAST* convert_ast(CST::ForStatement* cst, Allocator& alloc) {
	auto typed_for = alloc.make<ForStatement>();

	auto decl = static_cast<Declaration*>(convert_ast(&cst->m_declaration, alloc));
	typed_for->m_declaration = std::move(*decl);
	typed_for->m_condition = convert_ast(cst->m_condition, alloc);
	typed_for->m_action = convert_ast(cst->m_action, alloc);
	typed_for->m_body = convert_ast(cst->m_body, alloc);

	return typed_for;
}

TypedAST* convert_ast(CST::WhileStatement* cst, Allocator& alloc) {
	auto typed_while = alloc.make<WhileStatement>();

	typed_while->m_condition = convert_ast(cst->m_condition, alloc);
	typed_while->m_body = convert_ast(cst->m_body, alloc);

	return typed_while;
}

TypedAST* convert_ast(CST::UnionExpression* cst, Allocator& alloc) {
	auto typed_ast = alloc.make<UnionExpression>();

	for (auto& constructor : cst->m_constructors) {
		auto typed_field = static_cast<Identifier*>(convert_ast(&constructor, alloc));
		typed_ast->m_constructors.push_back(std::move(*typed_field));
	}

	for (auto type : cst->m_types) {
		typed_ast->m_types.push_back(convert_ast(type, alloc));
	}

	return typed_ast;
};

TypedAST* convert_ast(CST::StructExpression* cst, Allocator& alloc) {
	auto typed_ast = alloc.make<StructExpression>();

	for (auto& field : cst->m_fields) {
		auto typed_field = static_cast<Identifier*>(convert_ast(&field, alloc));
		typed_ast->m_fields.push_back(std::move(*typed_field));
	}

	for (auto type : cst->m_types) {
		typed_ast->m_types.push_back(convert_ast(type, alloc));
	}

	return typed_ast;
};

TypedAST* convert_ast(CST::TypeTerm* cst, Allocator& alloc) {
	auto typed_ast = alloc.make<TypeTerm>();

	typed_ast->m_callee = convert_ast(cst->m_callee, alloc);
	for (auto arg : cst->m_args){
		typed_ast->m_args.push_back(convert_ast(arg, alloc));
	}

	return typed_ast;
}

TypedAST* convert_ast(CST::CST* cst, Allocator& alloc) {
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

} // namespace TypedAST
