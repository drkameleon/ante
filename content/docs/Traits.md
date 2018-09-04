---
title: "Traits"
date: 2017-11-15T14:11:54-05:00
menu: main
---

Traits are a way to express the desired behaviour of
a type without stating which type you need specifically.
A function taking in a parameter with a trait type can
take any argument whose type implements that trait.  Traits
are generally advantageous over type variables because they
provide better error messages.  For example, if a given type
that does not implement a `print` function is passed to a
function expecting a `Printable` variable the compiler will
issue an error saying that the provided argument must implement
`Printable` and list the functions required to do so.  If the
function instead accepted a `'t` then the compiler would issue
an error at the callsite of `print` followed by possibly more cryptic errors.

A new trait is declared by specifying the trait's name,
which must be a valid type name, followed by a block of
function headers that values of that trait must implement.

```ante
trait Eq
    fun (=): Eq a b -> bool
```

Implementing a trait is done within a type extension.

```ante
ext Str: Eq
    //now we must implement (=) or get an error
    fun (=): Str l r =
        strcmp l r = 0
```

Multiple traits can be implemented in a single type extension
by separating each trait name with a comma.

```ante
ext Str: Eq, Printable
    fun (=): Str l r =
        strcmp l r = 0

    fun print: Str s
        puts s.cStr
```

---
## As Parameters

Functions taking in a trait-typed parameter accept any type
that implements that trait.  A trait type is distinct from the
type that implements it.  Fields from the implementing type
cannot be accessed with a value of the trait type because it
may be that the value is actually of a different type that
also implements the trait.

```ante
//iter cannot differentiate between a range of integers
//and an array of integers.
fun iter: Iterable 't it, 't->'u f
    //print it.step
    //=> error, method/field step not found in type Iterable
    for e in it do
        f e

iter (1..10) print
iter [1, 3, 5] print
```

---
## Default Functions

If a full function definition is provided within a trait instead
of just the header then that function definition will be used by
default and types implementing the trait will not need to
reimplement it to implement the trait.

```ante
trait RelOps
    //require cmp to be implemented
    fun cmp: RelOps a b -> i32

    //and implement all comparison operators based on cmp
    fun (=): RelOps a b =
        cmp a b = 0

    fun (<): RelOps a b =
        cmp a b < 0

    fun (!=): RelOps a b =
        cmp a b != 0

    fun (>=): RelOps a b =
        cmp a b >= 0

    fun (<=): RelOps a b =
        cmp a b <= 0

    fun (>): RelOps a b =
        cmp a b > 0


ext Str: RelOps
    fun cmp: Str l r =
        strcmp l r

//now cmp, =, <, !=, >=, <=, and > are defined for Str
```

---
## Combining Traits

Functions requiring a value with multiple traits may combine
traits with `+`.

```ante
fun print_eq: Eq+Printable a b
    print a "=" b "is" (a = b)
```

Alternatively, a name can be given to a trait combination by
using type aliasing:

```ante
type EqPrint is Eq + Printable

fun print_eq: EqPrint a b
    print a "=" b "is" (a = b)
```
