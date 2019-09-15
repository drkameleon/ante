---
title: "Traits"
date: 2017-11-15T14:11:54-05:00
menu: main
---

Traits are a way to express the desired behaviour of
a type without stating which type you need specifically.
A function taking in a parameter with a trait type can
take any argument whose type implements that trait.  In this
way, traits provide a means to specify function(s) are implemented
for given typevariable(s). If a type that does not implement, eg
`Trait 't` is passed to a function expecting `Trait 't` then the
compiler will issue an error explaining that the provided
argument must implement `Trait 't` and list the functions required to do so.

A new trait is declared by specifying the trait's name,
which must be a valid type name, followed by a block of
function headers that values of that trait must implement.

```ante
trait Eq 't
    (==) 't 't -> bool
```

Implementing a trait is done with a trait impl, short for trait implementation.
Note that because all the types are provided in the trait definition, the parameter
types of trait impls can always be inferred.

```ante
impl Eq Str
    //now we must implement (==) or get an error
    (==) l r =
        strcmp l r = 0
```

---
## As Parameters

As traits are not types themselves, you cannot require a parameter of
type `Eq 't` (for example).  Instead, the parameter is of type `'t` and
you must specify some impl for `Eq 't` is required via a given clause.

```ante
all_equal a:'t b:'t c:'t -> bool given Eq 't =
    a == b && b == c
```

---
## Default Functions

If a full function definition is provided within a trait instead
of just the header then that function definition will be used by
default and types implementing the trait will not need to
reimplement it to implement the trait.

```ante
trait RelOps 't
    //require cmp to be implemented
    cmp 't 't -> i32

    //and implement all comparison operators based on cmp
    (=) a:'t b:'t =
        cmp a b == 0

    (<) a:'t b:'t =
        cmp a b < 0

    (!=) a:'t b:'t =
        cmp a b != 0

    (>=) a:'t b:'t =
        cmp a b >= 0

    (<=) a:'t b:'t =
        cmp a b <= 0

    (>) a:'t b:'t =
        cmp a b > 0


impl RelOps Str
    cmp l r = strcmp l r

//now cmp, =, <, !=, >=, <=, and > are defined for Str
```

---
## Combining Traits

Multiple traits can be required via separating them in the given clause with ','

```ante
print_eq a:'t b:'t -> unit given Eq 't, Print 't
    print a "==" b "is" (a == b)
```

Alternatively, a name can be given to a trait combination by
using type aliasing:

```ante
type EqPrint 't is 't given Eq 't, Print 't

print_eq (a: EqPrint 't) (b: EqPrint 't) =
    print a "==" b "is" (a == b)
```
