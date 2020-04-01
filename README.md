# parse stuff

Experiments on figuring out how to write a parser that gives
somewhat useful error messages.

Here is an example of a sentence we want to be able to parse:

```c++
my_func := () {
	x : dec = 3.1;
	y := 40;

	z := x + dec(y);
}
```
