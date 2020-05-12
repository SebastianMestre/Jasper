#include "eval.hpp"

#include <iostream>

#include <cassert>

#include "ast.hpp"
#include "environment.hpp"
#include "value.hpp"

Type::Value* eval(ASTDeclarationList* ast, Type::Environment& e) {
	for(auto& decl : ast->m_declarations){
		assert(decl->type() == ast_type::Declaration);
		eval(static_cast<ASTDeclaration*>(decl.get()), e);
	}
	return nullptr;
};

Type::Value* eval(ASTDeclaration* ast, Type::Environment& e) {
	// TODO: type and mutable check -> return error
	if (!ast->m_value)
		e.m_scope->declare(ast->m_identifier, e.null());
	else
		e.m_scope->declare(ast->m_identifier, eval(ast->m_value.get(), e));

	return e.null();
};

Type::Value* eval(ASTNumberLiteral* ast, Type::Environment& e) {
	for (char a : ast->m_text)
		if (a == '.')
			return e.new_float(std::stof(ast->m_text));

	return e.new_integer(std::stoi(ast->m_text));
};

Type::Value* eval(ASTStringLiteral* ast, Type::Environment& e) {
	return e.new_string(ast->m_text);
};

Type::Value* eval(ASTObjectLiteral* ast, Type::Environment& e) {
	std::cerr << "WARNING: not implemented action (Creating an object)\n";
	return nullptr;
};

Type::Value* eval(ASTDictionaryLiteral* ast, Type::Environment& e) {
	std::cerr << "WARNING: not implemented action (Creating a dictionary)\n";
	return nullptr;
};

Type::Value* eval(ASTArrayLiteral* ast, Type::Environment& e) {
	std::vector<Type::Value*> elements;
	for(auto& element : ast->m_elements){
		elements.push_back(eval(element.get(), e));
	}
	return e.new_list(std::move(elements));
};

Type::Value* eval(ASTIdentifier* ast, Type::Environment& e) {
	return e.m_scope->access(ast->m_text);
};

Type::Value* eval(ASTBlock* ast, Type::Environment& e) {

	e.new_scope();

	for(auto &stmt : ast->m_body){
		eval(stmt.get(), e);
		if (e.m_return_value)
			break;
	}

	e.end_scope();

	return e.null();
};

Type::Value* eval(ASTReturnStatement* ast, Type::Environment& e) {
	// TODO: proper error handling
	auto* returning = eval(ast->m_value.get(), e);
	assert(returning);

	e.save_return_value(returning);
	return e.null();
};

Type::Value* eval(ASTArgumentList* ast, Type::Environment& e) {
	// TODO: return as list?
	assert(0);
	return e.null();
};

Type::Value* eval(ASTCallExpression* ast, Type::Environment& e) {
	std::vector<Type::Value*> args;

	// TODO: proper error handling

	auto* calleeTypeErased = eval(ast->m_callee.get(), e);
	assert(calleeTypeErased);

	assert(calleeTypeErased->type() == value_type::Function);
	Type::Function* callee = static_cast<Type::Function*>(calleeTypeErased);

	auto* arglist = dynamic_cast<ASTArgumentList*>(ast->m_args.get());
	assert(callee->m_def->m_args.size() == arglist->m_args.size());

	e.new_scope();

	for(int i = 0; i < int(callee->m_def->m_args.size()); ++i){
		auto* argdeclTypeErased = callee->m_def->m_args[i].get();
		assert(argdeclTypeErased);

		auto* argdecl = dynamic_cast<ASTDeclaration*>(argdeclTypeErased);
		assert(argdecl);

		auto* argvalue = eval(arglist->m_args[i].get(), e);
		e.declare(argdecl->m_identifier, argvalue);
	}

	auto* body = dynamic_cast<ASTBlock*>(callee->m_def->m_body.get());
	assert(body);

	eval(body, e);

	e.end_scope();

	return e.fetch_return_value();
};

Type::Value* eval(ASTBinaryExpression* ast, Type::Environment& e) {

	auto* lhs = eval(ast->m_lhs.get(), e);
	auto* rhs = eval(ast->m_rhs.get(), e);

	switch (ast->m_op) {
	case token_type::ADD: {
		
		// TODO: proper error handling
		assert(lhs->type() == rhs->type());

		switch (lhs->type()) {
		case value_type::Integer:
			return e.new_integer(
				static_cast<Type::Integer*>(lhs)->m_value +
				static_cast<Type::Integer*>(rhs)->m_value
			);
		case value_type::Float:
			return e.new_float(
				static_cast<Type::Float*>(lhs)->m_value +
				static_cast<Type::Float*>(rhs)->m_value
			);
		case value_type::String:
			return e.new_string(
				static_cast<Type::String*>(lhs)->m_value +
				static_cast<Type::String*>(rhs)->m_value
			);
		default:
			std::cerr
				<< "ERROR: can't add values of type "
				<< value_type_string[static_cast<int>(lhs->type())];
			assert(0);
		}
	}
	case token_type::SUB: {
		
		// TODO: proper error handling
		assert(lhs->type() == rhs->type());

		switch (lhs->type()) {
		case value_type::Integer:
			return e.new_integer(
				static_cast<Type::Integer*>(lhs)->m_value -
				static_cast<Type::Integer*>(rhs)->m_value
			);
		case value_type::Float:
			return e.new_float(
				static_cast<Type::Float*>(lhs)->m_value -
				static_cast<Type::Float*>(rhs)->m_value
			);
		default:
			std::cerr
				<< "ERROR: can't subtract values of type "
				<< value_type_string[static_cast<int>(lhs->type())];
			assert(0);
		}
	}
	case token_type::MUL: {
		
		// TODO: proper error handling
		assert(lhs->type() == rhs->type());

		switch (lhs->type()) {
		case value_type::Integer:
			return e.new_integer(
				static_cast<Type::Integer*>(lhs)->m_value *
				static_cast<Type::Integer*>(rhs)->m_value
			);
		case value_type::Float:
			return e.new_float(
				static_cast<Type::Float*>(lhs)->m_value *
				static_cast<Type::Float*>(rhs)->m_value
			);
		default:
			std::cerr
				<< "ERROR: can't multiply values of type "
				<< value_type_string[static_cast<int>(lhs->type())];
			assert(0);
		}
	}
	case token_type::DIV: {
		
		// TODO: proper error handling
		assert(lhs->type() == rhs->type());

		switch (lhs->type()) {
		case value_type::Integer:
			return e.new_integer(
				static_cast<Type::Integer*>(lhs)->m_value /
				static_cast<Type::Integer*>(rhs)->m_value
			);
		case value_type::Float:
			return e.new_float(
				static_cast<Type::Float*>(lhs)->m_value /
				static_cast<Type::Float*>(rhs)->m_value
			);
		default:
			std::cerr
				<< "ERROR: can't divide values of type "
				<< value_type_string[static_cast<int>(lhs->type())];
			assert(0);
		}
	}
	case token_type::AND: {
		
		// TODO: proper error handling
		if (lhs->type() == value_type::Boolean and rhs->type() == value_type::Boolean)
			return e.new_boolean(
				static_cast<Type::Boolean*>(lhs)->m_value and
				static_cast<Type::Boolean*>(rhs)->m_value
			);
		std::cerr
			<< "ERROR: logical and operator not defined for types "
			<< value_type_string[static_cast<int>(lhs->type())] << " and "
			<< value_type_string[static_cast<int>(rhs->type())];
		assert(0);
	}
	case token_type::IOR: {
		
		// TODO: proper error handling
		if (lhs->type() == value_type::Boolean and rhs->type() == value_type::Boolean)
			return e.new_boolean(
				static_cast<Type::Boolean*>(lhs)->m_value or
				static_cast<Type::Boolean*>(rhs)->m_value
			);
		std::cerr
			<< "ERROR: logical or operator not defined for types "
			<< value_type_string[static_cast<int>(lhs->type())] << " and "
			<< value_type_string[static_cast<int>(rhs->type())];
		assert(0);
	}
	case token_type::XOR: {
		
		// TODO: proper error handling
		if (lhs->type() == value_type::Boolean and rhs->type() == value_type::Boolean)
			return e.new_boolean(
				static_cast<Type::Boolean*>(lhs)->m_value xor
				static_cast<Type::Boolean*>(rhs)->m_value
			);
		std::cerr
			<< "ERROR: exclusive or operator not defined for types "
			<< value_type_string[static_cast<int>(lhs->type())] << " and "
			<< value_type_string[static_cast<int>(rhs->type())];
		assert(0);
	}
	case token_type::EQUAL: {

		// TODO: proper error handling
		assert(lhs->type() == rhs->type());

		switch (lhs->type()) {
		case value_type::Null:
			return e.new_boolean(true);
		case value_type::Integer:
			return e.new_boolean(
				static_cast<Type::Integer*>(lhs)->m_value ==
				static_cast<Type::Integer*>(rhs)->m_value
			);
		case value_type::Float:
			return e.new_boolean(
				static_cast<Type::Float*>(lhs)->m_value ==
				static_cast<Type::Float*>(rhs)->m_value
			);
		case value_type::String:
			return e.new_boolean(
				static_cast<Type::String*>(lhs)->m_value ==
				static_cast<Type::String*>(rhs)->m_value
			);
		case value_type::Boolean:
			return e.new_boolean(
				static_cast<Type::Boolean*>(lhs)->m_value ==
				static_cast<Type::Boolean*>(rhs)->m_value
			);
		default: {
			std::cerr
				<< "ERROR: can't compare equality of types "
				<< value_type_string[static_cast<int>(lhs->type())] << " and "
				<< value_type_string[static_cast<int>(rhs->type())];
			assert(0);
		}
		}
	}
	default:
		std::cerr << "WARNING: not implemented action"
		             "(Evaluating binary expression)\n";
	}

	return nullptr;
}

Type::Value* eval(ASTFunctionLiteral* ast, Type::Environment& e) {
	std::unordered_map<std::string, Type::Value*> captures;
	for(auto const& identifier : ast->m_captures){
		captures[identifier] = e.m_scope->access(identifier);
	}
	return e.new_function(ast, captures);
};

Type::Value* eval(ASTIfStatement* ast, Type::Environment& e) {
	auto* condition_result = eval(ast->m_condition.get(), e);
	assert(condition_result);

	assert(condition_result->type() == value_type::Boolean);
	auto* condition_result_b = static_cast<Type::Boolean*>(condition_result);

	if(condition_result_b->m_value){
		eval(ast->m_body.get(), e);
	}

	return e.null();
};

Type::Value* eval(AST* ast, Type::Environment& e) {

	switch (ast->type()) {
	case ast_type::NumberLiteral:
		return eval(static_cast<ASTNumberLiteral*>(ast), e);
	case ast_type::StringLiteral:
		return eval(static_cast<ASTStringLiteral*>(ast), e);
	case ast_type::ObjectLiteral:
		return eval(static_cast<ASTObjectLiteral*>(ast), e);
	case ast_type::ArrayLiteral:
		return eval(static_cast<ASTArrayLiteral*>(ast), e);
	case ast_type::DictionaryLiteral:
		return eval(static_cast<ASTDictionaryLiteral*>(ast), e);
	case ast_type::FunctionLiteral:
		return eval(static_cast<ASTFunctionLiteral*>(ast), e);
	case ast_type::DeclarationList:
		return eval(static_cast<ASTDeclarationList*>(ast), e);
	case ast_type::Declaration:
		return eval(static_cast<ASTDeclaration*>(ast), e);
	case ast_type::Identifier:
		return eval(static_cast<ASTIdentifier*>(ast), e);
	case ast_type::BinaryExpression:
		return eval(static_cast<ASTBinaryExpression*>(ast), e);
	case ast_type::CallExpression:
		return eval(static_cast<ASTCallExpression*>(ast), e);
	case ast_type::ArgumentList:
		return eval(static_cast<ASTArgumentList*>(ast), e);
	case ast_type::Block:
		return eval(static_cast<ASTBlock*>(ast), e);
	case ast_type::ReturnStatement:
		return eval(static_cast<ASTReturnStatement*>(ast), e);
	case ast_type::IfStatement:
		return eval(static_cast<ASTIfStatement*>(ast), e);
	default:
		std::cerr << "big problem: unhandled case in eval\n";
	}

	return nullptr;
}
