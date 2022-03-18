#include "typesystem.hpp"

#include <cassert>
#include <iostream>

#include "../log/log.hpp"

TypeSystemCore::TypeSystemCore() {
}

MonoId TypeSystemCore::new_term(
    TypeFunctionId tf, std::vector<int> args, char const* tag) {
	tf = find_type_function(tf);

	{
		// This block of code ensures that tf has the right arity
		TypeFunctionId dummy_tf = new_type_function_var();
		get_type_function_data(dummy_tf).argument_count = args.size();
		unify_type_function(tf, dummy_tf);
	}

	return ll_new_term(tf, std::move(args), tag);
}

PolyId TypeSystemCore::new_poly(MonoId mono, std::vector<MonoId> vars) {
	// TODO: check that the given vars are actually vars
	PolyData data;
	data.base = mono;
	data.vars = std::move(vars);
	PolyId poly = poly_data.size();
	poly_data.push_back(std::move(data));
	return poly;
}

MonoId TypeSystemCore::inst_impl(
    MonoId mono, std::unordered_map<MonoId, MonoId> const& mapping) {

	mono = ll_find(mono);
	NodeHeader header = ll_node_header[mono];

	if (header.tag == Tag::Var) {
		auto it = mapping.find(mono);
		return it == mapping.end() ? mono : it->second;
	} else {
		TermId term = header.data_idx;
		std::vector<MonoId> new_args;
		for (MonoId arg : ll_term_data[term].argument_idx)
			new_args.push_back(inst_impl(arg, mapping));
		return new_term(ll_term_data[term].function_id, std::move(new_args));
	}
}

MonoId TypeSystemCore::inst_with(PolyId poly, std::vector<MonoId> const& vals) {
	PolyData const& data = poly_data[poly];

	assert(data.vars.size() == vals.size());

	std::unordered_map<MonoId, MonoId> old_to_new;
	for (int i {0}; i != data.vars.size(); ++i) {
		old_to_new[data.vars[i]] = vals[i];
	}

	return inst_impl(data.base, old_to_new);
}

MonoId TypeSystemCore::inst_fresh(PolyId poly) {
	std::vector<MonoId> vals;
	for (int i {0}; i != poly_data[poly].vars.size(); ++i)
		vals.push_back(ll_new_var());
	return inst_with(poly, vals);
}

void TypeSystemCore::gather_free_vars(MonoId mono, std::unordered_set<MonoId>& free_vars) {
	mono = ll_find(mono);
	const NodeHeader& header = ll_node_header[mono];

	if (header.tag == Tag::Var) {
		free_vars.insert(mono);
	} else {
		TermId term = header.data_idx;
		for (MonoId arg : ll_term_data[term].argument_idx)
			gather_free_vars(arg, free_vars);
	}
}



TypeFunctionId TypeSystemCore::new_builtin_type_function(int arity) {
	return create_type_function(
	    TypeFunctionTag::Builtin, arity, {}, {}, TypeFunctionStrength::Full);
}

TypeFunctionId TypeSystemCore::new_type_function(
    TypeFunctionTag type,
    std::vector<InternedString> fields,
    std::unordered_map<InternedString, MonoId> structure) {
	return create_type_function(
	    type, 0, std::move(fields), std::move(structure), TypeFunctionStrength::Full);
}

TypeFunctionId TypeSystemCore::new_type_function_var() {
	return create_type_function(
	    TypeFunctionTag::Builtin, -1, {}, {}, TypeFunctionStrength::None);
}

TypeFunctionId TypeSystemCore::create_type_function(
    TypeFunctionTag tag,
    int arity,
    std::vector<InternedString> fields,
    std::unordered_map<InternedString, MonoId> structure,
    TypeFunctionStrength strength) {
	TypeFunctionId result = m_type_function_uf.new_node();
	m_type_functions.push_back(
	    {tag, arity, std::move(fields), std::move(structure), strength});
	return result;
}

TypeFunctionData& TypeSystemCore::type_function_data_of(MonoId mono) {
	mono = ll_find(mono);
	assert(ll_is_term(mono) && "tried to find function of non term");
	int t = ll_node_header[mono].data_idx;
	TypeFunctionId tf = ll_term_data[t].function_id;
	return get_type_function_data(tf);
}

static InternedString print_a_thing(int x) {
	if (x == 0) return "function";
	if (x == 1) return "int";
	if (x == 2) return "float";
	if (x == 3) return "string";
	if (x == 4) return "array";
	if (x == 5) return "boolean";
	if (x == 6) return "unit";
	return "a user defined type";
}

void TypeSystemCore::unify_type_function(TypeFunctionId i, TypeFunctionId j) {
	i = find_type_function(i);
	j = find_type_function(j);

	if (i == j)
		return;

	if (get_type_function_data(i).strength == TypeFunctionStrength::Full &&
		get_type_function_data(j).strength == TypeFunctionStrength::Full) {
		Log::fatal() << "unified " << print_a_thing(i) << " with " << print_a_thing(j);
		// Log::fatal() << "unified different type functions";
	}

	if (get_type_function_data(j).strength == TypeFunctionStrength::None)
		std::swap(i, j);

	if (get_type_function_data(i).strength == TypeFunctionStrength::None) {
		point_type_function_at_another(i, j);
		return;
	}

	if (get_type_function_data(j).strength == TypeFunctionStrength::Half)
		std::swap(i, j);

	if (get_type_function_data(i).strength == TypeFunctionStrength::Half) {
		// We get the data before calling point_type_function_at_another
		// because doing it after the call would give us the same reference
		// for both indices
		auto& i_data = get_type_function_data(i);
		auto& j_data = get_type_function_data(j);
		point_type_function_at_another(i, j);
		return;
	}

	Log::fatal() << "unified different typefuncs";
}

bool TypeSystemCore::occurs(VarId v, MonoId i) {
	i = ll_find(i);

	if (ll_is_var(i))
		return equals_var(i, v);

	int ti = ll_node_header[i].data_idx;
	for (int c : ll_term_data[ti].argument_idx)
		if (occurs(v, c))
			return true;

	return false;
}

void TypeSystemCore::unify_vars_left_to_right(VarId vi, VarId vj) {
	combine_constraints_left_to_right(vi, vj);
	m_type_var_uf.join_left_to_right(static_cast<int>(vi), static_cast<int>(vj));
}

void TypeSystemCore::combine_constraints_left_to_right(VarId vi, VarId vj) {
	// TODO
	return;
}

bool TypeSystemCore::satisfies(MonoId t, Constraint const& c) {
	return true;
}

void TypeSystemCore::ll_unify(int i, int j) {
	i = ll_find(i);
	j = ll_find(j);

	if (i == j) return;

	if (ll_is_var(j))
		std::swap(i, j);

	if (ll_is_var(i)) {

		auto vi = get_var_id(i);

		if (ll_node_header[j].tag == Tag::Term) {
			assert(!occurs(vi, j));
			assert(satisfies(j, m_constraints[static_cast<int>(vi)]));
			establish_substitution(vi, j);
		} else {
			unify_vars_left_to_right(vi, get_var_id(j));
		}

	} else {
		int vi = ll_node_header[i].data_idx;
		int vj = ll_node_header[j].data_idx;

		if (vi != vj) {
			TermData& i_data = ll_term_data[vi];
			TermData& j_data = ll_term_data[vj];

			unify_type_function(i_data.function_id, j_data.function_id);
		}

		assert(ll_term_data[vi].argument_idx.size() == ll_term_data[vj].argument_idx.size());
		for (int k = 0; k < ll_term_data[vi].argument_idx.size(); ++k)
			ll_unify(ll_term_data[vi].argument_idx[k], ll_term_data[vj].argument_idx[k]);
	}
}

int TypeSystemCore::ll_new_var(char const* debug) {
	return new_constrained_var({}, debug);
}

int TypeSystemCore::new_constrained_var(Constraint c, char const* debug) {
	int var_id = m_var_counter++;
	int uf_node = m_type_var_uf.new_node();

	assert(uf_node == var_id);
	assert(m_substitution.size() == var_id);
	assert(m_constraints.size() == var_id);
	m_substitution.push_back(-1);
	m_constraints.push_back(std::move(c));

	int type_id = m_type_counter++;
	assert(ll_node_header.size() == type_id);
	ll_node_header.push_back({Tag::Var, var_id, debug});

	return type_id;
}

int TypeSystemCore::ll_new_term(int f, std::vector<int> args, char const* debug) {
	int type_id = m_type_counter++;
	assert(ll_node_header.size() == type_id);
	ll_node_header.push_back({Tag::Term, static_cast<int>(ll_term_data.size()), debug});
	ll_term_data.push_back({f, std::move(args)});
	return type_id;
}


int TypeSystemCore::ll_find(int i) {
	if (!ll_is_var(i)) return i;
	VarId vi = get_var_id(i);
	if (m_substitution[static_cast<int>(vi)] == -1) return i;
	return ll_find(m_substitution[static_cast<int>(vi)]);
}

VarId TypeSystemCore::get_var_id(MonoId i) {
	assert(ll_is_var(i));
	return static_cast<VarId>(m_type_var_uf.find(ll_node_header[i].data_idx));
}

void TypeSystemCore::establish_substitution(VarId var_id, int type_id) {
	assert(m_substitution[static_cast<int>(var_id)] == -1);
	m_substitution[static_cast<int>(var_id)] = type_id;
}

bool TypeSystemCore::ll_is_term(int i) {
	return ll_node_header[i].tag == Tag::Term;
}

bool TypeSystemCore::ll_is_var(int i) {
	return ll_node_header[i].tag == Tag::Var;
}

void TypeSystemCore::point_type_function_at_another(TypeFunctionId a, TypeFunctionId b) {
	m_type_function_uf.join_left_to_right(a, b);
}

TypeFunctionData& TypeSystemCore::get_type_function_data(TypeFunctionId tf) {
	return m_type_functions[find_type_function(tf)];
}

TypeFunctionId TypeSystemCore::find_type_function(TypeFunctionId tf) {
	return m_type_function_uf.find(tf);
}

bool TypeSystemCore::equals_var(MonoId t, VarId v) {
	return ll_is_var(t) && static_cast<VarId>(ll_node_header[t].data_idx) == v;
}
