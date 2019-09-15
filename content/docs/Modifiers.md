---
title: "Modifiers"
date: 2017-11-15T14:10:13-05:00
menu: main
---

Depending on the semantics of the modifier, modifiers appear before
any function declaration, variable declaration, type declaration,
type, or arbitrary expression.  All modifiers can be used before
a block to apply the modifier to the entire block.

---
## Visibility Modifiers

The visibility modifiers control how "visible" declarations are
to external modules.  There are three visibility modifiers in Ante:
`pub`, `pri`, and `pro`, short for public, private, and protected
respectively.  Something that is public is visible to all modules,
something that is private is visible to only the current module,
and something that is protected is visible only to modules that
extend the current module.  The default visibility is public
for functions and types and private for global variables.

```ante
// math.an:
pro type T = Num

pub id x = x

pri add1 x:T = x + 1

pi = pub global acos (-1)

...
// circle.an:
module Math
    //T is visible
    add2 x:T =
        //add1 is not visible
        //add1 (add1 x)
        x + 2

radius = Math.add2 0
circ_area = Math.pi * radius^2
```

---
## Mut

Marks a type as mutable.  Variables created with `mut` following =
are mutable.  Like immutability, mutability is transitive.
That is, if some type T is mutable, all of its contained types are
as well, unless any of those types are explicitly marked `const`.

```ante
store (dest: mut ref 't) val:'t =
    @dest := val

store (new "") "Hello there"
```

---
## Const

Types in Ante are immutable by default, although they can be marked
as mutable with the `mut` keyword.  The `const` keyword is the counterpart
to `mut` in that it prevents a type from being mutable even when explicitly
marked as such.  Const is useful when designing types in such a way that
they are always used immutably.

```ante
type Age is const u8
type Person = name:Str age:Age

birthday (a: mut Age) =  //error, cannot use mut on const type Age
    a += 1

birthday (p: mut Person) =
    p.age += 1  //error, age field is marked const
```

---
## Global

Declares a global variable when used on a variable declaration, otherwise
it imports the given global variables into the current scope.

```ante
pi = global acos (-1)

circle_area radius:Num =
    global pi
    pi * radius^2

//The result of the expression global var1 var2 ...
//is the last variable imported.
sphere_volume radius:Num =
    4/3 * (global pi) * radius^3
```

---
## Ante

The `ante` modifier is Ante's primary way of executing compile-time code.  It is
used to either execute arbitrary expressions during compile-time or to mark types
as compile-time constants.  Using it before a function marks the function's parameters
and its output as compile-time constants and the function itself to be executed during
compile-time.  Before a type declaration, it signifies the value of the type should
always be known during compile-time.  Before a variable it signifies the variable should
be known during compile-time, and before an expression it will evaluate the expression
during compile-time.  Consider the type of an integer literal in Ante:

```ante
ante type IntLit = Int
```

Note that because modifiers can also apply to types directly the above is equivalent to:

```ante
type IntLit = ante Int
```

That is, an integer literal is an arbitrarily sized integer that is always known during
compile time.  This enables the compiler to perform implicit conversions on integers
safely without any runtime confusion or slowdowns.  For example, in time-sensitive code,
a function may take in an `i32` and Ante will convert the `IntLit` to an `i32` provided
it is small enough without ever needing to store the arbitrarily sized integer during runtime.
For the reverse, if an arbitrarily sized integer is needed, it will be converted into an
`Int` and simply lose its `ante` modifier.

Perhaps the most common use case is on functions to mark them to run during compile-time only:

```ante
ante load_static f:File = Image f

//load assets during compile-time
bg = load_static "background.png"
p = load_static "player.png"
```

As seen above, calling an ante function is no different from calling a normal function.
To execute an arbitrary expression during compile-time simply prefix it with ante:

```ante
ans = ante input "Do you want to stop compiling? (y/N): "
if ans == "y" then
    Ante.error "Well you did say you wanted to stop"
```

The one type of value that cannot be in an ante expression is a parameter from a function
as it is impossible to backtrace the value from every callsite simultaneously:

```ante
say_something a:Str =
    //error, a is a parameter, mark it with ante
    //to use in an ante expression
    ante print a


//as the error says, in this case the best fix would be to
//change the parameter's type from Str to ante Str
say_something a:(ante Str) =
    //print a during compile-time
    ante print a
```

Using compile-time code execution properly can lead to 
optimizations but also to better error messages and ease of debugging.  It is
also a building block for even more powerful features from Ante's integrated build
system to its gradually managed memory and extensible type system.
