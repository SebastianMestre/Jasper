#pragma once

#include <vector>

namespace Interpreter {

struct Value;
struct Error;

struct GC {
	std::vector<Value*> m_blocks;
	std::vector<Value*> m_roots;

	GC();
	~GC();

	void unmark_all();
	void mark_roots();
	void sweep();
	void sweep_all();
	int size () { return m_blocks.size(); }

	void adopt_impl(Value*);

	template <typename U>
	typename std::enable_if<std::is_base_of<Value, U>::value, U*>::type adopt(U* value) {
		adopt_impl(value);
		return value;
	}

	void add_root(Value* new_root);
};

} // namespace Interpreter
