#include "typesystem.hpp"

#include <cassert>

MonoId TypeSystemData::new_var() {
	int var = var_data.size();
	int mono = mono_data.size();

	// we could put -1 instead, not sure which is better
	var_data.push_back({ mono });
	mono_data.push_back({ MonoData::Type::Var, var });

	return mono;
}

MonoId TypeSystemData::new_term(TypeFunctionId type_function, std::vector<int> args) {
	int term = term_data.size();
	int mono = mono_data.size();

	term_data.push_back({ type_function, std::move(args) });
	mono_data.push_back({ MonoData::Type::Term, term });

	return mono;
}

// qualifies all unbound variables in the given monotype
// PolyId new_poly (MonoId mono, Env&) { } // TODO

MonoId TypeSystemData::find(MonoId mono) {
	if (mono_data[mono].type != MonoData::Type::Var)
		return mono;

	VarId var = mono_data[mono].data_id;
	VarData& data = var_data[var];

	// pointing to self
	if (data.equals == mono)
		return mono;

	return data.equals = find(data.equals);
}

bool TypeSystemData::occurs_in(VarId var, MonoId mono) {
	mono = find(mono);

	if (mono_data[mono].type == MonoData::Type::Var) {
		return mono_data[mono].data_id == var;
	}

	assert(mono_data[mono].type == MonoData::Type::Term);
	TermId term = mono_data[mono].data_id;
	TermData data = term_data[term];

	for (MonoId c : data.arguments)
		if (occurs_in(var, c))
			return true;

	return false;
}

void TypeSystemData::unify(MonoId a, MonoId b) {
	if (a == b)
		return;

	if (mono_data[a].type == MonoData::Type::Var) {
		assert(a == find(a));

		VarId va = mono_data[a].data_id;

		if (occurs_in(va, b)) {
			assert(0 && "recursive unification\n");
		}

		var_data[va].equals = b;
	} else if (mono_data[b].type == MonoData::Type::Var) {
		return unify(b, a);
	} else {
		assert(mono_data[a].type == MonoData::Type::Term);
		assert(mono_data[b].type == MonoData::Type::Term);

		TermId ta = mono_data[a].data_id;
		TermId tb = mono_data[b].data_id;

		TermData& a_data = term_data[ta];
		TermData& b_data = term_data[tb];

		if (term_data[ta].type_function != term_data[tb].type_function) {
			assert(0 && "deduced two different polymorphic types to be equal");
		}

		TypeFunctionId type_function = term_data[ta].type_function;
		int argument_count = type_function_data[type_function].argument_count;

		if (a_data.arguments.size() != argument_count
		    || b_data.arguments.size() != argument_count) {
			assert(0 && "instanciating polymorphic type with wrong argument count");
		}

		for (int i { 0 }; i != argument_count; ++i) {
			unify(a_data.arguments[i], b_data.arguments[i]);
		}
	}
}

MonoId TypeSystemData::inst_impl(
    MonoId mono, std::unordered_map<VarId, MonoId> const& mapping) {
	MonoData const& data = mono_data[mono];

	if (data.type == MonoData::Type::Var) {
		auto it = mapping.find(data.data_id);
		return it == mapping.end() ? mono : it->second;
	}

	if (data.type == MonoData::Type::Term) {
		TermData const& t_data = term_data[data.data_id];
		std::vector<MonoId> new_args;
		for (MonoId argument : t_data.arguments)
			new_args.push_back(inst_impl(argument, mapping));
		return new_term(t_data.type_function, std::move(new_args));
	}

	assert(0 && "invalid term type");
}

MonoId TypeSystemData::inst_with(PolyId poly, std::vector<MonoId> const& vals) {
	PolyData const& data = poly_data[poly];

	assert(data.vars.size() == vals.size());

	std::unordered_map<VarId, MonoId> old_to_new;
	for (int i { 0 }; i != data.vars.size(); ++i) {
		old_to_new[data.vars[i]] = vals[i];
	}

	inst_impl(data.base, old_to_new);
}

MonoId TypeSystemData::inst_fresh(PolyId poly) {
	std::vector<MonoId> vals;
	for (int i { 0 }; i != poly_data[poly].vars.size(); ++i) {
		vals.push_back(new_var());
	}
	return inst_with(poly, vals);
}

