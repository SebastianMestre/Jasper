#include "gc_cell.hpp"

#include "../log/log.hpp"
#include "value.hpp"

namespace Interpreter {

static void gc_visit(GcCell* v, int generation);

static void gc_visit(Value h, int generation) {
	if (!is_heap_type(h.tag)) {
		return;
	}

	auto* cell = h.get();
	cell->visit(generation);
}

static void gc_visit(String* v, int generation) { }
static void gc_visit(VariantConstructor* v, int generation) { }
static void gc_visit(RecordConstructor* v, int generation) { }

static void gc_visit(Array* l, int generation) {
	for (auto child : l->m_value) {
		gc_visit(child, generation);
	}
}

static void gc_visit(Record* o, int generation) {
	for (auto child : o->m_value)
		gc_visit(child.second, generation);
}

static void gc_visit(Variant* u, int generation) {
	gc_visit(u->m_inner_value, generation);
}

static void gc_visit(Function* f, int generation) {
	for (auto& capture : f->m_captures)
		gc_visit(capture, generation);
}

static void gc_visit(Variable* r, int generation) {
	gc_visit(r->m_value, generation);
}

static void gc_visit(GcCell* v, int generation) {
#define DISPATCH(type) case ValueTag::type: return gc_visit(static_cast<type*>(v), generation);
	switch (v->type()) {
	DISPATCH(String);
	DISPATCH(Array);
	DISPATCH(Record);
	DISPATCH(Variant);
	DISPATCH(Function);
	DISPATCH(Variable);
	DISPATCH(VariantConstructor);
	DISPATCH(RecordConstructor);
	default: Log::missing_case("gc_visit", v->type_string());
	}
}

void GcCell::visit(int generation) {
	if (is_marked(generation)) return;
	mark(generation);
	return gc_visit(this, generation);
}

} // namespace Interpreter
