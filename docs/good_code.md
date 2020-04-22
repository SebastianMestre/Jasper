# Como escribir buenos programas

## Evitar la abstraccion prematura

Por lo general, menos codigo es mejor. Menos que mantener, menos que
leer, menos que escribir, menos que ejecutar. Con este principio en
mano, cuando encaramos un problema, tratamos de abstraerlo para
reducir la cantidad de codigo.

Sin embargo, es comun embarcarse en esa mision antes de entender cual
es el patron que estamos tratando de abstraer. Como no entendemos el
patron, solemos abstraer el patron incorrecto. Esto lleva a que el
codigo que escribamos en el futuro tenga que hacer malabares para
esquivar las decisiones erradas que tomamos originalmente.

En cambio, es mas saludable esperar a tener mas de un ejemplo de
uso de un patron en particular antes de intentar abstraerlo o
encapsularlo. Martin Fowler, Casey Muratori, y muchos otros recomiendan
esta practica. En particular, Martin Fowler tiene su [Regla de tres](https://en.wikipedia.org/wiki/Rule_of_three_(computer_programming))

## Evitar la abstraccion excesiva

Se suele decir que el objetivo del codigo que escribimos es comunicar
lo que hace a un lector humano. Con esta idea en mente, solemos
escribir codigo que intenta tratar con conceptos de alto nivel lo mas
posible.

En ese marco, solemos olvidar que no solo es importante comunicar
"lo que *hace*" (de forma abstracta) si no, tambien, "lo que **hace**"
(de forma literal). Y terminamos escribiendo codigo que esconde todos
los detalles de implementacion hasta el punto que es imposible pensar
sobre como las distintas piezas interactuan en bajo nivel.

Aca hay un articulo sobre como el tema:

[John Carmack | On Inlined Code](http://number-none.com/blow/blog/programming/2014/09/26/carmack-on-inlined-code.html)

Otro problema con la abstraccion excesiva, es terminar con "codigo
expansivo" (i.e. mucho codigo que hace muy poco). Esto es un problema
porque nos deja con mas codigo que mantener, mas lugares donde podemos
meter bugs, mas cosas que leer para entender el sistema completo, y,
si no tenemos cuidado, mas instrucciones que ejecutar.

Un ejemplo no tan terrible (aunque aterrador, porque el presentador,
alguien respetado en la comunidad de C++, lo presenta como algo bueno)
se puede ver en esta charla:

[Jason Turner | Rich Code for Tiny Computers : A Simple Commodore 64 Game in C++17](https://www.youtube.com/watch?v=zBkNBP00wJE&t=4212s)

> Aunque es extremadamente cool lo que hace, demuestra la idea a la
> perfeccion. Por que su codigo compila a muy poquito assembly? Es porque
> detras de las capas de abstraccion que le pone encima, su codigo hace
> muy pocas cosas. En otras palabras, es mucho codigo que hace muy poco.

y otro ejemplo (aunque este es creado artificialmente con el proposito
de mostrar como revertir el problema) se ve en los ultimos 10 minutos
de esta charla:

[Kevlin Henney | Clean Coders Hate What Happens To Your Code When You Use These Enterprise Programming Tricks](https://youtu.be/brfqm9k6qzc?t=2609)


## Evitar la optimizacion prematura o arbitraria

Hay veces que intentamos escribir codigo eficiente al costo de la
legibilidad. Esto de por si no es del todo malo. Si tenemos un cierto
objetivo de rendimiento de nuestro programa, esta bien optimizar.

El problema viene de no entender nuestro programa, y optimizar cosas
que ocupan un porcentaje chico del tiempo de ejecucion del programa.

De esta forma, pagamos un "costo" en la calidad del codigo, a cambio
de una "ganancia", en tiempo de ejecucion, insignificante.

### Elegir bien que cosa optimizar

Cuando optimizamos, nuestra optimizacion va a tener el mayor impacto
posible si optimizamos una parte que ocupe un porcion grande del
tiempo de ejecucion. Para que esto sea asi, tenemos que encontrar
cual es esa parte.

La forma de hacer esto es con un profiler (e.g. "Linux perf"). Estos
programas nos muestran cuales son las partes del codigo que mas se
ejecutan, mostrandonos donde nos conviene ponernos a trabajar.

### Medir bien si la optimizacion fue efectiva

Siempre que se optimiza, hay que medir el tiempo de ejecucion del
componente en el que se esta trabajando antes y despues de optimizar.

Para lograr esto, podemos o bien (1) medir el impacto sobre el
programa entero (e.g. con un profiler), o bien (2) construir
micro-benchmarks que analizan unicamente el componente que modificamos.

Ambos tienen sus meritos:
 - Usar un profiler suele ser menos trabajo
 - Un profiler puede medir el impacto sobre el programa entero (util si, por ejemplo, optimizaste la comparacion de strings)
 - Los micro-benchmarks suelen ser mas precisos (aunque si es dificil saber si la optimizacion ayuda o no, quizas estas optimizando el componente incorrecto)
 - Los micro-benchmarks suelen correr mas rapido (importante si vas a correrlos muchas veces)

Otro ejemplo tipico de optimizacion prematura en C++ es definir
overloads para `T&&` y `T const&`, (en una funcion no critica) en vez
de usar simplemente `T` en la interfaz y mover el valor internamente.

## Evitar la pesimizacion prematura

Es comun usar el punto anterior para justificar no considerar el
rendimiento de un pedazo de codigo que estamos escribiendo.  Esto suele
ser una mala practica que lleva a sufrir una "muerte por mil cortes".
Es decir, cuando el programa es muy lento pero no hay ningun componente
en particular que consuma la mayoria de los recursos.

Por esa razon, si hay dos soluciones posibles con distinto rendimiento,
recomiendo elegir la mas eficiente, incluso si es *moderadamente* mas
complicada. (En especial dentro del runtime)

La forma mas comun de este antipatron en C++ es tener copias
innecesarias. (e.g. copias de strings en el AST)

## Discriminar apropiadamente los errores

En un programa hay distintos tipos de errores. En particular, hay
errores del usuario, errores del entorno, y errores de programacion.

Es muy comun no diferenciar, pero estos 3 tipos de errores se deben
manejar de formas distintas.

### Errores de usuario:

Se da una entrada invalida al programa.

El programa no puede hacer nada para corregir el desperfecto
automaticamente. Se debe reportar el error, posiblemente con
sugerencias para resolver el problema.

### Errores del entorno:

El programa intenta comunicarse con algun sistema externo y falla
(e.g.: leer un archivo, hacer una conexion TCP)

Basicamente, el programa tiene tres opciones: Continuar la ejecucion
a pesar del error (por ejemplo si falta un archivo de configuracion,
usar valores por defecto), reintentar de una forma limpia y no nociva
para el entorno (e.g. con delays incrementales y randomizados entre
intentos), o reportar el error al usuario.

tambien se pueden hacer combinaciones de las tecnicas. En particular,
probablemente siempre es buena idea reportar el error al usuario al
mismo tiempo que se continua con alguna de las otras estrategias.

### Errores de programacion:

Se rompe una invariante interna del programa.
(e.g.: el array que deberia estar ordenado no esta ordenado, el puntero
que no deberia ser null es null, etc)

Esto significa que el programa esta mal hecho. El usuario no tiene por
que enterarse de esto, asique tipicamente no se dan reportes detallados.

Al mismo tiempo, al haberse roto una invariante interna, la ejecucion
no puede dar resultados validos, por lo que se debe abortar el programa.

En general, la solucion es loggear, intentar mandar un reporte al
equipo de desarrollo y cerrar el programa.

> Nota: actualmente consideramos todos los errores como fatales. ese
> es un punto para trabajar a futuro.

## Evitar las dependencias ciclicas

En una escala controlada, las dependencias ciclicas estan bien (e.g.
en el parser). Pero es un problema grande cuando empiezan a abarcar
varios de los modulos principales de un proyecto.

No solo llevan a compilacion mas lenta, tambien hacen que decidir
donde se puede escribir un pedazo de codigo se vuelva muy complicado.
