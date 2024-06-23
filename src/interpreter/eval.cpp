#include "eval.hpp"

#include <sstream>

#include <cassert>
#include <climits>

#include "../ast.hpp"
#include "../log/log.hpp"
#include "../typechecker/typechecker.hpp"
#include "../utils/span.hpp"
#include "bytecode.hpp"
#include "garbage_collector.hpp"
#include "interpreter.hpp"
#include "utils.hpp"
#include "value.hpp"

namespace Interpreter {

using AST::StmtTag;
using AST::ExprTag;
 
static void exec(AST::Stmt* ast, Interpreter& e);
static void exec(AST::Declaration* ast, Interpreter& e);

void run(AST::Program* ast, Interpreter& e) {
	auto const& comps = *e.m_declaration_order;
	for (auto const& comp : comps) {
		for (auto decl : comp) {
			auto ref = e.new_variable(e.null());
			e.global_declare_direct(decl->identifier_text(), ref);
			eval(decl->m_value, e);
			auto value = e.m_stack.pop_unsafe();
			ref->m_value = value;
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
	e.m_stack.push(Value{result});
	for (auto& element : ast->m_elements) {
		eval(element, e);
		result->append(e.m_stack.pop_unsafe());
	}
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
		e.m_stack.push(e.m_stack.frame_at(ast->m_frame_offset).get_cast<Variable>()->m_value);
	} else {
		e.m_stack.push(e.global_access(ast->text())->m_value);
	}
};

auto is_callable_type(ValueTag t) -> bool {
	return t == ValueTag::Function || t == ValueTag::NativeFunction;
}

void eval(AST::CallExpression* ast, Interpreter& e) {

	eval(ast->m_callee, e);

	// NOTE: keep callee on the stack
	auto callee = e.m_stack.access(0);
	assert(is_callable_type(callee.type()));

	auto& arglist = ast->m_args;
	int arg_count = arglist.size();

	for (auto expr : arglist)
		eval(expr, e);

	e.m_stack.start_frame(arg_count);

	eval_call_callable(callee, arg_count, e);

	// pop the result of the function, and clobber the callee
	e.m_stack.frame_at(-1) = e.m_stack.pop_unsafe();

	e.m_stack.end_frame();
}

void eval(AST::AssignmentExpression* ast, Interpreter& e) {
	if (ast->m_target->type() == ExprTag::Identifier) {

		auto target_ast = static_cast<AST::Identifier*>(ast->m_target);

		Value target {nullptr};
		if (target_ast->m_origin == AST::Identifier::Origin::Local ||
				target_ast->m_origin == AST::Identifier::Origin::Capture) {
			if (target_ast->m_frame_offset == INT_MIN)
				Log::fatal() << "missing layout for identifier '" << target_ast->text() << "'";
			target = e.m_stack.frame_at(target_ast->m_frame_offset);
		} else {
			target = Value{e.global_access(target_ast->text())};
		}

		eval(ast->m_value, e);
		auto value = e.m_stack.pop_unsafe();

		target.get_cast<Variable>()->m_value = value;

		e.m_stack.push(e.null());
	} else if (ast->m_target->type() == ExprTag::IndexExpression) {

		auto target_ast = static_cast<AST::IndexExpression*>(ast->m_target);

		eval(target_ast->m_callee, e);

		eval(target_ast->m_index, e);
		auto index = e.m_stack.pop_unsafe().get_integer();

		eval(ast->m_value, e);

		auto value = e.m_stack.pop_unsafe();
		auto callee_ptr = e.m_stack.pop_unsafe();
		auto* callee = callee_ptr.get_cast<Array>();

		callee->m_value[index] = value;

		e.m_stack.push(e.null());
	} else {
		assert(0);
	}
}

void eval(AST::IndexExpression* ast, Interpreter& e) {
	// TODO: proper error handling

	eval(ast->m_callee, e);
	eval(ast->m_index, e);

	auto index = e.m_stack.pop_unsafe().get_integer();

	auto callee_ptr = e.m_stack.pop_unsafe();
	auto* callee = callee_ptr.get_cast<Array>();

	e.m_stack.push(callee->at(index));
};

void eval(AST::TernaryExpression* ast, Interpreter& e) {
	// TODO: proper error handling

	eval(ast->m_condition, e);
	auto condition = e.m_stack.pop_unsafe().get_boolean();

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
		captures[offset] = value.get_cast<Variable>();
	}

	auto result = e.new_function(ast, std::move(captures));
	e.m_stack.push(Value{result});
};

void eval(AST::AccessExpression* ast, Interpreter& e) {
	eval(ast->m_target, e);
	auto rec_ptr = e.m_stack.pop_unsafe();
	auto rec = rec_ptr.get_cast<Record>();
	e.m_stack.push(Value{rec->m_value[ast->m_member]});
}

void eval(AST::MatchExpression* ast, Interpreter& e) {
	// Put the matched-on variant on the top of the stack
	eval(&ast->m_target, e);

	auto variant = e.m_stack.access(0).get_cast<Variant>();

	auto constructor = variant->m_constructor;
	auto variant_value = variant->m_inner_value;

	// We won't pop it, because it is already lined up for the later
	// expressions. Instead, replace the variant with its inner value.
	// We also wrap it in a variable so it can be captured
	auto ref = e.new_variable(Value {nullptr});
	ref->m_value = variant_value;
	e.m_stack.access(0) = Value{ref};
	
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
	auto constructor = e.m_stack.access(0);

	if (constructor.type() == ValueTag::RecordConstructor) {
		auto record_constructor = constructor.get_cast<RecordConstructor>();

		assert(ast->m_args.size() == record_constructor->m_keys.size());

		int const arg_count = ast->m_args.size();

		// push arguments
		for (int i = 0; i < arg_count; ++i) {
			eval(ast->m_args[i], e);
		}

		// store all arguments in record object
		RecordType record;
		for (int i = 0; i < arg_count; ++i) {
			record[record_constructor->m_keys[i]] =
			    e.m_stack.access(arg_count - 1 - i);
		}

		// promote record object to heap
		auto result = e.new_record(std::move(record));

		// pop arguments
		for (int i = 0; i < arg_count; ++i) {
			e.m_stack.pop_unsafe();
		}

		// replace ctor with record
		e.m_stack.access(0) = Value{result};
	} else if (constructor.type() == ValueTag::VariantConstructor) {
		auto variant_constructor = constructor.get_cast<VariantConstructor>();

		assert(ast->m_args.size() == 1);

		eval(ast->m_args[0], e);
		auto result = e.new_variant(
		    variant_constructor->m_constructor, e.m_stack.access(0));

		// replace ctor with variant, and pop value
		e.m_stack.access(1) = Value{result};
		e.m_stack.pop_unsafe();
	}
}

void eval(AST::SequenceExpression* ast, Interpreter& e) {
	exec(ast->m_body, e);
	if (!e.m_returning)
		e.save_return_value(Value {});
	e.m_stack.push(e.fetch_return_value());
}

static void exec(AST::Declaration* ast, Interpreter& e) {
	auto ref = e.new_variable(Value {nullptr});
	e.m_stack.push(Value{ref});
	if (ast->m_value) {
		eval(ast->m_value, e);
		auto value = e.m_stack.pop_unsafe();
		ref->m_value = value;
	}
};

static void exec(AST::Block* ast, Interpreter& e) {
	e.m_stack.start_region();
	for (auto stmt : ast->m_body) {
		exec(stmt, e);
		if (e.m_returning)
			break;
	}
	e.m_stack.end_region();
};

static void exec(AST::ReturnStatement* ast, Interpreter& e) {
	// TODO: proper error handling
	eval(ast->m_value, e);
	auto value = e.m_stack.pop_unsafe();
	e.save_return_value(value);
};

static void exec(AST::IfElseStatement* ast, Interpreter& e) {
	// TODO: proper error handling

	eval(ast->m_condition, e);
	bool condition = e.m_stack.pop_unsafe().get_boolean();

	if (condition)
		exec(ast->m_body, e);
	else if (ast->m_else_body)
		exec(ast->m_else_body, e);
};

static void exec(AST::WhileStatement* ast, Interpreter& e) {
	while (1) {
		eval(ast->m_condition, e);
		auto condition = e.m_stack.pop_unsafe().get_boolean();

		if (!condition)
			break;

		exec(ast->m_body, e);

		if (e.m_returning)
			break;
	}
};

static void exec(AST::ExpressionStatement* ast, Interpreter& e) {
	eval(ast->m_expression, e);
	e.m_stack.pop_unsafe();
}


void eval(AST::StructExpression* ast, Interpreter& e) {
	e.push_record_constructor(ast->m_fields);
}

void eval(AST::UnionExpression* ast, Interpreter& e) {
	RecordType constructors;
	for(auto& constructor : ast->m_constructors) {
		constructors.insert(
		    {constructor, Value{e.m_gc->new_variant_constructor_raw(constructor)}});
	}
	auto result = e.new_record(std::move(constructors));
	e.m_stack.push(Value{result});
}

void eval(AST::BuiltinTypeFunction* ast, Interpreter& e) {
	return eval(ast->m_syntax, e);
}

void eval(AST::TypeTerm* ast, Interpreter& e) {
	eval(ast->m_callee, e);
}

static void exec(AST::Stmt* ast, Interpreter& e) {
#define DISPATCH(type)                                                         \
	case StmtTag::type:                                                        \
		return exec(static_cast<AST::type*>(ast), e)

	switch (ast->tag()) {
		DISPATCH(Declaration);
		DISPATCH(Block);
		DISPATCH(ReturnStatement);
		DISPATCH(IfElseStatement);
		DISPATCH(WhileStatement);
		DISPATCH(ExpressionStatement);
	}

	assert(false);
#undef DISPATCH
}

void eval(AST::Expr* ast, Interpreter& e) {

#define DISPATCH(type)                                                         \
	case ExprTag::type:                                                        \
		return eval(static_cast<AST::type*>(ast), e)

#ifdef DEBUG
	Log::info() << "case in eval: " << ast_expr_string[(int)ast->type()];
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
		DISPATCH(AssignmentExpression);
		DISPATCH(IndexExpression);
		DISPATCH(TernaryExpression);
		DISPATCH(AccessExpression);
		DISPATCH(MatchExpression);
		DISPATCH(ConstructorExpression);
		DISPATCH(SequenceExpression);

		DISPATCH(TypeTerm);
		DISPATCH(StructExpression);
		DISPATCH(UnionExpression);
		DISPATCH(BuiltinTypeFunction);
	}

	Log::fatal() << "(internal) unhandled case in eval: "
	             << AST::expr_string[(int)ast->type()];

#undef DISPATCH
}

} // namespace Interpreter
