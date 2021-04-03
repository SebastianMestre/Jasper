#pragma once

#include "gc_ptr.hpp"
#include "stack.hpp"
#include "value.hpp"
#include "memory_manager.hpp"

#include <map>

namespace AST {
struct Declaration;
}

namespace TypeChecker {
struct TypeChecker;
}

namespace Interpreter {

struct MemoryManager;
struct Error;

struct Scope {
	std::map<InternedString, Reference*> m_declarations;

	void declare(const Identifier& i, Reference* v);
	Reference* access(const Identifier& i);
};

struct Interpreter {
	Stack m_stack;
	TypeChecker::TypeChecker* m_tc;
	MemoryManager* m_gc;
	std::vector<std::vector<AST::Declaration*>> const* m_declaration_order;
	int m_gc_size_on_last_pass {64};
	Value* m_return_value {nullptr};
	Scope m_global_scope;

	Interpreter(
	    TypeChecker::TypeChecker* tc,
	    MemoryManager* gc,
	    std::vector<std::vector<AST::Declaration*>> const* declaration_order)
	    : m_tc {tc}
	    , m_gc {gc}
	    , m_declaration_order {declaration_order} {}

	void save_return_value(Value*);
	Value* fetch_return_value();

	void run_gc();
	void run_gc_if_needed();

	// Binds a global name to the given reference
	void global_declare_direct(const Identifier& i, Reference* v);
	void global_declare(const Identifier& i, Value* v);
	void global_declare(const Identifier& i, gc_ptr<Value> v);
	Reference* global_access(const Identifier& i);
	void assign(Reference* dst, Handle src);

	auto null() -> Null*;

	template<typename T, typename... Us>
	void push(Us&&... us) {
		m_stack.push(m_gc->alloc_raw<T>(std::forward<Us>(us)...));
		run_gc_if_needed();
	}

	template<typename T, typename... Us>
	gc_ptr<T> create(Us&&... us) {
		auto value = m_gc->alloc<T>(std::forward<Us>(us)...);
		run_gc_if_needed();
		return value;
	}

	auto new_reference(Value*) -> gc_ptr<Reference>;
};

} // namespace Interpreter
