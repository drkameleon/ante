---
title: "Variables"
date: 2017-11-15T14:09:45-05:00
menu: main
---

Variables come in two main varieties: mutable and immutable.
Immutable variables cannot be changed once set although they
can shadow other variables of the same name.  Mutable variables
on the other hand can be changed, or "assigned".  Following an
assignment the mutable variable then refers to the newly assigned
value and the old value, unless it is also stored elsewhere, is lost.

Both forms of declaration infer the type of the variable from the
type of the expression so it is never necessary to write out the
type of the variable.

## Let

Declares an immutable variable.

```ante
let x = 2
print x  //=> 2

//Note that in the binding below the original x is not
//changed.  Instead, the value 3 is bound to another x
//that shadows the original
let x = 3
print x  //=> 3
```

## Mut

Declares a mutable variable.  Assigning new values to mutable variables
can be done with the `=` operator.

```ante
mut x = 2
print x  //=> 2

x = 3
print x  //=> 3
```
---
## Explicit Types

If one wishes to specify the type of the variable to enforce that
it is of the desired type, the type can be optionaly specified after
let/var.

```ante
let i32 x = 5

let Str s = "hi"

let bool b = 10  //=> error, b was declared to be a bool
                 //but the expression is of the type i32
```
