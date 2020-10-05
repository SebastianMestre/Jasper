#include "ast_allocator.hpp"

#include <cassert>

namespace AST {

// TODO: is asserting here useful?
#define FETCH(tag, m_tag) \
	case ASTTag::tag: \
		assert(int(m_tag.size()) > index and "accessing invalid memory"); \
		return &m_tag[index];

AST* ASTAllocator::access(ASTTag tag, int index) {
	switch(tag) {
		FETCH(DeclarationList, m_declaration_lists)
		FETCH(Declaration, m_declarations)

		FETCH(IntegerLiteral, m_integers)
		FETCH(NumberLiteral, m_numbers)
		FETCH(StringLiteral, m_strings)
		FETCH(BooleanLiteral, m_booleans)
		FETCH(NullLiteral, m_nulls)
		FETCH(ObjectLiteral, m_objects)
		FETCH(ArrayLiteral, m_arrays)
		FETCH(DictionaryLiteral, m_dictionaries)
		FETCH(FunctionLiteral, m_functions)
		FETCH(ShortFunctionLiteral, m_short_functions)

		FETCH(Identifier, m_identifiers)
		FETCH(BinaryExpression, m_bps)
		FETCH(CallExpression, m_calls)
		FETCH(IndexExpression, m_indexes)
		FETCH(RecordAccessExpression, m_record_accesses)
		FETCH(TernaryExpression, m_ternaries)

		FETCH(Block, m_blocks)
		FETCH(ReturnStatement, m_returns)
		FETCH(IfElseStatement, m_conditionals)
		FETCH(ForStatement, m_fors)
		FETCH(WhileStatement, m_whiles)

		FETCH(TypeTerm, m_terms)
		FETCH(TypeVar, m_vars)
		FETCH(UnionExpression, m_unions)
		FETCH(TupleExpression, m_tuples)
		FETCH(StructExpression, m_structs)
	}

	assert(0 and "unknown ast tag");
}

#undef FETCH

AST* ASTAllocator::access(const ASTAllocator::Node& node) {
	return access(node.tag, node.index);
}

#define PUSH(tag, m_tag) \
	case ASTTag::tag: \
		node.index = m_tag.size(); \
		m_tag.push_back({}); \
		return node;

ASTAllocator::Node ASTAllocator::create(ASTTag tag) {
	ASTAllocator::Node node {tag};
	switch(tag) {
		PUSH(DeclarationList, m_declaration_lists)
		PUSH(Declaration, m_declarations)

		PUSH(IntegerLiteral, m_integers)
		PUSH(NumberLiteral, m_numbers)
		PUSH(StringLiteral, m_strings)
		PUSH(BooleanLiteral, m_booleans)
		PUSH(NullLiteral, m_nulls)
		PUSH(ObjectLiteral, m_objects)
		PUSH(ArrayLiteral, m_arrays)
		PUSH(DictionaryLiteral, m_dictionaries)
		PUSH(FunctionLiteral, m_functions)
		PUSH(ShortFunctionLiteral, m_short_functions)

		PUSH(Identifier, m_identifiers)
		PUSH(BinaryExpression, m_bps)
		PUSH(CallExpression, m_calls)
		PUSH(IndexExpression, m_indexes)
		PUSH(RecordAccessExpression, m_record_accesses)
		PUSH(TernaryExpression, m_ternaries)

		PUSH(Block, m_blocks)
		PUSH(ReturnStatement, m_returns)
		PUSH(IfElseStatement, m_conditionals)
		PUSH(ForStatement, m_fors)
		PUSH(WhileStatement, m_whiles)

		PUSH(TypeTerm, m_terms)
		PUSH(TypeVar, m_vars)
		PUSH(UnionExpression, m_unions)
		PUSH(TupleExpression, m_tuples)
		PUSH(StructExpression, m_structs)
	}

	assert(0 and "unknown ast tag");
}

#undef PUSH

} // namespace AST
