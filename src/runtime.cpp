#include "runtime.hpp"
#include "environment.hpp"

namespace Type {

void Null::gc_visit() {
	Value::m_visited = true;
}



Integer::Integer(int v) : m_value(v) {}

void Integer::gc_visit() {
	if (Value::m_visited)
		return;

	Value::m_visited = true;
}



Float::Float(float v) : m_value(v) {}

void Float::gc_visit() {
	if (Value::m_visited)
		return;

	Value::m_visited = true;
}



String::String(std::string s) : m_value(std::move(s)) {}

void String::gc_visit() {
	if (Value::m_visited)
		return;

	Value::m_visited = true;
}



List::List(ListType l) : m_value(std::move(l)) {}

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



Object::Object(ObjectType o) : m_value(std::move(o)) {}

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



Dictionary::Dictionary(ObjectType o) : m_value(std::move(o)) {}

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

Function::Function(FunctionType def, Scope* scope) : m_def(def), m_scope(scope) {}

void Function::gc_visit() {
	if (Value::m_visited)
		return;
	
	Value::m_visited = true;
	for (auto dec : m_scope->m_declarations)
		dec.second->gc_visit();
}

// TODO: implement call

} // namespace Type
