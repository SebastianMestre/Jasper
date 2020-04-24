#include <cassert>

#include "ast.hpp"
#include "runtime.hpp"
#include "garbage_collector.hpp"

void ASTDeclarationList::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ DeclarationList\n";
	for (auto& ast : m_declarations)
		ast->print(d + 1);
	std::cout << stab << "]\n";
}

Type::Value* ASTDeclarationList::eval(Type::Environment &e) {
	return e.m_gc->null();
};



void ASTDeclaration::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Declaration\n"
		<< tab << "Name: " << m_identifier << '\n'
		<< tab << "Type: " << m_typename << '\n';
	if (m_value) {
		std::cout << tab << "Initializer:\n";
		m_value->print(d + 1);
	}
	std::cout << stab << "]\n";
}

Type::Value* ASTDeclaration::eval(Type::Environment &e) {
	// TODO: type and mutable check -> return error
	if (m_value)
		e.m_scope->declare(m_identifier, e.m_gc->null());
	else
		e.m_scope->declare(m_identifier, m_value->eval(e));

	return e.m_gc->null();
};



void ASTNumberLiteral::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Number " << m_text << " ]\n";
}

Type::Value* ASTNumberLiteral::eval(Type::Environment &e) {
	// TODO: float / dec / int ...
	assert(0);
	return e.m_gc->null();
};



void ASTStringLiteral::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ StringLiteral\n"
		<< tab << "Value: " << m_text << "\n"
		<< stab << "]\n";
}

Type::Value* ASTStringLiteral::eval(Type::Environment &e) {
	return e.m_gc->new_string(m_text);
};



void ASTIdentifier::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Identifier\n"
		<< tab << "Name: " << m_text << '\n'
		<< stab << "]\n";
}

Type::Value* ASTIdentifier::eval(Type::Environment &e) {
	return e.m_scope->access(m_text);
};



void ASTBlock::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Block\n";
	for (auto& child : m_body)
		child->print(d + 1);
	std::cout << stab << "]\n";
}

Type::Value* ASTBlock::eval(Type::Environment &e) {
	return e.m_gc->null();
};



void ASTFunction::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Function\n"
		<< tab << "Arguments:\n";
	for(auto& arg : m_args){
		arg->print(d+1);
	}
	std::cout << tab << "Body:\n";
	if (m_body)
		m_body->print(d + 1);
	std::cout << stab << "]\n";
}

Type::Value* ASTFunction::eval(Type::Environment &e) {
	// TODO: create definition?
	assert(0);
	return e.m_gc->null();
};



void ASTBinaryExpression::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ BinaryExpression\n"
	          << tab << "Operator: " << token_type_string[int(m_op)] << '\n'
	          << tab << "Left Operand:\n";
	m_lhs->print(d + 1);
	std::cout << tab << "Right Operand:\n";
	m_rhs->print(d + 1);
	std::cout << stab << "]\n";
}

Type::Value* ASTBinaryExpression::eval(Type::Environment &e) {
	return nullptr;
}



void ASTCallExpression::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ CallExpression\n"
	          << tab << "Callee:\n";
	m_callee->print(d + 1);
	std::cout << tab << "Args:\n";
	m_args->print(d + 1);
	std::cout << stab << "]\n";
}

Type::Value* ASTCallExpression::eval(Type::Environment &e) {
	// TODO: fetch function definition and scope and run
	assert(0);
	return e.m_gc->null();
};



void ASTArgumentList::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ ArgumentList\n";
	for(auto& argument : m_args){
		argument->print(d+1);
	}
	std::cout << stab << "]\n";
}

Type::Value* ASTArgumentList::eval(Type::Environment &e) {
	// TODO: return as list?
	assert(0);
	return e.m_gc->null();
};
