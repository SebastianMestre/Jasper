# JS++

We are trying to make nice programming language.

 - easy to refactor
 - consistent
 - nice to use

Here is an example piece of code:

```c++
f := fn (name) {
	greeting := "Hello, ";
	return greeting + name;
};

__invoke := fn () {
	print(f("Programmer"));
};
```

And here is a possible refactor: Wrap a funcion in an object to add some configuration.

```c++
f := obt {
	greeting := "Hello, ";
	__invoke := fn (name) {
		return greeting + name;
	};
};

__invoke := fn () {
	f.greeting = "Hey, ";
	print(f("Programmer"));
};
```

## Compiling the source

The source files have to be compiled for the program
to work. So, for a standard compilation proccess
you'll have to execute the following statements.

```
cd src
make
```

If anything goes wrong you should execute 

```
make clean
```

To avoid further errors and try compiling again.

Once the program is compiled you can find it under the
_bin_ folder and execute it following the user guide.