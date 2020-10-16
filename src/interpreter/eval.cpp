#include "eval.hpp"

#include <iostream>

#include <cassert>
#include <climits>

#include "../span.hpp"
#include "../typed_ast.hpp"
#include "environment.hpp"
#include "utils.hpp"
#include "value.hpp"

namespace Interpreter {

static bool is_expression (TypedAST::TypedAST* ast){
	auto tag = ast->type();
	auto tag_idx = static_cast<int>(tag);
	return tag_idx < static_cast<int>(TypedASTTag::Block);
}

void eval(TypedAST::Declaration* ast, Environment& e) {
	auto ref = e.new_reference(e.null());
	e.push(ref.get());
	if (ast->m_value) {
		eval(ast->m_value, e);
		auto value = e.pop();
		ref->m_value = unboxed(value.get());
	}
};

void eval(TypedAST::DeclarationList* ast, Environment& e) {
	for (auto& decl : ast->m_declarations) {
		auto ref = e.new_reference(e.null());
		e.global_declare_direct(decl.identifier_text(), ref.get());
		eval(decl.m_value, e);
		auto value = e.pop();
		ref->m_value = value.get();
	}
};

void eval(TypedAST::NumberLiteral* ast, Environment& e) {
	e.push_float(ast->value());
}

void eval(TypedAST::IntegerLiteral* ast, Environment& e) {
	e.push_integer(ast->value());
}

void eval(TypedAST::StringLiteral* ast, Environment& e) {
	e.push(e.new_string(ast->text()).get());
};

void eval(TypedAST::BooleanLiteral* ast, Environment& e) {
	bool b = ast->m_token->m_type == TokenTag::KEYWORD_TRUE;
	e.push_boolean(b);
};

void eval(TypedAST::NullLiteral* ast, Environment& e) {
	e.push(e.null());
};

void eval(TypedAST::ObjectLiteral* ast, Environment& e) {
	auto result = e.new_object({});

	for (auto& decl : ast->m_body) {
		if (decl.m_value) {
			eval(decl.m_value, e);
			auto value = e.pop();
			result->m_value[decl.identifier_text()] = value.get();
		} else {
			std::cerr << "ERROR: declaration in object must have a value";
			assert(0);
		}
	}

	e.push(result.get());
}

void eval(TypedAST::DictionaryLiteral* ast, Environment& e) {
	auto result = e.new_dictionary({});

	for (auto& decl : ast->m_body) {
		if (decl.m_value) {
			eval(decl.m_value, e);
			auto value = e.pop();
			result->m_value[decl.identifier_text().str()] = value.get();
		} else {
			std::cerr << "ERROR: declaration in dictionary must have value";
			assert(0);
		}
	}

	e.push(result.get());
}

void eval(TypedAST::ArrayLiteral* ast, Environment& e) {
	auto result = e.new_list({});
	result->m_value.reserve(ast->m_elements.size());
	for (auto& element : ast->m_elements) {
		eval(element, e);
		auto value = e.pop();
		result->m_value.push_back(unboxed(value.get()));
	}
	e.push(result.get());
}

void eval(TypedAST::Identifier* ast, Environment& e) {
	if (ast->m_origin == TypedAST::Identifier::Origin::Local ||
	    ast->m_origin == TypedAST::Identifier::Origin::Capture) {
		if (ast->m_frame_offset == INT_MIN) {
			std::cerr << "MISSING LAYOUT FOR IDENTIFIER " << ast->text() << "\n";
			assert(0 && "MISSING LAYOUT FOR AN IDENTIFIER");
		}
		e.push(e.m_stack[e.m_frame_ptr + ast->m_frame_offset]);
	} else {
		e.push(e.global_access(ast->text()));
	}
};

void eval(TypedAST::Block* ast, Environment& e) {
	e.start_stack_region();
	for (auto stmt : ast->m_body) {
		eval(stmt, e);
		if (is_expression(stmt))
			e.pop();
		if (e.m_return_value)
			break;
	}
	e.end_stack_region();
};

void eval(TypedAST::ReturnStatement* ast, Environment& e) {
	// TODO: proper error handling
	eval(ast->m_value, e);
	auto value = e.pop();
	e.save_return_value(unboxed(value.get()));
};

auto is_callable_value(Value* v) -> bool {
	if (!v)
		return false;
	auto type = v->type();
	return type == ValueTag::Function || type == ValueTag::NativeFunction;
}

void eval_call_function(
    gc_ptr<Function> callee, int arg_count, Environment& e) {

	// TODO: error handling ?
	assert(callee->m_def->m_args.size() == arg_count);

	for (auto& kv : callee->m_captures) {
		assert(kv.second);
		assert(kv.second->type() == ValueTag::Reference);
		e.push(kv.second);
	}

	assert(callee->m_def->m_body->type() == TypedASTTag::Block);
	auto* body = static_cast<TypedAST::Block*>(callee->m_def->m_body);
	eval(body, e);
}

void eval_call_native_function(
    gc_ptr<NativeFunction> callee, int arg_count, Environment& e) {
	// TODO: don't do this conversion
	Span<Value*> args = {&e.m_stack[e.m_frame_ptr - arg_count], arg_count};
	e.save_return_value(callee->m_fptr(args, e));
}

void eval(TypedAST::CallExpression* ast, Environment& e) {

	eval(ast->m_callee, e);
	gc_ptr<Value> value = e.pop();
	auto* callee = unboxed(value.get());
	assert(callee);
	assert(is_callable_value(callee));

	auto& arglist = ast->m_args;
	int arg_count = arglist.size();

	e.start_stack_region();

	// arguments go before the frame pointer
	if (callee->type() == ValueTag::Function) {
		for (auto expr : arglist) {
			eval(expr, e);
			e.m_stack.back() = e.new_reference(unboxed(e.m_stack.back())).get();
		}
	} else {
		for (auto expr : arglist) {
			eval(expr, e);
		}
	}

	e.start_stack_frame();

	if (callee->type() == ValueTag::Function) {
		eval_call_function(static_cast<Function*>(callee), arg_count, e);
	} else if (callee->type() == ValueTag::NativeFunction) {
		eval_call_native_function(
		    static_cast<NativeFunction*>(callee), arg_count, e);
	} else {
		assert(0);
	}

	e.end_stack_frame();
	e.end_stack_region();

	e.push(e.fetch_return_value());
};

void eval(TypedAST::IndexExpression* ast, Environment& e) {
	// TODO: proper error handling

	eval(ast->m_callee, e);
	auto callee_value = e.pop();
	auto* callee = unboxed(callee_value.get());
	assert(callee);
	assert(callee->type() == ValueTag::Array);

	eval(ast->m_index, e);
	auto index_value = e.pop();
	auto* index = unboxed(index_value.get());
	assert(index);
	assert(index->type() == ValueTag::Integer);

	auto* array_callee = static_cast<Array*>(callee);
	auto* int_index = static_cast<Integer*>(index);

	e.push(array_callee->at(int_index->m_value));
};

void eval(TypedAST::TernaryExpression* ast, Environment& e) {
	// TODO: proper error handling

	eval(ast->m_condition, e);
	auto condition = e.pop();
	auto* condition_value = unboxed(condition.get());
	assert(condition_value);
	assert(condition_value->type() == ValueTag::Boolean);

	if (static_cast<Boolean*>(condition_value)->m_value)
		eval(ast->m_then_expr, e);
	else
		eval(ast->m_else_expr, e);
};

void eval(TypedAST::FunctionLiteral* ast, Environment& e) {
	auto result = e.new_function(ast, {});

	for (auto const& capture : ast->m_captures) {
		assert(capture.second.outer_frame_offset != INT_MIN);
		result->m_captures[capture.first] =
		    e.m_stack[e.m_frame_ptr + capture.second.outer_frame_offset];
	}

	e.push(result.get());
};

void eval(TypedAST::IfElseStatement* ast, Environment& e) {
	eval(ast->m_condition, e);
	auto condition_result = e.pop();
	assert(condition_result);

	assert(condition_result->type() == ValueTag::Boolean);
	auto* condition_result_b = static_cast<Boolean*>(condition_result.get());

	if (condition_result_b->m_value) {
		eval(ast->m_body, e);
		if (is_expression(ast->m_body))
			e.pop();
	} else if (ast->m_else_body) {
		eval(ast->m_else_body, e);
		if (is_expression(ast->m_else_body))
			e.pop();
	}
};

void eval(TypedAST::ForStatement* ast, Environment& e) {
	e.start_stack_region();

	// NOTE: this is kinda fishy. why do we assert here?
	eval(&ast->m_declaration, e);

	while (1) {
		eval(ast->m_condition, e);
		auto condition_result = e.pop();
		auto unboxed_condition_result = unboxed(condition_result.get());
		assert(unboxed_condition_result);

		assert(unboxed_condition_result->type() == ValueTag::Boolean);
		auto* condition_result_b = static_cast<Boolean*>(unboxed_condition_result);

		if (!condition_result_b->m_value)
			break;

		eval(ast->m_body, e);

		if (e.m_return_value)
			break;

		eval(ast->m_action, e);
		// i think this check is always true...
		if (is_expression(ast->m_action))
			e.pop();
	}

	e.end_stack_region();
};

void eval(TypedAST::WhileStatement* ast, Environment& e) {
	while (1) {
		eval(ast->m_condition, e);
		auto condition_result = e.pop();
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
};

void eval(TypedAST::TypedAST* ast, Environment& e) {

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
}

} // namespace Interpreter
