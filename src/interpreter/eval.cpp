#include "eval.hpp"

#include <iostream>

#include <cassert>
#include <climits>

#include "../typed_ast.hpp"
#include "environment.hpp"
#include "utils.hpp"
#include "value.hpp"

namespace Interpreter {

gc_ptr<Value> eval(TypedAST::Declaration* ast, Environment& e) {
	auto ref = e.new_reference(e.null());
	e.push_direct(ref.get());
	if (ast->m_value) {
		auto value = eval(ast->m_value, e);
		auto unboxed_val = unboxed(value.get());
		ref->m_value = unboxed_val;
	}
	return e.null();
};

gc_ptr<Value> eval(TypedAST::DeclarationList* ast, Environment& e) {
	for (auto& decl : ast->m_declarations) {
		auto ref = e.new_reference(e.null());
		e.global_declare_direct(decl.identifier_text(), ref.get());
		auto value = eval(decl.m_value, e);
		ref->m_value = value.get();
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
	bool b = ast->m_token->m_type == TokenTag::KEYWORD_TRUE;
	return e.new_boolean(b);
};

gc_ptr<Value> eval(TypedAST::NullLiteral* ast, Environment& e) {
	return e.null();
};

gc_ptr<Value> eval(TypedAST::ObjectLiteral* ast, Environment& e) {
	auto result = e.new_object({});

	for (auto& decl : ast->m_body) {
		if (decl.m_value) {
			auto value = eval(decl.m_value, e);
			result->m_value[decl.identifier_text()] = value.get();
		} else {
			std::cerr << "ERROR: declaration in object must have a value";
			assert(0);
		}
	}

	return result;
}

gc_ptr<Value> eval(TypedAST::DictionaryLiteral* ast, Environment& e) {
	auto result = e.new_dictionary({});

	for (auto& decl : ast->m_body) {
		if (decl.m_value) {
			auto value = eval(decl.m_value, e);
			result->m_value[decl.identifier_text().str()] = value.get();
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
		auto value = eval(element, e);
		result->m_value.push_back(unboxed(value.get()));
	}
	return result;
};

gc_ptr<Value> eval(TypedAST::Identifier* ast, Environment& e) {
	if (ast->m_origin == TypedAST::Identifier::Origin::Local ||
	    ast->m_origin == TypedAST::Identifier::Origin::Capture) {
		if (ast->m_frame_offset == -INT_MIN) {
			std::cerr << "MISSING LAYOUT FOR IDENTIFIER " << ast->text() << "\n";
			assert(0 && "MISSING LAYOUT FOR AN IDENTIFIER");
		}
		return e.m_stack[e.m_frame_ptr + ast->m_frame_offset];
	} else {
		// slow path
		return e.global_access(ast->text());
	}
};

gc_ptr<Value> eval(TypedAST::Block* ast, Environment& e) {

	e.start_stack_region();

	for (auto stmt : ast->m_body) {
		eval(stmt, e);
		if (e.m_return_value)
			break;
	}

	e.end_stack_region();

	return e.null();
};

gc_ptr<Value> eval(TypedAST::ReturnStatement* ast, Environment& e) {
	// TODO: proper error handling
	auto value = eval(ast->m_value, e);
	auto returning = unboxed(value.get());
	assert(returning);

	e.save_return_value(returning);
	return e.null();
};

auto is_callable_value(Value* v) -> bool {
	if (!v)
		return false;

	auto type = v->type();
	return type == ValueTag::Function || type == ValueTag::NativeFunction;
}

gc_ptr<Value> eval_call_function(
    gc_ptr<Function> callee, int arg_count, Environment& e) {

	// TODO: error handling ?
	assert(callee->m_def->m_args.size() == arg_count);

	// TODO: Do this more nicely.
	for (int i = e.m_frame_ptr - arg_count; i < e.m_frame_ptr; ++i)
		e.m_stack[i] = e.new_reference(unboxed(e.m_stack[i])).get();

	for (auto& kv : callee->m_captures) {
		assert(kv.second);
		assert(kv.second->type() == ValueTag::Reference);
		e.push_direct(static_cast<Reference*>(kv.second));
	}

	auto* body = dynamic_cast<TypedAST::Block*>(callee->m_def->m_body);
	assert(body);
	eval(body, e);

	return e.fetch_return_value();
}

gc_ptr<Value> eval_call_native_function(
    gc_ptr<NativeFunction> callee, int arg_count, Environment& e) {
	// TODO: don't do this conversion
	std::vector<Value*> args;
	args.reserve(arg_count);
	for (int i = e.m_frame_ptr - arg_count; i < e.m_frame_ptr; ++i)
		args.push_back(e.m_stack[i]);
	return callee->m_fptr(std::move(args), e);
}

gc_ptr<Value> eval(TypedAST::CallExpression* ast, Environment& e) {

	auto value = eval(ast->m_callee, e);
	auto* callee = unboxed(value.get());
	assert(callee);
	assert(is_callable_value(callee));

	auto& arglist = ast->m_args;
	int arg_count = arglist.size();

	std::vector<gc_ptr<Value>> args;
	args.reserve(arg_count);
	for (int i = 0; i < arg_count; ++i) {
		args.push_back(eval(arglist[i], e));
	}

	// arguments go before the frame pointer
	for(auto& value : args){
		if (value->type() == ValueTag::Reference)
			e.push_direct(static_cast<Reference*>(value.get()));
		else
			e.push(value.get());
	}

	e.start_stack_frame();

	gc_ptr<Value> result = nullptr;
	if (callee->type() == ValueTag::Function) {
		result = eval_call_function(static_cast<Function*>(callee), arg_count, e);
	} else if (callee->type() == ValueTag::NativeFunction) {
		result = eval_call_native_function(
		    static_cast<NativeFunction*>(callee), arg_count, e);
	} else {
		assert(0);
	}

	e.end_stack_frame();

	return result;
};

gc_ptr<Value> eval(TypedAST::IndexExpression* ast, Environment& e) {
	// TODO: proper error handling

	auto callee_value = eval(ast->m_callee, e);
	auto* callee = unboxed(callee_value.get());
	assert(callee);
	assert(callee->type() == ValueTag::Array);

	auto index_value = eval(ast->m_index, e);
	auto* index = unboxed(index_value.get());
	assert(index);
	assert(index->type() == ValueTag::Integer);

	auto* array_callee = static_cast<Array*>(callee);
	auto* int_index = static_cast<Integer*>(index);

	return array_callee->at(int_index->m_value);
};

gc_ptr<Value> eval(TypedAST::TernaryExpression* ast, Environment& e) {
	// TODO: proper error handling

	auto condition = eval(ast->m_condition, e);
	auto* condition_value = unboxed(condition.get());
	assert(condition_value);
	assert(condition_value->type() == ValueTag::Boolean);

	return static_cast<Boolean*>(condition_value)->m_value
	       ? eval(ast->m_then_expr, e)
	       : eval(ast->m_else_expr, e);
};

gc_ptr<Value> eval(TypedAST::FunctionLiteral* ast, Environment& e) {
	auto result = e.new_function(ast, {});

	for (auto const& capture : ast->m_captures) {
		assert(capture.second.outer_frame_offset != INT_MIN);
		result->m_captures[capture.first] =
		    e.m_stack[e.m_frame_ptr + capture.second.outer_frame_offset];
	}

	return result;
};

gc_ptr<Value> eval(TypedAST::IfElseStatement* ast, Environment& e) {
	auto condition_result = eval(ast->m_condition, e);
	assert(condition_result);

	assert(condition_result->type() == ValueTag::Boolean);
	auto* condition_result_b = static_cast<Boolean*>(condition_result.get());

	if (condition_result_b->m_value) {
		eval(ast->m_body, e);
	} else if (ast->m_else_body) {
		eval(ast->m_else_body, e);
	}

	return e.null();
};

gc_ptr<Value> eval(TypedAST::ForStatement* ast, Environment& e) {

	e.start_stack_region();

	// NOTE: this is kinda fishy. why do we assert here?
	auto declaration = eval(&ast->m_declaration, e);
	assert(declaration);

	while (1) {
		auto condition_result = eval(ast->m_condition, e);
		auto unboxed_condition_result = unboxed(condition_result.get());
		assert(unboxed_condition_result);

		assert(unboxed_condition_result->type() == ValueTag::Boolean);
		auto* condition_result_b = static_cast<Boolean*>(unboxed_condition_result);

		if (!condition_result_b->m_value)
			break;

		eval(ast->m_body, e);

		if (e.m_return_value)
			break;

		// NOTE: this is kinda fishy. why do we assert here?
		auto loop_action = eval(ast->m_action, e);
		assert(loop_action);
	}

	e.end_stack_region();

	return e.null();
};

gc_ptr<Value> eval(TypedAST::WhileStatement* ast, Environment& e) {
	while (1) {
		auto condition_result = eval(ast->m_condition, e);
		auto unboxed_condition_result = unboxed(condition_result.get());
		assert(unboxed_condition_result);

		assert(unboxed_condition_result->type() == ValueTag::Boolean);
		auto* condition_result_b = static_cast<Boolean*>(unboxed_condition_result);

		if (!condition_result_b->m_value)
			break;

		eval(ast->m_body, e);

		if (e.m_return_value)
			break;
	}

	return e.null();
};

gc_ptr<Value> eval(TypedAST::TypedAST* ast, Environment& e) {

#define DISPATCH(type)                                                         \
	case TypedASTTag::type:                                                    \
		return eval(static_cast<TypedAST::type*>(ast), e)

	switch (ast->type()) {
		DISPATCH(NumberLiteral);
		DISPATCH(IntegerLiteral);
		DISPATCH(StringLiteral);
		DISPATCH(BooleanLiteral);
		DISPATCH(NullLiteral);
		DISPATCH(ObjectLiteral);
		DISPATCH(ArrayLiteral);
		DISPATCH(DictionaryLiteral);
		DISPATCH(FunctionLiteral);
		DISPATCH(DeclarationList);
		DISPATCH(Declaration);
		DISPATCH(Identifier);
		DISPATCH(CallExpression);
		DISPATCH(IndexExpression);
		DISPATCH(TernaryExpression);
		DISPATCH(Block);
		DISPATCH(ReturnStatement);
		DISPATCH(IfElseStatement);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);
	}

	std::cerr << "@ Internal Error: unhandled case in eval:\n";
	std::cerr << "@   - AST type is: " << typed_ast_string[(int)ast->type()] << '\n';

	return nullptr;
}

} // namespace Interpreter
