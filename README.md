<div align="center">
	<img width="256" src="img/JasperLogo.png" alt="Jasper Programming Language logo">

# Jasper

Jasper is a scripting language inspired by Haskell, Javascript, and modern C++. Jasper can be embedded in C++ applications.

</div>

The aim of Jasper is to be:

 - Nice to use
 - Easy to refactor
 - Consistent

For this purpose, Jasper has:

 - Type deduction
 - First class functions
 - Consistent and context-free syntax
 - Syntactic sugar
 - Many others

Here is an example piece of code:

```c++
or_default := fn(str, default) =>
	(if str == ""
		then default
		else str);

make_greeting := fn(prefix, suffix) =>
	fn(name) =>
		prefix + name + suffix;

say_hello := fn(name) {
	greeting := make_greeting("Hello, ", "!");
	return name
		|> or_default("Friend")
		|> greeting();
};

__invoke := fn() {
	return say_hello("World");
};
```

## Using the interpreter

First, you have to compile it using the following command:

```shell
make -C ./src interpreter
```

Once it is compiled, you can find the executable under `bin/jasperi`, and execute
it following the user guide, which can be found in multiple languages in the
`docs` directory.

> NOTES:
> Our Makefile uses non-standard features of gnu make
>
> Jasper is written in C++14, so you will need a C++14 compatible compiler, such
> as GCC 6.1 or later (a version as early as GCC 4.9 might also work, but we make
> no promises)

## Running the tests

We also have small test suite, which can be compiled with

```shell
make -C ./src test_program
```

and then run with

```shell
./bin/test_program
```
