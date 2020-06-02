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

void hm_rec(Rec* l) {
	// rec stands for Recursion.
	
	// The rec syntax says that we can define self-referential names.
	//
	// Example:
	//
	// rec
	//   g = (\x. if x == 0 then 0 else 1 + g(x - 1))
	// in
	//   g(5)
	//
	// The type of g will be available as a monotype inside its own
	// definition, And it will only be generalized to a polytype
	// within the 'in block'
	//
	// This allows unification to ocurr between variables on the type
	// of g, and variables that show up in the body of g.
	//
	// Furthermore, rec allows multiple definitions to exist within
	// the same block, allowing for mutual recursion.
	//
	// rec
	//   g = (\x. if x == 0 then 0 else 1 + f(x - 1))
	//   f = (\x. if x == 0 then 0 else 2 + g(x - 1))
	// in
	//   g(5)
	//
	// Since we have a single type of name-binding in our language
	// (:=), we have to detect when each rule should be applied.
	//
	// In particular, we have to find which functions have mutual
	// reference, and group them together in a 'rec-and' block.
	//
	// f := fn(x){
	//   if(x == 0) return 0;
	//   return g(x-1)+1;
	// };
	//
	// g := fn(x){
	//   if(x == 0) return 0;
	//   return f(x-1)+2;
	// };
	//
	// h := fn(x){
	//   return g(x) + f(x);
	// }
	//
	// translates to:
	//
	// rec
	//   f = ...
	//   g = ...
	// in
	//   let
	//     h = ...
	//   in
	//     ...
	//
	// Doing this means finding the dependency graph between these
	// definitions and breaking it up into strongly-connected
	// components. Those components then become our rec blocks.
}

