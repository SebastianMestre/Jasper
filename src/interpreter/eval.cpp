#include "eval.hpp"

#include <iostream>

#include <cassert>

#include "../typed_ast.hpp"
#include "environment.hpp"
#include "value.hpp"

Type::Value* eval(TypedAST::DeclarationList* ast, Type::Environment& e) {
	for(auto& decl : ast->m_declarations){
		assert(decl->type() == ast_type::Declaration);
		eval(static_cast<TypedAST::Declaration*>(decl.get()), e);
	}
	return nullptr;
};

Type::Value* eval(TypedAST::Declaration* ast, Type::Environment& e) {
	// TODO: type and mutable check -> return error
	e.declare(ast->identifier_text(), e.null());
	if (ast->m_value) {
		auto* ref = e.access(ast->identifier_text());
		auto val = unboxed(eval(ast->m_value.get(), e));
		ref->m_value = val;
	}

	return e.null();
};

Type::Value* eval(TypedAST::NumberLiteral* ast, Type::Environment& e) {
	for (char a : ast->text())
		if (a == '.')
			return e.new_float(std::stof(ast->text()));

	return e.new_integer(std::stoi(ast->text()));
};

Type::Value* eval(TypedAST::StringLiteral* ast, Type::Environment& e) {
	return e.new_string(ast->text());
};

Type::Value* eval(TypedAST::BooleanLiteral* ast, Type::Environment& e) {
	bool b = ast->m_token->m_type == token_type::KEYWORD_TRUE;
	return e.new_boolean(b);
};

Type::Value* eval(TypedAST::NullLiteral* ast, Type::Environment& e) {
	return e.null();
};

Type::Value* eval(TypedAST::ObjectLiteral* ast, Type::Environment& e) {
	Type::ObjectType declarations;

	for(auto& declTypeErased : ast->m_body) {
		assert(declTypeErased->type() == ast_type::Declaration);
		TypedAST::Declaration* decl = static_cast<TypedAST::Declaration*>(declTypeErased.get());

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

Type::Value* eval(TypedAST::DictionaryLiteral* ast, Type::Environment& e) {
	Type::ObjectType declarations;

	for(auto& declTypeErased : ast->m_body) {
		assert(declTypeErased->type() == ast_type::Declaration);
		TypedAST::Declaration* decl = static_cast<TypedAST::Declaration*>(declTypeErased.get());

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

Type::Value* eval(TypedAST::ArrayLiteral* ast, Type::Environment& e) {
	std::vector<Type::Value*> elements;
	for(auto& element : ast->m_elements){
		elements.push_back(unboxed(eval(element.get(), e)));
	}
	return e.new_list(std::move(elements));
};

Type::Value* eval(TypedAST::Identifier* ast, Type::Environment& e) {
	return e.access(ast->text());
};

Type::Value* eval(TypedAST::Block* ast, Type::Environment& e) {

	e.new_nested_scope();

	for(auto &stmt : ast->m_body){
		eval(stmt.get(), e);
		if (e.m_return_value)
			break;
	}

	e.end_scope();

	return e.null();
};

Type::Value* eval(TypedAST::ReturnStatement* ast, Type::Environment& e) {
	// TODO: proper error handling
	auto* returning = unboxed(eval(ast->m_value.get(), e));
	assert(returning);

	e.save_return_value(returning);
	return e.null();
};

auto is_callable_value (Type::Value* v) -> bool {
	if (!v)
		return false;

	auto type = v->type();
	return type == value_type::Function || type == value_type::NativeFunction;
}

Type::Value* eval_call_function(
    Type::Function* callee, std::vector<Type::Value*> args, Type::Environment& e) {

	// TODO: error handling ?
	assert(callee->m_def->m_args.size() == args.size());

	e.new_scope();
	for (int i = 0; i < int(callee->m_def->m_args.size()); ++i) {
		auto& argdecl = callee->m_def->m_args[i];
		e.declare(argdecl.identifier_text(), unboxed(args[i]));
	}

	for (auto& kv : callee->m_captures) {
		assert(kv.second);
		assert(kv.second->type() == value_type::Reference);
		e.direct_declare(kv.first, static_cast<Type::Reference*>(kv.second));
	}

	auto* body = dynamic_cast<TypedAST::Block*>(callee->m_def->m_body.get());
	assert(body);

	eval(body, e);

	e.end_scope();

	return e.fetch_return_value();
}

Type::Value* eval_call_native_function(
    Type::NativeFunction* callee,
    std::vector<Type::Value*> args,
    Type::Environment& e) {
	return callee->m_fptr(std::move(args), e);
}

Type::Value* eval_call_callable(Type::Value* callee, std::vector<Type::Value*> args, Type::Environment& e){
	assert(is_callable_value(callee));
	if (callee->type() == value_type::Function) {
		return eval_call_function(
		    static_cast<Type::Function*>(callee), std::move(args), e);
	} else if (callee->type() == value_type::NativeFunction) {
		return eval_call_native_function(
		    static_cast<Type::NativeFunction*>(callee), std::move(args), e);
	} else {
		assert(0);
		return nullptr;
	}
}

Type::Value* eval(TypedAST::CallExpression* ast, Type::Environment& e) {
	// TODO: proper error handling

	auto* callee = unboxed(eval(ast->m_callee.get(), e));
	assert(callee);

	auto& arglist = ast->m_args;

	std::vector<Type::Value*> args;
	for (int i = 0; i < int(arglist.size()); ++i) {
		auto* argvalue = eval(arglist[i].get(), e);
		args.push_back(argvalue);
	}

	return eval_call_callable(callee, std::move(args), e);
};

Type::Value* eval(TypedAST::IndexExpression* ast, Type::Environment& e) {
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


Type::Value* eval(TypedAST::FunctionLiteral* ast, Type::Environment& e) {
	std::unordered_map<std::string, Type::Value*> captures;

	for(auto const& identifier : ast->m_captures){
		captures[identifier] = e.m_scope->access(identifier);
	}

	return e.new_function(ast, std::move(captures));
};

Type::Value* eval(TypedAST::IfStatement* ast, Type::Environment& e) {
	auto* condition_result = eval(ast->m_condition.get(), e);
	assert(condition_result);

	assert(condition_result->type() == value_type::Boolean);
	auto* condition_result_b = static_cast<Type::Boolean*>(condition_result);

	if(condition_result_b->m_value){
		eval(ast->m_body.get(), e);
	}

	return e.null();
};

Type::Value* eval(TypedAST::ForStatement* ast, Type::Environment& e) {
	e.new_nested_scope();
	
	auto* declaration = eval(ast->m_declaration.get(), e);
	assert(declaration);

	while (1) {
		auto* condition_result = unboxed(eval(ast->m_condition.get(), e));
		assert(condition_result);

		assert(condition_result->type() == value_type::Boolean);
		auto* condition_result_b = static_cast<Type::Boolean*>(condition_result);

		if (!condition_result_b->m_value)
			break;
		
		eval(ast->m_body.get(), e);

		if (e.m_return_value)
			break;

		auto* loop_action = eval(ast->m_action.get(), e);
		assert(loop_action);
	}

	e.end_scope();

	return e.null();
};

Type::Value* eval(TypedAST::TypedAST* ast, Type::Environment& e) {

	switch (ast->type()) {
	case ast_type::NumberLiteral:
		return eval(static_cast<TypedAST::NumberLiteral*>(ast), e);
	case ast_type::StringLiteral:
		return eval(static_cast<TypedAST::StringLiteral*>(ast), e);
	case ast_type::BooleanLiteral:
		return eval(static_cast<TypedAST::BooleanLiteral*>(ast), e);
	case ast_type::NullLiteral:
		return eval(static_cast<TypedAST::NullLiteral*>(ast), e);
	case ast_type::ObjectLiteral:
		return eval(static_cast<TypedAST::ObjectLiteral*>(ast), e);
	case ast_type::ArrayLiteral:
		return eval(static_cast<TypedAST::ArrayLiteral*>(ast), e);
	case ast_type::DictionaryLiteral:
		return eval(static_cast<TypedAST::DictionaryLiteral*>(ast), e);
	case ast_type::FunctionLiteral:
		return eval(static_cast<TypedAST::FunctionLiteral*>(ast), e);
	case ast_type::DeclarationList:
		return eval(static_cast<TypedAST::DeclarationList*>(ast), e);
	case ast_type::Declaration:
		return eval(static_cast<TypedAST::Declaration*>(ast), e);
	case ast_type::Identifier:
		return eval(static_cast<TypedAST::Identifier*>(ast), e);
	case ast_type::CallExpression:
		return eval(static_cast<TypedAST::CallExpression*>(ast), e);
	case ast_type::IndexExpression:
		return eval(static_cast<TypedAST::IndexExpression*>(ast), e);
	case ast_type::Block:
		return eval(static_cast<TypedAST::Block*>(ast), e);
	case ast_type::ReturnStatement:
		return eval(static_cast<TypedAST::ReturnStatement*>(ast), e);
	case ast_type::IfStatement:
		return eval(static_cast<TypedAST::IfStatement*>(ast), e);
	case ast_type::ForStatement:
		return eval(static_cast<TypedAST::ForStatement*>(ast), e);
	case ast_type::BinaryExpression:
		assert(0);
	default:
		std::cerr << "@ Internal Error: unhandled case in eval:\n";
		std::cerr << "@   - AST type is: " << ast_type_string[(int)ast->type()] << '\n';
	}

	return nullptr;
}
