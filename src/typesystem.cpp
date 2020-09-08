#include "typesystem.hpp"

#include <iostream>

#include <cassert>

#include "compile_time_environment.hpp"

void TypeSystemCore::print_type(MonoId mono, int d) {
	MonoData& data = mono_data[mono];
	for (int i = d; i--;)
		std::cerr << ' ';
	std::cerr << "[" << mono << "] ";
	if (data.type == mono_type::Var) {
		if (data.data_id == mono) {
			std::cerr << "Free Var\n";
		} else {
			std::cerr << "Var\n";
			print_type(data.data_id, d + 1);
		}
	} else {
		TermId term = data.data_id;
		TermData& data = term_data[term];
		std::cerr << "Term " << term << " (tf " << data.type_function << ")";
		if(data.debug_data) std::cerr << " [ " << data.debug_data << " ]";
		std::cerr << "!\n";
		for (int i = 0; i < data.arguments.size(); ++i)
			print_type(data.arguments[i], d + 1);
	}
}

MonoId TypeSystemCore::new_var() {
	int mono = mono_data.size();
	mono_data.push_back({mono_type::Var, mono});
	return mono;
}

MonoId TypeSystemCore::new_term(
    TypeFunctionId type_function, std::vector<int> args, char const* tag) {
	int argument_count = type_function_data[type_function].argument_count;

	if (argument_count != -1 && argument_count != args.size()) {
		assert(0 && "instanciating polymorphic type with wrong argument count");
	}

	int term = term_data.size();
	int mono = mono_data.size();

	term_data.push_back({type_function, std::move(args), tag});
	mono_data.push_back({mono_type::Term, term});

	return mono;
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

void TypeSystemCore::gather_free_vars(MonoId mono, std::unordered_set<MonoId>& free_vars) {
	MonoId repr = find(mono);
	MonoData const& data = mono_data[repr];
	if (data.type == mono_type::Var) {
		free_vars.insert(repr);
	} else {
		TermId term = data.data_id;
		for (MonoId arg : term_data[term].arguments)
			gather_free_vars(arg, free_vars);
	}
}

// qualifies all free variables in the given monotype
// NOTE(Mestre): I don't like how we take the CTenv as an
// argument. This calls for some refactoring...
PolyId TypeSystemCore::generalize(MonoId mono, Frontend::CompileTimeEnvironment& env) {
	std::unordered_set<MonoId> free_vars;
	gather_free_vars(mono, free_vars);

	std::vector<MonoId> new_vars;
	std::unordered_map<MonoId, MonoId> mapping;
	int i = 0;
	for (MonoId var : free_vars) {
		if (!env.has_type_var(var)) {
			auto fresh_var = new_var();
			new_vars.push_back(fresh_var);
			mapping[var] = fresh_var;
		}
	}

	MonoId base = inst_impl(mono, mapping);

	return new_poly(base, std::move(new_vars));
}

MonoId TypeSystemCore::find(MonoId mono) {
	MonoData& data = mono_data[mono];

	if (data.type != mono_type::Var)
		return mono;

	// pointing to self
	if (data.data_id == mono)
		return mono;

	return data.data_id = find(data.data_id);
}

bool TypeSystemCore::occurs_in(MonoId var, MonoId mono) {

	{
		// var must be a variable that points to itself
		MonoData const& var_mono_data = mono_data[var];
		assert(var_mono_data.type == mono_type::Var);
		assert(var == var_mono_data.data_id);
	}

	mono = find(mono);

	if (mono_data[mono].type == mono_type::Var) {
		return mono_data[mono].data_id == var;
	}

	assert(mono_data[mono].type == mono_type::Term);

	TermId term = mono_data[mono].data_id;
	TermData data = term_data[term];

	for (MonoId c : data.arguments)
		if (occurs_in(var, c))
			return true;

	return false;
}

void TypeSystemCore::unify(MonoId a, MonoId b) {
	a = find(a);
	b = find(b);

	if (a == b)
		return;

	if (mono_data[a].type == mono_type::Var) {
		if (mono_data[b].type == mono_type::Var) {
			if (a < b) {
				// make the newer one point to the older one
				std::swap(a, b);
			}
		}

		if (occurs_in(a, b)) {
			assert(0 && "recursive unification\n");
		}

		mono_data[a].data_id = b;
	} else if (mono_data[b].type == mono_type::Var) {
		return unify(b, a);
	} else {
		assert(mono_data[a].type == mono_type::Term);
		assert(mono_data[b].type == mono_type::Term);

		TermId ta = mono_data[a].data_id;
		TermId tb = mono_data[b].data_id;

		TermData& a_data = term_data[ta];
		TermData& b_data = term_data[tb];

		if (term_data[ta].type_function != term_data[tb].type_function) {
			assert(0 && "deduced two different polymorphic types to be equal");
		}

		if (a_data.arguments.size() != b_data.arguments.size()) {
			// for instance: (int,float)->int == (int)->int
			assert(0 && "deduced two instances of a polymorphic type with different amount of arguments to be equal.");
		}

		TypeFunctionId type_function = term_data[ta].type_function;
		int argument_count = a_data.arguments.size();

		for (int i {0}; i != argument_count; ++i) {
			unify(a_data.arguments[i], b_data.arguments[i]);
		}
	}
}

MonoId TypeSystemCore::inst_impl(
    MonoId mono, std::unordered_map<MonoId, MonoId> const& mapping) {

	// NOTE(Mestre): Is just calling find good enough? It means we
	// should only ever qualify variables that are their own
	// representative, which does seem to make sense. I think.
	mono = find(mono);
	MonoData data = mono_data[mono];

	if (data.type == mono_type::Var) {
		auto it = mapping.find(mono);
		return it == mapping.end() ? mono : it->second;
	}

	if (data.type == mono_type::Term) {
		TermId term = data.data_id;
		std::vector<MonoId> new_args;
		for (MonoId argument : term_data[term].arguments)
			new_args.push_back(inst_impl(argument, mapping));
		return new_term(term_data[term].type_function, std::move(new_args));
	}

	assert(0 && "invalid term type");
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
		vals.push_back(new_var());
	return inst_with(poly, vals);
}
