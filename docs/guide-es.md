# Jasper

Los objetivos principales del lenguaje son:

 - Buena usabilidad
 - Facilidad al refactorizar

Eso pensamos obtenerlo mediante varias features:

 - Sintaxis consistente
 - Funciones de primera clase
 - Clausuras
 - Polimorfismo parametrico
 - Deduccion de tipos
 - Todo es un valor (WIP)
 - Semantica de valor (TODO)

Aparte, hay cositas que metimos porque nos parecen lindas.

 - Operador pipeline `|>`
 - inyeccion de scopes (TODO)
 - Tipo de datos de numero decimal (TODO)
 - Objetos llamables (TODO)

## Introduccion - La estructura de un programa

En Jasper, un programa es una lista de declaraciones.

> Una declaracion asocia un valor con un nombre.
>
> Sintacticamente, es un identificador, seguido de un especificador de tipo
> (opcional) y un valor inicial. ejemplos: `a := 15; b : int = 10;`

Una de estas declaraciones debe ser el punto de entrada, una funcion llamada
`__invoke`.

> En Jasper definis funciones con la palabra clave `fn`, seguida de una lista de
> argumentos y el cuerpo de la funcion.

```rust
x : int = 10;

__invoke : fn() {
	return x;
};
```

Este programa devuelve un numero entero, `10`.

## Azucar sintactico

Como reconocemos que la sintaxis importa, Jasper tiene bastante azucar sintactico.

### Funciones cortas

Usando funciones cortas, las siguientes dos declaraciones son completamente
equivalentes:

```rust
f := fn (x) => x + 1;

f := fn (x) {
	return x + 1;
};
```

### Operador pipeline

El operador pipeline nos permite escribir pipelines en nuestro codigo.
es muy util para dos cosas:
 - Extender la funcionalidad de un objeto sin agregarle dependencias
 - Escribir codigo functional en estilo imperativo

El operador pizza transforma una expresion de la forma `x |> f(y...)` a una de
la forma `f(x,y...)`.

Aca hay un ejemplo de uso junto a su version des-azucarada:

```rust
// original
primos_al_cuadrado := fn (arr) => arr
	|> filter(es_primo)
	|> map(fn (x) => x * x);

// desugared
primos_al_cuadrado := fn (arr) {
	return map(
		filter(arr, es_primo),
		fn (x) {
			return x * x;
		});
};
```
