#pragma once

#include <vector>

#include "gc_ptr.hpp"
#include "value.hpp"

namespace Interpreter {

struct Error;

struct GC {
  public:
	std::vector<GcCell*> m_blocks;
	std::vector<GcCell*> m_roots;

	GC();
	~GC();

	void unmark_all();
	void mark_roots();
	void sweep();
	void sweep_all();
	int size () { return m_blocks.size(); }

	void add_root(GcCell* new_root);

	auto new_variant(InternedString constructor, Value v) -> gc_ptr<Variant>;
	auto new_record(RecordType) -> gc_ptr<Record>;
	auto new_list(ArrayType) -> gc_ptr<Array>;
	auto new_string(std::string) -> gc_ptr<String>;
	auto new_function(FunctionType, CapturesType) -> gc_ptr<Function>;
	auto new_error(std::string) -> gc_ptr<Error>;
	auto new_reference(Value) -> gc_ptr<Reference>;

	auto new_string_raw(std::string) -> String*;
	auto new_variant_constructor_raw(InternedString) -> VariantConstructor*;
	auto new_record_constructor_raw(std::vector<InternedString>) -> RecordConstructor*;
};

} // namespace Interpreter
