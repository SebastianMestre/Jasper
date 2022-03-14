#include "unification.hpp"

#include <cassert>
#include <iostream>

namespace Unification {

bool Core::ll_occurs(int v, int i){
	assert(ll_node_header[v].tag == Tag::Var);
	assert(ll_node_header[v].data_idx == v);

	i = ll_find(i);

	if (i == v)
		return true;

	if (ll_node_header[i].tag == Tag::Var)
		return false;

	int ti = ll_node_header[i].data_idx;
	for (int c : ll_term_data[ti].argument_idx)
		if (ll_occurs(v, c))
			return true;

	return false;
}

int Core::ll_find(int i) {
	if (ll_node_header[i].tag == Tag::Term) return i;
	if (ll_node_header[i].data_idx == i) return i;
	return ll_node_header[i].data_idx = ll_find(ll_node_header[i].data_idx);
}

int Core::ll_find_term(int i) {
	i = ll_find(i);
	return ll_is_term(i) ? ll_node_header[i].data_idx : -1;
}

int Core::ll_find_function(int i) {
	i = ll_find_term(i);
	assert(i != -1 && "tried to find function of non term");
	return ll_term_data[i].function_id;
}

void Core::ll_unify(int i, int j) {
	i = ll_find(i);
	j = ll_find(j);

	if (ll_node_header[j].tag == Tag::Var)
		std::swap(i, j);

	if (ll_node_header[i].tag == Tag::Var) {

		if (ll_node_header[j].tag == Tag::Term)
			assert(!ll_occurs(i, j));

		ll_node_header[i].data_idx = j;

	} else {
		int vi = ll_node_header[i].data_idx;
		int vj = ll_node_header[j].data_idx;

		ll_unify_function(*this, i, j);

		assert(ll_term_data[vi].argument_idx.size() == ll_term_data[vj].argument_idx.size());
		for (int k = 0; k < ll_term_data[vi].argument_idx.size(); ++k)
			ll_unify(ll_term_data[vi].argument_idx[k], ll_term_data[vj].argument_idx[k]);
	}
}

int Core::ll_new_var(char const* debug) {
	int id = ll_node_header.size();
	ll_node_header.push_back({Tag::Var, id, debug});
	return id;
}

int Core::ll_new_term(int f, std::vector<int> args, char const* debug) {
	int id = ll_node_header.size();
	ll_node_header.push_back({Tag::Term, static_cast<int>(ll_term_data.size()), debug});
	ll_term_data.push_back({f, std::move(args)});
	return id;
}

bool Core::ll_is_term(int i) {
	return ll_node_header[i].tag == Tag::Term;
}

bool Core::ll_is_var(int i) {
	return ll_node_header[i].tag == Tag::Var;
}

void Core::ll_print_node(int header, int d) {
	Core::NodeHeader& node = ll_node_header[ll_find(header)];

	for (int i = d; i--;)
		std::cerr << ' ';
	std::cerr << "[" << header;
	if (node.debug) std::cerr << " | " << node.debug;
	std::cerr << "] ";
	if (node.tag == Core::Tag::Var) {
		if (node.data_idx == header) {
			std::cerr << "Free Var\n";
		} else {
			std::cerr << "Var\n";
			ll_print_node(node.data_idx, d + 1);
		}
	} else {
		Core::TermData& data = ll_term_data[node.data_idx];
		std::cerr << "Term " << node.data_idx << " (tf " << data.function_id << ")\n";
		for (const auto arg : data.argument_idx)
			ll_print_node(arg, d + 1);
	}
}

} // namespace Unification
