#pragma once

#include "gc_ptr.hpp"
#include "stack.hpp"
#include "value.hpp"

#include <map>

namespace AST {
struct Declaration;
}

namespace TypeChecker {
struct TypeChecker;
}

namespace Interpreter {

struct GC;
struct Error;

struct GlobalScope {
	std::map<InternedString, Variable*> m_declarations;

	void declare(const Identifier& i, Variable* v);
	Variable* access(const Identifier& i);
};

struct Interpreter {
	Stack m_stack;
	GC* m_gc;
	std::vector<std::vector<AST::Declaration*>> const* m_declaration_order;
	int m_gc_size_on_last_pass {64};
	bool m_returning{false};
	Value m_return_value {nullptr};
	GlobalScope m_global_scope;

	Interpreter(
	    GC* gc,
	    std::vector<std::vector<AST::Declaration*>> const* declaration_order)
	    : m_gc {gc}
	    , m_declaration_order {declaration_order} {}

	void save_return_value(Value);
	Value fetch_return_value();

	void run_gc();
	void run_gc_if_needed();

	// Binds a global name to the given variable
	void global_declare_direct(const Identifier& i, Variable* v);
	void global_declare(const Identifier& i, Value v);
	Variable* global_access(const Identifier& i);

	auto null() -> Value;
	void push_integer(int);
	void push_float(float);
	void push_boolean(bool);
	void push_string(std::string);
	void push_variant_constructor(InternedString constructor);
	void push_record_constructor(std::vector<InternedString>);
	auto new_list(ArrayType) -> gc_ptr<Array>;
	auto new_record(RecordType) -> gc_ptr<Record>;
	auto new_function(FunctionType, CapturesType) -> gc_ptr<Function>;
	auto new_error(std::string) -> gc_ptr<Error>;
	auto new_variable(Value) -> gc_ptr<Variable>;
};

} // namespace Interpreter
