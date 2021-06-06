#include "eval.hpp"

#include <sstream>

#include <cassert>
#include <climits>

#include "../log/log.hpp"
#include "../typechecker.hpp"
#include "../ast.hpp"
#include "../utils/span.hpp"
#include "garbage_collector.hpp"
#include "interpreter.hpp"
#include "utils.hpp"
#include "value.hpp"

namespace Interpreter {

static bool is_expression (AST::AST* ast) {
	auto tag = ast->type();
	auto tag_idx = static_cast<int>(tag);
	return tag_idx < static_cast<int>(ASTTag::Block);
}

static void eval_stmt(AST::AST* ast, Interpreter& e) {
	eval(ast, e);
	if (is_expression(ast))
		e.m_stack.pop_unsafe();
}

void eval(AST::Declaration* ast, Interpreter& e) {
	auto ref = e.new_reference(e.null());
	e.m_stack.push(ref.handle());
	if (ast->m_value) {
		eval(ast->m_value, e);
		auto value = e.m_stack.pop_unsafe();
		ref->m_value = value_of(value);
	}
};

void eval(AST::DeclarationList* ast, Interpreter& e) {
	auto const& comps = *e.m_declaration_order;
	for (auto const& comp : comps) {
		for (auto decl : comp) {
			auto ref = e.new_reference(e.null());
			e.global_declare_direct(decl->identifier_text(), ref.get());
			eval(decl->m_value, e);
			auto value = e.m_stack.pop_unsafe();
			ref->m_value = value_of(value);
		}
	}
}

void eval(AST::NumberLiteral* ast, Interpreter& e) {
	e.push_float(ast->value());
}

void eval(AST::IntegerLiteral* ast, Interpreter& e) {
	e.push_integer(ast->value());
}

void eval(AST::StringLiteral* ast, Interpreter& e) {
	e.push_string(ast->text());
};

void eval(AST::BooleanLiteral* ast, Interpreter& e) {
	e.push_boolean(ast->m_value);
};

void eval(AST::NullLiteral* ast, Interpreter& e) {
	e.m_stack.push(e.null());
};

void eval(AST::ArrayLiteral* ast, Interpreter& e) {
	auto result = e.new_list({});
	result->m_value.reserve(ast->m_elements.size());
	for (auto& element : ast->m_elements) {
		eval(element, e);
		auto ref = e.new_reference(nullptr);
		ref->m_value = value_of(e.m_stack.pop_unsafe());
		result->append(ref.get());
	}
	e.m_stack.push(result.handle());
}

void eval(AST::Identifier* ast, Interpreter& e) {

#ifdef DEBUG
	Log::info() << "Identifier " << ast->text();
	if (ast->m_origin == TypedAST::Identifier::Origin::Local ||
	    ast->m_origin == TypedAST::Identifier::Origin::Capture) {
		Log::info() << "is local";
	} else {
		Log::info() << "is global";
	}
#endif

	if (ast->m_origin == AST::Identifier::Origin::Local ||
	    ast->m_origin == AST::Identifier::Origin::Capture) {
		if (ast->m_frame_offset == INT_MIN)
			Log::fatal() << "missing layout for identifier '" << ast->text() << "'";
		e.m_stack.push(e.m_stack.frame_at(ast->m_frame_offset));
	} else {
		e.m_stack.push(Handle{e.global_access(ast->text())});
	}
};

void eval(AST::Block* ast, Interpreter& e) {
	e.m_stack.start_stack_region();
	for (auto stmt : ast->m_body) {
		eval_stmt(stmt, e);
		if (e.m_returning)
			break;
	}
	e.m_stack.end_stack_region();
};

void eval(AST::ReturnStatement* ast, Interpreter& e) {
	// TODO: proper error handling
	eval(ast->m_value, e);
	auto value = e.m_stack.pop_unsafe();
	e.save_return_value(value_of(value));
};

auto is_callable_value(Value* v) -> bool {
	if (!v)
		return false;
	auto type = v->type();
	return type == ValueTag::Function || type == ValueTag::NativeFunction;
}

void eval_call_function(Function* callee, int arg_count, Interpreter& e) {

	// TODO: error handling ?
	assert(callee->m_def->m_args.size() == arg_count);

	for (auto capture : callee->m_captures)
		e.m_stack.push(Handle{capture});

	eval(callee->m_def->m_body, e);

}

void eval(AST::CallExpression* ast, Interpreter& e) {

	eval(ast->m_callee, e);

	// NOTE: keep callee on the stack
	auto callee = value_of(e.m_stack.access(0));
	assert(is_callable_value(callee.get()));

	auto& arglist = ast->m_args;
	int arg_count = arglist.size();

	int frame_start = e.m_stack.m_stack_ptr;
	if (callee.get()->type() == ValueTag::Function) {
		for (auto expr : arglist) {
			// put arg on stack
			eval(expr, e);

			// create ref to arg
			auto ref = e.new_reference(nullptr);
			ref->m_value = value_of(e.m_stack.access(0));

			// put ref on stack
			e.m_stack.access(0) = ref.handle();
		}
		e.m_stack.start_stack_frame(frame_start);

		eval_call_function(callee.get_cast<Function>(), arg_count, e);

		// pop the result of the function, and clobber the callee
		e.m_stack.frame_at(-1) = e.m_stack.pop_unsafe();
	} else if (callee.get()->type() == ValueTag::NativeFunction) {
		for (auto expr : arglist)
			eval(expr, e);
		e.m_stack.start_stack_frame(frame_start);
		auto args = e.m_stack.frame_range(0, arg_count);

		// compute the result of the function, and clobber the callee
		e.m_stack.frame_at(-1) = callee.get_cast<NativeFunction>()->m_fptr(args, e);
	} else {
		Log::fatal("Attempted to call a non function at runtime");
	}


	e.m_stack.end_stack_frame();
}

void eval(AST::IndexExpression* ast, Interpreter& e) {
	// TODO: proper error handling

	eval(ast->m_callee, e);
	eval(ast->m_index, e);

	auto index_ptr = e.m_stack.pop_unsafe();
	auto* index = value_as<Integer>(index_ptr);

	auto callee_ptr = e.m_stack.pop_unsafe();
	auto* callee = value_as<Array>(callee_ptr);

	e.m_stack.push(Handle{callee->at(index->m_value)});
};

void eval(AST::TernaryExpression* ast, Interpreter& e) {
	// TODO: proper error handling

	eval(ast->m_condition, e);
	auto condition = value_of(e.m_stack.pop_unsafe()).get_boolean();

	if (condition)
		eval(ast->m_then_expr, e);
	else
		eval(ast->m_else_expr, e);
};

void eval(AST::FunctionLiteral* ast, Interpreter& e) {

	CapturesType captures;
	captures.assign(ast->m_captures.size(), nullptr);
	for (auto const& capture : ast->m_captures) {
		assert(capture.second.outer_frame_offset != INT_MIN);
		auto value = e.m_stack.frame_at(capture.second.outer_frame_offset);
		auto offset = capture.second.inner_frame_offset - ast->m_args.size();
		captures[offset] = as<Reference>(value.get());
	}

	auto result = e.new_function(ast, std::move(captures));
	e.m_stack.push(result.handle());
};

void eval(AST::AccessExpression* ast, Interpreter& e) {
	eval(ast->m_record, e);
	auto rec_ptr = e.m_stack.pop_unsafe();
	auto rec = value_as<Record>(rec_ptr);
	e.m_stack.push(Handle{rec->m_value[ast->m_member]});
}

void eval(AST::MatchExpression* ast, Interpreter& e) {
	// Put the matched-on variant on the top of the stack
	eval(&ast->m_matchee, e);

	auto variant = value_as<Variant>(e.m_stack.access(0).get());

	auto constructor = variant->m_constructor;
	auto variant_value = variant->m_inner_value;

	// We won't pop it, because it is already lined up for the later
	// expressions. Instead, replace the variant with its inner value.
	// We also wrap it in a reference so it can be captured
	auto ref = e.new_reference(nullptr);
	ref->m_value = value_of(variant_value);
	e.m_stack.access(0) = ref.handle();
	
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

void eval(AST::ConstructorExpression* ast, Interpreter& e) {
	// NOTE: we leave the ctor on the stack for the time being

	eval(ast->m_constructor, e);
	auto constructor_ptr = e.m_stack.access(0);
	auto constructor = value_of(constructor_ptr).get();

	if (constructor->type() == ValueTag::RecordConstructor) {
		auto record_constructor = static_cast<RecordConstructor*>(constructor);

		assert(ast->m_args.size() == record_constructor->m_keys.size());


		// eval arguments
		// arguments start at storage_point
		int storage_point = e.m_stack.m_stack_ptr;
		for (int i = 0; i < ast->m_args.size(); ++i)
			eval(ast->m_args[i], e);

		// store all arguments in record object
		RecordType record;
		for (int i = 0; i < ast->m_args.size(); ++i) {
			record[record_constructor->m_keys[i]] =
			    value_of(e.m_stack.m_stack[storage_point + i]);
		}
		
		// promote record object to heap
		auto result = e.m_gc->new_record(std::move(record));

		// pop arguments
		while (e.m_stack.m_stack_ptr > storage_point)
			e.m_stack.pop_unsafe();

		// replace ctor with record
		e.m_stack.access(0) = Handle{result.get()};
	} else if (constructor->type() == ValueTag::VariantConstructor) {
		auto variant_constructor = static_cast<VariantConstructor*>(constructor);

		assert(ast->m_args.size() == 1);

		eval(ast->m_args[0], e);
		auto result = e.m_gc->new_variant(
		    variant_constructor->m_constructor, value_of(e.m_stack.access(0)).get());

		// replace ctor with variant, and pop value
		e.m_stack.access(1) = Handle{result.get()};
		e.m_stack.pop_unsafe();
	}
}

void eval(AST::SequenceExpression* ast, Interpreter& e) {
	eval(ast->m_body, e);
	if (!e.m_returning)
		e.save_return_value(Handle {});
	e.m_stack.push(e.fetch_return_value());
}

void eval(AST::IfElseStatement* ast, Interpreter& e) {
	// TODO: proper error handling

	eval(ast->m_condition, e);
	bool condition = value_of(e.m_stack.pop_unsafe()).get_boolean();

	if (condition)
		eval_stmt(ast->m_body, e);
	else if (ast->m_else_body)
		eval_stmt(ast->m_else_body, e);
};

void eval(AST::WhileStatement* ast, Interpreter& e) {
	while (1) {
		eval(ast->m_condition, e);
		auto condition = value_of(e.m_stack.pop_unsafe()).get_boolean();

		if (!condition)
			break;

		eval_stmt(ast->m_body, e);

		if (e.m_returning)
			break;
	}
};

void eval(AST::StructExpression* ast, Interpreter& e) {
	e.push_record_constructor(ast->m_fields);
}

void eval(AST::UnionExpression* ast, Interpreter& e) {
	RecordType constructors;
	for(auto& constructor : ast->m_constructors) {
		constructors.insert(
		    {constructor, Handle{e.m_gc->new_variant_constructor_raw(constructor)}});
	}
	auto result = e.new_record(std::move(constructors));
	e.m_stack.push(result.handle());
}

void eval(AST::TypeFunctionHandle* ast, Interpreter& e) {
	eval(ast->m_syntax, e);
}

void eval(AST::MonoTypeHandle* ast, Interpreter& e) {
	TypeFunctionId type_function_header =
	    e.m_tc->m_core.m_mono_core.find_function(ast->m_value);
	int type_function = e.m_tc->m_core.m_tf_core.find_function(type_function_header);
	auto& type_function_data = e.m_tc->m_core.m_type_functions[type_function];
	e.push_record_constructor(type_function_data.fields);
}

void eval(AST::Constructor* ast, Interpreter& e) {
	TypeFunctionId tf_header = e.m_tc->m_core.m_mono_core.find_function(ast->m_mono);
	int tf = e.m_tc->m_core.m_tf_core.find_function(tf_header);
	auto& tf_data = e.m_tc->m_core.m_type_functions[tf];

	if (tf_data.tag == TypeFunctionTag::Record) {
		e.push_record_constructor(tf_data.fields);
	} else if (tf_data.tag == TypeFunctionTag::Variant) {
		e.push_variant_constructor(ast->m_id);
	} else {
		Log::fatal("not implemented this type function for construction");
	}
}

void eval(AST::TypeTerm* ast, Interpreter& e) {
	eval(ast->m_callee, e);
}

void eval(AST::AST* ast, Interpreter& e) {

#define DISPATCH(type)                                                         \
	case ASTTag::type:                                                    \
		return eval(static_cast<AST::type*>(ast), e)

#ifdef DEBUG
	Log::info() << "case in eval: " << typed_ast_string[(int)ast->type()];
#endif

	switch (ast->type()) {
		DISPATCH(NumberLiteral);
		DISPATCH(IntegerLiteral);
		DISPATCH(StringLiteral);
		DISPATCH(BooleanLiteral);
		DISPATCH(NullLiteral);
		DISPATCH(ArrayLiteral);
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
		DISPATCH(WhileStatement);

		DISPATCH(TypeTerm);
		DISPATCH(StructExpression);
		DISPATCH(UnionExpression);
		DISPATCH(TypeFunctionHandle);
		DISPATCH(MonoTypeHandle);
		DISPATCH(Constructor);
	}

	Log::fatal() << "(internal) unhandled case in eval: "
	             << ast_string[(int)ast->type()];
}

} // namespace Interpreter
