#pragma once

#include <vector>

#include "gc_ptr.hpp"
#include "value.hpp"

namespace Interpreter {

struct Error;

struct GC {
  public:
	std::vector<Value*> m_blocks;
	std::vector<Value*> m_roots;

	GC();
	~GC();

	void unmark_all();
	void mark_roots();
	void sweep();
	void sweep_all();
	int size () { return m_blocks.size(); }

	void add_root(Value* new_root);

	auto new_variant(InternedString constructor, Handle v) -> gc_ptr<Variant>;
	auto new_record(RecordType) -> gc_ptr<Record>;
	auto new_list(ArrayType) -> gc_ptr<Array>;
	auto new_float(float) -> gc_ptr<Float>;
	auto new_string(std::string) -> gc_ptr<String>;
	auto new_function(FunctionType, CapturesType) -> gc_ptr<Function>;
	auto new_native_function(NativeFunctionType*) -> gc_ptr<NativeFunction>;
	auto new_error(std::string) -> gc_ptr<Error>;
	auto new_reference(Value*) -> gc_ptr<Reference>;
	auto new_reference(Handle) -> gc_ptr<Reference>;

	auto new_float_raw(float) -> Float*;
	auto new_string_raw(std::string) -> String*;
	auto new_variant_constructor_raw(InternedString) -> VariantConstructor*;
	auto new_record_constructor_raw(std::vector<InternedString>) -> RecordConstructor*;
};

} // namespace Interpreter
