#include "pretty_print.hpp"

#include "log/log.hpp"

#include <sstream>

namespace CST {

static void pp(const CST* cst, std::ostream& out);

static void pp(const NumberLiteral* cst, std::ostream& out) {
	if (cst->m_sign)
		out << cst->m_sign;
	out << cst->m_token;
}

static void pp(const IntegerLiteral* cst, std::ostream& out) {
	if (cst->m_sign)
		out << cst->m_sign;
	out << cst->m_token;
}

static void pp(const StringLiteral* cst, std::ostream& out) {
	out << cst->m_token;
}

static void pp(const BooleanLiteral* cst, std::ostream& out) {
	out << cst->m_token;
}

static void pp(const NullLiteral* cst, std::ostream& out) {
	out << "null";
}

static void comma_separated_values(
    const std::vector<Expr*>& elements, std::ostream& out) {
	for (size_t i = 0; i < elements.size(); ++i) {
		if (i > 0)
			out << ", ";
		out << elements[i];
	}
}

static void pp(const ArrayLiteral* cst, std::ostream& out) {
	// TODO: wrap around?
	out << "[";
	comma_separated_values(cst->m_elements, out);
	out << "]";
}

static void pp_decl_data(DeclarationData dd, std::ostream& out) {
	out << dd.m_identifier_token;
	if (dd.m_type_hint)
		out << ": ";
	pp(dd.m_type_hint, out);
	if (dd.m_value)
		out << " = ";
	pp(dd.m_value, out);
	out << ";";
}

static void comma_separated_args(
    const std::vector<DeclarationData>& args, std::ostream& out) {
	for (size_t i = 0; i < args.size(); ++i) {
		if (i > 0)
			out << ", ";
		pp_decl_data(args[i], out);
	}
}

static void pp_statements(const std::vector<Stmt*>& stmts, std::ostream& out) {
	for (const auto* stmt : stmts)
		pp(stmt, out);
}

static void pp(const FunctionLiteral* cst, std::ostream& out) {
	// TODO: wrap around?
	out << "fn (";
	comma_separated_args(cst->m_args, out);
	out << ") => ";
	pp(cst->m_body, out);
}

static void pp(const BlockFunctionLiteral* cst, std::ostream& out) {
	out << "fn (";
	comma_separated_args(cst->m_args, out);
	out << ") ";
	pp(cst->m_body, out);
}

static void pp(const Identifier* cst, std::ostream& out) {
	out << cst->m_token;
}

static void pp(const BinaryExpression* cst, std::ostream& out) {
	pp(cst->m_lhs, out);
	out << " " << cst->m_op_token << " ";
	pp(cst->m_rhs, out);
}

static void pp(const IndexExpression* cst, std::ostream& out) {
	pp(cst->m_callee, out);
	out << "[";
	pp(cst->m_index, out);
	out << "]";
}

static void pp(const CallExpression* cst, std::ostream& out) {
	pp(cst->m_callee, out);
	out << "(";
	comma_separated_values(cst->m_args, out);
	out << ")";
}

static void pp(const TernaryExpression* cst, std::ostream& out) {
	// TODO: best way to print this?
	out << "if";
	pp(cst->m_condition, out);
	out << "then";
	pp(cst->m_then_expr, out);
	out << "else";
	pp(cst->m_else_expr, out);
}

static void pp(const AccessExpression* cst, std::ostream& out) {
	pp(cst->m_record, out);
	out << "." << cst->m_member;
}

static void pp(const MatchExpression* cst, std::ostream& out) {
	out << "match (" << cst->m_matchee.m_token;
	if (cst->m_type_hint)
		out << ": " << cst->m_type_hint;
	out << ") {";
	for (const auto m_case : cst->m_cases) {
		out << m_case.m_name << " { " << m_case.m_identifier;
		if (m_case.m_type_hint)
			out << ": " << m_case.m_type_hint;
		out << " } =>";
		pp(m_case.m_expression, out);
		out << ";\n";
	}
}

static void pp(const ConstructorExpression* cst, std::ostream& out) {
	pp(cst->m_constructor, out);
	out << " { ";
	comma_separated_values(cst->m_args, out);
	out << " }";
}

static void pp(const SequenceExpression* cst, std::ostream& out) {
	out << "seq";
	pp(cst->m_body, out);
}

static void pp(const PlainDeclaration* cst, std::ostream& out) {
	pp_decl_data(cst->m_data, out);
}

static void pp(const FuncDeclaration* cst, std::ostream& out) {
	out << "fn " << cst->m_identifier << "(";
	comma_separated_args(cst->m_args, out);
	out << ") => ";
	pp(cst->m_body, out);
}

static void pp(const BlockFuncDeclaration* cst, std::ostream& out) {
	out << "fn " << cst->m_identifier << "(";
	comma_separated_args(cst->m_args, out);
	out << ") ";
	pp(cst->m_body, out);
}

static void pp(const Block* cst, std::ostream& out) {
	out << "{\n";
	pp_statements(cst->m_body, out);
	out << "}\n";
}

static void pp(const ReturnStatement* cst, std::ostream& out) {
	out << "return ";
	pp(cst->m_value, out);
	out << ";";
}

static void pp(const IfElseStatement* cst, std::ostream& out) {
	out << "if (";
	pp(cst->m_condition, out);
	out << ") ";
	pp(cst->m_body, out);
	if (cst->m_else_body) {
		out << " else ";
		pp(cst->m_else_body, out);
	}
}

static void pp(const ForStatement* cst, std::ostream& out) {
	out << "for (";
	pp_decl_data(cst->m_declaration, out);
	out << " ";
	pp(cst->m_condition, out);
	out << "; ";
	pp(cst->m_action, out);
	out << ") ";
	pp(cst->m_body, out);
}

static void pp(const WhileStatement* cst, std::ostream& out) {
	out << "while (";
	pp(cst->m_condition, out);
	out << ") ";
	pp(cst->m_body, out);
}

static void pp(const ExpressionStatement* cst, std::ostream& out) {
	pp(cst->m_expression, out);
	out << ";";
}

static void pp(const TypeTerm* cst, std::ostream& out) {
	pp(cst->m_callee, out);
	if (!cst->m_args.empty()) {
		out << "<";
		comma_separated_values(cst->m_args, out);
		out << ">";
	}
}

static void pp(const TypeVar* cst, std::ostream& out) {
	out << cst->m_token;
}

static void comma_separated_identifiers(
    const std::vector<Identifier>& identifiers, std::ostream& out) {
	for (size_t i = 0; i < identifiers.size(); ++i) {
		if (i > 0)
			out << ", ";
		out << identifiers[i].m_token;
	}
}

static void pp(const UnionExpression* cst, std::ostream& out) {
	out << "union <";
	comma_separated_identifiers(cst->m_constructors, out);
	out << "> { ";
	comma_separated_values(cst->m_types, out);
	out << " }";
}

static void pp(const StructExpression* cst, std::ostream& out) {
	out << "struct <";
	comma_separated_identifiers(cst->m_fields, out);
	out << "> { ";
	comma_separated_values(cst->m_types, out);
	out << " }";
}

static void pp(const Program* cst, std::ostream& out) {
	for (const auto* decl : cst->m_declarations) {
		pp(decl, out);
		out << "\n";
	}
}

static void pp(const CST* cst, std::ostream& out) {
#define DISPATCH(type)                                                         \
	case CSTTag::type:                                                         \
		return pp(static_cast<const type*>(cst), out);

	switch (cst->type()) {
		// Literals
		DISPATCH(NumberLiteral);
		DISPATCH(IntegerLiteral);
		DISPATCH(StringLiteral);
		DISPATCH(BooleanLiteral);
		DISPATCH(NullLiteral);
		DISPATCH(ArrayLiteral);
		DISPATCH(FunctionLiteral);
		DISPATCH(BlockFunctionLiteral);

		// Expressions
		DISPATCH(Identifier);
		DISPATCH(BinaryExpression);
		DISPATCH(IndexExpression);
		DISPATCH(CallExpression);
		DISPATCH(TernaryExpression);
		DISPATCH(AccessExpression);
		DISPATCH(MatchExpression);
		DISPATCH(ConstructorExpression);
		DISPATCH(SequenceExpression);

		// Statements
		DISPATCH(PlainDeclaration);
		DISPATCH(FuncDeclaration);
		DISPATCH(BlockFuncDeclaration);
		DISPATCH(Block);
		DISPATCH(ReturnStatement);
		DISPATCH(IfElseStatement);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);
		DISPATCH(ExpressionStatement);

		// Types
		DISPATCH(TypeTerm);
		DISPATCH(TypeVar);
		DISPATCH(UnionExpression);
		DISPATCH(StructExpression);

		// Special
		DISPATCH(Program);
	}

#undef DISPATCH

	assert(false && "Unknown CST node type in pretty printer");
}

void pretty_print(const CST* cst, std::ostream& out) {
	pp(cst, out);
}

std::string pretty_print(CST* cst) {
	std::stringstream s;
	pp(cst, s);
	return s.str();
}

} // namespace CST