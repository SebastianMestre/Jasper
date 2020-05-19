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
	e.declare(ast->identifier_text(), e.null());
	if (ast->m_value) {
		auto* ref = e.access(ast->identifier_text());
		auto val = unboxed(eval(ast->m_value.get(), e));
		ref->m_value = val;
	}

	return e.null();
};

Type::Value* eval(ASTNumberLiteral* ast, Type::Environment& e) {
	for (char a : ast->text())
		if (a == '.')
			return e.new_float(std::stof(ast->text()));

	return e.new_integer(std::stoi(ast->text()));
};

Type::Value* eval(ASTStringLiteral* ast, Type::Environment& e) {
	return e.new_string(ast->text());
};

Type::Value* eval(ASTBooleanLiteral* ast, Type::Environment& e) {
	bool b = ast->m_token->m_type == token_type::KEYWORD_TRUE;
	return e.new_boolean(b);
};

Type::Value* eval(ASTNullLiteral* ast, Type::Environment& e) {
	return e.null();
};

Type::Value* eval(ASTObjectLiteral* ast, Type::Environment& e) {
	Type::ObjectType declarations;

	for(auto& declTypeErased : ast->m_body) {
		assert(declTypeErased->type() == ast_type::Declaration);
		ASTDeclaration* decl = static_cast<ASTDeclaration*>(declTypeErased.get());

		if (decl->m_value) {
			declarations[decl->identifier_text()] = eval(decl->m_value.get(), e);
		}
		else {
			std::cerr << "ERROR: declaration in object must have a value";
			assert(0);
		}

	}
	return e.new_object(std::move(declarations));
};

Type::Value* eval(ASTDictionaryLiteral* ast, Type::Environment& e) {
	Type::ObjectType declarations;

	for(auto& declTypeErased : ast->m_body) {
		assert(declTypeErased->type() == ast_type::Declaration);
		ASTDeclaration* decl = static_cast<ASTDeclaration*>(declTypeErased.get());

		if (decl->m_value) {
			declarations[decl->identifier_text()] = eval(decl->m_value.get(), e);
		}
		else {
			std::cerr << "ERROR: declaration in dictionary must have value";
			assert(0);
		}
	}
	return e.new_dictionary(std::move(declarations));
};

Type::Value* eval(ASTArrayLiteral* ast, Type::Environment& e) {
	std::vector<Type::Value*> elements;
	for(auto& element : ast->m_elements){
		elements.push_back(eval(element.get(), e));
	}
	return e.new_list(std::move(elements));
};

Type::Value* eval(ASTIdentifier* ast, Type::Environment& e) {
	return e.access(ast->text());
};

Type::Value* eval(ASTBlock* ast, Type::Environment& e) {

	e.new_nested_scope();

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
	auto* returning = unboxed(eval(ast->m_value.get(), e));
	assert(returning);

	e.save_return_value(returning);
	return e.null();
};

Type::Value* eval(ASTArgumentList* ast, Type::Environment& e) {
	// TODO: return as list?
	assert(0);
	return e.null();
};

auto is_callable_value (Type::Value* v) -> bool {
	if (!v)
		return false;

	auto type = v->type();
	return type == value_type::Function || type == value_type::NativeFunction;
}

Type::Value* eval_call_function(Type::Function* callee, ASTCallExpression* ast, Type::Environment& e){

	auto* arglist = dynamic_cast<ASTArgumentList*>(ast->m_args.get());
	assert(callee->m_def->m_args.size() == arglist->m_args.size());


	std::vector<Type::Value*> args;
	for (int i = 0; i < int(callee->m_def->m_args.size()); ++i) {
		auto* argvalue = unboxed(eval(arglist->m_args[i].get(), e));
		assert(argvalue->type() != value_type::Reference);
		args.push_back(argvalue);
	}

	e.new_scope();
	for (int i = 0; i < int(callee->m_def->m_args.size()); ++i) {
		auto* argdeclTypeErased = callee->m_def->m_args[i].get();
		assert(argdeclTypeErased);

		assert(argdeclTypeErased->type() == ast_type::Declaration);
		auto* argdecl = dynamic_cast<ASTDeclaration*>(argdeclTypeErased);
		assert(argdecl);

		e.declare(argdecl->identifier_text(), args[i]);
	}

	for (auto& kv : callee->m_captures) {
		assert(kv.second);
		assert(kv.second->type() == value_type::Reference);
		e.direct_declare(kv.first, static_cast<Type::Reference*>(kv.second));
	}

	auto* body = dynamic_cast<ASTBlock*>(callee->m_def->m_body.get());
	assert(body);

	eval(body, e);

	e.end_scope();

	return e.fetch_return_value();
}

Type::Value* eval_call_native_function(Type::NativeFunction* callee, ASTCallExpression* ast, Type::Environment& e) {
	auto* arglist = dynamic_cast<ASTArgumentList*>(ast->m_args.get());
	assert(arglist);
	assert(arglist->m_args.size() == 1);

	auto* argvalue = eval(arglist->m_args[0].get(), e);
	callee->m_fptr(argvalue, e);

	return e.fetch_return_value();
}

Type::Value* eval(ASTCallExpression* ast, Type::Environment& e) {
	std::vector<Type::Value*> args;

	// TODO: proper error handling

	auto* callee = unboxed(eval(ast->m_callee.get(), e));
	assert(callee);

	assert(is_callable_value(callee));
	if (callee->type() == value_type::Function) {
		return eval_call_function(static_cast<Type::Function*>(callee), ast, e);
	}else if(callee->type() == value_type::NativeFunction){
		return eval_call_native_function(static_cast<Type::NativeFunction*>(callee), ast, e);
	}else{
		assert(0);
		return nullptr;
	}
};

Type::Value* eval(ASTIndexExpression* ast, Type::Environment& e) {
	// TODO: proper error handling

	auto* callee = unboxed(eval(ast->m_callee.get(), e));
	assert(callee);
	assert(callee->type() == value_type::Array);

	auto* index = unboxed(eval(ast->m_index.get(), e));
	assert(index);
	assert(index->type() == value_type::Integer);

	auto* array_callee = static_cast<Type::Array*>(callee);
	auto* int_index = static_cast<Type::Integer*>(index);

	return array_callee->at(int_index->m_value);
};

Type::Value* eval(ASTBinaryExpression* ast, Type::Environment& e) {

	// NOTE: lhs_ref and rhs_ref can still be plain values
	// unboxing only guarantees that
	auto* lhs_ref = eval(ast->m_lhs.get(), e);
	auto* rhs_ref = eval(ast->m_rhs.get(), e);
	auto* lhs = unboxed(lhs_ref);
	auto* rhs = unboxed(rhs_ref);

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
	case token_type::LT: {
		// TODO: proper error handling
		assert(lhs->type() == rhs->type());

		switch (lhs->type()) {
		case value_type::Integer:
			return e.new_boolean(
				static_cast<Type::Integer*>(lhs)->m_value <
				static_cast<Type::Integer*>(rhs)->m_value
			);
		case value_type::Float:
			return e.new_boolean(
				static_cast<Type::Float*>(lhs)->m_value <
				static_cast<Type::Float*>(rhs)->m_value
			);
		case value_type::String:
			return e.new_boolean(
				static_cast<Type::String*>(lhs)->m_value <
				static_cast<Type::String*>(rhs)->m_value
			);
		default:
			std::cerr
				<< "ERROR: can't compare values of type "
				<< value_type_string[static_cast<int>(lhs->type())];
			assert(0);
		}
	}
	case token_type::ASSIGN: {
		// TODO: proper error handling
		assert(lhs_ref->type() == value_type::Reference);
		// NOTE: copied by reference, matters if rhs_ref is actually a reference
		// TODO: change in another pr, perhaps adding Environment::copy_value?
		static_cast<Type::Reference*>(lhs_ref)->m_value = rhs;
		return e.null();
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
	return e.new_function(ast, std::move(captures));
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

Type::Value* eval(ASTForStatement* ast, Type::Environment& e) {
	e.new_nested_scope();
	
	auto* declaration = eval(ast->m_declaration.get(), e);
	assert(declaration);

	while (1) {
		auto* condition_result = eval(ast->m_condition.get(), e);
		assert(condition_result);

		assert(condition_result->type() == value_type::Boolean);
		auto* condition_result_b = static_cast<Type::Boolean*>(condition_result);

		if (condition_result_b->m_value){
			eval(ast->m_body.get(), e);
		} else {
			break;
		}

		auto* loop_action = eval(ast->m_action.get(), e);
		assert(loop_action);
	}

	e.end_scope();

	return e.null();
};

Type::Value* eval(AST* ast, Type::Environment& e) {

	switch (ast->type()) {
	case ast_type::NumberLiteral:
		return eval(static_cast<ASTNumberLiteral*>(ast), e);
	case ast_type::StringLiteral:
		return eval(static_cast<ASTStringLiteral*>(ast), e);
	case ast_type::BooleanLiteral:
		return eval(static_cast<ASTBooleanLiteral*>(ast), e);
	case ast_type::NullLiteral:
		return eval(static_cast<ASTNullLiteral*>(ast), e);
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
	case ast_type::IndexExpression:
		return eval(static_cast<ASTIndexExpression*>(ast), e);
	case ast_type::ArgumentList:
		return eval(static_cast<ASTArgumentList*>(ast), e);
	case ast_type::Block:
		return eval(static_cast<ASTBlock*>(ast), e);
	case ast_type::ReturnStatement:
		return eval(static_cast<ASTReturnStatement*>(ast), e);
	case ast_type::IfStatement:
		return eval(static_cast<ASTIfStatement*>(ast), e);
	case ast_type::ForStatement:
		return eval(static_cast<ASTForStatement*>(ast), e);
	default:
		std::cerr << "@ Internal Error: unhandled case in eval:\n";
		std::cerr << "@   - AST type is: " << ast_type_string[(int)ast->type()] << '\n';
	}

	return nullptr;
}
