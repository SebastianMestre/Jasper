#include "ast.hpp"

#include <iostream>

#include <cassert>

namespace AST {

constexpr char tabc = ' ';

void print(DeclarationList* ast, int d) {
	std::string stab(d - 1, tabc);
	std::cout << stab << "[ DeclarationList\n";
	for (auto& decl : ast->m_declarations)
		print(decl.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(Declaration* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Declaration\n"
	          << tab << "Name: " << ast->identifier_text() << '\n';

	if (ast->m_type) {
		std::cout << tab << "Type:\n";
		print(ast->m_type.get(), d + 1);
	}

	if (ast->m_value) {
		std::cout << tab << "Initializer:\n";
		print(ast->m_value.get(), d + 1);
	}
	std::cout << stab << "]\n";
}

void print(NumberLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::cout << stab << "[ Number " << ast->text() << " ]\n";
}

void print(IntegerLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::cout << stab << "[ Integer " << ast->text() << " ]\n";
}

void print(StringLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::cout << stab << "[ StringLiteral " << ast->text() << "]\n";
}

void print(BooleanLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::cout << stab << "[ BooleanLiteral " << ast->text() << "]\n";
}

void print(NullLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::cout << stab << "[ NullLiteral ]\n";
}

void print(ObjectLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ ObjectLiteral\n" << tab << "Declarations:\n";
	for (auto& decl : ast->m_body)
		print(decl.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(ArrayLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ ArrayLiteral\n" << tab << "Elements:\n";
	for (auto& elem : ast->m_elements)
		print(elem.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(DictionaryLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ DictionaryLiteral\n" << tab << "Declarations:\n";
	for (auto& decl : ast->m_body)
		print(decl.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(Identifier* ast, int d) {
	std::string stab(d - 1, tabc);
	std::cout << stab << "[ Identifier " << ast->text() << " ]\n";
}

void print(Block* ast, int d) {
	std::string stab(d - 1, tabc);
	std::cout << stab << "[ Block\n";
	for (auto& child : ast->m_body)
		print(child.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(FunctionLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Function\n" << tab << "Arguments:\n";
	for (auto& arg : ast->m_args) {
		print(arg.get(), d + 1);
	}
	std::cout << tab << "Body:\n";
	if (ast->m_body)
		print(ast->m_body.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(BinaryExpression* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ BinaryExpression\n"
	          << tab
	          << "Operator: " << token_type_string[int(ast->m_op_token->m_type)]
	          << '\n'
	          << tab << "Left Operand:\n";
	print(ast->m_lhs.get(), d + 1);
	std::cout << tab << "Right Operand:\n";
	print(ast->m_rhs.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(CallExpression* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ CallExpression\n" << tab << "Callee:\n";
	print(ast->m_callee.get(), d + 1);
	std::cout << tab << "Args:\n";
	for (auto& arg : ast->m_args)
		print(arg.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(IndexExpression* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ IndexExpression\n" << tab << "Callee:\n";
	print(ast->m_callee.get(), d + 1);
	std::cout << tab << "Index:\n";
	print(ast->m_index.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(TernaryExpression* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ TernaryExpression\n" << tab << "Condition:\n";
	print(ast->m_condition.get(), d + 1);
	std::cout << tab << "Then:\n";
	print(ast->m_then_expr.get(), d + 1);
	std::cout << tab << "Else:\n";
	print(ast->m_else_expr.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(ReturnStatement* ast, int d) {
	std::string stab(d - 1, tabc);
	std::cout << stab << "[ ReturnStatement\n";
	print(ast->m_value.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(IfElseStatement* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ IfElseStatement\n" << tab << "Condition:\n";
	print(ast->m_condition.get(), d + 1);
	std::cout << tab << "Body:\n";
	print(ast->m_body.get(), d + 1);
	std::cout << tab << "Else:\n";
	if (ast->m_else_body)
		print(ast->m_else_body.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(ForStatement* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ ForStatement\n" << tab << "Declaration:\n";
	print(ast->m_declaration.get(), d + 1);
	std::cout << tab << "Condition:\n";
	print(ast->m_condition.get(), d + 1);
	std::cout << tab << "Action:\n";
	print(ast->m_action.get(), d + 1);
	std::cout << tab << "Body:\n";
	print(ast->m_body.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(WhileStatement* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ WhileStatement\n" << tab << "Condition:\n";
	print(ast->m_condition.get(), d + 1);
	std::cout << tab << "Body:\n";
	print(ast->m_body.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(TypeTerm* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ TypeTerm\n";
	print(ast->m_callee.get(), d + 1);
	for (auto& arg : ast->m_args)
		print(arg.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(AST* ast, int d) {
	switch (ast->type()) {
	case ASTType::NumberLiteral:
		return print(static_cast<NumberLiteral*>(ast), d);
	case ASTType::IntegerLiteral:
		return print(static_cast<IntegerLiteral*>(ast), d);
	case ASTType::StringLiteral:
		return print(static_cast<StringLiteral*>(ast), d);
	case ASTType::BooleanLiteral:
		return print(static_cast<BooleanLiteral*>(ast), d);
	case ASTType::NullLiteral:
		return print(static_cast<NullLiteral*>(ast), d);
	case ASTType::ObjectLiteral:
		return print(static_cast<ObjectLiteral*>(ast), d);
	case ASTType::ArrayLiteral:
		return print(static_cast<ArrayLiteral*>(ast), d);
	case ASTType::DictionaryLiteral:
		return print(static_cast<DictionaryLiteral*>(ast), d);
	case ASTType::FunctionLiteral:
		return print(static_cast<FunctionLiteral*>(ast), d);
	case ASTType::DeclarationList:
		return print(static_cast<DeclarationList*>(ast), d);
	case ASTType::Declaration:
		return print(static_cast<Declaration*>(ast), d);
	case ASTType::Identifier:
		return print(static_cast<Identifier*>(ast), d);
	case ASTType::BinaryExpression:
		return print(static_cast<BinaryExpression*>(ast), d);
	case ASTType::CallExpression:
		return print(static_cast<CallExpression*>(ast), d);
	case ASTType::IndexExpression:
		return print(static_cast<IndexExpression*>(ast), d);
	case ASTType::TernaryExpression:
		return print(static_cast<TernaryExpression*>(ast), d);
	case ASTType::Block:
		return print(static_cast<Block*>(ast), d);
	case ASTType::ReturnStatement:
		return print(static_cast<ReturnStatement*>(ast), d);
	case ASTType::IfElseStatement:
		return print(static_cast<IfElseStatement*>(ast), d);
	case ASTType::ForStatement:
		return print(static_cast<ForStatement*>(ast), d);
	case ASTType::WhileStatement:
		return print(static_cast<WhileStatement*>(ast), d);
	case ASTType::TypeTerm:
		return print(static_cast<TypeTerm*>(ast), d);
	}
}

} // namespace AST
