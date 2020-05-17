#include "value.hpp"

#include <iostream>

#include <cassert>

#include "environment.hpp"
#include "error.hpp"

namespace Type {

Null::Null() : Value(value_type::Null) {}



Integer::Integer() : Value(value_type::Integer) {}
Integer::Integer(int v) : Value(value_type::Integer), m_value(v) {}



Float::Float() : Value(value_type::Float) {}
Float::Float(float v) : Value(value_type::Float), m_value(v) {}



Boolean::Boolean() : Value(value_type::Boolean) {}
Boolean::Boolean(bool b) : Value(value_type::Boolean), m_value(b) {}



String::String() : Value(value_type::String) {}
String::String(std::string s) : Value(value_type::String), m_value(std::move(s)) {}



Array::Array() : Value(value_type::Array) {}
Array::Array(ArrayType l) : Value(value_type::Array), m_value(std::move(l)) {}

void Array::append(Value* v) {
	m_value.push_back(v);
}

Value* Array::at(int position) {
	if (position < 0 or position >= int(m_value.size())) {
		// TODO: return RangeError
		return nullptr;
	} else {
		return m_value[position];
	}
}



Object::Object() : Value(value_type::Object) {}
Object::Object(ObjectType o) : Value(value_type::Object), m_value(std::move(o)) {}

void Object::addMember(Identifier const& id, Value* v) {
	m_value[id] = v;
}

Value* Object::getMember(Identifier const& id) {
	auto it = m_value.find(id);
	if (it == m_value.end()) {
		// TODO: return RangeError
		return nullptr;
	} else {
		return it->second;
	}
}



Dictionary::Dictionary() : Value(value_type::Dictionary) {}
Dictionary::Dictionary(ObjectType o) : Value(value_type::Dictionary), m_value(std::move(o)) {}

void Dictionary::addMember(Identifier const& id, Value* v) {
	m_value[id] = v;
}

Value* Dictionary::getMember(Identifier const& id) {
	auto it = m_value.find(id);
	if (it == m_value.end()) {
		// TODO: return RangeError
		return nullptr;
	} else {
		return it->second;
	}
}

void Dictionary::removeMember(Identifier const& id) {
	m_value.erase(id);
}



Function::Function(FunctionType def, ObjectType captures)
    : Value(value_type::Function), m_def(def), m_captures(std::move(captures)) {}



NativeFunction::NativeFunction(NativeFunctionType* fptr)
    : Value { value_type::NativeFunction }, m_fptr { fptr } {}


Reference::Reference(Value* value)
	: Value { value_type::Reference }, m_value{value} {}


void gc_visit(Null* v) { v->m_visited = true; }
void gc_visit(Integer* v) { v->m_visited = true; }
void gc_visit(Float* v) { v->m_visited = true; }
void gc_visit(String* v) { v->m_visited = true; }
void gc_visit(Boolean* v) { v->m_visited = true; }
void gc_visit(Error* v) { v->m_visited = true; }
void gc_visit(NativeFunction* v) { v->m_visited = true; }

void gc_visit(Array* l) {
	if (l->m_visited)
		return;

	l->m_visited = true;
	for (auto* child : l->m_value) {
		gc_visit(child);
	}
}

void gc_visit(Object* o) {
	if (o->m_visited)
		return;

	o->m_visited = true;
	for (auto child : o->m_value)
		gc_visit(child.second);
}

void gc_visit(Dictionary* d) {
	if (d->m_visited)
		return;

	d->m_visited = true;
	for (auto child : d->m_value)
		gc_visit(child.second);
}

void gc_visit(Function* f) {
	if (f->m_visited)
		return;
	
	f->m_visited = true;
	for (auto& dec : f->m_captures)
		gc_visit(dec.second);
}

void gc_visit(Reference* r) {
	if (r->m_visited)
		return;
	
	r->m_visited = true;
	gc_visit(r->m_value);
}


void gc_visit(Value* v) {

	switch(v->type()) {
	case value_type::Null:
		return gc_visit(static_cast<Null*>(v));
	case value_type::Integer:
		return gc_visit(static_cast<Integer*>(v));
	case value_type::Float:
		return gc_visit(static_cast<Float*>(v));
	case value_type::String:
		return gc_visit(static_cast<String*>(v));
	case value_type::Boolean:
		return gc_visit(static_cast<Boolean*>(v));
	case value_type::Error:
		return gc_visit(static_cast<Error*>(v));
	case value_type::Array:
		return gc_visit(static_cast<Array*>(v));
	case value_type::Object:
		return gc_visit(static_cast<Object*>(v));
	case value_type::Dictionary:
		return gc_visit(static_cast<Dictionary*>(v));
	case value_type::Function:
		return gc_visit(static_cast<Function*>(v));
	case value_type::NativeFunction:
		return gc_visit(static_cast<NativeFunction*>(v));
	case value_type::Reference:
		return gc_visit(static_cast<Reference*>(v));
	}
}

// = === === print === === = //

void print_spaces(int n) {
	for(int i = 0; i < n; ++i)
		std::cout << ' ';
}

void print(Integer const* v, int d) {
	print_spaces(d);
	std::cout << value_type_string[int(v->type())] << ' ' << v->m_value << '\n';
}

void print(Null* v, int d) {
	print_spaces(d);
	std::cout << value_type_string[int(v->type())] << '\n';
}

void print(Float* v, int d) {
	print_spaces(d);
	std::cout << value_type_string[int(v->type())] << ' ' << v->m_value << '\n';
}

void print(String* v, int d) {
	print_spaces(d);
	std::cout << value_type_string[int(v->type())] << ' ' << '"' << v->m_value << '"' << '\n';
}

void print(Boolean* v, int d) {
	print_spaces(d);
	std::cout << value_type_string[int(v->type())] << ' ' << v->m_value << '\n';
}

void print(Error* v, int d) {
	// TODO
	print_spaces(d);
	std::cout << value_type_string[int(v->type())] << '\n';
}

void print(Object* o, int d) {
	// TODO
	print_spaces(d);
	std::cout << value_type_string[int(o->type())] << '\n';
}

void print(Dictionary* m, int d) {
	// TODO
	print_spaces(d);
	std::cout << value_type_string[int(m->type())] << '\n';
}

void print(Function* f, int d) {
	// TODO
	print_spaces(d);
	std::cout << value_type_string[int(f->type())] << '\n';
}

void print(NativeFunction* f, int d) {
	// TODO
	print_spaces(d);
	std::cout << value_type_string[int(f->type())] << '\n';
}

void print(Array* l, int d) {
	print_spaces(d);
	std::cout << value_type_string[int(l->type())] << '\n';
	for (auto* child : l->m_value) {
		print(child, d+1);
	}
}

void print(Reference* l, int d) {
	print_spaces(d);
	std::cout << value_type_string[int(l->type())] << '\n';
	print(l->m_value, d+1);
}

void print(Value* v, int d) {

	switch(v->type()) {
	case value_type::Null:
		return print(static_cast<Null*>(v), d);
	case value_type::Integer:
		return print(static_cast<Integer*>(v), d);
	case value_type::Float:
		return print(static_cast<Float*>(v), d);
	case value_type::String:
		return print(static_cast<String*>(v), d);
	case value_type::Boolean:
		return print(static_cast<Boolean*>(v), d);
	case value_type::Error:
		return print(static_cast<Error*>(v), d);
	case value_type::Array:
		return print(static_cast<Array*>(v), d);
	case value_type::Object:
		return print(static_cast<Object*>(v), d);
	case value_type::Dictionary:
		return print(static_cast<Dictionary*>(v), d);
	case value_type::Function:
		return print(static_cast<Function*>(v), d);
	case value_type::NativeFunction:
		return print(static_cast<NativeFunction*>(v), d);
	case value_type::Reference:
		return print(static_cast<Reference*>(v), d);
	}
}

} // namespace Type
