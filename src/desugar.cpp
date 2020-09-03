#include "desugar.hpp"

#include "ast.hpp"

#include <cassert>
#include <iostream>

namespace AST {

template <typename T>
using Own = std::unique_ptr<T>;

Own<AST> desugar(Own<ObjectLiteral> ast) {
	// TODO: convert
	// obt { x : t1 = e1; y : t2 = e2; }
	// into
	// struct { x : t1; y : t2; } { e1; e2; }
	for (int i = 0; i < ast->m_body.size(); ++i)
		ast->m_body[i] = desugar(std::move(ast->m_body[i]));

	return ast;
}

Own<AST> desugar(Own<ArrayLiteral> ast) {
	for (int i = 0; i < ast->m_elements.size(); ++i)
		ast->m_elements[i] = desugar(std::move(ast->m_elements[i]));

	return ast;
}

Own<AST> desugar(Own<DictionaryLiteral> ast) {
	for (int i = 0; i < ast->m_body.size(); ++i)
		ast->m_body[i] = desugar(std::move(ast->m_body[i]));

	return ast;
}

Own<AST> desugar(Own<FunctionLiteral> ast) {
	for (auto&& arg : ast->m_args)
		arg = desugar(std::move(arg));

	ast->m_body = desugar(std::move(ast->m_body));

	return ast;
}

Own<AST> desugar(Own<DeclarationList> ast) {
	for (auto&& declaration : ast->m_declarations)
		declaration = desugar(std::move(declaration));

	return ast;
}

Own<AST> desugar(Own<Declaration> ast) {
	// TODO: handle type hint
	if (ast->m_value)
		ast->m_value = desugar(std::move(ast->m_value));

	return ast;
}

Own<AST> desugarPizza(Own<BinaryExpression> ast) {
	// TODO: error handling
	assert(ast->m_rhs->type() == ast_type::CallExpression);

	auto rhs = std::move(ast->m_rhs);
	auto call = static_cast<CallExpression*>(rhs.get());

	call->m_args.insert(call->m_args.begin(), desugar(std::move(ast->m_lhs)));

	return rhs;
}

Own<AST> desugarDot(Own<BinaryExpression> ast) {
	// TODO: error handling
	// TODO: move this check to the parser
	assert(ast->m_rhs->type() == ast_type::Identifier);
	auto ident = static_cast<Identifier*>(ast->m_rhs.get());

	auto tok = ast->m_op_token;
	std::cerr << "Error: @" << tok->m_line0 + 1 << ":" << tok->m_col0
	          << " | Dot (.) operator not implemented yet\n";

	return std::make_unique<NullLiteral>();
}

// This function desugars binary operators into function calls
Own<AST> desugar(Own<BinaryExpression> ast) {

	if (ast->m_op_token->m_type == token_type::PIZZA)
		return desugarPizza(std::move(ast));

	if (ast->m_op_token->m_type == token_type::DOT)
		return desugarDot(std::move(ast));

	auto identifier = std::make_unique<Identifier>();
	identifier->m_token = ast->m_op_token;

	auto result = std::make_unique<CallExpression>();
	result->m_callee = std::move(identifier);

	result->m_args.push_back(desugar(std::move(ast->m_lhs)));
	result->m_args.push_back(desugar(std::move(ast->m_rhs)));

	return result;
}

Own<AST> desugar(Own<CallExpression> ast) {
	for (auto&& arg : ast->m_args) {
		arg = desugar(std::move(arg));
	}

	ast->m_callee = desugar(std::move(ast->m_callee));

	return ast;
}

Own<AST> desugar(Own<IndexExpression> ast) {
	ast->m_callee = desugar(std::move(ast->m_callee));
	ast->m_index = desugar(std::move(ast->m_index));

	return ast;
}

Own<AST> desugar(Own<Block> ast) {
	for (auto&& element : ast->m_body) {
		element = desugar(std::move(element));
	}

	return ast;
}

Own<AST> desugar(Own<ReturnStatement> ast) {
	ast->m_value = desugar(std::move(ast->m_value));

	return ast;
}

Own<AST> desugar(Own<IfElseStatement> ast) {
	ast->m_condition = desugar(std::move(ast->m_condition));
	ast->m_body = desugar(std::move(ast->m_body));

	if (ast->m_else_body)
		ast->m_else_body = desugar(std::move(ast->m_else_body));

	return ast;
}

Own<AST> desugar(Own<ForStatement> ast) {
	ast->m_declaration = desugar(std::move(ast->m_declaration));
	ast->m_condition = desugar(std::move(ast->m_condition));
	ast->m_action = desugar(std::move(ast->m_action));
	ast->m_body = desugar(std::move(ast->m_body));

	return ast;
}

Own<AST> desugar(Own<WhileStatement> ast) {
	ast->m_condition = desugar(std::move(ast->m_condition));
	ast->m_body = desugar(std::move(ast->m_body));

	return ast;
}

Own<AST> desugar(Own<AST> ast) {
#define DISPATCH(type)                                                         \
	case ast_type::type:                                                       \
		return desugar(Own<type>(static_cast<type*>(ast.release())));

#define RETURN(type)                                                           \
	case ast_type::type:                                                       \
		return ast;

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
		DISPATCH(DeclarationList);
		DISPATCH(Declaration);
		RETURN(Identifier);
		DISPATCH(BinaryExpression);
		DISPATCH(CallExpression);
		DISPATCH(IndexExpression);
		DISPATCH(Block);
		DISPATCH(ReturnStatement);
		DISPATCH(IfElseStatement);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);
		RETURN(TypeTerm);
	}
	std::cerr << "Error: AST type not handled in desugar: "
	          << ast_type_string[(int)ast->type()] << std::endl;
	assert(0);

#undef RETURN
#undef DISPATCH
}

} // namespace AST
