#include "value.hpp"

#include <iostream>

#include "error.hpp"

namespace Interpreter {

String::String()
    : GcCell(ValueTag::String) {}
String::String(std::string s)
    : GcCell(ValueTag::String)
    , m_value(std::move(s)) {}

Array::Array()
    : GcCell(ValueTag::Array) {}
Array::Array(ArrayType l)
    : GcCell(ValueTag::Array)
    , m_value(std::move(l)) {}

void Array::append(Reference* v) {
	m_value.push_back(v);
}

Reference* Array::at(int position) {
	if (position < 0 or position >= int(m_value.size())) {
		// TODO: return RangeError
		return nullptr;
	} else {
		return m_value[position];
	}
}

Record::Record()
    : GcCell(ValueTag::Record) {}
Record::Record(RecordType o)
    : GcCell(ValueTag::Record)
    , m_value(std::move(o)) {}

void Record::addMember(Identifier const& id, Value v) {
	m_value[id] = v;
}

Value Record::getMember(Identifier const& id) {
	auto it = m_value.find(id);
	if (it == m_value.end()) {
		// TODO: return RangeError
		return Value {nullptr};
	} else {
		return it->second;
	}
}

Variant::Variant(InternedString constructor)
    : GcCell(ValueTag::Variant)
    , m_constructor(constructor) {}

Variant::Variant(InternedString constructor, Value v)
    : GcCell(ValueTag::Variant)
    , m_constructor(constructor)
    , m_inner_value(v) {}

Function::Function(FunctionType def, CapturesType captures)
    : GcCell(ValueTag::Function)
    , m_def(def)
    , m_captures(std::move(captures)) {}

Reference::Reference(Value value)
    : GcCell {ValueTag::Reference}
    , m_value {value} {}

VariantConstructor::VariantConstructor(InternedString constructor)
    : GcCell {ValueTag::VariantConstructor}
    , m_constructor {constructor} {}

RecordConstructor::RecordConstructor(std::vector<InternedString> keys)
    : GcCell {ValueTag::RecordConstructor}
    , m_keys {std::move(keys)} {}


// = === === print === === = //

void print_spaces(int n) {
	for (int i = 0; i < n; ++i)
		std::cout << ' ';
}

static void print(int v, int d) {
	print_spaces(d);
	std::cout << value_string[int(ValueTag::Integer)] << ' ' << v << '\n';
}

static void print(float v, int d) {
	print_spaces(d);
	std::cout << value_string[int(ValueTag::Float)] << ' ' << v << '\n';
}

static void print(String* v, int d) {
	print_spaces(d);
	std::cout << value_string[int(v->type())] << ' ' << '"' << v->m_value
	          << '"' << '\n';
}

static void print(bool b, int d) {
	print_spaces(d);
	std::cout << value_string[int(ValueTag::Boolean)] << ' ' << b << '\n';
}

static void print(Error* v, int d) {
	// TODO
	print_spaces(d);
	std::cout << value_string[int(v->type())] << '\n';
}

static void print(Record* o, int d) {
	print_spaces(d);
	std::cout << value_string[int(o->type())] << '\n';
	for (auto& kv : o->m_value){
		print_spaces(d+1);
		std::cerr << kv.first << " := \n";
		print(kv.second, d+1);
	}
}

static void print(Variant* m, int d) {
	print_spaces(d);
	std::cout << value_string[int(m->type())] << " " << m->m_constructor << '\n';
	print(m->m_inner_value, d);
}

static void print(Function* f, int d) {
	// TODO
	print_spaces(d);
	std::cout << value_string[int(f->type())] << '\n';
}

static void print(NativeFunction* f, int d) {
	// TODO
	print_spaces(d);
	std::cout << value_string[int(ValueTag::NativeFunction)] << '\n';
}

static void print(Array* l, int d) {
	print_spaces(d);
	std::cout << value_string[int(l->type())] << '\n';
	for (auto* child : l->m_value) {
		print(child, d + 1);
	}
}

static void print(Reference* l, int d) {
	print_spaces(d);
	std::cout << value_string[int(l->type())] << '\n';
	print(l->m_value, d + 1);
}

static void print(RecordConstructor* l, int d) {
	print_spaces(d);
	std::cout << "RecordConstructor\n";
}

static void print(VariantConstructor* l, int d) {
	print_spaces(d);
	std::cout << "VariantConstructor\n";
}

static void print(GcCell* v, int d) {

	switch (v->type()) {
	case ValueTag::String:
		return print(static_cast<String*>(v), d);
	case ValueTag::Error:
		return print(static_cast<Error*>(v), d);
	case ValueTag::Array:
		return print(static_cast<Array*>(v), d);
	case ValueTag::Record:
		return print(static_cast<Record*>(v), d);
	case ValueTag::Variant:
		return print(static_cast<Variant*>(v), d);
	case ValueTag::Function:
		return print(static_cast<Function*>(v), d);
	case ValueTag::Reference:
		return print(static_cast<Reference*>(v), d);
	case ValueTag::VariantConstructor:
		return print(static_cast<VariantConstructor*>(v), d);
	case ValueTag::RecordConstructor:
		return print(static_cast<RecordConstructor*>(v), d);
	default:
		assert(0);
	}
}

void print(Value h, int d) {
	if (is_heap_type(h.type()))
		return print(h.get(), d);
	switch (h.type()) {
	case ValueTag::Boolean:
		return print(h.as_boolean, d);
	case ValueTag::Integer:
		return print(h.as_integer, d);
	case ValueTag::Float:
		return print(h.as_float, d);
	case ValueTag::NativeFunction:
		return print(h.as_native_func, d);
	case ValueTag::Null:
		print_spaces(d);
		return void(std::cout << "(null)\n");
	default:
		assert(0);
	}
}

} // namespace Interpreter
