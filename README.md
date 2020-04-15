# parse stuff

Experiments on figuring out how to write a parser that gives
somewhat useful error messages. Also, it should parse stuff.

Here is an example of a sentence we want to be able to parse:

```c++
greeting := "Hello, ";
f := fn (name) {
	return greeting + name;
};

__invoke := fn () {
	greeting = "Hello, ";
	print(f("Programmer"));
};
```

Here is another one, it is almost equivalent, but now we can
pass `obj` around:

```c++
obj := {
	greeting := "Hello, ";
	f := fn (name) {
		return greeting + name;
	};
};

__invoke := fn () {
	obj.greeting = "Hey, ";
	print(obj.f("Programmer"));
};
```
