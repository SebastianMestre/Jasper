#include <vector>
#include <unordered_map>

using TypeFunctionId = int;
using VarId = int;
using TermId = int;
using MonoId = int;
using PolyId = int;

struct TypeFunctionData {
	// -1 means variadic.
	int argument_count;
};

enum class mono_type { Var, Term };
struct MonoData {
	mono_type type;
	int data_id;
};

struct VarData {
	MonoId equals;
};

struct TermData {
	TypeFunctionId type_function;
	std::vector<MonoId> arguments;
};

struct PolyData {
	MonoId base;
	std::vector<VarId> vars;
};

struct TypeSystemCore {
	std::vector<MonoData> mono_data;
	std::vector<VarData> var_data;
	std::vector<TermData> term_data;

	std::vector<TypeFunctionData> type_function_data;
	std::vector<PolyData> poly_data;

	// TODO: move to a separate class
	// std::vector<std::unordered_map<std::string, >> scopes;

	MonoId new_var();
	MonoId new_term(TypeFunctionId type_function, std::vector<int> args);

	// qualifies all unbound variables in the given monotype
	// PolyId new_poly (MonoId mono, Env&) { } // TODO

	MonoId find(MonoId mono);
	bool occurs_in(VarId var, MonoId mono);
	void unify(MonoId a, MonoId b);

	MonoId inst_impl(MonoId mono, std::unordered_map<VarId, MonoId> const& mapping);
	MonoId inst_with(PolyId poly, std::vector<MonoId> const& vals);
	MonoId inst_fresh(PolyId poly);
};
