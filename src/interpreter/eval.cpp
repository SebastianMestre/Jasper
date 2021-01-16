#include "eval.hpp"

#include <sstream>

#include <cassert>
#include <climits>

#include "../log/log.hpp"
#include "../typechecker.hpp"
#include "../typed_ast.hpp"
#include "../utils/span.hpp"
#include "garbage_collector.hpp"
#include "interpreter.hpp"
#include "utils.hpp"
#include "value.hpp"

namespace Interpreter {

static bool is_expression (TypedAST::TypedAST* ast) {
	auto tag = ast->type();
	auto tag_idx = static_cast<int>(tag);
	return tag_idx < static_cast<int>(TypedASTTag::Block);
}

void eval_stmt(TypedAST::TypedAST* ast, Interpreter& e) {
	eval(ast, e);
	if (is_expression(ast))
		e.m_stack.pop_unsafe();
}

gc_ptr<Reference> rewrap(Value* x, Interpreter& e) {
	return e.new_reference(value_of(x));
}

void eval(TypedAST::Declaration* ast, Interpreter& e) {
	auto ref = e.new_reference(e.null());
	e.m_stack.push(ref.get());
	if (ast->m_value) {
		eval(ast->m_value, e);
		auto value = e.m_stack.pop();
		ref->m_value = value_of(value.get());
	}
};

void eval(TypedAST::DeclarationList* ast, Interpreter& e) {
	for (auto& decl : ast->m_declarations) {
		auto ref = e.new_reference(e.null());
		e.global_declare_direct(decl.identifier_text(), ref.get());
		eval(decl.m_value, e);
		auto value = e.m_stack.pop();
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
	e.m_stack.push(e.null());
};

void eval(TypedAST::DictionaryLiteral* ast, Interpreter& e) {
	auto result = e.new_dictionary({});

	for (auto& decl : ast->m_body) {
		if (decl.m_value) {
			eval(decl.m_value, e);
			auto value = e.m_stack.pop();
			result->m_value[decl.identifier_text().str()] = value.get();
		} else {
			Log::fatal("declaration in dictionary must have a value");
		}
	}

	e.m_stack.push(result.get());
}

void eval(TypedAST::ArrayLiteral* ast, Interpreter& e) {
	auto result = e.new_list({});
	result->m_value.reserve(ast->m_elements.size());
	for (auto& element : ast->m_elements) {
		eval(element, e);
		auto value_handle = e.m_stack.pop();
		result->append(rewrap(value_handle.get(), e).get());
	}
	e.m_stack.push(result.get());
}

void eval(TypedAST::Identifier* ast, Interpreter& e) {
	if (ast->m_origin == TypedAST::Identifier::Origin::Local ||
	    ast->m_origin == TypedAST::Identifier::Origin::Capture) {
		if (ast->m_frame_offset == INT_MIN) {
			Log::FatalStream() << "MISSING LAYOUT FOR AN IDENTIFIER" << ast->text().str();
		}
		e.m_stack.push(e.m_stack.frame_at(ast->m_frame_offset));
	} else {
		e.m_stack.push(e.global_access(ast->text()));
	}
};

void eval(TypedAST::Block* ast, Interpreter& e) {
	e.m_stack.start_stack_region();
	for (auto stmt : ast->m_body) {
		eval_stmt(stmt, e);
		if (e.m_return_value)
			break;
	}
	e.m_stack.end_stack_region();
};

void eval(TypedAST::ReturnStatement* ast, Interpreter& e) {
	// TODO: proper error handling
	eval(ast->m_value, e);
	auto value = e.m_stack.pop();
	e.save_return_value(value_of(value.get()));
};

auto is_callable_value(Value* v) -> bool {
	if (!v)
		return false;
	auto type = v->type();
	return type == ValueTag::Function || type == ValueTag::NativeFunction;
}

void eval_call_function(gc_ptr<Function> callee, int arg_count, Interpreter& e) {

	// TODO: error handling ?
	assert(callee->m_def->m_args.size() == arg_count);

	// TODO: This is a big hack. pushing nullptr into the
	// stack should actually never happen.
	for (int i = callee->m_captures.size(); i--;)
		e.m_stack.push(nullptr);

	for (auto& kv : callee->m_captures) {
		auto capture_value = value_of(as<Reference>(kv.second));
		// TODO: I would like to get rid of this hash table access
		auto offset = callee->m_def->m_captures[kv.first].inner_frame_offset;
		e.m_stack.frame_at(offset) = kv.second;
	}

	// this feels really dumb:
	// we get a value on the stack, then we move it to the return value
	// slot, then we pop some stuff off the stack, and put it back on the
	// stack. It is doubly dumb when we eval a seq-expr (because it does a
	// save-pop sequence)
	eval(callee->m_def->m_body, e);
	e.save_return_value(e.m_stack.pop_unsafe());
}

void eval(TypedAST::CallExpression* ast, Interpreter& e) {

	eval(ast->m_callee, e);
	auto* callee = value_of(e.m_stack.access(0));
	assert(is_callable_value(callee));

	auto& arglist = ast->m_args;
	int arg_count = arglist.size();

	int frame_start = e.m_stack.m_stack_ptr;
	if (callee->type() == ValueTag::Function) {
		for (auto expr : arglist) {
			eval(expr, e);
			e.m_stack.access(0) = rewrap(e.m_stack.access(0), e).get();
		}
		e.m_stack.start_stack_frame(frame_start);
		eval_call_function(static_cast<Function*>(callee), arg_count, e);
		e.m_stack.frame_at(-1) = e.fetch_return_value();
	} else if (callee->type() == ValueTag::NativeFunction) {
		for (auto expr : arglist)
			eval(expr, e);
		e.m_stack.start_stack_frame(frame_start);
		Span<Value*> args = {&e.m_stack.frame_at(0), arg_count};
		e.m_stack.frame_at(-1) = static_cast<NativeFunction*>(callee)->m_fptr(args, e);
	} else {
		Log::fatal("Attempted to call a non function at runtime");
	}

	e.m_stack.end_stack_frame();
}

void eval(TypedAST::IndexExpression* ast, Interpreter& e) {
	// TODO: proper error handling

	eval(ast->m_callee, e);
	auto callee_handle = e.m_stack.pop();
	auto* callee = value_as<Array>(callee_handle.get());

	eval(ast->m_index, e);
	auto index_handle = e.m_stack.pop();
	auto* index = value_as<Integer>(index_handle.get());

	e.m_stack.push(callee->at(index->m_value));
};

void eval(TypedAST::TernaryExpression* ast, Interpreter& e) {
	// TODO: proper error handling

	eval(ast->m_condition, e);
	auto condition_handle = e.m_stack.pop();
	auto* condition = value_as<Boolean>(condition_handle.get());

	if (condition->m_value)
		eval(ast->m_then_expr, e);
	else
		eval(ast->m_else_expr, e);
};

void eval(TypedAST::FunctionLiteral* ast, Interpreter& e) {

	CapturesType captures;
	captures.reserve(ast->m_captures.size());
	for (auto const& capture : ast->m_captures) {
		assert(capture.second.outer_frame_offset != INT_MIN);
		captures.push_back({
			capture.first,
			e.m_stack.frame_at(capture.second.outer_frame_offset)});
	}

	auto result = e.new_function(ast, std::move(captures));
	e.m_stack.push(result.get());
};

void eval(TypedAST::AccessExpression* ast, Interpreter& e) {
	eval(ast->m_record, e);
	auto rec_handle = e.m_stack.pop();
	auto rec = value_as<Record>(rec_handle.get());
	e.m_stack.push(rec->m_value[ast->m_member->m_text]);
}

void eval(TypedAST::MatchExpression* ast, Interpreter& e) {
	// Put the matched-on variant on the top of the stack
	eval(&ast->m_matchee, e);

	auto variant = value_as<Variant>(e.m_stack.access(0));

	auto constructor = variant->m_constructor;
	auto variant_value = variant->m_inner_value;

	// We won't pop it, because it is already lined up for the later
	// expressions. Instead, replace the variant with its inner value.
	// We also wrap it in a reference so it can be captured
	e.m_stack.access(0) = rewrap(variant_value, e).get();
	
	auto case_it = ast->m_cases.find(constructor);
	// TODO: proper error handling
	assert(case_it != ast->m_cases.end());

	// put the result on the top of the stack
	eval(case_it->second.m_expression, e);

	// evil tinkering with the stack internals
	// (we just delete the variant value from behind the result)
	e.m_stack.access(1) = e.m_stack.access(0);
	e.m_stack.pop_unsafe();
}

void eval(TypedAST::ConstructorExpression* ast, Interpreter& e) {
	eval(ast->m_constructor, e);
	auto constructor_handle = e.m_stack.pop();
	auto constructor = value_of(constructor_handle.get());

	if (constructor->type() == ValueTag::RecordConstructor) {
		auto record_constructor = static_cast<RecordConstructor*>(constructor);

		assert(ast->m_args.size() == record_constructor->m_keys.size());

		int storage_point = e.m_stack.m_stack_ptr;
		RecordType record;
		for (int i = 0; i < ast->m_args.size(); ++i)
			eval(ast->m_args[i], e);

		for (int i = 0; i < ast->m_args.size(); ++i) {
			record[record_constructor->m_keys[i]] =
			    e.m_stack.m_stack[storage_point + i];
		}
		
		auto result = e.m_gc->new_record(std::move(record));

		while (e.m_stack.m_stack_ptr > storage_point)
			e.m_stack.pop();

		e.m_stack.push(result.get());
	} else if (constructor->type() == ValueTag::VariantConstructor) {
		auto variant_constructor = static_cast<VariantConstructor*>(constructor);

		assert(ast->m_args.size() == 1);

		eval(ast->m_args[0], e);
		auto result = e.m_gc->new_variant(
		    variant_constructor->m_constructor, e.m_stack.access(0));

		// replace value with variant wrapper
		e.m_stack.access(0) = result.get();
	}
}

void eval(TypedAST::SequenceExpression* ast, Interpreter& e) {
	eval(ast->m_body, e);
	assert(e.m_return_value);
	e.m_stack.push(e.fetch_return_value());
}

void eval(TypedAST::IfElseStatement* ast, Interpreter& e) {
	// TODO: proper error handling

	eval(ast->m_condition, e);
	auto condition_handle = e.m_stack.pop();
	auto* condition = value_as<Boolean>(condition_handle.get());

	if (condition->m_value)
		eval_stmt(ast->m_body, e);
	else if (ast->m_else_body)
		eval_stmt(ast->m_else_body, e);
};

void eval(TypedAST::ForStatement* ast, Interpreter& e) {
	e.m_stack.start_stack_region();

	eval(&ast->m_declaration, e);

	while (1) {
		eval(ast->m_condition, e);
		auto condition_handle = e.m_stack.pop();
		auto* condition = value_as<Boolean>(condition_handle.get());

		if (!condition->m_value)
			break;

		eval_stmt(ast->m_body, e);

		if (e.m_return_value)
			break;

		eval(ast->m_action, e);
		e.m_stack.pop_unsafe();
	}

	e.m_stack.end_stack_region();
};

void eval(TypedAST::WhileStatement* ast, Interpreter& e) {
	while (1) {
		eval(ast->m_condition, e);
		auto condition_handle = e.m_stack.pop();
		auto* condition = value_as<Boolean>(condition_handle.get());

		if (!condition->m_value)
			break;

		eval_stmt(ast->m_body, e);

		if (e.m_return_value)
			break;
	}
};

// TODO: include variant implementations? if so, remove duplication
void eval(TypedAST::TypeFunctionHandle* ast, Interpreter& e) {
	int type_function = e.m_tc->m_core.m_tf_core.find_function(ast->m_value);
	auto& type_function_data = e.m_tc->m_core.m_type_functions[type_function];
	e.push_record_constructor(type_function_data.fields);
}

void eval(TypedAST::MonoTypeHandle* ast, Interpreter& e) {
	TypeFunctionId type_function_header =
	    e.m_tc->m_core.m_mono_core.find_function(ast->m_value);
	int type_function = e.m_tc->m_core.m_tf_core.find_function(type_function_header);
	auto& type_function_data = e.m_tc->m_core.m_type_functions[type_function];
	e.push_record_constructor(type_function_data.fields);
}

void eval(TypedAST::Constructor* ast, Interpreter& e) {
	TypeFunctionId tf_header = e.m_tc->m_core.m_mono_core.find_function(ast->m_mono);
	int tf = e.m_tc->m_core.m_tf_core.find_function(tf_header);
	auto& tf_data = e.m_tc->m_core.m_type_functions[tf];

	if (tf_data.tag == TypeFunctionTag::Record) {
		e.push_record_constructor(tf_data.fields);
	} else if (tf_data.tag == TypeFunctionTag::Variant) {
		e.push_variant_constructor(ast->m_id->m_text);
	} else {
		Log::fatal("not implemented this type function for construction");
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
		DISPATCH(ArrayLiteral);
		DISPATCH(DictionaryLiteral);
		DISPATCH(FunctionLiteral);

		DISPATCH(Identifier);
		DISPATCH(CallExpression);
		DISPATCH(IndexExpression);
		DISPATCH(TernaryExpression);
		DISPATCH(AccessExpression);
		DISPATCH(MatchExpression);
		DISPATCH(ConstructorExpression);
		DISPATCH(SequenceExpression);

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

	Log::FatalStream() << "(internal) unhandled case in eval: "
	                   << typed_ast_string[(int)ast->type()];
}

} // namespace Interpreter
