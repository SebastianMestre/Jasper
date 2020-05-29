#include <vector>
#include <memory>

namespace TypeChecker {

/* TODO: these link whatever the type deduction system knows about a
 * type, and what it actually is.
 *
 * Really, most details about a type are not needed when doing type
 * deduction, so this should be kept separate from the representation
 * that's used in that task.
 *
 * The only thing we need to know about a type is its kind (i.e. the
 * amount of type arguments it takes)
 *
struct TypeDescriptor {
	int kind; // essentially the amount of type arguments
};
struct Sum : TypeDescriptor {};
struct Prod : TypeDescriptor {};
struct Builtin : TypeDescriptor {};
*/

struct Mono {};

struct Variable : Mono {
	int id;
};

struct Term : Mono {
	int base_id;
	std::vector<Mono*> args;
};

namespace Builtin {

	// Ultimately, all types are polytypes. It just so
	// happens that some of them take no arguments.
	// auto Int = Poly { ... };
	// auto Float = Poly { ... };
	// auto Array = Poly { ... };
	// auto Dictionary = Poly { ... };

} // namespace Builtin

// Example:
// here is how we express the type:
//
// forall a b c. array(a) -> array ( tuple(a,int) )
//
// in our framework
//
// Poly{
//   { 1 }
//   Function {
//     { Builtin::Array ( Variable { 1 } ) },
//     Builtin::Array ( tuple({ Variable { 1 }, Builtin::Int })
//   }
// }
//
// Note: Everything is actually tied together with pointers.
// The above is merely ilustrative.

// What follows is the definition of the Hindley Milner type system.
// It was originally designed for use in the typed lambda calculus,
// but it turns out to be useful in many real programming languages.
//
// The description given is not for the algorithm that can be used
// for deduction, but for the high-level deduction rules that govern
// the overall system.

struct App {
	AST* callee;
	std::vector<AST*> calls;
	int type_id;
}

// Notation:
// G is a set of typings (a.k.a. the context)
// s is a polytype
// t is a monotype
// e is an expression
// x is a variable

void hm_var (Var* v) {
	// if i am given a (poly)type hint, then that is the type

	// if
	//   x : s in G
	// then
	//   G |- x : s
}

void hm_app(App* c) {
	// App stands for application. This rule dictates how the callee
	// and arguments of a call affect the type of the result.
	//
	// Note how the type of e is not a poly type, but a monotype.
	//
	// The natural consecuence of this is that we can store
	// polymorphic functions, but not pass them, not without
	// monomorphising them first.
	//
	// Their type can still contain wildcards, but those must be bound
	// to the free variables of the callee.
	//
	// In short,
	//
	// forall a b. (a, (b)->b) -> a
	//
	// is not the same type as
	//
	// forall a. (a, forall b. (b)->b) -> a
	//
	// and, furthermore, the second one is not allowed.

	// if
	//   f : t -> t'
	//   e : t
	// then
	//   f(e) : t'

	// It is worth highlighting how this rule interacts with hm_inst.
	// Inst says that:
	//
	// if
	//   f : forall a. a -> t'
	// then
	//   f : t -> t'
	//
	// Thus, we can say that
	//
	// if
	//   f : forall a. a -> t'
	//   e : t
	// then
	//   f(e) : t'
}

void hm_abs(Abs* v) {
	// Abs stands for "lambda abstraction". This rule dictates how the
	// argument of a function relates to its return type.
	//
	// Esentially, the return type of the function is the type of the
	// value in the return statement. The rule also says that the
	// type of the argument can affect the return type, through the
	// application of this one or other rules.

	// if
	//   x:t implies e:t'
	// then
	//   fn(x) { return e; } : t -> t'
}

void hm_let(Let* l) {
	// Let stands for Let-In. This rules dictates how variable
	// declarations affect the type of expressions.

	// There is an interesting subtlety here. Note how the type of e0
	// is not a monotype, but a polytype. Even though function
	// arguments and return values must be monomophic, let-bindings
	// are polymorphic.
	//
	// This is not surprising, all it says is that we can give names
	// to polymorphic functions.

	// if
	//   e0 : s
	//   x  : s implies e1 : t
	// then
	//   (let x = e0 in e1) : t
}

void hm_inst() {
	// Inst stands for instance. This rule explains how generality
	// relationships affect typing judgements.
	//
	// ================ Side Note ================
	// In the hindley milner system, there is a partial order to
	// generality of types. It is really quite simple:
	//
	// if we can replace a wildcard in a type 'a', and get a different
	// type, 'b', then 'a' is equally or more general than 'b'.
	//
	// examples:
	// 'List(Int)' is less general than 'forall a. List(a)'
	//
	// 'forall a. a' is more general than 'forall a. List(a)'
	// ===========================================
	//
	// The rule says that a type can be replaced by another one of
	// lesser generality.
	//
	// Example:
	// If we have a type 'forall a. List(a)', any piece of code that
	// deals with `List(a)` should also work with `List(Int)`

	// if
	//   e : s'
	//   s' is more general than s
	// then
	//   e : s
}

void hm_gen() {
	// Gen stands for Generalize.
	//
	// I don't really understand this rule. In particular, I dont
	// understand what free(G) stands for. It is explained here:
	//
	// https://en.wikipedia.org/wiki/Hindley%E2%80%93Milner_type_system#Free_type_variables
	//
	// But The symbolic definition is quite unclear.
	// It is defined on a bunch of different things.
	//
	// names:              free(a) = {a}
	// polytypes:          free(forall a. s) = free(s) - {a}
	// polytype instances: free(C t1 t2 ... tn) = union of free(ti) for i=0 to n
	// contexts:           free(G) = union of free(s) foreach typing (x:s) in G
	// judgements:         free(G |- e : s) = free(s) - free(G)
	//
	// Let's go through it:
	// the type '(a,b,c)' has free variables {a,b,c}
	// the type 'forall a. (a,b,c)' has free variables {b,c}, since a
	// is bound to the 'forall a.' qualifier.
	// However, we should probably have implicit forall whenever a
	// type is explicitly written, so '(a,b,c)' should implicitly
	// expand to 'forall a b c. (a,b,c)'.
	// This makes things nicer to use.

	// if
	//   e : s
	//   a not in free(G)
	// then
	//   e : forall a. s
}

} // namespace TypeChecker


namespace HM {

struct TypeDescriptor {
	int argcount;
};

struct Type {
	virtual bool is_var () const { return false; }
};
struct Term : Type {
	int name;
	std::vector<Type*> args;
};
struct Var : Type {
	Type* instance { nullptr };
	bool is_var() const override { return true; }
};


const int function_name = 0;
const int array_name    = 1;

Var* new_var() {
	return new Var;
}
Term new_array_type (Type* stores) {
	auto result = new Term();
	result->name = array_name;
	result->args.push_back(stores);
	return result;
}
Term new_function_type(std::vector<Type*> arg_types, Type* return_type) {
	// We treat functions as any other polymorphic type (aka Term)
	// There is something really elegant about not having special
	// cases for things that are so seemingly fundamental
	auto result = new Term();
	result->name = function_name;
	for(auto* arg_type : arg_types)
		result->args.push_back(arg_type);
	result->args.push_back(return_type);
	return result;
}


// Basically join on disjoint set
// TODO: maybe decouple the data structure from the logic?
void unify(Type* a, Type* b) {
	if (a->is_var()) {
		if(a != b){
			auto va = static_cast<Var*>(a);
			assert(!occurs_in(va, b));
			va->instance = b; // repr[a] = b
		}
	} else if (b->is_var()) {
		return unify(b, a);
	} else {
		auto ta = static_cast<Term*>(a);
		auto tb = static_cast<Term*>(b);

		assert(ta->name == tb->name);
		assert(ta->args.size() == tb->args.size());

		for (int i { 0 }; i != ta->args.size(); ++i)
			unify(ta->args[i], tb->args[i]);
	}
}

// Basically find on disjoint set
// TODO: maybe decouple the data structure from the logic?
Type* prune(Type* t) {
	assert(t);

	if (!t->is_var())
		return t;

	auto vt = static_cast<Var*>(t);

	if (!vt->instance)
		return vt;

	return vt->instance = prune(vt->instance); // repr[a] = find(a);
}

// v must be its own representative.
// (i.e. you have to give a prunned Variable)
bool occurs_in (Var* v, Type* t) {
	auto tt = prune(t);

	if(v == tt) return true;
	if(tt->is_var()) return false;
	
	auto ttt = static_cast<Term*>(tt);
	for(Type* c : ttt->args)
		if(occurs_in(v, c))
			return true;

	return false;
}



struct AST {
	virtual Type* deduce (Env& env, set<string>& ng) = 0;
};

std::vector<Type*> deduce_all(std::vector<AST*> vals, Env& env, set<string>& ng) {
	std::vector<Type*> result;
	for(auto ast : vals)
		result.push_back(ast->deduce(env, ng));
	return result;
}

struct Ident : AST {
	string s;
	Type* type;
	Decl* decl;

	Type* deduce (Env& env, set<string>& ng) override {
		// since we have access to our own declaration, we don't need
		// to do a name lookup
		return decl->type(env,ng);
	}
};

struct Call : AST {
	Type* deduce (Env& env, set<string>& ng) override {
		auto callee_type = callee->deduce(env, ng);
		auto arg_types = deduce_all(args, env, ng);
		auto result_type = new_var();
		// We assume all callables are functions. This can be
		// troublesome when dealing with callable objects.
		// Maybe we can desugar callable object calls to regular
		// function calls?
		unify(new_function_type(arg_types, result_type), callee_type);
		return result_type;
	}
};

//TODO: Env y get_type

}
