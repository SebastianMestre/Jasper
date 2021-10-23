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

			int const new_argument_count = [&] {
				if (a_data.argument_count == b_data.argument_count) {
					return a_data.argument_count;
				} else if (b_data.is_dummy || b_data.argument_count == -1) {
					return -1;
				} else {
					std::string argc_a =
					    a_data.argument_count == -1
					        ? std::string("variadic")
					        : std::to_string(a_data.argument_count);
					std::string argc_b =
					    b_data.argument_count == -1
					        ? std::string("variadic")
					        : std::to_string(b_data.argument_count);
					Log::fatal()
					    << "Deduced type functions with incompatible argument "
					       "counts to be equal (with "
					    << argc_a << " and " << argc_b << " arguments)";
				}
			}();

			// Make a point to b. this way, if more fields get added to a or b,
			// both get updated
			//
			// Also, we do it before unifying their data to prevent infinite
			// recursion
			core.node_header[a].tag = Unification::Core::Tag::Var;
			core.node_header[a].data_idx = b;
			b_data.argument_count = new_argument_count;

			for (auto& kv_a : a_data.result_data.structure) {
				auto kv_b = b_data.result_data.structure.find(kv_a.first);

				if (kv_b == b_data.result_data.structure.end())
					// if b doesn't have a field of a, act accordingly
					if (b_data.is_dummy)
						b_data.result_data.structure.insert(kv_a);
					else
						Log::fatal() << "Accessing non-existing field '" << kv_a.first << "' of a record";
				else
					// else the fields must have equivalent types
					unify(kv_a.second, kv_b->second);
			}

		} else {
			Log::fatal()
			    << "Deduced two different type functions to be equal (with IDs "
			    << fa << " and " << fb << ")";
		}
	};
}

MonoId TypeSystemCore::new_constrained_term(
    TypeFunctionTag type, std::unordered_map<InternedString, MonoId> structure) {
	int id = m_monos_uf.new_var();
	m_monos.push_back(
	    {MonoFr::Tag::Constr, -1, {}, {type, {}, std::move(structure)}});
	return id;
}

MonoId TypeSystemCore::new_term(TypeFunctionId tf, std::vector<int> args) {
	tf = m_tf_core.find(tf);

	{
		// we create a dummy typefunc to achieve unification of typefunc argument counts

		// TODO: add a TypeFunctionTag::Unknown tag, to express
		// that it's a dummy of unknown characteristics

		// TODO: add some APIs to make this less jarring
		TypeFunctionId dummy_tf =
		    new_type_function(TypeFunctionTag::Builtin, {}, {}, true);

		int dummy_tf_data_id = m_tf_core.find_function(dummy_tf);
		m_type_functions[dummy_tf_data_id].argument_count = args.size();

		m_tf_core.unify(tf, dummy_tf);
	}

	int id = m_monos_uf.new_var();
	assert(id == m_monos.size());
	m_monos.push_back(MonoFr{MonoFr::Tag::App, tf, args, {}});

	return id;
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
	m_type_functions.push_back({{TypeFunctionTag::Builtin}, arguments});
	return id;
}

TypeFunctionId TypeSystemCore::new_type_function(
    TypeFunctionTag type,
    std::vector<InternedString> fields,
    std::unordered_map<InternedString, MonoId> structure,
    bool dummy) {
	TypeFunctionId id = m_tf_core.new_term(m_type_functions.size());
	m_type_functions.push_back(
	    {{type, std::move(fields), std::move(structure)}, 0, dummy});
	return id;
}

MonoId TypeSystemCore::inst_impl(
    MonoId mono, std::unordered_map<MonoId, MonoId> const& mapping) {

	// NOTE(Mestre): Is just calling find good enough? It means we
	// should only ever qualify variables that are their own
	// representative, which does seem to make sense. I think.
	mono = m_monos_uf.find(mono);

	if (m_monos[mono].tag == MonoFr::Tag::Constr) {
		auto it = mapping.find(mono);
		return it == mapping.end() ? mono : it->second;
	} else {
		std::vector<MonoId> new_args;
		for (MonoId arg : m_monos[mono].args)
			new_args.push_back(inst_impl(arg, mapping));
		return new_term(m_monos[mono].func_id, std::move(new_args));
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
		vals.push_back(new_var());
	return inst_with(poly, vals);
}

void TypeSystemCore::gather_free_vars(MonoId mono, std::unordered_set<MonoId>& free_vars) {
	mono = m_monos_uf.find(mono);
	if (m_monos[mono].tag == MonoFr::Tag::Constr) {
		free_vars.insert(mono);
	} else {
		for (MonoId arg : m_monos[mono].args)
			gather_free_vars(arg, free_vars);
	}
}

void TypeSystemCore::unify(MonoId lhs, MonoId rhs) {
	lhs = m_monos_uf.find(lhs);
	rhs = m_monos_uf.find(rhs);

	// si rhs es var, lo pongo a la izq
	if (m_monos[rhs].tag == MonoFr::Tag::Constr)
		std::swap(lhs, rhs);

	combine_left_to_right(lhs, rhs);

	if (m_monos[lhs].tag == MonoFr::Tag::Constr) {
		if (m_monos[rhs].tag == MonoFr::Tag::App) {
			if (occurs(lhs, rhs))
				Log::fatal() << "Tried to construct infinite type";
			// si lhs es var, lo apunto a rhs
			m_monos_uf.join_left_to_right(lhs, rhs);
		}
	} else {
		// recursion del unify tipico

		m_tf_core.unify(m_monos[lhs].func_id, m_monos[rhs].func_id);

		assert(m_monos[lhs].args.size() == m_monos[rhs].args.size());
		for (int i = 0; i < m_monos[lhs].args.size(); ++i) {
			unify(m_monos[lhs].args[i], m_monos[rhs].args[i]);
		}
	}
}

bool TypeSystemCore::occurs(int i, int j) {
	return false;
}

int TypeSystemCore::find_function(MonoId x) {
	x = m_monos_uf.find(x);
	if (m_monos[x].tag != MonoFr::Tag::App)
		Log::fatal() << "Tried to read typefunc of non-app type";
	return m_monos[x].func_id;
}

void TypeSystemCore::combine_left_to_right(MonoId lhs, MonoId rhs) {
}
