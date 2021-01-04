#include "desugar.hpp"

#include "ast.hpp"
#include "ast_allocator.hpp"

#include <cassert>
#include <iostream>

namespace AST {

Declaration* desugar(Declaration* ast, Allocator& alloc) {
	if (ast->m_type_hint)
		ast->m_type_hint = desugar(ast->m_type_hint, alloc);

	if (ast->m_value)
		ast->m_value = desugar(ast->m_value, alloc);

	return ast;
}

AST* desugar(DeclarationList* ast, Allocator& alloc) {
	for (auto& declaration : ast->m_declarations)
		desugar(&declaration, alloc);

	return ast;
}

AST* desugar(ObjectLiteral* ast, Allocator& alloc) {
	// TODO: convert
	// obt { x : t1 = e1; y : t2 = e2; }
	// into
	// struct { x : t1; y : t2; } { e1; e2; }
	for (int i = 0; i < ast->m_body.size(); ++i)
		desugar(&ast->m_body[i], alloc);

	return ast;
}

AST* desugar(ArrayLiteral* ast, Allocator& alloc) {
	for (int i = 0; i < ast->m_elements.size(); ++i)
		ast->m_elements[i] = desugar(ast->m_elements[i], alloc);

	return ast;
}

AST* desugar(DictionaryLiteral* ast, Allocator& alloc) {
	for (int i = 0; i < ast->m_body.size(); ++i)
		desugar(&ast->m_body[i], alloc);

	return ast;
}

AST* desugar(FunctionLiteral* ast, Allocator& alloc) {
	for (auto& arg : ast->m_args)
		desugar(&arg, alloc);

	ast->m_body = desugar(ast->m_body, alloc);

	return ast;
}

AST* desugar(ShortFunctionLiteral* ast, Allocator& alloc) {
	auto return_stmt = alloc.make<ReturnStatement>();
	return_stmt->m_value = desugar(ast->m_body, alloc);

	auto block = alloc.make<Block>();
	block->m_body.push_back(return_stmt);

	auto func = alloc.make<FunctionLiteral>();
	func->m_args = std::move(ast->m_args);

	for (auto& arg : func->m_args)
		desugar(&arg, alloc);

	func->m_body = block;

	return func;
}

AST* desugarPizza(BinaryExpression* ast, Allocator& alloc) {
	// TODO: error handling
	assert(ast->m_rhs->type() == ASTTag::CallExpression);

	auto rhs = desugar(ast->m_rhs, alloc);
	auto call = static_cast<CallExpression*>(rhs);

	call->m_args.insert(call->m_args.begin(), desugar(ast->m_lhs, alloc));

	return rhs;
}

AST* desugar(AccessExpression* ast, Allocator& alloc) {
	ast->m_record = desugar(ast->m_record, alloc);
	return ast;
}

// This function desugars binary operators into function calls
AST* desugar(BinaryExpression* ast, Allocator& alloc) {

	if (ast->m_op_token->m_type == TokenTag::PIZZA)
		return desugarPizza(ast, alloc);

	if (ast->m_op_token->m_type == TokenTag::DOT)
		assert(0);

	auto identifier = alloc.make<Identifier>();
	identifier->m_token = ast->m_op_token;

	auto result = alloc.make<CallExpression>();
	result->m_callee = identifier;

	result->m_args.push_back(desugar(ast->m_lhs, alloc));
	result->m_args.push_back(desugar(ast->m_rhs, alloc));

	return result;
}

AST* desugar(CallExpression* ast, Allocator& alloc) {
	for (auto& arg : ast->m_args) {
		arg = desugar(arg, alloc);
	}

	ast->m_callee = desugar(ast->m_callee, alloc);

	return ast;
}

AST* desugar(IndexExpression* ast, Allocator& alloc) {
	ast->m_callee = desugar(ast->m_callee, alloc);
	ast->m_index = desugar(ast->m_index, alloc);

	return ast;
}

AST* desugar(TernaryExpression* ast, Allocator& alloc) {
	ast->m_condition = desugar(ast->m_condition, alloc);
	ast->m_then_expr = desugar(ast->m_then_expr, alloc);
	ast->m_else_expr = desugar(ast->m_else_expr, alloc);

	return ast;
}

AST* desugar(ConstructorExpression* ast, Allocator& alloc) {
	ast->m_constructor = desugar(ast->m_constructor, alloc);
	for (auto& arg : ast->m_args)
		arg = desugar(arg, alloc);

	return ast;
}

AST* desugar(Block* ast, Allocator& alloc) {
	for (auto& element : ast->m_body) {
		element = desugar(element, alloc);
	}

	return ast;
}

AST* desugar(ReturnStatement* ast, Allocator& alloc) {
	ast->m_value = desugar(ast->m_value, alloc);

	return ast;
}

AST* desugar(IfElseStatement* ast, Allocator& alloc) {
	ast->m_condition = desugar(ast->m_condition, alloc);
	ast->m_body = desugar(ast->m_body, alloc);

	if (ast->m_else_body)
		ast->m_else_body = desugar(ast->m_else_body, alloc);

	return ast;
}

AST* desugar(ForStatement* ast, Allocator& alloc) {
	desugar(&ast->m_declaration, alloc);
	ast->m_condition = desugar(ast->m_condition, alloc);
	ast->m_action = desugar(ast->m_action, alloc);
	ast->m_body = desugar(ast->m_body, alloc);

	return ast;
}

AST* desugar(WhileStatement* ast, Allocator& alloc) {
	ast->m_condition = desugar(ast->m_condition, alloc);
	ast->m_body = desugar(ast->m_body, alloc);

	return ast;
}

AST* desugar(MatchExpression* ast, Allocator& alloc) {
	if (ast->m_type_hint)
		ast->m_type_hint = desugar(ast->m_type_hint, alloc);

	for (auto& case_data : ast->m_cases) {
		case_data.m_expression = desugar(case_data.m_expression, alloc);
		if (case_data.m_type_hint)
			case_data.m_type_hint = desugar(case_data.m_type_hint, alloc);
	}

	return ast;
}

AST* desugar(SequenceExpression* ast, Allocator& alloc) {
	ast->m_body = static_cast<Block*>(desugar(ast->m_body, alloc));
	return ast;
}

AST* desugar(AST* ast, Allocator& alloc) {
#define DISPATCH(type)                                                         \
	case ASTTag::type:                                                         \
		return desugar(static_cast<type*>(ast), alloc);

#define RETURN(type)                                                           \
	case ASTTag::type:                                                         \
		return ast;

#define REJECT(type)                                                           \
	case ASTTag::type:                                                         \
		assert(0);
	
	switch (ast->type()) {
		RETURN(NumberLiteral);
		RETURN(IntegerLiteral);
		RETURN(StringLiteral);
		RETURN(BooleanLiteral);
		RETURN(NullLiteral);
		DISPATCH(ObjectLiteral);
		DISPATCH(ArrayLiteral);
		DISPATCH(DictionaryLiteral);
		DISPATCH(FunctionLiteral);
		DISPATCH(ShortFunctionLiteral);

		RETURN(Identifier);
		DISPATCH(BinaryExpression);
		DISPATCH(TernaryExpression);
		DISPATCH(CallExpression);
		DISPATCH(IndexExpression);
		DISPATCH(AccessExpression);
		DISPATCH(ConstructorExpression);

		DISPATCH(DeclarationList);
		DISPATCH(Declaration);

		DISPATCH(Block);
		DISPATCH(ReturnStatement);
		DISPATCH(IfElseStatement);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);
		DISPATCH(MatchExpression);
		DISPATCH(SequenceExpression);

		RETURN(TypeTerm);
		RETURN(TypeVar);
		RETURN(UnionExpression);
		RETURN(StructExpression);
		RETURN(TupleExpression);
	}
	std::cerr << "Error: AST type not handled in desugar: "
	          << ast_string[(int)ast->type()] << std::endl;
	assert(0);

#undef RETURN
#undef DISPATCH
#undef REJECT
}

} // namespace AST
