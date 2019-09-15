---
title: "Type Casts"
date: 2017-11-15T14:02:48-05:00
menu: main
---

Type casts in Ante are performed with the `Type val` syntax in
which `val` is casted to the type `Type`.  Unlike functions,
type casts cast a singular value to another type, and thus its
arguments are not separated by spaces.  If multiple arguments
are needed to cast to a given type they can be wrapped in a tuple.
The contents of the tuple should match the tuple that the type
was originally defined as.

```ante
type Person = name:Str age:u8  //Str and u8

p = Person("Mary", 42u8)       //Str and u8
```

---
## Explicit Casts

All casts in Ante are explicit by default, and only through a
compiler extension can they be made implicit.

```ante
three = Str 3  //=> "3"

add a:usz b:usz = a + b

add 4u8 5u8  //=> error, mismatching types!
add 4usz 5usz  //good!
```

---
## Implicit Casts

Implicit casts can be achieved with the `!implicit` compiler directive.
See its section in [compiler directives](../compiler-directives).
