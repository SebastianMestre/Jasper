#include "typesystem.hpp"

#include <cassert>

#include "./log/log.hpp"

TypeSystemCore::TypeSystemCore() {

	m_mono_core.unify_function = [this](Unification::Core& core, int a, int b) {
		a = core.find_term(a);
		b = core.find_term(b);

		if (a == b)
			return;
	
		Unification::Core::TermData& a_data = core.term_data[a];
		Unification::Core::TermData& b_data = core.term_data[b];

		unify_tf(a_data.function_id, b_data.function_id);
	};
}


MonoId TypeSystemCore::new_term(
    TypeFunctionId tf, std::vector<int> args, char const* tag) {
	tf = find_tf(tf);

	{
		// This block of code ensures that tf has the right arity
		TypeFunctionId dummy_tf = new_tf_var();
		get_tf_data(dummy_tf).argument_count = args.size();
		unify_tf(tf, dummy_tf);
	}

	return m_mono_core.new_term(tf, std::move(args), tag);
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

	// NOTE(Mestre): Is just calling find good enough? It means we
	// should only ever qualify variables that are their own
	// representative, which does seem to make sense. I think.
	mono = m_mono_core.find(mono);
	Unification::Core::NodeHeader header = m_mono_core.node_header[mono];

	if (header.tag == Unification::Core::Tag::Var) {
		auto it = mapping.find(mono);
		return it == mapping.end() ? mono : it->second;
	} else {
		TermId term = header.data_idx;
		std::vector<MonoId> new_args;
		for (MonoId arg : m_mono_core.term_data[term].argument_idx)
			new_args.push_back(inst_impl(arg, mapping));
		return new_term(m_mono_core.term_data[term].function_id, std::move(new_args));
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
		vals.push_back(m_mono_core.new_var());
	return inst_with(poly, vals);
}

void TypeSystemCore::gather_free_vars(MonoId mono, std::unordered_set<MonoId>& free_vars) {
	mono = m_mono_core.find(mono);
	const Unification::Core::NodeHeader& header = m_mono_core.node_header[mono];

	if (header.tag == Unification::Core::Tag::Var) {
		free_vars.insert(mono);
	} else {
		TermId term = header.data_idx;
		for (MonoId arg : m_mono_core.term_data[term].argument_idx)
			gather_free_vars(arg, free_vars);
	}
}



TypeFunctionId TypeSystemCore::new_builtin_type_function(int arity) {
	return create_tf(TypeFunctionTag::Builtin, arity, {}, {}, false);
}

TypeFunctionId TypeSystemCore::new_type_function(
    TypeFunctionTag type,
    std::vector<InternedString> fields,
    std::unordered_map<InternedString, MonoId> structure,
    bool dummy) {
	return create_tf(type, 0, std::move(fields), std::move(structure), dummy);
}

TypeFunctionId TypeSystemCore::new_tf_var() {
	return create_tf(TypeFunctionTag::Builtin, -1, {}, {}, true);
}

TypeFunctionId TypeSystemCore::create_tf(
    TypeFunctionTag tag,
    int arity,
    std::vector<InternedString> fields,
    std::unordered_map<InternedString, MonoId> structure,
    bool is_dummy) {
	TypeFunctionId result = m_tf_uf.new_var();
	m_type_functions.push_back(
	    {tag, arity, std::move(fields), std::move(structure), is_dummy});
	return result;
}

TypeFunctionData& TypeSystemCore::type_function_data_of(MonoId mono){
	TypeFunctionId tf = m_mono_core.find_function(mono);
	return get_tf_data(tf);
}

void TypeSystemCore::unify_tf(TypeFunctionId i, TypeFunctionId j) {
	i = find_tf(i);
	j = find_tf(j);

	if (i == j)
		return;

	if (get_tf_data(j).is_dummy)
		std::swap(i, j);

	if (get_tf_data(i).is_dummy) {
		point_tf_at_another(i, j);
		unify_tf_data(get_tf_data(i), get_tf_data(j));
	} else {
		Log::fatal() << "unified different typefuncs";
	}
}

void TypeSystemCore::point_tf_at_another(TypeFunctionId a, TypeFunctionId b) {
	m_tf_uf.join_left_to_right(a, b);
}

int TypeSystemCore::compute_new_argument_count(
    TypeFunctionData const& a_data, TypeFunctionData const& b_data) const {
	if (a_data.argument_count == b_data.argument_count) {
		return a_data.argument_count;
	} else if (b_data.is_dummy || b_data.argument_count == -1) {
		return -1;
	} else {
		auto present_argument_count = [](int x) -> std::string {
			return x == -1 ? "variadic" : std::to_string(x);
		};
		std::string argc_a = present_argument_count(a_data.argument_count);
		std::string argc_b = present_argument_count(b_data.argument_count);
		Log::fatal() << "Deduced type functions with incompatible argument "
		                "counts to be equal (with "
		             << argc_a << " and " << argc_b << " arguments)";
	}
}

void TypeSystemCore::unify_tf_data(TypeFunctionData& a_data, TypeFunctionData& b_data) {

	int const new_argument_count = compute_new_argument_count(a_data, b_data);

	b_data.argument_count = new_argument_count;

	for (auto& kv_a : a_data.structure) {
		auto kv_b = b_data.structure.find(kv_a.first);

		if (kv_b == b_data.structure.end())
			// if b doesn't have a field of a, act accordingly
			if (b_data.is_dummy)
				b_data.structure.insert(kv_a);
			else
				Log::fatal() << "Accessing non-existing field '" << kv_a.first << "' of a record";
		else
			// else the fields must have equivalent types
			m_mono_core.unify(kv_a.second, kv_b->second);
	}
}

TypeFunctionData& TypeSystemCore::get_tf_data(TypeFunctionId tf) {
	return m_type_functions[find_tf(tf)];
}

TypeFunctionId TypeSystemCore::find_tf(TypeFunctionId tf) {
	return m_tf_uf.find(tf);
}
