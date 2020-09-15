#include "eval.hpp"

#include <iostream>

#include <cassert>

#include "../typed_ast.hpp"
#include "environment.hpp"
#include "utils.hpp"
#include "value.hpp"

namespace Interpreter {

gc_ptr<Value> eval(TypedAST::DeclarationList* ast, Environment& e) {
	for (auto& decl : ast->m_declarations) {
		assert(decl->type() == ASTType::Declaration);
		eval(static_cast<TypedAST::Declaration*>(decl.get()), e);
	}
	return e.null();
};

gc_ptr<Value> eval(TypedAST::Declaration* ast, Environment& e) {
	// TODO: type and mutable check -> return error
	e.declare(ast->identifier_text(), e.null());
	if (ast->m_value) {
		auto* ref = e.access(ast->identifier_text());
		auto value = eval(ast->m_value.get(), e);
		auto unboxed_val = unboxed(value.get());
		ref->m_value = unboxed_val;
	}
	return e.null();
};

gc_ptr<Value> eval(TypedAST::NumberLiteral* ast, Environment& e) {
	return e.new_float(std::stof(ast->text()));
}

gc_ptr<Value> eval(TypedAST::IntegerLiteral* ast, Environment& e) {
	return e.new_integer(std::stoi(ast->text()));
}

gc_ptr<Value> eval(TypedAST::StringLiteral* ast, Environment& e) {
	return e.new_string(ast->text());
};

gc_ptr<Value> eval(TypedAST::BooleanLiteral* ast, Environment& e) {
	bool b = ast->m_token->m_type == token_type::KEYWORD_TRUE;
	return e.new_boolean(b);
};

gc_ptr<Value> eval(TypedAST::NullLiteral* ast, Environment& e) {
	return e.null();
};

gc_ptr<Value> eval(TypedAST::ObjectLiteral* ast, Environment& e) {
	auto result = e.new_object({});

	for (auto& declTypeErased : ast->m_body) {
		assert(declTypeErased->type() == ASTType::Declaration);
		TypedAST::Declaration* decl =
		    static_cast<TypedAST::Declaration*>(declTypeErased.get());

		if (decl->m_value) {
			auto value = eval(decl->m_value.get(), e);
			result->m_value[decl->identifier_text()] = value.get();
		} else {
			std::cerr << "ERROR: declaration in object must have a value";
			assert(0);
		}
	}

	return result;
}

gc_ptr<Value> eval(TypedAST::DictionaryLiteral* ast, Environment& e) {
	auto result = e.new_dictionary({});

	for (auto& declTypeErased : ast->m_body) {
		assert(declTypeErased->type() == ASTType::Declaration);
		TypedAST::Declaration* decl =
		    static_cast<TypedAST::Declaration*>(declTypeErased.get());

		if (decl->m_value) {
			auto value = eval(decl->m_value.get(), e);
			result->m_value[decl->identifier_text()] = value.get();
		} else {
			std::cerr << "ERROR: declaration in dictionary must have value";
			assert(0);
		}
	}

	return result;
}

gc_ptr<Value> eval(TypedAST::ArrayLiteral* ast, Environment& e) {
	auto result = e.new_list({});
	result->m_value.reserve(ast->m_elements.size());
	for (auto& element : ast->m_elements) {
		auto value = eval(element.get(), e);
		result->m_value.push_back(unboxed(value.get()));
	}
	return result;
};

gc_ptr<Value> eval(TypedAST::Identifier* ast, Environment& e) {
	return e.access(ast->text());
};

gc_ptr<Value> eval(TypedAST::Block* ast, Environment& e) {

	e.new_nested_scope();

	for (auto& stmt : ast->m_body) {
		eval(stmt.get(), e);
		if (e.m_return_value)
			break;
	}

	e.end_scope();

	return e.null();
};

gc_ptr<Value> eval(TypedAST::ReturnStatement* ast, Environment& e) {
	// TODO: proper error handling
	auto value = eval(ast->m_value.get(), e);
	auto returning = unboxed(value.get());
	assert(returning);

	e.save_return_value(returning);
	return e.null();
};

auto is_callable_value(Value* v) -> bool {
	if (!v)
		return false;

	auto type = v->type();
	return type == value_type::Function || type == value_type::NativeFunction;
}

gc_ptr<Value> eval_call_function(
    gc_ptr<Function> callee, std::vector<gc_ptr<Value>> args, Environment& e) {

	// TODO: error handling ?
	assert(callee->m_def->m_args.size() == args.size());

	e.new_scope();
	for (int i = 0; i < int(callee->m_def->m_args.size()); ++i) {
		auto& argdecl = callee->m_def->m_args[i];
		e.declare(argdecl.identifier_text(), unboxed(args[i].get()));
	}
	// NOTE: we could `args.clear()` at this point. Is it worth doing?

	for (auto& kv : callee->m_captures) {
		assert(kv.second);
		assert(kv.second->type() == value_type::Reference);
		e.direct_declare(kv.first, static_cast<Reference*>(kv.second));
	}

	auto* body = dynamic_cast<TypedAST::Block*>(callee->m_def->m_body.get());
	assert(body);

	eval(body, e);

	e.end_scope();

	return e.fetch_return_value();
}

gc_ptr<Value> eval_call_native_function(
    gc_ptr<NativeFunction> callee, std::vector<gc_ptr<Value>> args, Environment& e) {
	// TODO: don't do this conversion
	std::vector<Value*> passable_args;
	passable_args.reserve(args.size());
	for (auto& arg : args)
		passable_args.push_back(arg.get());
	return callee->m_fptr(std::move(passable_args), e);
}

gc_ptr<Value> eval_call_callable(
    gc_ptr<Value> callee, std::vector<gc_ptr<Value>> args, Environment& e) {
	// TODO: proper error handling
	assert(is_callable_value(callee.get()));
	if (callee->type() == value_type::Function) {
		return eval_call_function(
		    static_cast<Function*>(callee.get()), std::move(args), e);
	} else if (callee->type() == value_type::NativeFunction) {
		return eval_call_native_function(
		    static_cast<NativeFunction*>(callee.get()), std::move(args), e);
	} else {
		assert(0);
		return nullptr;
	}
}

gc_ptr<Value> eval(TypedAST::CallExpression* ast, Environment& e) {

	auto value = eval(ast->m_callee.get(), e);
	auto* callee = unboxed(value.get());
	assert(callee);

	auto& arglist = ast->m_args;

	std::vector<gc_ptr<Value>> args;
	for (int i = 0; i < int(arglist.size()); ++i) {
		args.push_back(eval(arglist[i].get(), e));
	}

	return eval_call_callable(callee, std::move(args), e);
};

gc_ptr<Value> eval(TypedAST::IndexExpression* ast, Environment& e) {
	// TODO: proper error handling

	auto callee_value = eval(ast->m_callee.get(), e);
	auto* callee = unboxed(callee_value.get());
	assert(callee);
	assert(callee->type() == value_type::Array);

	auto index_value = eval(ast->m_index.get(), e);
	auto* index = unboxed(index_value.get());
	assert(index);
	assert(index->type() == value_type::Integer);

	auto* array_callee = static_cast<Array*>(callee);
	auto* int_index = static_cast<Integer*>(index);

	return array_callee->at(int_index->m_value);
};

gc_ptr<Value> eval(TypedAST::TernaryExpression* ast, Environment& e) {
	// TODO: proper error handling

	auto condition = eval(ast->m_condition.get(), e);
	auto* condition_value = unboxed(condition.get());
	assert(condition_value);
	assert(condition_value->type() == value_type::Boolean);

	return static_cast<Boolean*>(condition_value)->m_value
	       ? eval(ast->m_then_expr.get(), e)
	       : eval(ast->m_else_expr.get(), e);
};

gc_ptr<Value> eval(TypedAST::FunctionLiteral* ast, Environment& e) {
	auto result = e.new_function(ast, {});

	for (auto const& identifier : ast->m_captures) {
		result->m_captures[identifier] = e.m_scope->access(identifier);
	}

	return result;
};

gc_ptr<Value> eval(TypedAST::IfElseStatement* ast, Environment& e) {
	auto condition_result = eval(ast->m_condition.get(), e);
	assert(condition_result);

	assert(condition_result->type() == value_type::Boolean);
	auto* condition_result_b = static_cast<Boolean*>(condition_result.get());

	if (condition_result_b->m_value) {
		eval(ast->m_body.get(), e);
	} else if (ast->m_else_body) {
		eval(ast->m_else_body.get(), e);
	}

	return e.null();
};

gc_ptr<Value> eval(TypedAST::ForStatement* ast, Environment& e) {
	e.new_nested_scope();

	// NOTE: this is kinda fishy. why do we assert here?
	auto declaration = eval(ast->m_declaration.get(), e);
	assert(declaration);

	while (1) {
		auto condition_result = eval(ast->m_condition.get(), e);
		auto unboxed_condition_result = unboxed(condition_result.get());
		assert(unboxed_condition_result);

		assert(unboxed_condition_result->type() == value_type::Boolean);
		auto* condition_result_b = static_cast<Boolean*>(unboxed_condition_result);

		if (!condition_result_b->m_value)
			break;

		eval(ast->m_body.get(), e);

		if (e.m_return_value)
			break;

		// NOTE: this is kinda fishy. why do we assert here?
		auto loop_action = eval(ast->m_action.get(), e);
		assert(loop_action);
	}

	e.end_scope();

	return e.null();
};

gc_ptr<Value> eval(TypedAST::WhileStatement* ast, Environment& e) {
	e.new_nested_scope();

	while (1) {
		auto condition_result = eval(ast->m_condition.get(), e);
		auto unboxed_condition_result = unboxed(condition_result.get());
		assert(unboxed_condition_result);

		assert(unboxed_condition_result->type() == value_type::Boolean);
		auto* condition_result_b = static_cast<Boolean*>(unboxed_condition_result);

		if (!condition_result_b->m_value)
			break;

		eval(ast->m_body.get(), e);

		if (e.m_return_value)
			break;
	}

	e.end_scope();

	return e.null();
};

gc_ptr<Value> eval(TypedAST::TypedAST* ast, Environment& e) {

	switch (ast->type()) {
	case ASTType::NumberLiteral:
		return eval(static_cast<TypedAST::NumberLiteral*>(ast), e);
	case ASTType::IntegerLiteral:
		return eval(static_cast<TypedAST::IntegerLiteral*>(ast), e);
	case ASTType::StringLiteral:
		return eval(static_cast<TypedAST::StringLiteral*>(ast), e);
	case ASTType::BooleanLiteral:
		return eval(static_cast<TypedAST::BooleanLiteral*>(ast), e);
	case ASTType::NullLiteral:
		return eval(static_cast<TypedAST::NullLiteral*>(ast), e);
	case ASTType::ObjectLiteral:
		return eval(static_cast<TypedAST::ObjectLiteral*>(ast), e);
	case ASTType::ArrayLiteral:
		return eval(static_cast<TypedAST::ArrayLiteral*>(ast), e);
	case ASTType::DictionaryLiteral:
		return eval(static_cast<TypedAST::DictionaryLiteral*>(ast), e);
	case ASTType::FunctionLiteral:
		return eval(static_cast<TypedAST::FunctionLiteral*>(ast), e);
	case ASTType::DeclarationList:
		return eval(static_cast<TypedAST::DeclarationList*>(ast), e);
	case ASTType::Declaration:
		return eval(static_cast<TypedAST::Declaration*>(ast), e);
	case ASTType::Identifier:
		return eval(static_cast<TypedAST::Identifier*>(ast), e);
	case ASTType::CallExpression:
		return eval(static_cast<TypedAST::CallExpression*>(ast), e);
	case ASTType::IndexExpression:
		return eval(static_cast<TypedAST::IndexExpression*>(ast), e);
	case ASTType::TernaryExpression:
		return eval(static_cast<TypedAST::TernaryExpression*>(ast), e);
	case ASTType::Block:
		return eval(static_cast<TypedAST::Block*>(ast), e);
	case ASTType::ReturnStatement:
		return eval(static_cast<TypedAST::ReturnStatement*>(ast), e);
	case ASTType::IfElseStatement:
		return eval(static_cast<TypedAST::IfElseStatement*>(ast), e);
	case ASTType::ForStatement:
		return eval(static_cast<TypedAST::ForStatement*>(ast), e);
	case ASTType::WhileStatement:
		return eval(static_cast<TypedAST::WhileStatement*>(ast), e);
	case ASTType::BinaryExpression:
		assert(0);
	}

	std::cerr << "@ Internal Error: unhandled case in eval:\n";
	std::cerr << "@   - AST type is: " << ast_type_string[(int)ast->type()] << '\n';

	return nullptr;
}

} // namespace Interpreter
