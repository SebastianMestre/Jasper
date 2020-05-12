#include "value.hpp"

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



List::List() : Value(value_type::List) {}
List::List(ListType l) : Value(value_type::List), m_value(std::move(l)) {}

void List::append(Value* v) {
	m_value.push_back(v);
}

Value* List::at(int position) {
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

void gc_visit(Null* v) { v->m_visited = true; }
void gc_visit(Integer* v) { v->m_visited = true; }
void gc_visit(Float* v) { v->m_visited = true; }
void gc_visit(String* v) { v->m_visited = true; }
void gc_visit(Boolean* v) { v->m_visited = true; }
void gc_visit(Error* v) { v->m_visited = true; }

void gc_visit(List* l) {
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
	case value_type::List:
		return gc_visit(static_cast<List*>(v));
	case value_type::Object:
		return gc_visit(static_cast<Object*>(v));
	case value_type::Dictionary:
		return gc_visit(static_cast<Dictionary*>(v));
	case value_type::Function:
		return gc_visit(static_cast<Function*>(v));
	}
}

} // namespace Type
