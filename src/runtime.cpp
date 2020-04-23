#include "runtime.hpp"
#include "ast.hpp"

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



Function::Function(FunctionType def, Environment env) : m_def(std::move(def)), m_env(std::move(env)) {}

Value* Function::call(ListType args) {
	// FIXME: how to declare argument values, given a definition?
	return m_def->run(m_env);
}

void Function::gc_visit() {
	if (Value::m_visited)
		return;

	Value::m_visited = true;
	for(auto& child : m_env.m_scope->m_declarations)
		child.second->gc_visit();
}

} // namespace Type
