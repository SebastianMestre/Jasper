// Este archivo tiene explicaciones e ideas. El codigo que haya es
// simplemente para darme una idea de como deberian funcionar las
// cosas, y no es parte del PR.
//
// Este archivo va a ser eliminado eventualmente

// What follows is the definition of the Hindley Milner type system.
// It was originally designed for use in the typed lambda calculus,
// but it turns out to be useful in many real programming languages.
//
// The description given is not for the algorithm that can be used
// for deduction, but for the high-level deduction rules that govern
// the overall system.
//
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

