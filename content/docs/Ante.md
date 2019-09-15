---
title: "Ante"
date: 2018-08-11T17:36:02-05:00
menu: main
---

The `ante` keyword is used for all things compile-time in Ante.
It is a [modifier](modifers) that can be applied to function declarations,
variable declarations, type declarations, and arbitrary expressions.

The most common use however, is the declaration of compile-time functions.
Note that when declaring such a function (an "ante function"), you do not
need to use the `ante` keyword to call the function, only to declare it.

```ante
ante make_private t:Type =
    import Ante.Modifier
    for field in t do
        t.modifiers.remove Pub
        t.modifiers.remove Pro
        t.modifiers.append Pri

type MyType =
    one: i32
    two: pub i32
    three: pro i32

make_private MyType

Ante.debug MyType //=> MyType = (one:pri i32) (two:pri i32) (three:pri i32)
```

`ante` can also be used to evaluate individual expressions during compile-time.
There are restrictions on what can be evaluated however.  Mutable variables
cannot be used unless they are `ante mut`, loop bindings cannot be evaluated
unless the entire loop is evaluated, and function parameters cannot be
evaluated unless they are also declared as `ante`.  Note that declaring a
parameter as `ante` makes the function a template, forcing it to be re-evaluated
each time in case any compile-time side effects need to be re-evaluated.  If this
is not desired then odds are you do not need an `ante` parameter.  Here is an
example of an `ante` parameter used to make a template function:

```ante
type Vec = data:(ref 't) len:usz cap:usz

//Take a type as a compile-time parameter and create a
//new function that returns a vector of that type
module Vec
    of t:Type = Vec with
        data = malloc (Ante.sizeof 't) as ref 't
        len = 0usz
        cap = 1usz

v = Vec.of i32
```

`ante` parameters provide a convenient way of templating functions without any
obscure syntax and while maintaining strong, static typing.  Compile-time functions,
like normal functions, can be declared to accept any parameter types, including `Type 't`,
the type of types themselves in Ante.  Resultingly, compile-time functions can be used
to implement new functionality for types (or functions, expressions, etc).

```ante
!macro
derive_eq (t:Type 't) -> Expr =
    //expand the macro into:
    (=) a:'t b:'t -> bool =
        for field in t do
            if field a != field b then
                return false
        true


type Person = name:Str age:u8
derive_eq Person
```
