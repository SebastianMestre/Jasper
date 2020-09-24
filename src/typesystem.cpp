#include "typesystem.hpp"

#include <iostream>

#include <cassert>

#include "compile_time_environment.hpp"

void TypeSystemCore::print_type(MonoId mono, int d) {
	MonoHeader& header = mono_header[mono];
	for (int i = d; i--;)
		std::cerr << ' ';
	std::cerr << "[" << mono;
	if (header.debug_data) std::cerr << " | " << header.debug_data;
	std::cerr << "] ";
	if (header.type == MonoTag::Var) {
		if (header.data_id == mono) {
			std::cerr << "Free Var\n";
		} else {
			std::cerr << "Var\n";
			print_type(header.data_id, d + 1);
		}
	} else {
		TermId term = header.data_id;
		TermData& data = term_data[term];
		std::cerr << "Term " << term << " (tf " << data.type_function << ")\n";
		for (int i = 0; i < data.arguments.size(); ++i)
			print_type(data.arguments[i], d + 1);
	}
}


MonoId TypeSystemCore::new_var() {
	int mono = mono_header.size();
	mono_header.push_back({MonoTag::Var, mono});
	return mono;
}

MonoId TypeSystemCore::new_term(
    TypeFunctionId tf, std::vector<int> args, char const* tag) {
	tf = func_find(tf);
	TypeFunctionHeader& tf_header = type_function_header[tf];

	int argument_count = type_function_data[tf_header.equals].argument_count;

	if (argument_count != -1 && argument_count != args.size()) {
		assert(0 && "instanciating polymorphic type with wrong argument count");
	}

	int term = term_data.size();
	int mono = mono_header.size();

	term_data.push_back({tf, std::move(args)});
	mono_header.push_back({MonoTag::Term, term, tag});

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
	TypeFunctionId type_function_var = type_function_header.size();

	type_function_header.push_back({TypeFunctionTag::Builtin, type_function_data.size()});
	type_function_data.push_back({arguments});

	return type_function_var;
}

TypeFunctionId TypeSystemCore::new_dummy_type_function
    (TypeFunctionTag type, std::unordered_map<std::string, MonoId> structure) {
	TypeFunctionId type_function_var = type_function_header.size();

	type_function_header.push_back({type, type_function_data.size()});
	type_function_data.push_back({-1, structure, true});

	return type_function_var;
}

TypeFunctionId TypeSystemCore::new_type_function_var() {
	TypeFunctionId type_function_var = type_function_header.size();
	type_function_header.push_back({TypeFunctionTag::Var, type_function_var});
	return type_function_var;
}

// NOTE: I use int here to make this fail if we change
// the typesystem types to be type safe
TypeVarId TypeSystemCore::new_type_var(KindTag kind, int type_id) {
	TypeVarId type_var = type_vars.size();
	type_vars.push_back({kind, type_id});
	return type_var;
}


MonoId TypeSystemCore::find(MonoId mono) {
	MonoHeader& header = mono_header[mono];

	if (header.type != MonoTag::Var)
		return mono;

	// pointing to self
	if (header.data_id == mono)
		return mono;

	return header.data_id = find(header.data_id);
}

bool TypeSystemCore::occurs_in(MonoId var, MonoId mono) {

	{
		// var must be a variable that points to itself
		MonoHeader const& var_mono_header = mono_header[var];
		assert(var_mono_header.type == MonoTag::Var);
		assert(var == var_mono_header.data_id);
	}

	mono = find(mono);

	if (mono_header[mono].type == MonoTag::Var) {
		return mono_header[mono].data_id == var;
	}

	assert(mono_header[mono].type == MonoTag::Term);

	TermId term = mono_header[mono].data_id;
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

	if (mono_header[a].type == MonoTag::Var) {
		if (mono_header[b].type == MonoTag::Var) {
			if (a < b) {
				// make the newer one point to the older one
				std::swap(a, b);
			}
		}

		if (occurs_in(a, b)) {
			assert(0 && "recursive unification\n");
		}

		mono_header[a].data_id = b;
	} else if (mono_header[b].type == MonoTag::Var) {
		return unify(b, a);
	} else {
		assert(mono_header[a].type == MonoTag::Term);
		assert(mono_header[b].type == MonoTag::Term);

		TermId ta = mono_header[a].data_id;
		TermId tb = mono_header[b].data_id;

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


TypeFunctionId TypeSystemCore::func_find(TypeFunctionId func) {
	TypeFunctionHeader& func_data = type_function_header[func];

	if (func_data.type != TypeFunctionTag::Var or
	    func_data.equals == func)
		return func;

	return func_data.equals = find(func_data.equals);
}

void TypeSystemCore::func_unify(TypeFunctionId a, TypeFunctionId b) {
	// TODO: handle recursive unification

	a = func_find(a);
	b = func_find(b);

	if (a == b)
		return;

	assert(type_function_header[a].type == type_function_header[b].type);

	// ensure a is a var if at least one of them is a var
	if (type_function_header[b].type == TypeFunctionTag::Var)
		std::swap(a, b);

	if (type_function_header[a].type == TypeFunctionTag::Var) {
		type_function_header[a].equals = b;
	} else {
		// neither a nor b is a var. we will try to unify their data.

		int a_data_idx = type_function_header[a].equals;
		int b_data_idx = type_function_header[b].equals;

		if (a_data_idx == b_data_idx)
			return;

		// ensure a is a dummy if at least one of them is a dummy
		if (type_function_data[b_data_idx].is_dummy) {
			// we don't really use a and b anymore, but let's keep it consistent.
			std::swap(a, b);
			std::swap(a_data_idx, b_data_idx);
		}

		TypeFunctionData& a_data = type_function_data[a_data_idx];
		TypeFunctionData& b_data = type_function_data[b_data_idx];

		if (type_function_data[a_data_idx].is_dummy) {
			// do member-wise unification

			for (auto& kv_a : a_data.structure) {
				// for every field in a

				// check that b has the same field
				auto kv_b = b_data.structure.find(kv_a.first);

				if (kv_b == b_data.structure.end())
					// if b doesn't have, act accordingly
					if (b_data.is_dummy)
						b_data.structure.insert(kv_a);
					else
						assert(0 && "accessing non-existing field of a record type");
				else
					// if the field exists in b, check that the types match
					unify(kv_a.second, kv_b->second);
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
			type_function_header[a].type = TypeFunctionTag::Var;
			type_function_header[a].equals = b;

		} else {
			// neither one is a dummy, and they are different, so we can't unify.
			assert(0 and "unifying two different known type functions");
		}
	}
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


MonoId TypeSystemCore::inst_impl(
    MonoId mono, std::unordered_map<MonoId, MonoId> const& mapping) {

	// NOTE(Mestre): Is just calling find good enough? It means we
	// should only ever qualify variables that are their own
	// representative, which does seem to make sense. I think.
	mono = find(mono);
	MonoHeader header = mono_header[mono];

	if (header.type == MonoTag::Var) {
		auto it = mapping.find(mono);
		return it == mapping.end() ? mono : it->second;
	}

	if (header.type == MonoTag::Term) {
		TermId term = header.data_id;
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

void TypeSystemCore::gather_free_vars(MonoId mono, std::unordered_set<MonoId>& free_vars) {
	MonoId repr = find(mono);
	MonoHeader const& header = mono_header[repr];
	if (header.type == MonoTag::Var) {
		free_vars.insert(repr);
	} else {
		TermId term = header.data_id;
		for (MonoId arg : term_data[term].arguments)
			gather_free_vars(arg, free_vars);
	}
}

