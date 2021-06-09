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
		return {nullptr};
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

NativeFunction::NativeFunction(NativeFunctionType* fptr)
    : GcCell {ValueTag::NativeFunction}
    , m_fptr {fptr} {}

Reference::Reference(Value value)
    : GcCell {ValueTag::Reference}
    , m_value {value} {}

VariantConstructor::VariantConstructor(InternedString constructor)
    : GcCell {ValueTag::VariantConstructor}
    , m_constructor {constructor} {}

RecordConstructor::RecordConstructor(std::vector<InternedString> keys)
    : GcCell {ValueTag::RecordConstructor}
    , m_keys {std::move(keys)} {}

void gc_visit(String* v) {
	v->m_visited = true;
}
void gc_visit(Error* v) {
	v->m_visited = true;
}
void gc_visit(NativeFunction* v) {
	v->m_visited = true;
}
void gc_visit(VariantConstructor* v) {
	v->m_visited = true;
}
void gc_visit(RecordConstructor* v) {
	v->m_visited = true;
}

void gc_visit(Array* l) {
	if (l->m_visited)
		return;

	l->m_visited = true;
	for (auto* child : l->m_value) {
		gc_visit(child);
	}
}

void gc_visit(Record* o) {
	if (o->m_visited)
		return;

	o->m_visited = true;
	for (auto child : o->m_value)
		gc_visit(child.second);
}

void gc_visit(Variant* u) {
	if (u->m_visited)
		return;

	u->m_visited = true;
	gc_visit(u->m_inner_value);
}

void gc_visit(Function* f) {
	if (f->m_visited)
		return;

	f->m_visited = true;
	for (auto& capture : f->m_captures)
		gc_visit(capture);
}

void gc_visit(Reference* r) {
	if (r->m_visited)
		return;

	r->m_visited = true;
	gc_visit(r->m_value);
}

void gc_visit(GcCell* v) {
	switch (v->type()) {
	case ValueTag::String:
		return gc_visit(static_cast<String*>(v));
	case ValueTag::Error:
		return gc_visit(static_cast<Error*>(v));
	case ValueTag::Array:
		return gc_visit(static_cast<Array*>(v));
	case ValueTag::Record:
		return gc_visit(static_cast<Record*>(v));
	case ValueTag::Variant:
		return gc_visit(static_cast<Variant*>(v));
	case ValueTag::Function:
		return gc_visit(static_cast<Function*>(v));
	case ValueTag::NativeFunction:
		return gc_visit(static_cast<NativeFunction*>(v));
	case ValueTag::Reference:
		return gc_visit(static_cast<Reference*>(v));
	case ValueTag::VariantConstructor:
		return gc_visit(static_cast<VariantConstructor*>(v));
	case ValueTag::RecordConstructor:
		return gc_visit(static_cast<RecordConstructor*>(v));
	}
}

void gc_visit(Value h) {
	if (is_heap_type(h.type()))
		return gc_visit(h.get());
}

// = === === print === === = //

void print_spaces(int n) {
	for (int i = 0; i < n; ++i)
		std::cout << ' ';
}

void print(int v, int d) {
	print_spaces(d);
	std::cout << value_string[int(ValueTag::Integer)] << ' ' << v << '\n';
}

void print(float v, int d) {
	print_spaces(d);
	std::cout << value_string[int(ValueTag::Float)] << ' ' << v << '\n';
}

void print(String* v, int d) {
	print_spaces(d);
	std::cout << value_string[int(v->type())] << ' ' << '"' << v->m_value
	          << '"' << '\n';
}

void print(bool b, int d) {
	print_spaces(d);
	std::cout << value_string[int(ValueTag::Boolean)] << ' ' << b << '\n';
}

void print(Error* v, int d) {
	// TODO
	print_spaces(d);
	std::cout << value_string[int(v->type())] << '\n';
}

void print(Record* o, int d) {
	print_spaces(d);
	std::cout << value_string[int(o->type())] << '\n';
	for (auto& kv : o->m_value){
		print_spaces(d+1);
		std::cerr << kv.first << " := \n";
		print(kv.second, d+1);
	}
}

void print(Variant* m, int d) {
	print_spaces(d);
	std::cout << value_string[int(m->type())] << " " << m->m_constructor << '\n';
	print(m->m_inner_value, d);
}

void print(Function* f, int d) {
	// TODO
	print_spaces(d);
	std::cout << value_string[int(f->type())] << '\n';
}

void print(NativeFunction* f, int d) {
	// TODO
	print_spaces(d);
	std::cout << value_string[int(f->type())] << '\n';
}

void print(Array* l, int d) {
	print_spaces(d);
	std::cout << value_string[int(l->type())] << '\n';
	for (auto* child : l->m_value) {
		print(child, d + 1);
	}
}

void print(Reference* l, int d) {
	print_spaces(d);
	std::cout << value_string[int(l->type())] << '\n';
	print(l->m_value, d + 1);
}

void print(RecordConstructor* l, int d) {
	print_spaces(d);
	std::cout << "RecordConstructor\n";
}

void print(VariantConstructor* l, int d) {
	print_spaces(d);
	std::cout << "VariantConstructor\n";
}

void print(GcCell* v, int d) {

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
	case ValueTag::NativeFunction:
		return print(static_cast<NativeFunction*>(v), d);
	case ValueTag::Reference:
		return print(static_cast<Reference*>(v), d);
	case ValueTag::VariantConstructor:
		return print(static_cast<VariantConstructor*>(v), d);
	case ValueTag::RecordConstructor:
		return print(static_cast<RecordConstructor*>(v), d);
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
	case ValueTag::Null:
		print_spaces(d);
		return void(std::cout << "(null)\n");
	}
}

} // namespace Interpreter
