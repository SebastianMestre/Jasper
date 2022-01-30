#include "typesystem.hpp"

#include <cassert>

#include "./log/log.hpp"

TypeSystemCore::TypeSystemCore() {
	m_tf_core.unify_function = [this](Unification::Core& core, int a, int b) {
		// TODO: Redo this once we add polymorphic records
		if (core.find_term(a) == core.find_term(b))
			return;
	
		int fa = core.find_function(a);
		int fb = core.find_function(b);

		if (m_type_functions[fb].is_dummy) {
			std::swap(a, b);
			std::swap(fa, fb);
		}

		TypeFunctionData& a_data = m_type_functions[fa];
		TypeFunctionData& b_data = m_type_functions[fb];

		if (a_data.is_dummy) {

			// Make a point to b. this way, if more fields get added to a or b,
			// both get updated
			//
			// Also, we do it before unifying their data to prevent infinite
			// recursion
			point_tf_at_another(a, b);

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

		} else {
			Log::fatal()
			    << "Deduced two different type functions to be equal (with IDs "
			    << fa << " and " << fb << ")";
		}
	};

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
	tf = m_tf_core.find(tf);

	{
		// TODO: add a TypeFunctionTag::Unknown tag, to express
		// that it's a dummy of unknown characteristics

		// TODO: add some APIs to make this less jarring
		TypeFunctionId dummy_tf =
		    new_type_function(TypeFunctionTag::Builtin, {}, {}, true);

		int dummy_tf_data_id = m_tf_core.find_function(dummy_tf);
		m_type_functions[dummy_tf_data_id].argument_count = args.size();

		m_tf_core.unify(tf, dummy_tf);
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


TypeFunctionId TypeSystemCore::new_builtin_type_function(int arguments) {
	TypeFunctionId id = m_tf_core.new_term(m_type_functions.size());
	m_type_functions.push_back({TypeFunctionTag::Builtin, arguments});
	return id;
}

TypeFunctionId TypeSystemCore::new_type_function(
    TypeFunctionTag type,
    std::vector<InternedString> fields,
    std::unordered_map<InternedString, MonoId> structure,
    bool dummy) {
	TypeFunctionId id = m_tf_core.new_term(m_type_functions.size());
	m_type_functions.push_back(
	    {type, 0, std::move(fields), std::move(structure), dummy});
	return id;
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




TypeFunctionData& TypeSystemCore::type_function_data_of(MonoId mono){
	TypeFunctionId tf_header = m_mono_core.find_function(mono);
	int tf = m_tf_core.find_function(tf_header);
	return m_type_functions[tf];
}

TypeFunctionId TypeSystemCore::new_tf_var() {
	return m_tf_core.new_var();
}

void TypeSystemCore::unify_tf(TypeFunctionId i, TypeFunctionId j) {
	m_tf_core.unify(i, j);
}

void TypeSystemCore::point_tf_at_another(TypeFunctionId a, TypeFunctionId b) {
	m_tf_core.node_header[a].tag = Unification::Core::Tag::Var;
	m_tf_core.node_header[a].data_idx = b;
}

int TypeSystemCore::compute_new_argument_count(
    TypeFunctionData const& a_data, TypeFunctionData const& b_data) const {
	if (a_data.argument_count == b_data.argument_count) {
		return a_data.argument_count;
	} else if (b_data.is_dummy || b_data.argument_count == -1) {
		return -1;
	} else {
		std::string argc_a = a_data.argument_count == -1
			? std::string("variadic")
			: std::to_string(a_data.argument_count);
		std::string argc_b = b_data.argument_count == -1
			? std::string("variadic")
			: std::to_string(b_data.argument_count);
		Log::fatal()
			<< "Deduced type functions with incompatible argument "
			"counts to be equal (with "
			<< argc_a << " and " << argc_b << " arguments)";
	}
}

// void TypeSystemCore::technobabble(TypeFunctionData& a_data, TypeFunctionData& b_data) { }
