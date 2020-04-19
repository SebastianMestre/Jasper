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
		<< tab << "Type::Value: " << m_text << "\n"
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
