#include <cassert>
#include <iostream>

#include "./log/log.hpp"
#include "cst.hpp"
#include "ast.hpp"
#include "ast_allocator.hpp"

namespace AST {

static Expr* convert_expr(CST::CST* cst, Allocator& alloc);

InternedString const& Declaration::identifier_text() const {
	if (m_identifier.is_null()) {
		if (!m_cst)
			Log::fatal() << "No identifier string or fallback on declaration";

		auto cst = static_cast<CST::Declaration*>(m_cst);

		auto const& found_identifier = cst->identifier_virtual();

		Log::warning() << "No identifier on declaration, using token data as fallback: '" << found_identifier << "'";

		return found_identifier;
	}

	return m_identifier;
}

Token const* Identifier::token() const {
	return static_cast<CST::Identifier*>(m_cst)->m_token;
}


static SequenceExpression* convert_and_wrap_in_seq(CST::Block* cst, Allocator& alloc) {
	auto block = static_cast<Block*>(convert_ast(cst, alloc));
	auto seq_expr = alloc.make<SequenceExpression>();
	seq_expr->m_body = block;
	return seq_expr;
}


static IntegerLiteral* convert(CST::IntegerLiteral* cst, Allocator& alloc) {
	auto ast = alloc.make<IntegerLiteral>();
	ast->m_value = std::stoi(cst->text());
	if (cst->m_negative)
		ast->m_value = -ast->m_value;
	return ast;
}

static NumberLiteral* convert(CST::NumberLiteral* cst, Allocator& alloc) {
	auto ast = alloc.make<NumberLiteral>();
	ast->m_value = std::stof(cst->text());
	if (cst->m_negative)
		ast->m_value = -ast->m_value;
	return ast;
}

static StringLiteral* convert(CST::StringLiteral* cst, Allocator& alloc) {
	auto ast = alloc.make<StringLiteral>();
	ast->m_text = cst->m_token->m_text;
	return ast;
}

static BooleanLiteral* convert(CST::BooleanLiteral* cst, Allocator& alloc) {
	auto ast = alloc.make<BooleanLiteral>();
	ast->m_value = cst->m_token->m_type == TokenTag::KEYWORD_TRUE;
	return ast;
}

static NullLiteral* convert(CST::NullLiteral* cst, Allocator& alloc) {
	return alloc.make<NullLiteral>();
}

static ArrayLiteral* convert(CST::ArrayLiteral* cst, Allocator& alloc) {
	auto ast = alloc.make<ArrayLiteral>();

	for (auto element : cst->m_elements) {
		ast->m_elements.push_back(convert_expr(element, alloc));
	}

	return ast;
}

static Declaration convert_declaration(CST::Declaration* cst, CST::DeclarationData& data, Allocator& alloc) {
	Declaration decl;
	decl.m_cst = cst;
	decl.m_identifier = data.identifier();
	if (data.m_type_hint)
		decl.m_type_hint = convert_expr(data.m_type_hint, alloc);
	if (data.m_value)
		decl.m_value = convert_expr(data.m_value, alloc);
	return decl;
}

static std::vector<Declaration> convert_args(
    CST::FuncParameters& cst_args,
    FunctionLiteral* surrounding_function,
    Allocator& alloc) {

	std::vector<Declaration> result;
	for (auto arg : cst_args) {
		Declaration decl = convert_declaration(nullptr, arg, alloc);
		decl.m_surrounding_function = surrounding_function;

		result.push_back(std::move(decl));
	}
	return result;
}

static FunctionLiteral* convert(CST::FunctionLiteral* cst, Allocator& alloc) {
	auto ast = alloc.make<FunctionLiteral>();

	ast->m_args = convert_args(cst->m_args, ast, alloc);
	ast->m_body = convert_expr(cst->m_body, alloc);

	return ast;
}

static FunctionLiteral* convert(CST::BlockFunctionLiteral* cst, Allocator& alloc) {
	auto ast = alloc.make<FunctionLiteral>();

	ast->m_args = convert_args(cst->m_args, ast, alloc);
	ast->m_body = convert_and_wrap_in_seq(cst->m_body, alloc);

	return ast;
}

// convert pizza expressions into plain calls
static CallExpression* convert_pizza(CST::BinaryExpression* cst, Allocator& alloc) {
	// TODO: better error
	if (cst->m_rhs->type() != CSTTag::CallExpression)
		Log::fatal("parsing error: bad pizza");

	auto ast = alloc.make<CallExpression>();

	ast->m_args.push_back(convert_expr(cst->m_lhs, alloc));

	auto call_cst = static_cast<CST::CallExpression*>(cst->m_rhs);
	ast->m_callee = convert_expr(call_cst->m_callee, alloc);
	for (auto arg : call_cst->m_args)
		ast->m_args.push_back(convert_expr(arg, alloc));

	return ast;
}

// convert binary operators into calls
static CallExpression* convert(CST::BinaryExpression* cst, Allocator& alloc) {
	if (cst->m_op_token->m_type == TokenTag::PIZZA)
		return convert_pizza(cst, alloc);

	if (cst->m_op_token->m_type == TokenTag::DOT)
		Log::fatal("found '.' used as a binary operator");
	
	auto ast = alloc.make<CallExpression>();

	auto identifier = alloc.make<Identifier>();
	identifier->m_text = cst->m_op_token->m_text;

	ast->m_callee = identifier;

	ast->m_args.push_back(convert_expr(cst->m_lhs, alloc));
	ast->m_args.push_back(convert_expr(cst->m_rhs, alloc));

	return ast;
}

static Declaration* convert(CST::PlainDeclaration* cst, Allocator& alloc) {
	auto ast = alloc.make<Declaration>();
	*ast = convert_declaration(cst, cst->m_data, alloc);
	return ast;
}

static Declaration* convert(CST::FuncDeclaration* cst, Allocator& alloc) {
	auto func_ast = alloc.make<FunctionLiteral>();
	func_ast->m_args = convert_args(cst->m_args, func_ast, alloc);
	func_ast->m_body = convert_expr(cst->m_body, alloc);

	auto ast = alloc.make<Declaration>();
	ast->m_cst = cst;
	ast->m_identifier = cst->identifier();
	ast->m_value = func_ast;

	return ast;
}

static Declaration* convert(CST::BlockFuncDeclaration* cst, Allocator& alloc) {
	auto func_ast = alloc.make<FunctionLiteral>();
	func_ast->m_args = convert_args(cst->m_args, func_ast, alloc);
	func_ast->m_body = convert_and_wrap_in_seq(cst->m_body, alloc);

	auto ast = alloc.make<Declaration>();
	ast->m_cst = cst;
	ast->m_identifier = cst->identifier();
	ast->m_value = func_ast;

	return ast;
}

static AST* convert(CST::Program* cst, Allocator& alloc) {
	auto ast = alloc.make<Program>();

	for (auto& declaration : cst->m_declarations) {
		auto decl = static_cast<Declaration*>(convert_ast(declaration, alloc));
		ast->m_declarations.push_back(std::move(*decl));
	}

	return ast;
}

static Identifier* convert(CST::Identifier* cst, Allocator& alloc) {
	auto ast = alloc.make<Identifier>();
	ast->m_cst = cst;
	ast->m_text = cst->m_token->m_text;
	return ast;
}

static CallExpression* convert(CST::CallExpression* cst, Allocator& alloc) {
	auto ast = alloc.make<CallExpression>();

	for (auto arg : cst->m_args) {
		ast->m_args.push_back(convert_expr(arg, alloc));
	}

	ast->m_callee = convert_expr(cst->m_callee, alloc);

	return ast;
}

static IndexExpression* convert(CST::IndexExpression* cst, Allocator& alloc) {
	auto ast = alloc.make<IndexExpression>();

	ast->m_callee = convert_expr(cst->m_callee, alloc);
	ast->m_index = convert_expr(cst->m_index, alloc);

	return ast;
}

static TernaryExpression* convert(CST::TernaryExpression* cst, Allocator& alloc) {
	auto ast = alloc.make<TernaryExpression>();

	ast->m_condition = convert_expr(cst->m_condition, alloc);
	ast->m_then_expr = convert_expr(cst->m_then_expr, alloc);
	ast->m_else_expr = convert_expr(cst->m_else_expr, alloc);

	return ast;
}

static AccessExpression* convert(CST::AccessExpression* cst, Allocator& alloc) {
	auto ast = alloc.make<AccessExpression>();

	ast->m_member = cst->m_member->m_text;
	ast->m_target = convert_expr(cst->m_record, alloc);

	return ast;
}

static MatchExpression* convert(CST::MatchExpression* cst, Allocator& alloc) {
	auto ast = alloc.make<MatchExpression>();

	auto matchee = static_cast<Identifier*>(convert_expr(&cst->m_matchee, alloc));
	ast->m_target = std::move(*matchee);

	if (cst->m_type_hint)
		ast->m_type_hint = convert_expr(cst->m_type_hint, alloc);

	std::unordered_map<InternedString, MatchExpression::CaseData> cases;
	for (auto& case_data : cst->m_cases) {
		auto case_name = case_data.m_name->m_text;

		Declaration declaration;
		// TODO: store match expression cst in declarations?
		declaration.m_identifier = case_data.m_identifier->m_text;

		if (case_data.m_type_hint)
			declaration.m_type_hint = convert_expr(case_data.m_type_hint, alloc);

		auto expression = convert_expr(case_data.m_expression, alloc);

		auto insertion_result = cases.insert(
		    {case_name,
		     MatchExpression::CaseData {std::move(declaration), expression}});

		if (!insertion_result.second) {
			// TODO: add location information
			Log::fatal() << "Duplicate case in match expression";
		}
	}

	ast->m_cases = std::move(cases);

	return ast;
}

static ConstructorExpression* convert(CST::ConstructorExpression* cst, Allocator& alloc) {
	auto ast = alloc.make<ConstructorExpression>();

	ast->m_constructor = convert_expr(cst->m_constructor, alloc);

	for (auto& arg : cst->m_args)
		ast->m_args.push_back(convert_expr(arg, alloc));

	return ast;
}

static SequenceExpression* convert(CST::SequenceExpression* cst, Allocator& alloc) {
	auto result = alloc.make<SequenceExpression>();
	result->m_body = static_cast<Block*>(convert_ast(cst->m_body, alloc));
	return result;
}

static Block* convert(CST::Block* cst, Allocator& alloc) {
	auto ast = alloc.make<Block>();

	for (auto element : cst->m_body) {
		ast->m_body.push_back(convert_ast(element, alloc));
	}

	return ast;
}

static ReturnStatement* convert(CST::ReturnStatement* cst, Allocator& alloc) {
	auto ast = alloc.make<ReturnStatement>();

	ast->m_value = convert_expr(cst->m_value, alloc);

	return ast;
}

static IfElseStatement* convert(CST::IfElseStatement* cst, Allocator& alloc) {
	auto ast = alloc.make<IfElseStatement>();

	ast->m_condition = convert_expr(cst->m_condition, alloc);
	ast->m_body = convert_ast(cst->m_body, alloc);

	if (cst->m_else_body)
		ast->m_else_body = convert_ast(cst->m_else_body, alloc);

	return ast;
}

/*
desugar
	for (x := e; p(x); x = f(x)) stmt;
to
	{ x := e; while (p(x)) { stmt; x = f(x); } }
*/
static Block* convert(CST::ForStatement* cst, Allocator& alloc) {

	auto body = convert_ast(cst->m_body, alloc);
	auto block_body = [&] {
		if (body->type() == ASTTag::Block)
			return static_cast<Block*>(body);
		auto inner_block = alloc.make<Block>();
		inner_block->m_body.push_back(body);
		return inner_block;
	}();

	auto action = convert_expr(cst->m_action, alloc);
	block_body->m_body.push_back(action);

	auto while_ast = alloc.make<WhileStatement>();
	while_ast->m_body = block_body;
	while_ast->m_condition = convert_expr(cst->m_condition, alloc);;

	auto outter_block_ast = alloc.make<Block>();
	auto decl = convert_declaration(nullptr, cst->m_declaration, alloc);
	auto heap_decl = alloc.make<Declaration>();
	*heap_decl = std::move(decl);
	outter_block_ast->m_body.push_back(heap_decl);
	outter_block_ast->m_body.push_back(while_ast);

	return outter_block_ast;
}

static WhileStatement* convert(CST::WhileStatement* cst, Allocator& alloc) {
	auto ast = alloc.make<WhileStatement>();

	ast->m_condition = convert_expr(cst->m_condition, alloc);
	ast->m_body = convert_ast(cst->m_body, alloc);

	return ast;
}

static UnionExpression* convert(CST::UnionExpression* cst, Allocator& alloc) {
	auto ast = alloc.make<UnionExpression>();

	for (auto& constructor : cst->m_constructors) {
		auto field_name = static_cast<CST::Identifier&>(constructor).text();
		ast->m_constructors.push_back(field_name);
	}

	for (auto type : cst->m_types) {
		ast->m_types.push_back(convert_expr(type, alloc));
	}

	return ast;
};

static StructExpression* convert(CST::StructExpression* cst, Allocator& alloc) {
	auto ast = alloc.make<StructExpression>();

	for (auto& cst_field : cst->m_fields) {
		auto field_name = static_cast<CST::Identifier&>(cst_field).text();
		ast->m_fields.push_back(field_name);
	}

	for (auto type : cst->m_types) {
		ast->m_types.push_back(convert_expr(type, alloc));
	}

	return ast;
};

static TypeTerm* convert(CST::TypeTerm* cst, Allocator& alloc) {
	auto ast = alloc.make<TypeTerm>();

	ast->m_callee = convert_expr(cst->m_callee, alloc);
	for (auto arg : cst->m_args){
		ast->m_args.push_back(convert_expr(arg, alloc));
	}

	return ast;
}

static Expr* convert_expr(CST::CST* cst, Allocator& alloc) {
#define DISPATCH(type)                                                         \
	case CSTTag::type:                                                         \
		return convert(static_cast<CST::type*>(cst), alloc)

	switch (cst->type()) {
		DISPATCH(NumberLiteral);
		DISPATCH(IntegerLiteral);
		DISPATCH(StringLiteral);
		DISPATCH(BooleanLiteral);
		DISPATCH(NullLiteral);
		DISPATCH(ArrayLiteral);
		DISPATCH(FunctionLiteral);
		DISPATCH(BlockFunctionLiteral);

		DISPATCH(Identifier);
		DISPATCH(CallExpression);
		DISPATCH(IndexExpression);
		DISPATCH(TernaryExpression);
		DISPATCH(AccessExpression);
		DISPATCH(MatchExpression);
		DISPATCH(ConstructorExpression);
		DISPATCH(SequenceExpression);
		DISPATCH(BinaryExpression);

		DISPATCH(UnionExpression);
		DISPATCH(StructExpression);
		DISPATCH(TypeTerm);
	}

	Log::fatal() << "(internal) CST type not handled in convert_ast: "
	             << cst_string[(int)cst->type()];

#undef DISPATCH
}

static AST* convert_stmt(CST::CST* cst, Allocator& alloc) {
#define DISPATCH(type)                                                         \
	case CSTTag::type:                                                         \
		return convert(static_cast<CST::type*>(cst), alloc)

	switch (cst->type()) {

		DISPATCH(Block);
		DISPATCH(ReturnStatement);
		DISPATCH(IfElseStatement);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);

		DISPATCH(PlainDeclaration);
		DISPATCH(FuncDeclaration);
		DISPATCH(BlockFuncDeclaration);

	default:
		return convert_expr(cst, alloc);
	}

#undef DISPATCH
}

AST* convert_ast(CST::CST* cst, Allocator& alloc) {
#define DISPATCH(type)                                                         \
	case CSTTag::type:                                                         \
		return convert(static_cast<CST::type*>(cst), alloc)

	switch (cst->type()) {
		DISPATCH(Program);
	default:
		return convert_stmt(cst, alloc);
	}

#undef DISPATCH
}

} // namespace AST
