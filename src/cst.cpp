#include "cst.hpp"

#include <iostream>

namespace CST {

namespace {

constexpr int indent_width = 4;
constexpr char tabc = ' ';
static void print_indentation (int d) {
	for(int i = d; i-- > 0;)
		std::cout << tabc;
}

}

void print_impl(Program* cst, int d) {
	print_indentation(d);
	std::cout << "(decl-list";
	for (auto& decl : cst->m_declarations) {
		std::cout << "\n";
		print(decl, d + indent_width);
	}
	std::cout << ")";
}

void print(DeclarationData& data, int d) {
	print_indentation(d);
	std::cout << "(decl \"" << data.identifier() << "\"\n";

	print(data.m_type_hint, d + 6);
	std::cout << "\n";

	print(data.m_value, d + 6);
	std::cout << ")";
}

void print_impl(PlainDeclaration* cst, int d) {
	print(cst->m_data, d);
}

void print_impl(NumberLiteral* cst, int d) {
	print_indentation(d);
	char const* sign = cst->m_negative ? "-" : "";
	std::cout << "(number-literal \"" << sign << cst->text() << "\")";
}

void print_impl(IntegerLiteral* cst, int d) {
	print_indentation(d);
	char const* sign = cst->m_negative ? "-" : "";
	std::cout << "(integer-literal \"" << sign << cst->text() << "\")";
}

void print_impl(StringLiteral* cst, int d) {
	print_indentation(d);
	std::cout << "(string-literal \"" << cst->text() << "\")";
}

void print_impl(BooleanLiteral* cst, int d) {
	print_indentation(d);
	std::cout << "(boolean-literal \"" << cst->text() << "\")";
}

void print_impl(NullLiteral* cst, int d) {
	print_indentation(d);
	std::cout << "(null-literal)";
}

void print_impl(ArrayLiteral* cst, int d) {
	print_indentation(d);
	std::cout << "(array-literal";
	for (auto elem : cst->m_elements) {
		std::cout << "\n";
		print(elem, d + indent_width);
	}
	std::cout << ")";
}

void print_impl(Identifier* cst, int d) {
	print_indentation(d);
	std::cout << "(identifier \"" << cst->text() << "\")";
}

void print_impl(Block* cst, int d) {
	print_indentation(d);
	std::cout << "(block-stmt ";
	for (auto child : cst->m_body) {
		std::cout << "\n";
		print(child, d + indent_width);
	}
	std::cout << ")";
}

void print_impl(FuncParameters& args, int d) {
	for (DeclarationData& arg : args) {
		std::cout << "\n";
		print(arg, d + indent_width);
	}
}

void print_impl(BlockFunctionLiteral* cst, int d) {
	print_indentation(d);
	std::cout << "(function-literal (";
	print_impl(cst->m_args, d);
	std::cout << ")\n";

	print(cst->m_body, d + indent_width);

	std::cout << ")";
}

void print_impl(FunctionLiteral* cst, int d) {
	print_indentation(d);
	std::cout << "(short-function-literal (";
	print_impl(cst->m_args, d);
	std::cout << ")\n";

	print(cst->m_body, d + indent_width);

	std::cout << ")";
}

void print_impl(BinaryExpression* cst, int d) {
	print_indentation(d);
	std::cout << "(binary-expr\n";

	print_indentation(d + indent_width);
	std::cout << token_string[int(cst->m_op_token->m_type)] << '\n';

	print(cst->m_lhs, d + indent_width);
	std::cout << "\n";

	print(cst->m_rhs, d + indent_width);

	std::cout << ")";
}

void print_impl(CallExpression* cst, int d) {
	print_indentation(d);
	std::cout << "(call-expr\n";
	print(cst->m_callee, d + indent_width);
	for (auto arg : cst->m_args) {
		std::cout << "\n";
		print(arg, d + indent_width);
	}
	std::cout << ")";
}

void print_impl(IndexExpression* cst, int d) {
	print_indentation(d);
	std::cout << "(index-expression\n";
	print(cst->m_callee, d + indent_width);
	std::cout << "\n";
	print(cst->m_index, d + indent_width);
	std::cout << ")";
}

void print_impl(AccessExpression* cst, int d) {
	print_indentation(d);
	std::cout << "(access-expression\n";
	print(cst->m_record, d + indent_width);
	std::cout << "\n";
	print_indentation(d + indent_width);
	std::cout << "\"" << cst->m_member->m_text << "\"";
	std::cout << ")";
}

void print_impl(TernaryExpression* cst, int d) {
	print_indentation(d);
	std::cout << "(ternary-expression\n";
	print(cst->m_condition, d + indent_width);
	std::cout << "\n";
	print(cst->m_then_expr, d + indent_width);
	std::cout << "\n";
	print(cst->m_else_expr, d + indent_width);
	std::cout << ")";
}

void print_impl(MatchExpression* cst, int d) {
	print_indentation(d);
	std::cout << "(match-expression\n";
	print(&cst->m_matchee, d + indent_width);
	std::cout << "\n";
	print(cst->m_type_hint, d + indent_width);
	for (auto const& case_data : cst->m_cases) {
		std::cout << "\n";
		print_indentation(d + indent_width + 1);
		std::cout << "(\"" << case_data.m_name->m_text.str() << "\" \""
		          << case_data.m_identifier->m_text.str() << "\"\n";
		print(case_data.m_type_hint, d + indent_width + 1);
		std::cout << "\n";
		print(case_data.m_expression, d + indent_width + 1);
		std::cout << ")";
	}
	std::cout << ")";
}

void print_impl(SequenceExpression* cst, int d) {
	print_indentation(d);
	std::cout << "(sequence-expr";
	print(cst->m_body, d + 2);
	std::cout << ")";
}

void print_impl(ReturnStatement* cst, int d) {
	print_indentation(d);
	std::cout << "(return-stmt\n";
	print(cst->m_value, d + indent_width);
	std::cout << ")";
}

void print_impl(IfElseStatement* cst, int d) {
	print_indentation(d);
	std::cout << "(if-else-stmt\n";
	print(cst->m_condition, d + indent_width);
	std::cout << "\n";
	print(cst->m_body, d + indent_width);
	std::cout << "\n";
	print(cst->m_else_body, d + indent_width);
	std::cout << ")";
}

void print_impl(ForStatement* cst, int d) {
	print_indentation(d);
	std::cout << "(for-stmt\n";
	print(cst->m_declaration, d + indent_width);
	std::cout << "\n";
	print(cst->m_condition, d + indent_width);
	std::cout << "\n";
	print(cst->m_action, d + indent_width);
	std::cout << "\n";
	print(cst->m_body, d + indent_width);
	std::cout << ")";
}

void print_impl(WhileStatement* cst, int d) {
	print_indentation(d);
	std::cout << "(while-statement\n";
	print(cst->m_condition, d + indent_width);
	std::cout << "\n";
	print(cst->m_body, d + indent_width);
	std::cout << ")";
}

void print_impl(TypeTerm* cst, int d) {
	print_indentation(d);
	std::cout << "(type-term\n";
	print(cst->m_callee, d + indent_width);
	for (auto arg : cst->m_args) {
		std::cout << "\n";
		print(arg, d + indent_width);
	}
	std::cout << ")";
}

void print_impl(UnionExpression* cst, int d) {
	print_indentation(d);
	std::cout << "(union-expression ";
	for (int i = 0; i < cst->m_constructors.size(); ++i) {
		std::cout << "\n";
		print_indentation(d + indent_width);
		std::cout << "(\"" << cst->m_constructors[i].text() << "\"\n";
		print(cst->m_types[i], d + indent_width + 1);
		std::cout << ")";
	}
	std::cout << ")";
}

void print_impl(StructExpression* cst, int d) {
	print_indentation(d);
	std::cout << "(struct-expression ";
	for (int i = 0; i < cst->m_fields.size(); ++i) {
		std::cout << "\n";
		print_indentation(d + indent_width);
		std::cout << "(\"" << cst->m_fields[i].text() << "\"\n";
		print(cst->m_types[i], d + indent_width + 1);
		std::cout << ")";
	}
	std::cout << ")";
}

void print_impl(ConstructorExpression* cst, int d) {
	print_indentation(d);
	std::cout << "(construct-expression\n";
	print(cst->m_constructor, d + indent_width);
	for (auto* arg : cst->m_args) {
		std::cout << "\n";
		print(arg, d + indent_width);
	}
	std::cout << ")";
}

void print_impl(CST* cst, int d) {
#define DISPATCH(type)                                                         \
	case CSTTag::type:                                                         \
		return print_impl(static_cast<type*>(cst), d);

	switch (cst->type()) {
		DISPATCH(NumberLiteral)
		DISPATCH(IntegerLiteral)
		DISPATCH(StringLiteral)
		DISPATCH(BooleanLiteral)
		DISPATCH(NullLiteral)
		DISPATCH(ArrayLiteral)
		DISPATCH(BlockFunctionLiteral)
		DISPATCH(FunctionLiteral)

		DISPATCH(Identifier)
		DISPATCH(BinaryExpression)
		DISPATCH(CallExpression)
		DISPATCH(IndexExpression)
		DISPATCH(TernaryExpression)
		DISPATCH(AccessExpression)
		DISPATCH(SequenceExpression)
		DISPATCH(MatchExpression)
		DISPATCH(ConstructorExpression)

		DISPATCH(Program)
		DISPATCH(PlainDeclaration)

		DISPATCH(Block)
		DISPATCH(ReturnStatement)
		DISPATCH(IfElseStatement)
		DISPATCH(ForStatement)
		DISPATCH(WhileStatement)

		DISPATCH(TypeTerm)
		DISPATCH(UnionExpression)
		DISPATCH(StructExpression)
	}

#undef DISPATCH

	print_indentation(d);
	std::cout << "(UNSUPPORTED " << cst_string[int(cst->type())] << ")";
}

void print(CST* cst, int d) {
	if (!cst) {
		print_indentation(d);
		std::cout << "'()";
	} else {
		print_impl(cst, d);
	}
}

} // namespace CST
