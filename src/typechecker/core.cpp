#include "core.hpp"

#include <cassert>
#include <iostream>

#include "../log/log.hpp"

TypeSystemCore::TypeSystemCore() {
}

Type TypeSystemCore::new_term(TypeFunc tf, std::vector<Type> args) {
	return ll_new_term(tf, std::move(args));
}

PolyId TypeSystemCore::forall(std::vector<VarId> vars, Type ty) {
	PolyId poly = poly_data.size();
	poly_data.push_back({ty, std::move(vars)});
	return poly;
}

Type TypeSystemCore::inst_impl(
    Type mono, std::unordered_map<VarId, Type> const& mapping) {

	mono = ll_find(mono);

	if (ll_is_var(mono)) {
		auto it = mapping.find(get_var_id(mono));
		return it == mapping.end() ? mono : it->second;
	} else {
		NodeHeader header = data(mono);
		TermId term = header.data_idx;
		std::vector<Type> new_args;
		for (Type arg : ll_term_data[term].argument_idx)
			new_args.push_back(inst_impl(arg, mapping));
		return new_term(ll_term_data[term].function_id, std::move(new_args));
	}
}

Type TypeSystemCore::inst_with(PolyId poly, std::vector<Type> const& vals) {
	PolyData const& data = poly_data[poly];

	assert(data.vars.size() == vals.size());

	std::unordered_map<VarId, Type> old_to_new;
	for (int i {0}; i != data.vars.size(); ++i) {
		old_to_new[data.vars[i]] = vals[i];
	}

	return inst_impl(data.base, old_to_new);
}

Type TypeSystemCore::inst_fresh(PolyId poly) {
	std::vector<Type> vals;
	for (int i {0}; i != poly_data[poly].vars.size(); ++i)
		vals.push_back(ll_new_var());
	return inst_with(poly, vals);
}

std::unordered_set<VarId> TypeSystemCore::free_vars(Type mono) {
	std::unordered_set<VarId> result;
	gather_free_vars(mono, result);
	return result;
}

void TypeSystemCore::gather_free_vars(Type mono, std::unordered_set<VarId>& free_vars) {
	mono = ll_find(mono);

	if (ll_is_var(mono)) {
		free_vars.insert(get_var_id(mono));
	} else {
		const NodeHeader& header = data(mono);
		TermId term = header.data_idx;
		for (Type arg : ll_term_data[term].argument_idx)
			gather_free_vars(arg, free_vars);
	}
}



TypeFunc TypeSystemCore::new_builtin_type_function(int arity) {
	return create_type_function(TypeFunctionTag::Builtin, arity, {}, {});
}

TypeFunc TypeSystemCore::new_type_function(
    TypeFunctionTag type,
    std::vector<InternedString> fields,
    std::unordered_map<InternedString, Type> structure) {
	return create_type_function(type, 0, std::move(fields), std::move(structure));
}

TypeFunc TypeSystemCore::create_type_function(
    TypeFunctionTag tag,
    int arity,
    std::vector<InternedString> fields,
    std::unordered_map<InternedString, Type> structure) {
	auto result = TypeFunc(m_type_functions.size());
	m_type_functions.push_back(
	    {tag, arity, std::move(fields), std::move(structure)});
	return result;
}

std::vector<InternedString> const& TypeSystemCore::fields(TypeFunc tf) {
	assert(is_record(tf));
	return data(tf).fields;
}

Type TypeSystemCore::type_of_field(TypeFunc tf, InternedString name) {
	assert(is_record(tf) || is_variant(tf));
	auto it = data(tf).structure.find(name);
	assert(it != data(tf).structure.end());
	return it->second;
}

bool TypeSystemCore::is_record(TypeFunc tf) {
	return data(tf).tag == TypeFunctionTag::Record;
}

bool TypeSystemCore::is_variant(TypeFunc tf) {
	return data(tf).tag == TypeFunctionTag::Variant;
}

TypeFunc TypeSystemCore::type_function_of(Type mono) {
	mono = ll_find(mono);
	assert(ll_is_term(mono) && "tried to find function of non term");
	int t = data(mono).data_idx;
	return ll_term_data[t].function_id;
}

static InternedString print_a_thing(TypeFunc tf) {
	auto x = int(tf);
	if (x == 0) return "function";
	if (x == 1) return "int";
	if (x == 2) return "float";
	if (x == 3) return "string";
	if (x == 4) return "array";
	if (x == 5) return "boolean";
	if (x == 6) return "unit";
	return "a user defined type";
}

void TypeSystemCore::unify_type_function(TypeFunc i, TypeFunc j) {
	if (i == j)
		return;

	Log::fatal() << "unified " << print_a_thing(i) << " with " << print_a_thing(j);
}

bool TypeSystemCore::occurs(VarId v, Type i) {
	i = ll_find(i);

	if (ll_is_var(i))
		return equals_var(i, v);

	int ti = data(i).data_idx;
	for (Type c : ll_term_data[ti].argument_idx)
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
	auto& i_constraints = m_constraints[static_cast<int>(vi)];
	auto& j_constraints = m_constraints[static_cast<int>(vj)];
	for (auto const& kv : i_constraints.structure) {
		auto it = j_constraints.structure.find(kv.first);
		if (it == j_constraints.structure.end()) {
			j_constraints.structure.insert(kv);
		} else {
			ll_unify(kv.second, it->second);
		}
	}
	return;
}

bool TypeSystemCore::satisfies(Type t, Constraint const& c) {
	return true;
}

void TypeSystemCore::ll_unify(Type i, Type j) {
	i = ll_find(i);
	j = ll_find(j);

	if (i == j) return;

	if (ll_is_var(j))
		std::swap(i, j);

	if (ll_is_var(i)) {

		auto vi = get_var_id(i);

		if (ll_is_term(j)) {
			assert(!occurs(vi, j));
			assert(satisfies(j, m_constraints[static_cast<int>(vi)]));
			establish_substitution(vi, j);
		} else {
			unify_vars_left_to_right(vi, get_var_id(j));
		}

	} else {
		int vi = data(i).data_idx;
		int vj = data(j).data_idx;

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

Type TypeSystemCore::ll_new_var() {
	int var_id = m_var_counter++;
	m_type_var_uf.new_node();
	m_substitution.push_back(Type(-1));
	m_constraints.push_back({});
	int type_id = m_type_counter++;
	ll_node_header.push_back({Tag::Var, var_id});
	return Type(type_id);
}

Type TypeSystemCore::ll_new_term(TypeFunc f, std::vector<Type> args) {
	int type_id = m_type_counter++;
	assert(ll_node_header.size() == type_id);
	ll_node_header.push_back({Tag::Term, static_cast<int>(ll_term_data.size())});
	ll_term_data.push_back({f, std::move(args)});
	return Type(type_id);
}

Type TypeSystemCore::ll_find(Type i) {
	if (!ll_is_var(i)) return i;
	VarId vi = get_var_id(i);
	if (m_substitution[static_cast<int>(vi)] == Type(-1)) return i;
	return m_substitution[static_cast<int>(vi)];
}

VarId TypeSystemCore::get_var_id(Type i) {
	assert(ll_is_var(i));
	return static_cast<VarId>(m_type_var_uf.find(data(i).data_idx));
}

void TypeSystemCore::establish_substitution(VarId var_id, Type type_id) {
	assert(m_substitution[static_cast<int>(var_id)] == Type(-1));
	m_substitution[static_cast<int>(var_id)] = type_id;
}

bool TypeSystemCore::ll_is_term(Type i) {
	return data(i).tag == Tag::Term;
}

bool TypeSystemCore::ll_is_var(Type i) {
	return data(i).tag == Tag::Var;
}

TypeSystemCore::NodeHeader& TypeSystemCore::data(Type ty) {
	return ll_node_header[int(ty)];
}

TypeSystemCore::TypeFunctionData& TypeSystemCore::data(TypeFunc tf) {
	return m_type_functions[int(tf)];
}

bool TypeSystemCore::equals_var(Type t, VarId v) {
	return ll_is_var(t) && get_var_id(t) == v;
}


void TypeSystemCore::add_record_constraint(VarId v) {
	int i = static_cast<int>(v);
	if (m_constraints[i].shape == Constraint::Shape::Unknown) {
		m_constraints[i].shape = Constraint::Shape::Record;
	} else if (m_constraints[i].shape != Constraint::Shape::Record) {
		Log::fatal() << "object used both as record and variant";
	}
}

void TypeSystemCore::add_variant_constraint(VarId v) {
	int i = static_cast<int>(v);
	if (m_constraints[i].shape == Constraint::Shape::Unknown) {
		m_constraints[i].shape = Constraint::Shape::Variant;
	} else if (m_constraints[i].shape != Constraint::Shape::Variant) {
		Log::fatal() << "object used both as record and variant";
	}
}

void TypeSystemCore::add_field_constraint(VarId v, InternedString name, Type ty) {
	int i = static_cast<int>(v);
	if (m_constraints[i].structure.count(name)) {
		ll_unify(m_constraints[i].structure[name], ty);
	} else {
		m_constraints[i].structure[name] = ty;
	}
}
