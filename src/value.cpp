#include "value.hpp"

#include "environment.hpp"

namespace Type {

Null::Null() : Value(value_type::Null) {}

void Null::gc_visit() {
	Value::m_visited = true;
}



Integer::Integer() : Value(value_type::Integer) {}
Integer::Integer(int v) : Value(value_type::Integer), m_value(v) {}

void Integer::gc_visit() {
	if (Value::m_visited)
		return;

	Value::m_visited = true;
}



Float::Float() : Value(value_type::Float) {}
Float::Float(float v) : Value(value_type::Float), m_value(v) {}

void Float::gc_visit() {
	if (Value::m_visited)
		return;

	Value::m_visited = true;
}



Boolean::Boolean() : Value(value_type::Boolean) {}
Boolean::Boolean(bool b) : Value(value_type::Boolean), m_value(b) {}

void Boolean::gc_visit() {
	if (Value::m_visited)
		return;

	Value::m_visited = true;
}



String::String() : Value(value_type::String) {}
String::String(std::string s) : Value(value_type::String), m_value(std::move(s)) {}

void String::gc_visit() {
	if (Value::m_visited)
		return;

	Value::m_visited = true;
}



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

void List::gc_visit() {
	if (Value::m_visited)
		return;

	Value::m_visited = true;
	for (auto* child : m_value) {
		child->gc_visit();
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

void Object::gc_visit() {
	if (Value::m_visited)
		return;

	Value::m_visited = true;
	for (auto child : m_value) {
		child.second->gc_visit();
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

void Dictionary::gc_visit() {
	if (Value::m_visited)
		return;

	Value::m_visited = true;
	for (auto child : m_value) {
		child.second->gc_visit();
	}
}

Function::Function() : Value(value_type::Function) {}
Function::Function(FunctionType def, Scope* scope)
	: Value(value_type::Function), m_def(def), m_scope(scope) {}

void Function::gc_visit() {
	if (Value::m_visited)
		return;
	
	Value::m_visited = true;
	for (auto dec : m_scope->m_declarations)
		dec.second->gc_visit();
}

// TODO: implement call

} // namespace Type
