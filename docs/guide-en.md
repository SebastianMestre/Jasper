# Jasper

The main objectives of the language are:

 - Nice to use
 - Easy to refactor

We aim to be able to get those through various features:

 - Consistent syntax (WIP)
 - Everything is a value (WIP)
 - Value semantics (TODO)
 - Parametric polymorphism (WIP)
 - Type deduction (WIP)

Furthermore, we have put some things because they appeared pretty to us.

 - Pipeline operator `|>`
 - inyeccion de scopes (TODO) (?)
 - Float type (TODO)
 - Callable objects (TODO)
 - Function closures (WIP)

## Introdution - The structure of a program

In JS++, a program is a list of declarations.

> A delaration associates a value with a name.e.
>
> Syntaxically, it is an identifier, following a type specifier
> (optional) and an initial value. Examples: `a := 15; b : int = 10;`

Among those declarations, one must be the entry point, a function calledmada
`__invoke`.

> In JS++, functions are defined with the keyword `fn`, followed by a list of
> arguments and the body of the function

```rust
x : int = 10;

__invoke : fn() {
	return x;
};
```

This program returns the inputed number, `10`.

## Syntaxic sugar
Since we acknowledge the importance of the syntax, JS++ constains plenty of syntaxic sugar.

### Short functions

While using short functions, the following declarations are both completely equivalent:

```rust
f := fn (x) => x + 1;

f := fn (x) {
	return x + 1;
};
```

### Pipeline operatior or pizza operator

The pizza operator allow us to write pipelines in our code. This is very useful
for two things:
 - Extending the functionality of an object without adding dependencies
 - Write the code in a functional style

The pizza operator converts an expression like `x |> f(y...)` into `f(x,y,...)`

Here is an example of such use, along with its sugar-free version:

```rust
squared_primes := fn (arr) => arr
	|> filter(is_prime)
	|> map(fn (x) => x * x);

squared_primes := fn (arr) {
	return map(filter(arr, is_prime), fn (x) => x * x);
};
```
