#include "ast.hpp"

#include <iostream>

namespace AST {

namespace {

constexpr int indent_width = 4;
constexpr char tabc = ' ';
static void print_indentation (int d) {
	for(int i = d; i-- > 0;)
		std::cout << tabc;
}

}

void print_impl(DeclarationList* ast, int d) {
	print_indentation(d);
	std::cout << "(decl-list";
	for (auto& decl : ast->m_declarations) {
		std::cout << "\n";
		print(&decl, d + indent_width);
	}
	std::cout << ")";
}

void print_impl(Declaration* ast, int d) {
	print_indentation(d);
	std::cout << "(decl \"" << ast->identifier_text() << "\"\n";

	print(ast->m_type_hint, d + 6);
	std::cout << "\n";

	print(ast->m_value, d + 6);
	std::cout << ")";
}

void print_impl(NumberLiteral* ast, int d) {
	print_indentation(d);
	char const* sign = ast->m_negative ? "-" : "";
	std::cout << "(number-literal \"" << sign << ast->text() << "\")";
}

void print_impl(IntegerLiteral* ast, int d) {
	print_indentation(d);
	char const* sign = ast->m_negative ? "-" : "";
	std::cout << "(integer-literal \"" << sign << ast->text() << "\")";
}

void print_impl(StringLiteral* ast, int d) {
	print_indentation(d);
	std::cout << "(string-literal \"" << ast->text() << "\")";
}

void print_impl(BooleanLiteral* ast, int d) {
	print_indentation(d);
	std::cout << "(boolean-literal \"" << ast->text() << "\")";
}

void print_impl(NullLiteral* ast, int d) {
	print_indentation(d);
	std::cout << "(null-literal)";
}

void print_impl(ObjectLiteral* ast, int d) {
	print_indentation(d - 1);
	std::cout << "[ ObjectLiteral\n";
	print_indentation(d);
	std::cout << "Declarations:\n";
	for (auto decl : ast->m_body)
		print(&decl, d + 1);
	print_indentation(d - 1);
	std::cout << "]\n";
}

void print_impl(ArrayLiteral* ast, int d) {
	print_indentation(d);
	std::cout << "(array-literal";
	for (auto elem : ast->m_elements) {
		std::cout << "\n";
		print(elem, d + indent_width);
	}
	std::cout << ")";
}

void print_impl(DictionaryLiteral* ast, int d) {
	print_indentation(d);
	std::cout << "(dictionary-literal";
	print_indentation(d+1);
	for (auto decl : ast->m_body) {
		std::cout << "\n";
		print(&decl, d + indent_width);
	}
	std::cout << ")";
}

void print_impl(Identifier* ast, int d) {
	print_indentation(d);
	std::cout << "(identifier \"" << ast->text() << "\")";
}

void print_impl(Block* ast, int d) {
	print_indentation(d);
	std::cout << "(block-stmt ";
	for (auto child : ast->m_body) {
		std::cout << "\n";
		print(child, d + indent_width);
	}
	std::cout << ")";
}

void print_impl(BlockFunctionLiteral* ast, int d) {
	print_indentation(d);
	std::cout << "(function-literal (";
	for (auto arg : ast->m_args) {
		std::cout << "\n";
		print(&arg, d + indent_width);
	}
	std::cout << ")\n";

	print(ast->m_body, d + indent_width);

	std::cout << ")";
}

void print_impl(FunctionLiteral* ast, int d) {
	print_indentation(d);
	std::cout << "(short-function-literal (";
	for (auto arg : ast->m_args) {
		std::cout << "\n";
		print(&arg, d + indent_width);
	}
	std::cout << ")\n";

	print(ast->m_body, d + indent_width);

	std::cout << ")";
}

void print_impl(BinaryExpression* ast, int d) {
	print_indentation(d);
	std::cout << "(binary-expr\n";

	print_indentation(d + indent_width);
	std::cout << token_string[int(ast->m_op_token->m_type)] << '\n';

	print(ast->m_lhs, d + indent_width);
	std::cout << "\n";

	print(ast->m_rhs, d + indent_width);

	std::cout << ")";
}

void print_impl(CallExpression* ast, int d) {
	print_indentation(d);
	std::cout << "(call-expr\n";
	print(ast->m_callee, d + indent_width);
	for (auto arg : ast->m_args) {
		std::cout << "\n";
		print(arg, d + indent_width);
	}
	std::cout << ")";
}

void print_impl(IndexExpression* ast, int d) {
	print_indentation(d);
	std::cout << "(index-expression\n";
	print(ast->m_callee, d + indent_width);
	std::cout << "\n";
	print(ast->m_index, d + indent_width);
	std::cout << ")";
}

void print_impl(AccessExpression* ast, int d) {
	print_indentation(d);
	std::cout << "(access-expression\n";
	print(ast->m_record, d + indent_width);
	std::cout << "\n";
	print_indentation(d + indent_width);
	std::cout << "\"" << ast->m_member->m_text << "\"";
	std::cout << ")";
}

void print_impl(TernaryExpression* ast, int d) {
	print_indentation(d);
	std::cout << "(ternary-expression\n";
	print(ast->m_condition, d + indent_width);
	std::cout << "\n";
	print(ast->m_then_expr, d + indent_width);
	std::cout << "\n";
	print(ast->m_else_expr, d + indent_width);
	std::cout << ")";
}

void print_impl(MatchExpression* ast, int d) {
	print_indentation(d);
	std::cout << "(match-expression\n";
	print(&ast->m_matchee, d + indent_width);
	std::cout << "\n";
	print(ast->m_type_hint, d + indent_width);
	for (auto const& case_data : ast->m_cases) {
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

void print_impl(SequenceExpression* ast, int d) {
	print_indentation(d);
	std::cout << "(sequence-expr";
	print(ast->m_body, d + 2);
	std::cout << ")";
}

void print_impl(ReturnStatement* ast, int d) {
	print_indentation(d);
	std::cout << "(return-stmt\n";
	print(ast->m_value, d + indent_width);
	std::cout << ")";
}

void print_impl(IfElseStatement* ast, int d) {
	print_indentation(d);
	std::cout << "(if-else-stmt\n";
	print(ast->m_condition, d + indent_width);
	std::cout << "\n";
	print(ast->m_body, d + indent_width);
	std::cout << "\n";
	print(ast->m_else_body, d + indent_width);
	std::cout << ")";
}

void print_impl(ForStatement* ast, int d) {
	print_indentation(d);
	std::cout << "(for-stmt\n";
	print(&ast->m_declaration, d + indent_width);
	std::cout << "\n";
	print(ast->m_condition, d + indent_width);
	std::cout << "\n";
	print(ast->m_action, d + indent_width);
	std::cout << "\n";
	print(ast->m_body, d + indent_width);
	std::cout << ")";
}

void print_impl(WhileStatement* ast, int d) {
	print_indentation(d);
	std::cout << "(while-statement\n";
	print(ast->m_condition, d + indent_width);
	std::cout << "\n";
	print(ast->m_body, d + indent_width);
	std::cout << ")";
}

void print_impl(TypeTerm* ast, int d) {
	print_indentation(d);
	std::cout << "(type-term\n";
	print(ast->m_callee, d + indent_width);
	for (auto arg : ast->m_args) {
		std::cout << "\n";
		print(arg, d + indent_width);
	}
	std::cout << ")";
}

void print_impl(UnionExpression* ast, int d) {
	print_indentation(d);
	std::cout << "(union-expression ";
	for (int i = 0; i < ast->m_constructors.size(); ++i) {
		std::cout << "\n";
		print_indentation(d + indent_width);
		std::cout << "(\"" << ast->m_constructors[i].text() << "\"\n";
		print(ast->m_types[i], d + indent_width + 1);
		std::cout << ")";
	}
	std::cout << ")";
}

void print_impl(StructExpression* ast, int d) {
	print_indentation(d);
	std::cout << "(struct-expression ";
	for (int i = 0; i < ast->m_fields.size(); ++i) {
		std::cout << "\n";
		print_indentation(d + indent_width);
		std::cout << "(\"" << ast->m_fields[i].text() << "\"\n";
		print(ast->m_types[i], d + indent_width + 1);
		std::cout << ")";
	}
	std::cout << ")";
}

void print_impl(ConstructorExpression* ast, int d) {
	print_indentation(d);
	std::cout << "(construct-expression\n";
	print(ast->m_constructor, d + indent_width);
	for (auto* arg : ast->m_args) {
		std::cout << "\n";
		print(arg, d + indent_width);
	}
	std::cout << ")";
}

void print_impl(AST* ast, int d) {
	switch (ast->type()) {
	case ASTTag::NumberLiteral:
		return print_impl(static_cast<NumberLiteral*>(ast), d);
	case ASTTag::IntegerLiteral:
		return print_impl(static_cast<IntegerLiteral*>(ast), d);
	case ASTTag::StringLiteral:
		return print_impl(static_cast<StringLiteral*>(ast), d);
	case ASTTag::BooleanLiteral:
		return print_impl(static_cast<BooleanLiteral*>(ast), d);
	case ASTTag::NullLiteral:
		return print_impl(static_cast<NullLiteral*>(ast), d);
	case ASTTag::ObjectLiteral:
		return print_impl(static_cast<ObjectLiteral*>(ast), d);
	case ASTTag::ArrayLiteral:
		return print_impl(static_cast<ArrayLiteral*>(ast), d);
	case ASTTag::DictionaryLiteral:
		return print_impl(static_cast<DictionaryLiteral*>(ast), d);
	case ASTTag::BlockFunctionLiteral:
		return print_impl(static_cast<BlockFunctionLiteral*>(ast), d);
	case ASTTag::FunctionLiteral:
		return print_impl(static_cast<FunctionLiteral*>(ast), d);

	case ASTTag::Identifier:
		return print_impl(static_cast<Identifier*>(ast), d);
	case ASTTag::BinaryExpression:
		return print_impl(static_cast<BinaryExpression*>(ast), d);
	case ASTTag::CallExpression:
		return print_impl(static_cast<CallExpression*>(ast), d);
	case ASTTag::IndexExpression:
		return print_impl(static_cast<IndexExpression*>(ast), d);
	case ASTTag::TernaryExpression:
		return print_impl(static_cast<TernaryExpression*>(ast), d);
	case ASTTag::AccessExpression:
		return print_impl(static_cast<AccessExpression*>(ast), d);
	case ASTTag::SequenceExpression:
		return print_impl(static_cast<SequenceExpression*>(ast), d);
	case ASTTag::MatchExpression:
		return print_impl(static_cast<MatchExpression*>(ast), d);
	case ASTTag::ConstructorExpression:
		return print_impl(static_cast<ConstructorExpression*>(ast), d);

	case ASTTag::DeclarationList:
		return print_impl(static_cast<DeclarationList*>(ast), d);
	case ASTTag::Declaration:
		return print_impl(static_cast<Declaration*>(ast), d);

	case ASTTag::Block:
		return print_impl(static_cast<Block*>(ast), d);
	case ASTTag::ReturnStatement:
		return print_impl(static_cast<ReturnStatement*>(ast), d);
	case ASTTag::IfElseStatement:
		return print_impl(static_cast<IfElseStatement*>(ast), d);
	case ASTTag::ForStatement:
		return print_impl(static_cast<ForStatement*>(ast), d);
	case ASTTag::WhileStatement:
		return print_impl(static_cast<WhileStatement*>(ast), d);

	case ASTTag::TypeTerm:
		return print_impl(static_cast<TypeTerm*>(ast), d);
	case ASTTag::UnionExpression:
		return print_impl(static_cast<UnionExpression*>(ast), d);
	case ASTTag::StructExpression:
		return print_impl(static_cast<StructExpression*>(ast), d);
	}

	print_indentation(d);
	std::cout << "(UNSUPPORTED " << ast_string[int(ast->type())] << ")";
}

void print(AST* ast, int d) {
	if (!ast) {
		print_indentation(d);
		std::cout << "'()";
	} else {
		print_impl(ast, d);
	}
}

} // namespace AST
