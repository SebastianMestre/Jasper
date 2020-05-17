# JS++

El lenguaje no tiene nombre. Le decimos JS++ provisionalmente.

Los objetivos principales son:

 - Buena usabilidad
 - Facilidad al refactorizar

Eso pensamos obtenerlo mediante varias features:

 - Sintaxis consistente (WIP)
 - Todo es un valor (WIP)
 - Semantica de valor (TODO)
 - Polimorfismo parametrico (TODO)
 - Deduccion de tipos (TODO)

Aparte, hay cositas que metimos porque nos parecen lindas.

 - Operador pipeline `|>` (TODO)
 - inyeccion de scopes (TODO)
 - Tipo de datos decimal (TODO)
 - Objetos llamables (TODO)
 - Funciones con clausuras (WIP)

## Introduccion - La estructura de un programa

En JS++, un programa es una lista de declaraciones.

> Una declaracion es un identificador, seguido de un especificador de tipo
> opcional y un valor inicial. ejemplos: `a := 15; b : int = 10;`

Una de estas declaraciones debe ser el punto de entrada, una funcion llamada
`__invoke`.

> En JS++ definis funciones con la palabra clave `fn`, seguida de una lista de
> argumentos y el cuerpo de la funcion.

```rust
x : int = 10;

__invoke : fn() {
	return x;
};
```

Este programa devuelve un numero entero, `10`.

## Azucar sintactico

Como reconocemos que la sintaxis importa, JS++ tiene bastante azucar sintactico.

### Funciones cortas

Usando funciones cortas, las siguientes dos declaraciones son completamente
equivalentes:

```rust
f := fn (x) => x + 1;

f := fn (x) {
	return x + 1;
};
```

### Operador pipeline u Operador pizza

El operador pizza nos permite escribir pipelines en nuestro codigo. Es muy util
para dos cosas:
 - Extender la funcionalidad de un objeto sin agregarle dependencias
 - Escribir codigo en estilo funcional

Aca hay un ejemplo de uso junto a su version des-azucarada:

```rust
primos_al_cuadrado := fn (arr) => arr
	|> filter(es_primo)
	|> map(fn (x) => x * x);

primos_al_cuadrado := fn (arr) {
	return map(filter(arr, es_primo), fn (x) => x * x);
};
```
