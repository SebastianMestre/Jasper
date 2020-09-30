#include "unification.hpp"

#include <cassert>

namespace Unification {

bool Core::occurs(int v, int i){
	assert(node_header[v].tag == Tag::Var);
	assert(node_header[v].data_idx == v);

	i = find(i);

	if (i == v)
		return true;

	if (node_header[i].tag == Tag::Var)
		return false;

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

int Core::find_function(int i) {
	i = find_term(i);
	assert(i != -1 && "tried to find function of non term");
	return term_data[i].function_id;
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

		unify_function(i, j);

		assert(term_data[vi].argument_idx.size() == term_data[vj].argument_idx.size());
		for (int k = 0; k < term_data[vi].argument_idx.size(); ++k)
			unify(term_data[vi].argument_idx[k], term_data[vj].argument_idx[k]);
	}
}

int Core::new_var(char const* debug) {
	int id = node_header.size();
	node_header.push_back({Tag::Var, id, debug});
	return id;
}

int Core::new_term(int f, std::vector<int> args, char const* debug) {
	int id = node_header.size();
	node_header.push_back({Tag::Term, term_data.size(), debug});
	term_data.push_back({f, std::move(args)});
	return id;
}

} // namespace Unification
