#pragma once

#include <vector>

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

	auto new_variant_raw(InternedString constructor, Value v) -> Variant*;
	auto new_record_raw(RecordType) -> Record*;
	auto new_list_raw(ArrayType) -> Array*;
	auto new_string_raw(std::string) -> String*;
	auto new_function_raw(FunctionType, CapturesType) -> Function*;
	auto new_error_raw(std::string) -> Error*;
	auto new_variable_raw(Value) -> Variable*;
	auto new_variant_constructor_raw(InternedString) -> VariantConstructor*;
	auto new_record_constructor_raw(std::vector<InternedString>) -> RecordConstructor*;

};

} // namespace Interpreter
