#pragma once

#include "ast.hpp"

#include <vector>

namespace AST {

struct ASTHandler {
	struct Node {
		ASTTag tag;
		int index;
	};

	std::vector<DeclarationList> m_declaration_lists;
	std::vector<Declaration> m_declarations;

	// potential for reuse
	std::vector<IntegerLiteral> m_integers;
	std::vector<NumberLiteral> m_numbers;
	std::vector<StringLiteral> m_strings;
	std::vector<BooleanLiteral> m_booleans;
	std::vector<NullLiteral> m_nulls;
	std::vector<ObjectLiteral> m_objects;
	std::vector<ArrayLiteral> m_arrays;
	std::vector<DictionaryLiteral> m_dictionaries;
	std::vector<FunctionLiteral> m_functions;
	std::vector<ShortFunctionLiteral> m_short_functions;

	std::vector<Identifier> m_identifiers;
	std::vector<BinaryExpression> m_bps;
	std::vector<CallExpression> m_calls;
	std::vector<IndexExpression> m_indexes;
	std::vector<RecordAccessExpression> m_record_accesses;
	std::vector<TernaryExpression> m_ternaries;

	std::vector<Block> m_blocks;
	std::vector<ReturnStatement> m_returns;
	std::vector<IfElseStatement> m_conditionals;
	std::vector<ForStatement> m_fors;
	std::vector<WhileStatement> m_whiles;

	std::vector<TypeTerm> m_terms;
	std::vector<TypeVar> m_vars;
	std::vector<UnionExpression> m_unions;
	std::vector<TupleExpression> m_tuples;
	std::vector<StructExpression> m_structs;

	// TODO: is overload of [] possible or better?
	AST* access(ASTTag tag, int index);
	AST* access(const Node& node);

	Node create(ASTTag tag);
};

} // namespace AST
