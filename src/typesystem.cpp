#include "typesystem.hpp"

#include <iostream>

#include <cassert>

#include "compile_time_environment.hpp"

TypeSystemCore::TypeSystemCore() {
	m_tf_core.unify_function = [&](int a, int b) {
		if (m_tf_core.find_term(a) == m_tf_core.find_term(b))
			return;
	
		TypeFunctionData& a_data = m_type_functions[m_tf_core.find_function(a)];
		TypeFunctionData& b_data = m_type_functions[m_tf_core.find_function(b)];

		if (b_data.is_dummy) {
			std::swap(a, b);
			std::swap(a_data, b_data);
		}

		if (a_data.is_dummy) {
			for (auto& kv_a : a_data.structure) {
				auto kv_b = b_data.structure.find(kv_a.first);

				if (kv_b == b_data.structure.end())
					// if b doesn't have a field of a, act accordingly
					if (b_data.is_dummy)
						b_data.structure.insert(kv_a);
					else
						assert(0 && "accessing non-existing field of a record type");
				else
					// else the fields must have equivalent types
					m_mono_core.unify(kv_a.second, kv_b->second);
			}

			// make a point to b after unifying their data.
			// this way, if more fields get added to a or b, both get updated
			//
			// TODO: think more thoroughly about this...
			// I get the feeling that we shouldn't really do this if b is a
			// polymorphic record. why? Because b will have type variables that
			// are bound to a forall kinda thing, and thus should NOT be unified.
			// instead, we should do something similar to inst_fresh, but for
			// type funcs.
			//
			// NOTE: this makes the unify_function need to access node_header
			m_tf_core.node_header[a].tag = Unification::Core::Tag::Var;
			m_tf_core.node_header[a].data_idx = b;

		} else {
			assert(0 and "unifying two different known type functions");
		}

	};

	m_mono_core.unify_function = [&](int a, int b) {
		a = m_mono_core.find_term(a);
		b = m_mono_core.find_term(b);

		if (a == b)
			return;
	
		Unification::Core::TermData& a_data = m_mono_core.term_data[a];
		Unification::Core::TermData& b_data = m_mono_core.term_data[b];

		m_tf_core.unify(a_data.function_id, b_data.function_id);
	};

	m_meta_core.unify_function = [&](int a, int b) {
		a = m_mono_core.find_term(a);
		b = m_mono_core.find_term(b);

#if DEBUG
		{
			if (a == b)
				return;

			char const* empty = "(no data)";

			char const* data_i = m_meta_core.node_header[i].debug
			                     ? m_meta_core.node_header[i].debug
			                     : empty;

			char const* data_j = m_metatype_header[j].debug
			                     ? m_metatype_header[j].debug
			                     : empty;

			std::cerr << "Tried to unify different metatypes.\n"
			          << "Debug data:\n"
			          << "i: " << data_i << '\n'
			          << "j: " << data_j << '\n';
		}
#else
		assert(a == b and "unified two different metatypes");
#endif
	};
}


void TypeSystemCore::print_type(MonoId mono, int d) {
	Unification::Core::NodeHeader& header =
	    m_mono_core.node_header[m_mono_core.find(mono)];

	for (int i = d; i--;)
		std::cerr << ' ';
	std::cerr << "[" << mono;
	if (header.debug) std::cerr << " | " << header.debug;
	std::cerr << "] ";
	if (header.tag == Unification::Core::Tag::Var) {
		if (header.data_idx == mono) {
			std::cerr << "Free Var\n";
		} else {
			std::cerr << "Var\n";
			print_type(header.data_idx, d + 1);
		}
	} else {
		TermId term = header.data_idx;
		Unification::Core::TermData& data = m_mono_core.term_data[term];
		std::cerr << "Term " << term << " (tf " << data.function_id << ")\n";
		for (const auto arg : data.argument_idx)
			print_type(arg, d + 1);
	}
}


MonoId TypeSystemCore::new_term(
    TypeFunctionId tf, std::vector<int> args, char const* tag) {
	tf = m_tf_core.find_function(tf);
	int argument_count = m_type_functions[tf].argument_count;

	if (argument_count != -1 && argument_count != args.size())
		assert(0 && "instanciating polymorphic type with wrong argument count");

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
	TypeFunctionId id = m_tf_core.new_term(m_type_functions.size(), {});
	m_type_functions.push_back({TypeFunctionTag::Builtin, arguments});
	return id;
}

TypeFunctionId TypeSystemCore::new_dummy_type_function
    (TypeFunctionTag type, std::unordered_map<std::string, MonoId> structure) {
	TypeFunctionId id = m_tf_core.new_term(m_type_functions.size(), {});
	m_type_functions.push_back({type, 0, structure, true});
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
