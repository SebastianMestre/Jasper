#include "desugar.hpp"

#include "./log/log.hpp"
#include "cst.hpp"
#include "cst_allocator.hpp"

#include <cassert>
#include <sstream>

namespace CST {

Declaration* desugar(Declaration* cst, Allocator& alloc) {
	if (cst->m_type_hint)
		cst->m_type_hint = desugar(cst->m_type_hint, alloc);

	if (cst->m_value)
		cst->m_value = desugar(cst->m_value, alloc);

	return cst;
}

CST* desugar(DeclarationList* cst, Allocator& alloc) {
	for (auto& declaration : cst->m_declarations)
		desugar(&declaration, alloc);

	return cst;
}

CST* desugar(ArrayLiteral* cst, Allocator& alloc) {
	for (int i = 0; i < cst->m_elements.size(); ++i)
		cst->m_elements[i] = desugar(cst->m_elements[i], alloc);

	return cst;
}

CST* desugar(FunctionLiteral* cst, Allocator& alloc) {
	cst->m_body = desugar(cst->m_body, alloc);

	for (auto& arg : cst->m_args)
		desugar(&arg, alloc);

	return cst;
}

CST* desugar(BlockFunctionLiteral* cst, Allocator& alloc) {
	auto seq_expr = alloc.make<SequenceExpression>();
	seq_expr->m_body = static_cast<Block*>(cst->m_body);
	cst->m_body = nullptr;

	auto result = alloc.make<FunctionLiteral>();
	result->m_body = seq_expr;
	result->m_args = std::move(cst->m_args);

	return desugar(result, alloc);
}

CST* desugarPizza(BinaryExpression* cst, Allocator& alloc) {
	// TODO: error handling
	assert(cst->m_rhs->type() == CSTTag::CallExpression);

	auto rhs = desugar(cst->m_rhs, alloc);
	auto call = static_cast<CallExpression*>(rhs);

	call->m_args.insert(call->m_args.begin(), desugar(cst->m_lhs, alloc));

	return rhs;
}

CST* desugar(AccessExpression* cst, Allocator& alloc) {
	cst->m_record = desugar(cst->m_record, alloc);
	return cst;
}

// This function desugars binary operators into function calls
CST* desugar(BinaryExpression* cst, Allocator& alloc) {

	if (cst->m_op_token->m_type == TokenTag::PIZZA)
		return desugarPizza(cst, alloc);

	if (cst->m_op_token->m_type == TokenTag::DOT)
		assert(0);

	auto identifier = alloc.make<Identifier>();
	identifier->m_token = cst->m_op_token;

	auto result = alloc.make<CallExpression>();
	result->m_callee = identifier;

	result->m_args.push_back(desugar(cst->m_lhs, alloc));
	result->m_args.push_back(desugar(cst->m_rhs, alloc));

	return result;
}

CST* desugar(CallExpression* cst, Allocator& alloc) {
	for (auto& arg : cst->m_args) {
		arg = desugar(arg, alloc);
	}

	cst->m_callee = desugar(cst->m_callee, alloc);

	return cst;
}

CST* desugar(IndexExpression* cst, Allocator& alloc) {
	cst->m_callee = desugar(cst->m_callee, alloc);
	cst->m_index = desugar(cst->m_index, alloc);

	return cst;
}

CST* desugar(TernaryExpression* cst, Allocator& alloc) {
	cst->m_condition = desugar(cst->m_condition, alloc);
	cst->m_then_expr = desugar(cst->m_then_expr, alloc);
	cst->m_else_expr = desugar(cst->m_else_expr, alloc);

	return cst;
}

CST* desugar(ConstructorExpression* cst, Allocator& alloc) {
	cst->m_constructor = desugar(cst->m_constructor, alloc);
	for (auto& arg : cst->m_args)
		arg = desugar(arg, alloc);

	return cst;
}

CST* desugar(Block* cst, Allocator& alloc) {
	for (auto& element : cst->m_body) {
		element = desugar(element, alloc);
	}

	return cst;
}

CST* desugar(ReturnStatement* cst, Allocator& alloc) {
	cst->m_value = desugar(cst->m_value, alloc);

	return cst;
}

CST* desugar(IfElseStatement* cst, Allocator& alloc) {
	cst->m_condition = desugar(cst->m_condition, alloc);
	cst->m_body = desugar(cst->m_body, alloc);

	if (cst->m_else_body)
		cst->m_else_body = desugar(cst->m_else_body, alloc);

	return cst;
}

CST* desugar(ForStatement* cst, Allocator& alloc) {
	desugar(&cst->m_declaration, alloc);
	cst->m_condition = desugar(cst->m_condition, alloc);
	cst->m_action = desugar(cst->m_action, alloc);
	cst->m_body = desugar(cst->m_body, alloc);

	return cst;
}

CST* desugar(WhileStatement* cst, Allocator& alloc) {
	cst->m_condition = desugar(cst->m_condition, alloc);
	cst->m_body = desugar(cst->m_body, alloc);

	return cst;
}

CST* desugar(MatchExpression* cst, Allocator& alloc) {
	if (cst->m_type_hint)
		cst->m_type_hint = desugar(cst->m_type_hint, alloc);

	for (auto& case_data : cst->m_cases) {
		case_data.m_expression = desugar(case_data.m_expression, alloc);
		if (case_data.m_type_hint)
			case_data.m_type_hint = desugar(case_data.m_type_hint, alloc);
	}

	return cst;
}

CST* desugar(SequenceExpression* cst, Allocator& alloc) {
	cst->m_body = static_cast<Block*>(desugar(cst->m_body, alloc));
	return cst;
}

CST* desugar(CST* cst, Allocator& alloc) {
#define DISPATCH(type)                                                         \
	case CSTTag::type:                                                         \
		return desugar(static_cast<type*>(cst), alloc);

#define RETURN(type)                                                           \
	case CSTTag::type:                                                         \
		return cst;

#define REJECT(type)                                                           \
	case CSTTag::type:                                                         \
		assert(0);
	
	switch (cst->type()) {
		RETURN(NumberLiteral);
		RETURN(IntegerLiteral);
		RETURN(StringLiteral);
		RETURN(BooleanLiteral);
		RETURN(NullLiteral);
		DISPATCH(ArrayLiteral);
		DISPATCH(BlockFunctionLiteral);
		DISPATCH(FunctionLiteral);

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

	Log::fatal() << "(internal) CST type not handled in desugar: " << cst_string[(int)cst->type()];

#undef RETURN
#undef DISPATCH
#undef REJECT
}

} // namespace CST
