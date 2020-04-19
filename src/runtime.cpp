#include "runtime.hpp"

namespace Type {

void Null::gc_visit() {
	Value::visited = true;
}



Integer::Integer(int v) : value(v) {}

void Integer::gc_visit() {
	if (Value::visited)
		return;

	Value::visited = true;
}



String::String(std::string s) : value(std::move(s)) {}

void String::gc_visit() {
	if (Value::visited)
		return;

	Value::visited = true;
}



List::List(ListType l) : value(std::move(l)) {}

void List::append(Value* v) {
	value.push_back(v);
}

Value* List::at(int position) {
	if (position < 0 or position >= int(value.size())) {
		// TODO: return RangeError
		return nullptr;
	} else {
		return value[position];
	}
}

void List::gc_visit() {
	if (Value::visited)
		return;

	Value::visited = true;
	for (auto* child : value) {
		child->gc_visit();
	}
}



Object::Object(ObjectType o) : value(std::move(o)) {}

void Object::addMember(Identifier const& id, Value* v) {
	value[id] = v;
}

Value* Object::getMember(Identifier const& id) {
	auto it = value.find(id);
	if (it == value.end()) {
		// TODO: return RangeError
		return nullptr;
	} else {
		return it->second;
	}
}

void Object::gc_visit() {
	if (Value::visited)
		return;

	Value::visited = true;
	for (auto child : value) {
		child.second->gc_visit();
	}
}

// TODO: implement Function
// TODO: implement call

} // namespace Type
