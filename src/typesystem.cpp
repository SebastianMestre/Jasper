#include "typesystem.hpp"

#include <iostream>

#include <cassert>

#include "compile_time_environment.hpp"

void TypeSystemCore::print_type(MonoId mono, int d) {
	MonoData& data = mono_data[mono];
	for (int i = d; i--;)
		std::cerr << ' ';
	std::cerr << "[" << mono << "] ";
	if (data.type == MonoTag::Var) {
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
	mono_data.push_back({MonoTag::Var, mono});
	return mono;
}

MonoId TypeSystemCore::new_term(
    TypeFunctionId type_function, std::vector<int> args, char const* tag) {
	TypeFunctionId tf_id = func_find(type_function);
	TypeFunctionData& tf_data = type_function_data[tf_id];

	if (tf_data.type == TypeFunctionTag::Var)
		assert(0 && "instantiating type function that could not be deduced");

	int argument_count = type_functions[tf_data.equals].argument_count;

	if (argument_count != -1 && argument_count != args.size()) {
		assert(0 && "instanciating polymorphic type with wrong argument count");
	}

	int term = term_data.size();
	int mono = mono_data.size();

	term_data.push_back({tf_id, std::move(args), tag});
	mono_data.push_back({MonoTag::Term, term});

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

TypeFunctionId TypeSystemCore::new_builtin_type_function(int arguments) {
	TypeFunctionId type_function_var = type_function_data.size();

	type_function_data.push_back({TypeFunctionTag::Builtin, type_functions.size()});
	type_functions.push_back({arguments});

	return type_function_var;
}

TypeFunctionId TypeSystemCore::new_type_function_var() {
	TypeFunctionId type_function_var = type_function_data.size();
	type_function_data.push_back({TypeFunctionTag::Var, type_function_var});
	return type_function_var;
}

// NOTE: I use int here to make this fail if we change
// the typesystem types to be type safe
TypeVarId TypeSystemCore::new_type_var(KindTag kind, int type_id) {
	TypeVarId type_var = type_vars.size();
	type_vars.push_back({kind, type_id});
	return type_var;
}

void TypeSystemCore::gather_free_vars(MonoId mono, std::unordered_set<MonoId>& free_vars) {
	MonoId repr = find(mono);
	MonoData const& data = mono_data[repr];
	if (data.type == MonoTag::Var) {
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

	if (data.type != MonoTag::Var)
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
		assert(var_mono_data.type == MonoTag::Var);
		assert(var == var_mono_data.data_id);
	}

	mono = find(mono);

	if (mono_data[mono].type == MonoTag::Var) {
		return mono_data[mono].data_id == var;
	}

	assert(mono_data[mono].type == MonoTag::Term);

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

	if (mono_data[a].type == MonoTag::Var) {
		if (mono_data[b].type == MonoTag::Var) {
			if (a < b) {
				// make the newer one point to the older one
				std::swap(a, b);
			}
		}

		if (occurs_in(a, b)) {
			assert(0 && "recursive unification\n");
		}

		mono_data[a].data_id = b;
	} else if (mono_data[b].type == MonoTag::Var) {
		return unify(b, a);
	} else {
		assert(mono_data[a].type == MonoTag::Term);
		assert(mono_data[b].type == MonoTag::Term);

		TermId ta = mono_data[a].data_id;
		TermId tb = mono_data[b].data_id;

		TermData& a_data = term_data[ta];
		TermData& b_data = term_data[tb];

		func_unify(a_data.type_function, b_data.type_function);

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

	if (data.type == MonoTag::Var) {
		auto it = mapping.find(mono);
		return it == mapping.end() ? mono : it->second;
	}

	if (data.type == MonoTag::Term) {
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

TypeFunctionId TypeSystemCore::func_find(TypeFunctionId func) {
	TypeFunctionData& func_data = type_function_data[func];

	if (func_data.type == TypeFunctionTag::Builtin or
	    func_data.equals == func)
		return func;

	return func_data.equals = find(func_data.equals);
}

void TypeSystemCore::func_unify(TypeFunctionId a, TypeFunctionId b) {
	a = func_find(a);
	b = func_find(b);

	if (a == b)
		return;

	TypeFunctionData& rep_a = type_function_data[a];
	TypeFunctionData& rep_b = type_function_data[b];

	if (rep_a.type == TypeFunctionTag::Var)
		rep_a.equals = b;
	else if (rep_b.type == TypeFunctionTag::Var)
		rep_b.equals = a;
	else
		assert(0 and "unifying two different known type functions");
}

TypeVarData TypeSystemCore::var_find(TypeVarId type_var) {
	TypeVarData& var_data = type_vars[type_var];

	switch(var_data.kind) {
	case KindTag::Mono:
		return {KindTag::Mono, find(var_data.type_var_id)};
	case KindTag::Poly:
		return {KindTag::Poly, var_data.type_var_id};
	case KindTag::TypeFunction:
		return {KindTag::TypeFunction, func_find(var_data.type_var_id)};
	}

	assert(0 and "unknown kind");
}

void TypeSystemCore::var_unify(TypeVarId a, TypeVarId b) {
	TypeVarData rep_a = var_find(a);
	TypeVarData rep_b = var_find(b);

	assert(rep_a.kind == rep_b.kind and "cannot unify different kinds");

	if (rep_a.type_var_id == rep_b.type_var_id)
		return;

	switch(rep_a.kind) {
	case KindTag::Mono:
		unify(rep_a.type_var_id, rep_b.type_var_id);
		break;
	case KindTag::Poly:
		break;
	case KindTag::TypeFunction:
		func_unify(rep_a.type_var_id, rep_b.type_var_id);
		break;
	}

	assert(0 and "unknown kind");
}
