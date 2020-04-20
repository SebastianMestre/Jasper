#include "ast.hpp"
#include "runtime.hpp"

void ASTDeclarationList::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ DeclarationList\n";
	for (auto& ast : m_declarations)
		ast->print(d + 1);
	std::cout << stab << "]\n";
}

// TODO: implement with GC and with actual values that make sense
Type::Value* ASTDeclarationList::run(Type::Scope &s) {
	return new Type::Null();
};



void ASTDeclaration::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Declaration\n"
		<< tab << "Name: " << m_identifier << '\n'
		<< tab << "Type: " << m_typename << '\n'
		<< tab << "Initializer:\n";
	m_value->print(d + 1);
	std::cout << stab << "]\n";
}

// TODO: implement with GC and with actual values that make sense
Type::Value* ASTDeclaration::run(Type::Scope &s) {
	return new Type::Null();
};



void ASTNumberLiteral::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Number\n"
		<< tab << "Value: " << m_text << "\n"
		<< stab << "]\n";
}

// TODO: implement with GC and with actual values that make sense
Type::Value* ASTNumberLiteral::run(Type::Scope &s) {
	return new Type::Null();
};



void ASTIdentifier::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Identifier\n"
		<< tab << "Name: " << m_text << '\n'
		<< stab << "]\n";
}

// TODO: implement with GC and with actual values that make sense
Type::Value* ASTIdentifier::run(Type::Scope &s) {
	return new Type::Null();
};



void ASTBlock::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Block\n";
	for (auto& child : m_body)
		child->print(d + 1);
	std::cout << stab << "]\n";
}

// TODO: implement with GC and with actual values that make sense
Type::Value* ASTBlock::run(Type::Scope &s) {
	return new Type::Null();
};



void ASTFunction::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Function\n";
	if (m_body)
		m_body->print(d + 1);
	std::cout << stab << "]\n";
}

// TODO: implement with GC and with actual values that make sense
Type::Value* ASTFunction::run(Type::Scope &s) {
	return new Type::Null();
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

Type::Value* ASTBinaryExpression::run(Type::Scope &s) {
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

// TODO: implement with GC and with actual values that make sense
Type::Value* ASTCallExpression::run(Type::Scope &s) {
	return new Type::Null();
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

// TODO: implement with GC and with actual values that make sense
Type::Value* ASTArgumentList::run(Type::Scope &s) {
	return new Type::Null();
};
