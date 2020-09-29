#include "unification.hpp"

#include <cassert>

namespace Unification {

bool Core::occurs(int v, int i){
	assert(node_header[v].tag == Tag::Var);
	assert(node_header[v].data_idx == v);

	i = find(i);

	if (i == v)
		return true;

	assert(node_header[i].tag == Tag::Term);

	int ti = node_header[i].data_idx;
	for (int c : term_data[ti].argument_idx)
		if (occurs(v, c))
			return true;

	return false;
}

int Core::find(int i) {
	if (node_header[i].tag == Tag::Term) return i;
	if (node_header[i].data_idx == i) return i;
	return node_header[i].data_idx = find(node_header[i].data_idx);
}

int Core::find_term(int i) {
	i = find(i);
	return node_header[i].tag == Tag::Term ? node_header[i].data_idx : -1;
}

void Core::unify(int i, int j) {
	i = find(i);
	j = find(j);

	if (node_header[j].tag == Tag::Var)
		std::swap(i, j);

	if (node_header[i].tag == Tag::Var) {

		if (node_header[j].tag == Tag::Term)
			assert(!occurs(i, j));

		node_header[i].data_idx = j;

	} else {
		int vi = node_header[i].data_idx;
		int vj = node_header[j].data_idx;

		unify_function(term_data[vi].function_id, term_data[vj].function_id);

		assert(term_data[vi].argument_idx.size() == term_data[vj].argument_idx.size());
		for (int k = 0; k < term_data[vi].argument_idx.size(); ++k)
			unify(term_data[vi].argument_idx[k], term_data[vj].argument_idx[k]);
	}
}

} // namespace Unification
