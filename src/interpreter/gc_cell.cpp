#include "gc_cell.hpp"

#include "value.hpp"
#include "error.hpp"

namespace Interpreter {

static void gc_visit(GcCell* v);

static void gc_visit(Value h) {
	if (is_heap_type(h.type()))
		return gc_visit(h.get());
}

static void gc_visit(String* v) {
	v->m_visited = true;
}
static void gc_visit(Error* v) {
	v->m_visited = true;
}
static void gc_visit(VariantConstructor* v) {
	v->m_visited = true;
}
static void gc_visit(RecordConstructor* v) {
	v->m_visited = true;
}

static void gc_visit(Array* l) {
	if (l->m_visited)
		return;

	l->m_visited = true;
	for (auto* child : l->m_value) {
		gc_visit(child);
	}
}

static void gc_visit(Record* o) {
	if (o->m_visited)
		return;

	o->m_visited = true;
	for (auto child : o->m_value)
		gc_visit(child.second);
}

static void gc_visit(Variant* u) {
	if (u->m_visited)
		return;

	u->m_visited = true;
	gc_visit(u->m_inner_value);
}

static void gc_visit(Function* f) {
	if (f->m_visited)
		return;

	f->m_visited = true;
	for (auto& capture : f->m_captures)
		gc_visit(capture);
}

static void gc_visit(Reference* r) {
	if (r->m_visited)
		return;

	r->m_visited = true;
	gc_visit(r->m_value);
}

static void gc_visit(GcCell* v) {
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
	case ValueTag::Reference:
		return gc_visit(static_cast<Reference*>(v));
	case ValueTag::VariantConstructor:
		return gc_visit(static_cast<VariantConstructor*>(v));
	case ValueTag::RecordConstructor:
		return gc_visit(static_cast<RecordConstructor*>(v));
	default:
		assert(0);
	}
}

void GcCell::visit() {
	return gc_visit(this);
}

} // namespace Interpreter
