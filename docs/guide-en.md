# Jasper

The main objectives of the language are to be:

 - Nice to use
 - Easy to refactor
 - Consistent

We aim to be able to get those through various features:

 - Consistent syntax
 - First class functions
 - Closures
 - Parametric polymorphism
 - Type deduction
 - Everything is a value (WIP)
 - Value semantics (TODO)

Furthermore, we have put some things because they appeared pretty to us.

 - Pipeline operator `|>`
 - Scope injection (TODO)
 - Decimal number data type (TODO)
 - Callable objects (TODO)

## Introduction - The structure of a program

In Jasper, a program is a list of declarations.

> A declaration associates a value with a name.
>
> Syntactically, it is an identifier, followed by a type specifier
> (optional) and an initial value. Examples: `a := 15; b : int = 10;`

Among those declarations, one must be the entry point, a function named
`__invoke`.

> In Jasper, functions are defined with the keyword `fn`, followed by a list of
> arguments and the body of the function

```rust
x : int = 10;

__invoke : fn() {
	return x;
};
```

This program returns the integer `10`.

## Syntactic sugar
Since we acknowledge the importance of the syntax, Jasper contains plenty of syntactic sugar.

### Short functions

While using short function syntax, the following declarations are
completely equivalent:

```rust
f := fn (x) => x + 1;

f := fn (x) {
	return x + 1;
};
```

### Pipeline operator

The pipeline operator allows us to write pipelines in our code.
It is very useful for two things:
 - Extending the functionality of an object without adding dependencies
 - Write functional code in imperative style

The pizza operator converts an expression like `x |> f(y,z)` into `f(x,y,z)`

Here is an example of their use, along with its desugared version:

```rust
// original
squared_primes := fn (arr) => arr
	|> filter(is_prime)
	|> map(fn (x) => x * x);

// desugared
squared_primes := fn (arr) {
	return map(
		filter(arr, is_prime),
		fn (x) {
			return x * x;
		});
};
```
