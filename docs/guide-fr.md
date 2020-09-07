# Jasper

Les principaux objectifs de ce language sont:

 - Pratique d'utilisation
 - Facile à réusiner

Nous cherchons à atteindre ces objectifs à travers diverses fonctionalités :

 - Syntaxe cohérente (WIP)
 - Tout a un type (WIP)
 - Sémantique de valeur (TODO)
 - Polymorphisme paramétrique (WIP)
 - Déduction de type (WIP)

De plus, nous avons ajouté certain éléments pour leur élégance.ty to us.

 - Opérateur pipeline `|>`
 - inyeccion de scopes (TODO) (?)
 - Type de nombres décimaux (TODO)
 - Objets appeleables (TODO)
 - Clôtures de fonctions (WIP)

## Introduction - La structure d'un programme

In Jasper, un programme est une liste de déclarations.

> Une déclaration associe une valeur à un nom.e.
>
> Syntaxiquement, il s'agit d'un identifiant, suivi d'un spécificateur de type
> (optionnel) et d'une valeur initionale. Exemples : `a := 15; b : int = 10;`

Parmi ces délarations, l'une d'entre elle doit être un point d'entrée, une fonction appelée
`__invoke`.

> En Jasper, les fonctions sont définies avec le mot-clé `fn`, suivi d'une liste
> d'arguments ansi que le corps de la fonction.

```rust
x : int = 10;

__invoke : fn() {
	return x;
};
```

Ce programme renvoie la valeur fournie, `10`..

## Sucre syntaxique
Puisque nous reconnaissons l'importance de la syntaxe, Jasper contient une certaine quantité de sucre syntaxique.

### Fonctions courtes

En utilisant des fonctions courtes, les deux déclarations suivantes sont complétement équivalentes :

```rust
f := fn (x) => x + 1;

f := fn (x) {
	return x + 1;
};
```

### Opérateur pipeline ou opérateur pizza

L'opérateur pizza nous permet d'écrire des pipelines dans notre code. C'est très pratique
pour deux choses :
 - Étendre la fonctionnalité d'un objet sans lui ajouter de dépendances
 - Écrire le code dans un style fonctionnel
 
L'opérateur pizza transforme une expression de la forme `x |> f(y,...)` en `f(x,y,...)`

Voici ici un exemple d'utilisation, adjoint de sa version désucrée :

```rust
squared_primes := fn (arr) => arr
	|> filter(is_prime)
	|> map(fn (x) => x * x);

squared_primes := fn (arr) {
	return map(filter(arr, is_prime), fn (x) => x * x);
};
```
