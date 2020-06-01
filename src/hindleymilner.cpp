// Este archivo tiene explicaciones e ideas. El codigo que haya es
// simplemente para darme una idea de como deberian funcionar las
// cosas, y no es parte del PR.
//
// Este archivo va a ser eliminado eventualmente

// Notation:
// G is a set of typings (a.k.a. the context)
// s is a polytype
// t is a monotype
// e is an expression
// x is a variable

void hm_app(App* c) {
	// App stands for application. This rule dictates how the callee
	// and arguments of a call affect the type of the result.
	//
	// Note how the type of e is not a poly type, but a monotype. The
	// consecuence of this is that we cannot pass polymorphic
	// functions without instanciating them first.
	//
	// We do this by instanciating them with fresh variables, which
	// lets them be unified on usage.

	// if
	//   f : t -> t'
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

	// Note how the type of e0 is a polytype. This means that we can
	// give names to polymorphic functions.

	// if
	//   e0 : s
	//   x  : s implies e1 : t
	// then
	//   (let x = e0 in e1) : t
}

