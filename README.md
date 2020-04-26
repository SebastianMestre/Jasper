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
f := {
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
