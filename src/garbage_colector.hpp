#include "runtime.hpp";

using namespace GarbageCollector;

void soft_clean(Value* v) {
	for(const Value* child : v->references()) {
		/* we are freeing a value, its children lose a reference */
		child->lost_reference();
		soft_clean(child);
	}

	if (v.refcount == 0)
		free(v);
}

void hard_clean(/* whole memory reference */) {
	// dfs
}
