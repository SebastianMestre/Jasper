#include "value.hpp"

#include <iostream>

#include "error.hpp"

namespace Interpreter {

Null::Null()
    : Value(ValueTag::Null) {}

Integer::Integer()
    : Value(ValueTag::Integer) {}
Integer::Integer(int v)
    : Value(ValueTag::Integer)
    , m_value(v) {}

Float::Float()
    : Value(ValueTag::Float) {}
Float::Float(float v)
    : Value(ValueTag::Float)
    , m_value(v) {}

Boolean::Boolean()
    : Value(ValueTag::Boolean) {}
Boolean::Boolean(bool b)
    : Value(ValueTag::Boolean)
    , m_value(b) {}

String::String()
    : Value(ValueTag::String) {}
String::String(std::string s)
    : Value(ValueTag::String)
    , m_value(std::move(s)) {}

Array::Array()
    : Value(ValueTag::Array) {}
Array::Array(ArrayType l)
    : Value(ValueTag::Array)
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
    : Value(ValueTag::Record) {}
Record::Record(RecordType o)
    : Value(ValueTag::Record)
    , m_value(std::move(o)) {}

void Record::addMember(Identifier const& id, Handle v) {
	m_value[id] = v;
}

Handle Record::getMember(Identifier const& id) {
	auto it = m_value.find(id);
	if (it == m_value.end()) {
		// TODO: return RangeError
		return {nullptr};
	} else {
		return it->second;
	}
}

Variant::Variant(InternedString constructor)
    : Value(ValueTag::Variant)
    , m_constructor(constructor) {}

Variant::Variant(InternedString constructor, Value* v)
    : Value(ValueTag::Variant)
    , m_constructor(constructor)
    , m_inner_value(v) {}

Function::Function(FunctionType def, CapturesType captures)
    : Value(ValueTag::Function)
    , m_def(def)
    , m_captures(std::move(captures)) {}

NativeFunction::NativeFunction(NativeFunctionType* fptr)
    : Value {ValueTag::NativeFunction}
    , m_fptr {fptr} {}

Reference::Reference(Handle value)
    : Value {ValueTag::Reference}
    , m_value {value} {}

Reference::Reference(Value* value)
    : Reference {Handle{value}} {}

VariantConstructor::VariantConstructor(InternedString constructor)
    : Value {ValueTag::VariantConstructor}
    , m_constructor {constructor} {}

RecordConstructor::RecordConstructor(std::vector<InternedString> keys)
    : Value {ValueTag::RecordConstructor}
    , m_keys {std::move(keys)} {}

void gc_visit(Null* v) {
	v->m_visited = true;
}
void gc_visit(Integer* v) {
	v->m_visited = true;
}
void gc_visit(Float* v) {
	v->m_visited = true;
}
void gc_visit(String* v) {
	v->m_visited = true;
}
void gc_visit(Boolean* v) {
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
	if (r->m_value.get())
		gc_visit(r->m_value);
}

void gc_visit(Value* v) {

	switch (v->type()) {
	case ValueTag::Null:
		return gc_visit(static_cast<Null*>(v));
	case ValueTag::Integer:
		return gc_visit(static_cast<Integer*>(v));
	case ValueTag::Float:
		return gc_visit(static_cast<Float*>(v));
	case ValueTag::String:
		return gc_visit(static_cast<String*>(v));
	case ValueTag::Boolean:
		return gc_visit(static_cast<Boolean*>(v));
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

void gc_visit(Handle h) {
	return gc_visit(h.get());
}

// = === === print === === = //

void print_spaces(int n) {
	for (int i = 0; i < n; ++i)
		std::cout << ' ';
}

void print(Integer const* v, int d) {
	print_spaces(d);
	std::cout << value_string[int(v->type())] << ' ' << v->m_value << '\n';
}

void print(Null* v, int d) {
	print_spaces(d);
	std::cout << value_string[int(v->type())] << '\n';
}

void print(Float* v, int d) {
	print_spaces(d);
	std::cout << value_string[int(v->type())] << ' ' << v->m_value << '\n';
}

void print(String* v, int d) {
	print_spaces(d);
	std::cout << value_string[int(v->type())] << ' ' << '"' << v->m_value
	          << '"' << '\n';
}

void print(Boolean* v, int d) {
	print_spaces(d);
	std::cout << value_string[int(v->type())] << ' ' << v->m_value << '\n';
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

void print(Value* v, int d) {

	switch (v->type()) {
	case ValueTag::Null:
		return print(static_cast<Null*>(v), d);
	case ValueTag::Integer:
		return print(static_cast<Integer*>(v), d);
	case ValueTag::Float:
		return print(static_cast<Float*>(v), d);
	case ValueTag::String:
		return print(static_cast<String*>(v), d);
	case ValueTag::Boolean:
		return print(static_cast<Boolean*>(v), d);
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

void print(Handle h, int d) {
	return print(h.get(), d);
}

} // namespace Interpreter
