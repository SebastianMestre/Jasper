#include "eval.hpp"

#include <iostream>

#include <cassert>
#include <climits>

#include "../typechecker.hpp"
#include "../typed_ast.hpp"
#include "../utils/span.hpp"
#include "garbage_collector.hpp"
#include "interpreter.hpp"
#include "utils.hpp"
#include "value.hpp"

namespace Interpreter {

static bool is_expression (TypedAST::TypedAST* ast){
	auto tag = ast->type();
	auto tag_idx = static_cast<int>(tag);
	return tag_idx < static_cast<int>(TypedASTTag::Block);
}

void eval(TypedAST::Declaration* ast, Interpreter& e) {
	auto ref = e.new_reference(e.null());
	e.m_env.push(ref.get());
	if (ast->m_value) {
		eval(ast->m_value, e);
		auto value = e.m_env.pop();
		ref->m_value = unboxed(value.get());
	}
};

void eval(TypedAST::DeclarationList* ast, Interpreter& e) {
	for (auto& decl : ast->m_declarations) {
		auto ref = e.new_reference(e.null());
		e.global_declare_direct(decl.identifier_text(), ref.get());
		eval(decl.m_value, e);
		auto value = e.m_env.pop();
		ref->m_value = value.get();
	}
};

void eval(TypedAST::NumberLiteral* ast, Interpreter& e) {
	e.push_float(ast->value());
}

void eval(TypedAST::IntegerLiteral* ast, Interpreter& e) {
	e.push_integer(ast->value());
}

void eval(TypedAST::StringLiteral* ast, Interpreter& e) {
	e.push_string(ast->text());
};

void eval(TypedAST::BooleanLiteral* ast, Interpreter& e) {
	bool b = ast->m_token->m_type == TokenTag::KEYWORD_TRUE;
	e.push_boolean(b);
};

void eval(TypedAST::NullLiteral* ast, Interpreter& e) {
	e.m_env.push(e.null());
};

void eval(TypedAST::ObjectLiteral* ast, Interpreter& e) {
	auto result = e.new_object({});

	for (auto& decl : ast->m_body) {
		if (decl.m_value) {
			eval(decl.m_value, e);
			auto value = e.m_env.pop();
			result->m_value[decl.identifier_text()] = value.get();
		} else {
			std::cerr << "ERROR: declaration in object must have a value";
			assert(0);
		}
	}

	e.m_env.push(result.get());
}

void eval(TypedAST::DictionaryLiteral* ast, Interpreter& e) {
	auto result = e.new_dictionary({});

	for (auto& decl : ast->m_body) {
		if (decl.m_value) {
			eval(decl.m_value, e);
			auto value = e.m_env.pop();
			result->m_value[decl.identifier_text().str()] = value.get();
		} else {
			std::cerr << "ERROR: declaration in dictionary must have value";
			assert(0);
		}
	}

	e.m_env.push(result.get());
}

void eval(TypedAST::ArrayLiteral* ast, Interpreter& e) {
	auto result = e.new_list({});
	result->m_value.reserve(ast->m_elements.size());
	for (auto& element : ast->m_elements) {
		eval(element, e);
		auto value = e.m_env.pop();
		result->m_value.push_back(unboxed(value.get()));
	}
	e.m_env.push(result.get());
}

void eval(TypedAST::Identifier* ast, Interpreter& e) {
	if (ast->m_origin == TypedAST::Identifier::Origin::Local ||
	    ast->m_origin == TypedAST::Identifier::Origin::Capture) {
		if (ast->m_frame_offset == INT_MIN) {
			std::cerr << "MISSING LAYOUT FOR IDENTIFIER " << ast->text() << "\n";
			assert(0 && "MISSING LAYOUT FOR AN IDENTIFIER");
		}
		e.m_env.push(e.m_env.m_stack[e.m_env.m_frame_ptr + ast->m_frame_offset]);
	} else {
		e.m_env.push(e.global_access(ast->text()));
	}
};

void eval(TypedAST::Block* ast, Interpreter& e) {
	e.m_env.start_stack_region();
	for (auto stmt : ast->m_body) {
		eval(stmt, e);
		if (is_expression(stmt))
			e.m_env.pop();
		if (e.m_return_value)
			break;
	}
	e.m_env.end_stack_region();
};

void eval(TypedAST::ReturnStatement* ast, Interpreter& e) {
	// TODO: proper error handling
	eval(ast->m_value, e);
	auto value = e.m_env.pop();
	e.save_return_value(unboxed(value.get()));
};

auto is_callable_value(Value* v) -> bool {
	if (!v)
		return false;
	auto type = v->type();
	return type == ValueTag::Function || type == ValueTag::NativeFunction;
}

void eval_call_function(
    gc_ptr<Function> callee, int arg_count, Interpreter& e) {

	// TODO: error handling ?
	assert(callee->m_def->m_args.size() == arg_count);

	// TODO: This is a big hack. pushing nullptr into the
	// stack should actually never happen.
	for (int i = callee->m_captures.size(); i--;)
		e.m_env.push(nullptr);

	for (auto& kv : callee->m_captures) {
		assert(kv.second);
		assert(kv.second->type() == ValueTag::Reference);
		auto capture_value = unboxed(kv.second);
		auto offset = callee->m_def->m_captures[kv.first].inner_frame_offset;
		e.m_env.m_stack[e.m_env.m_frame_ptr + offset] = kv.second;
	}

	assert(callee->m_def->m_body->type() == TypedASTTag::Block);
	auto* body = static_cast<TypedAST::Block*>(callee->m_def->m_body);
	eval(body, e);
}

void eval_call_native_function(
    gc_ptr<NativeFunction> callee, int arg_count, Interpreter& e) {
	// TODO: don't do this conversion
	Span<Value*> args = {&e.m_env.m_stack[e.m_env.m_frame_ptr - arg_count], arg_count};
	e.save_return_value(callee->m_fptr(args, e));
}

void eval(TypedAST::CallExpression* ast, Interpreter& e) {

	eval(ast->m_callee, e);
	gc_ptr<Value> value = e.m_env.pop();
	auto* callee = unboxed(value.get());
	assert(callee);
	assert(is_callable_value(callee));

	auto& arglist = ast->m_args;
	int arg_count = arglist.size();

	e.m_env.start_stack_region();

	// arguments go before the frame pointer
	if (callee->type() == ValueTag::Function) {
		for (auto expr : arglist) {
			eval(expr, e);
			e.m_env.m_stack.back() = e.new_reference(unboxed(e.m_env.m_stack.back())).get();
		}
	} else {
		for (auto expr : arglist) {
			eval(expr, e);
		}
	}

	e.m_env.start_stack_frame();

	if (callee->type() == ValueTag::Function) {
		eval_call_function(static_cast<Function*>(callee), arg_count, e);
	} else if (callee->type() == ValueTag::NativeFunction) {
		eval_call_native_function(
		    static_cast<NativeFunction*>(callee), arg_count, e);
	} else {
		assert(0);
	}

	e.m_env.end_stack_frame();
	e.m_env.end_stack_region();

	e.m_env.push(e.fetch_return_value());
};

void eval(TypedAST::IndexExpression* ast, Interpreter& e) {
	// TODO: proper error handling

	eval(ast->m_callee, e);
	auto callee_value = e.m_env.pop();
	auto* callee = unboxed(callee_value.get());
	assert(callee);
	assert(callee->type() == ValueTag::Array);

	eval(ast->m_index, e);
	auto index_value = e.m_env.pop();
	auto* index = unboxed(index_value.get());
	assert(index);
	assert(index->type() == ValueTag::Integer);

	auto* array_callee = static_cast<Array*>(callee);
	auto* int_index = static_cast<Integer*>(index);

	e.m_env.push(array_callee->at(int_index->m_value));
};

void eval(TypedAST::TernaryExpression* ast, Interpreter& e) {
	// TODO: proper error handling

	eval(ast->m_condition, e);
	auto condition = e.m_env.pop();
	auto* condition_value = unboxed(condition.get());
	assert(condition_value);
	assert(condition_value->type() == ValueTag::Boolean);

	if (static_cast<Boolean*>(condition_value)->m_value)
		eval(ast->m_then_expr, e);
	else
		eval(ast->m_else_expr, e);
};

void eval(TypedAST::FunctionLiteral* ast, Interpreter& e) {
	auto result = e.new_function(ast, {});

	for (auto const& capture : ast->m_captures) {
		assert(capture.second.outer_frame_offset != INT_MIN);
		result->m_captures[capture.first] =
		    e.m_env.m_stack[e.m_env.m_frame_ptr + capture.second.outer_frame_offset];
	}

	e.m_env.push(result.get());
};

void eval(TypedAST::AccessExpression* ast, Interpreter& e) {
	eval(ast->m_object, e);
	auto rec = e.m_env.pop();
	auto rec_val = unboxed(rec.get());
	assert(rec_val->type() == ValueTag::Object);
	auto rec_actually = static_cast<Object*>(rec_val);
	e.m_env.push(rec_actually->m_value[ast->m_member->m_text]);
}

void eval(TypedAST::ConstructorExpression* ast, Interpreter& e) {
	eval(ast->m_constructor, e);
	auto constructor = e.m_env.pop();
	auto constructor_value = unboxed(constructor.get());

	if (constructor_value->type() == ValueTag::StructConstructor) {
		auto constructor_actually = static_cast<StructConstructor*>(constructor_value);

		assert(ast->m_args.size() == constructor_actually->m_keys.size());

		int storage_point = e.m_env.m_stack_ptr;
		ObjectType record;
		for (int i = 0; i < ast->m_args.size(); ++i)
			eval(ast->m_args[i], e);

		for (int i = 0; i < ast->m_args.size(); ++i) {
			record[constructor_actually->m_keys[i]] =
			    e.m_env.m_stack[storage_point + i];
		}
		
		auto result = e.m_gc->new_object(std::move(record));

		while (e.m_env.m_stack_ptr > storage_point)
			e.m_env.pop();

		e.m_env.push(result.get());
	} else if (constructor_value->type() == ValueTag::UnionConstructor) {
		auto constructor_actually = static_cast<UnionConstructor*>(constructor_value);

		assert(ast->m_args.size() == 1);

		eval(ast->m_args[0], e);
		auto result = e.m_gc->new_union(
		    constructor_actually->m_constructor, e.m_env.m_stack.back());
		e.m_env.pop();

		e.m_env.push(result.get());
	}
}

void eval(TypedAST::IfElseStatement* ast, Interpreter& e) {
	eval(ast->m_condition, e);
	auto condition_result = e.m_env.pop();
	assert(condition_result);

	assert(condition_result->type() == ValueTag::Boolean);
	auto* condition_result_b = static_cast<Boolean*>(condition_result.get());

	if (condition_result_b->m_value) {
		eval(ast->m_body, e);
		if (is_expression(ast->m_body))
			e.m_env.pop();
	} else if (ast->m_else_body) {
		eval(ast->m_else_body, e);
		if (is_expression(ast->m_else_body))
			e.m_env.pop();
	}
};

void eval(TypedAST::ForStatement* ast, Interpreter& e) {
	e.m_env.start_stack_region();

	// NOTE: this is kinda fishy. why do we assert here?
	eval(&ast->m_declaration, e);

	while (1) {
		eval(ast->m_condition, e);
		auto condition_result = e.m_env.pop();
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
			e.m_env.pop();
	}

	e.m_env.end_stack_region();
};

void eval(TypedAST::WhileStatement* ast, Interpreter& e) {
	while (1) {
		eval(ast->m_condition, e);
		auto condition_result = e.m_env.pop();
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

// TODO: include union implementations? if so, remove duplication
void eval(TypedAST::TypeFunctionHandle* ast, Interpreter& e) {
	int type_function = e.m_tc->m_core.m_tf_core.find_function(ast->m_value);
	auto& type_function_data = e.m_tc->m_core.m_type_functions[type_function];
	e.push_struct_constructor(type_function_data.fields);
}

void eval(TypedAST::MonoTypeHandle* ast, Interpreter& e) {
	TypeFunctionId type_function_header =
	    e.m_tc->m_core.m_mono_core.find_function(ast->m_value);
	int type_function = e.m_tc->m_core.m_tf_core.find_function(type_function_header);
	auto& type_function_data = e.m_tc->m_core.m_type_functions[type_function];
	e.push_struct_constructor(type_function_data.fields);
}

void eval(TypedAST::Constructor* ast, Interpreter& e) {
	TypeFunctionId tf_header =
	    e.m_tc->m_core.m_mono_core.find_function(ast->m_mono->m_value);
	int tf = e.m_tc->m_core.m_tf_core.find_function(tf_header);
	auto& tf_data = e.m_tc->m_core.m_type_functions[tf];

	if (tf_data.tag == TypeFunctionTag::Record) {
		e.push_struct_constructor(tf_data.fields);
	} else if (tf_data.tag == TypeFunctionTag::Sum) {
		e.push_union_constructor(ast->m_id->m_text);
	} else {
		assert(0 && "not implemented this type function for construction");
	}
}

void eval(TypedAST::TypedAST* ast, Interpreter& e) {

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

		DISPATCH(Identifier);
		DISPATCH(CallExpression);
		DISPATCH(IndexExpression);
		DISPATCH(TernaryExpression);
		DISPATCH(AccessExpression);
		DISPATCH(ConstructorExpression);

		DISPATCH(DeclarationList);
		DISPATCH(Declaration);

		DISPATCH(Block);
		DISPATCH(ReturnStatement);
		DISPATCH(IfElseStatement);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);

		DISPATCH(TypeFunctionHandle);
		DISPATCH(MonoTypeHandle);
		DISPATCH(Constructor);
	}

	std::cerr << "@ Internal Error: unhandled case in eval:\n";
	std::cerr << "@   - AST type is: " << typed_ast_string[(int)ast->type()] << '\n';
}

} // namespace Interpreter
