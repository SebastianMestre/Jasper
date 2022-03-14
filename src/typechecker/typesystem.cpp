#include "typesystem.hpp"

#include <cassert>

#include "../log/log.hpp"

TypeSystemCore::TypeSystemCore() {

	m_mono_core.ll_unify_function = [this](Unification::Core& core, int a, int b) {
		a = core.ll_find_term(a);
		b = core.ll_find_term(b);

		if (a == b)
			return;
	
		Unification::Core::TermData& a_data = core.ll_term_data[a];
		Unification::Core::TermData& b_data = core.ll_term_data[b];

		unify_type_function(a_data.function_id, b_data.function_id);
	};
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

	return m_mono_core.ll_new_term(tf, std::move(args), tag);
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
	mono = m_mono_core.ll_find(mono);
	Unification::Core::NodeHeader header = m_mono_core.ll_node_header[mono];

	if (header.tag == Unification::Core::Tag::Var) {
		auto it = mapping.find(mono);
		return it == mapping.end() ? mono : it->second;
	} else {
		TermId term = header.data_idx;
		std::vector<MonoId> new_args;
		for (MonoId arg : m_mono_core.ll_term_data[term].argument_idx)
			new_args.push_back(inst_impl(arg, mapping));
		return new_term(m_mono_core.ll_term_data[term].function_id, std::move(new_args));
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
		vals.push_back(m_mono_core.ll_new_var());
	return inst_with(poly, vals);
}

void TypeSystemCore::gather_free_vars(MonoId mono, std::unordered_set<MonoId>& free_vars) {
	mono = m_mono_core.ll_find(mono);
	const Unification::Core::NodeHeader& header = m_mono_core.ll_node_header[mono];

	if (header.tag == Unification::Core::Tag::Var) {
		free_vars.insert(mono);
	} else {
		TermId term = header.data_idx;
		for (MonoId arg : m_mono_core.ll_term_data[term].argument_idx)
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
    std::unordered_map<InternedString, MonoId> structure,
    bool dummy) {
	auto strength = dummy ? TypeFunctionStrength::Half : TypeFunctionStrength::Full;
	return create_type_function(
	    type, 0, std::move(fields), std::move(structure), strength);
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

TypeFunctionData& TypeSystemCore::type_function_data_of(MonoId mono){
	TypeFunctionId tf = m_mono_core.ll_find_function(mono);
	return get_type_function_data(tf);
}

void TypeSystemCore::unify_type_function(TypeFunctionId i, TypeFunctionId j) {
	i = find_type_function(i);
	j = find_type_function(j);

	if (i == j)
		return;

	if (get_type_function_data(i).strength == TypeFunctionStrength::Full &&
		get_type_function_data(j).strength == TypeFunctionStrength::Full)
		Log::fatal() << "unified different type functions";

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
		unify_type_function_data(i_data, j_data);
		return;
	}

	Log::fatal() << "unified different typefuncs";
}

void TypeSystemCore::point_type_function_at_another(TypeFunctionId a, TypeFunctionId b) {
	m_type_function_uf.join_left_to_right(a, b);
}

void TypeSystemCore::unify_type_function_data(TypeFunctionData& a_data, TypeFunctionData& b_data) {
	assert(a_data.strength == TypeFunctionStrength::Half);

	b_data.argument_count = compute_new_argument_count(a_data, b_data);

	bool can_add_fields_to_b = b_data.strength != TypeFunctionStrength::Full ;
	for (auto& kv_a : a_data.structure) {
		auto const& field_name = kv_a.first;
		auto const kv_b = b_data.structure.find(field_name);

		bool const b_has_field = kv_b != b_data.structure.end();
		if (b_has_field)
			m_mono_core.ll_unify(kv_a.second, kv_b->second);
		else if (can_add_fields_to_b)
			b_data.structure.insert(kv_a);
		else
			Log::fatal() << "Accessing non-existing field '" << field_name << "' of a record";
	}

}

int TypeSystemCore::compute_new_argument_count(
    TypeFunctionData const& a_data, TypeFunctionData const& b_data) const {

	if (a_data.argument_count == b_data.argument_count)
		return a_data.argument_count;

	if (b_data.strength == TypeFunctionStrength::Half || b_data.argument_count == -1)
		return -1;

	auto present_argument_count = [](int x) -> std::string {
		return x == -1 ? "variadic" : std::to_string(x);
	};
	std::string argc_a = present_argument_count(a_data.argument_count);
	std::string argc_b = present_argument_count(b_data.argument_count);
	Log::fatal() << "Deduced type functions with incompatible argument "
	                "counts to be equal (with "
	             << argc_a << " and " << argc_b << " arguments)";
}

TypeFunctionData& TypeSystemCore::get_type_function_data(TypeFunctionId tf) {
	return m_type_functions[find_type_function(tf)];
}

TypeFunctionId TypeSystemCore::find_type_function(TypeFunctionId tf) {
	return m_type_function_uf.find(tf);
}
