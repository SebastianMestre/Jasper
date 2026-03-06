#include "pretty_print.hpp"

#include "log/log.hpp"

#include <sstream>

namespace CST {

static void tabs(std::ostream& out, size_t count) {
	for (size_t i = 0; i < count; ++i) {
		out << "\t";
	}
}

static void pp(const CST* cst, std::ostream& out, size_t indent = 0);

static void pp(const NumberLiteral* cst, std::ostream& out, size_t indent) {
	if (cst->m_sign) {
		out << cst->m_sign;
	}
	out << cst->m_token->m_text;
}

static void pp(const IntegerLiteral* cst, std::ostream& out, size_t indent) {
	if (cst->m_sign) {
		out << cst->m_sign;
	}
	out << cst->m_token->m_text;
}

static void pp(const StringLiteral* cst, std::ostream& out, size_t indent) {
	out << '"' << cst->m_token->m_text << '"';
}

static void pp(const BooleanLiteral* cst, std::ostream& out, size_t indent) {
	out << cst->m_token->m_text;
}

static void pp(const NullLiteral* cst, std::ostream& out, size_t indent) {
	out << "null";
}

static void semicolon_separated_values(
    const std::vector<Expr*>& elements, std::ostream& out, size_t indent) {
	for (size_t i = 0; i < elements.size(); ++i) {
		if (i > 0) {
			out << "; ";
		}
		out << elements[i];
	}
}

static void pp(const ArrayLiteral* cst, std::ostream& out, size_t indent) {
	// TODO: wrap around?
	out << "array { ";
	semicolon_separated_values(cst->m_elements, out, indent);
	out << "}";
}

static void pp_decl_data(DeclarationData dd, std::ostream& out, size_t indent) {
	out << dd.m_identifier_token->m_text;
	if (dd.m_type_hint) {
		out << " : ";
		pp(dd.m_type_hint, out, indent);
		if (dd.m_value) {
			out << " = ";
			pp(dd.m_value, out, indent);
		}
	} else if (dd.m_value) {
		out << " := ";
		pp(dd.m_value, out, indent);
	}
}

static void comma_separated_args(
    const std::vector<DeclarationData>& args, std::ostream& out, size_t indent) {
	for (size_t i = 0; i < args.size(); ++i) {
		if (i > 0) {
			out << ", ";
		}
		pp_decl_data(args[i], out, indent);
	}
}

static void pp_statements(
    const std::vector<Stmt*>& stmts, std::ostream& out, size_t indent) {
	for (const auto* stmt : stmts) {
		pp(stmt, out, indent);
	}
}

static void pp(const FunctionLiteral* cst, std::ostream& out, size_t indent) {
	// TODO: wrap around?
	out << "fn (";
	comma_separated_args(cst->m_args, out, indent);
	out << ") => ";
	pp(cst->m_body, out, indent);
}

static void pp(const BlockFunctionLiteral* cst, std::ostream& out, size_t indent) {
	out << "fn (";
	comma_separated_args(cst->m_args, out, indent);
	out << ") ";
	pp(cst->m_body, out, indent);
}

static void pp(const Identifier* cst, std::ostream& out, size_t indent) {
	out << cst->m_token->m_text;
}

static void pp(const BinaryExpression* cst, std::ostream& out, size_t indent) {
	pp(cst->m_lhs, out, indent);
	out << " " << cst->m_op_token->m_text << " ";
	pp(cst->m_rhs, out, indent);
}

static void pp(const IndexExpression* cst, std::ostream& out, size_t indent) {
	pp(cst->m_callee, out, indent);
	out << "[";
	pp(cst->m_index, out, indent);
	out << "]";
}

static void pp(const CallExpression* cst, std::ostream& out, size_t indent) {
	pp(cst->m_callee, out, indent);
	out << "(";
	semicolon_separated_values(cst->m_args, out, indent);
	out << ")";
}

static void pp(const TernaryExpression* cst, std::ostream& out, size_t indent) {
	// TODO: best way to print this?
	out << "if (";
	pp(cst->m_condition, out, indent);
	out << ") then ";
	pp(cst->m_then_expr, out, indent);
	out << " else ";
	pp(cst->m_else_expr, out, indent);
}

static void pp(const AccessExpression* cst, std::ostream& out, size_t indent) {
	pp(cst->m_record, out, indent);
	out << "." << cst->m_member->m_text;
}

static void pp(const MatchExpression* cst, std::ostream& out, size_t indent) {
	out << "match (" << cst->m_matchee.m_token->m_text;
	if (cst->m_type_hint) {
		out << " : " << cst->m_type_hint;
	}
	out << ") {\n";
	for (const auto m_case : cst->m_cases) {
		tabs(out, indent + 1);
		out << m_case.m_name << " { " << m_case.m_identifier;
		if (m_case.m_type_hint) {
			out << " : " << m_case.m_type_hint;
		}
		out << " } => ";
		pp(m_case.m_expression, out, indent + 1);
		out << ";\n";
	}
	tabs(out, indent);
	out << "}";
}

static void pp(const ConstructorExpression* cst, std::ostream& out, size_t indent) {
	pp(cst->m_constructor, out, indent);
	out << " { ";
	semicolon_separated_values(cst->m_args, out, indent);
	out << " }";
}

static void pp(const SequenceExpression* cst, std::ostream& out, size_t indent) {
	out << "seq ";
	pp(cst->m_body, out, indent);
}

static void pp(const PlainDeclaration* cst, std::ostream& out, size_t indent) {
	tabs(out, indent);
	pp_decl_data(cst->m_data, out, indent);
	out << ";\n";
}

static void pp(const FuncDeclaration* cst, std::ostream& out, size_t indent) {
	out << "fn " << cst->m_identifier << "(";
	comma_separated_args(cst->m_args, out, indent);
	out << ") => ";
	pp(cst->m_body, out, indent);
}

static void pp(const BlockFuncDeclaration* cst, std::ostream& out, size_t indent) {
	out << "fn " << cst->m_identifier << "(";
	comma_separated_args(cst->m_args, out, indent);
	out << ") ";
	pp(cst->m_body, out, indent);
}

static void pp(const Block* cst, std::ostream& out, size_t indent) {
	// NOTE: no tabs here because no block happens "in the wild"
	// this should be changed if we add block statements!
	out << "{\n";
	pp_statements(cst->m_body, out, indent + 1);
	tabs(out, indent);
	out << "}";
}

static void pp(const ReturnStatement* cst, std::ostream& out, size_t indent) {
	tabs(out, indent);
	out << "return ";
	pp(cst->m_value, out, indent);
	out << ";\n";
}

static void pp(const IfElseStatement* cst, std::ostream& out, size_t indent) {
	// TODO: add special case for else-if?
	tabs(out, indent);
	out << "if (";
	pp(cst->m_condition, out, indent);
	out << ") {\n";
	tabs(out, indent);
	pp(cst->m_body, out, indent + 1);
	out << "}";
	if (cst->m_else_body) {
		out << " else {\n";
		pp(cst->m_else_body, out, indent + 1);
		tabs(out, indent);
		out << "}\n";
	} else {
		out << "\n";
	}
}

static void pp(const ForStatement* cst, std::ostream& out, size_t indent) {
	tabs(out, indent);
	out << "for (";
	pp_decl_data(cst->m_declaration, out, indent);
	out << "; ";
	pp(cst->m_condition, out, indent);
	out << "; ";
	pp(cst->m_action, out, indent);
	out << ") {\n";
	pp(cst->m_body, out, indent + 1);
	tabs(out, indent);
	out << "}\n";
}

static void pp(const WhileStatement* cst, std::ostream& out, size_t indent) {
	tabs(out, indent);
	out << "while (";
	pp(cst->m_condition, out, indent);
	out << ") {\n";
	pp(cst->m_body, out, indent + 1);
	tabs(out, indent);
	out << "}\n";
}

static void pp(const ExpressionStatement* cst, std::ostream& out, size_t indent) {
	tabs(out, indent);
	pp(cst->m_expression, out, indent);
	out << ";\n";
}

static void pp(const TypeTerm* cst, std::ostream& out, size_t indent) {
	pp(cst->m_callee, out, indent);
	if (!cst->m_args.empty()) {
		out << "<:";
		semicolon_separated_values(cst->m_args, out, indent);
		out << ":>";
	}
}

static void pp(const TypeVar* cst, std::ostream& out, size_t indent) {
	out << cst->m_token->m_text;
}

static void pp(const UnionExpression* cst, std::ostream& out, size_t indent) {
	out << "union { ";
	for (size_t i = 0; i < cst->m_constructors.size(); ++i) {
		if (i > 0) {
			out << "; ";
		}
		out << cst->m_constructors[i].m_token->m_text << " : ";
		pp(cst->m_types[0], out, indent);
	}
	out << " }";
}

static void pp(const StructExpression* cst, std::ostream& out, size_t indent) {
	out << "struct { ";
	for (size_t i = 0; i < cst->m_fields.size(); ++i) {
		if (i > 0) {
			out << "; ";
		}
		out << cst->m_fields[i].m_token->m_text << " : ";
		pp(cst->m_types[i], out, indent);
	}
	out << " }";
}

static void pp(const Program* cst, std::ostream& out, size_t indent) {
	for (const auto* decl : cst->m_declarations) {
		pp(decl, out, indent);
	}
}

static void pp(const CST* cst, std::ostream& out, size_t indent) {
#define DISPATCH(type)                                                         \
	case CSTTag::type:                                                         \
		return pp(static_cast<const type*>(cst), out, indent);

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
	pp(cst, out, 0);
}

std::string pretty_print(const CST* cst) {
	std::stringstream s;
	pp(cst, s, 0);
	return s.str();
}

} // namespace CST