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

 - Sum types
 - Record types
 - Type deduction
 - First class functions with closure
 - Consistent and context-free syntax
 - Syntactic sugar
 - Many others

Here is an example piece of "functional-style" code:

```c++
fn greeting(name) =>
	"Hello, "
	+ (if (name == "")
		then "friend"
		else name)
	+ "!";
```

It can also be written like this, in a more imperative style, and using
some intermediate variables:

```c++
fn greeting(name) {
	if (name == "")
		name = "friend";
	prefix := "Hello, ";
	suffix := "!";
	return prefix + name + suffix;
};
```

or like this, mixing functional, and imperative style:

```c++
fn greeting(name) =>
	seq {
		if (name == "")
			return "friend";
		return name;
	}
	|> (fn(name) => "Hello, " + name + "!")();
```
## Using the interpreter

First, you have to compile it using the following command:

```shell
make interpreter
```

Once it is compiled, you can find the executable under `bin/jasperi`, and execute
it following the user guide, which can be found in multiple languages in the
`docs` directory.

> NOTES:
> Our Makefile uses non-standard features of gnu make
>
> Jasper is written in C++14, so you will need a C++14 compatible compiler, such
> as GCC 6.1 or later

## Running the tests

We also have small test suite, which can be compiled with

```shell
make tests
```

and then run with

```shell
./bin/run_tests
```
