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
type Person = Str name, u8 age  //Str and u8

let p = Person("Mary", 42u8) //Str and u8
```

---
## Explicit Casts

All casts in Ante are explicit by default, and only through a
compiler extension can they be made implicit.

```ante
let three = Str 3  //=> "3"

fun add: usz a b = a + b

add 4u8 5u8  //=> error, mismatching types!
add 4usz 5usz  //good!
```

---
## Generic Casts

When casting to generic types, Ante makes gracious use of type
inference, inferring the specific variant of the generic type based
on the type being cast from and the cast functions available.

```ante
// cast from [3 i32] to Vec i32
let v = Vec[1, 2, 3]
```

If the specific generic variant is ever needed to be explicitly stated,
such as when declaring an empty container type, the type arguments
follow the type name.

```ante
let empty = Vec i32()
```

---
## Implicit Casts

Implicit casts can be achieved with the `!implicit` compiler directive.
See its section in [compiler directives](../compiler-directives).
