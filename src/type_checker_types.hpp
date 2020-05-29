#include <vector>

namespace TypeChecker {

struct Type {
	virtual bool is_var() const { return false; }
};

struct Term : Type {
	// TODO: replace name with a TypeDescriptor* or somth
	// We eventually need to know what the type actually is.
	int name;
	std::vector<Type*> args;
};

struct Var : Type {
	Type* instance { nullptr };
	bool is_var() const override { return true; }
};

Type* prune(Type* t);

// v must be prunned before passing
bool occurs_in(Var* v, Type* t);

// a must be prunned before passing
void unify(Type* a, Type* b);

}
