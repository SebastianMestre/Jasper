#pragma once

#include <vector>

#include "value.hpp"

namespace Interpreter {

struct GC {
public:
	std::vector<GcCell*> m_blocks;
	int m_generation = 0;

	GC();
	~GC();

	int generation() const { return m_generation; }
	void end_generation() { ++m_generation; }
	void sweep();
	int size () { return m_blocks.size(); }

	auto new_variant_raw(InternedString constructor, Value v) -> Variant*;
	auto new_record_raw(RecordType) -> Record*;
	auto new_list_raw(ArrayType) -> Array*;
	auto new_string_raw(std::string) -> String*;
	auto new_function_raw(FunctionType, CapturesType) -> Function*;
	auto new_variable_raw(Value) -> Variable*;
	auto new_variant_constructor_raw(InternedString) -> VariantConstructor*;
	auto new_record_constructor_raw(std::vector<InternedString>) -> RecordConstructor*;

};

} // namespace Interpreter
