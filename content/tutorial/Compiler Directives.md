---
title: "Compiler Directives"
date: 2017-11-15T14:11:01-05:00
menu: main
---

As the "compile-time" language Ante naturally needs a mechanism for
interacting with the compiler and other compile-time constructs.
Of course, you can use the `ante` modifier or any of the builtin
compile-time functions in the `Ante` module but to properly leverage
the full power of Ante it is necessary to learn about compiler directives.

In Ante, compiler directives are essentially user-defined modifiers.  Just
like modifiers, compiler-directives can be used to modify variable declarations,
function declarations, or even arbitrary expressions.  The semantics of this
modification of course depends on the the modifier itself.

A compiler directive is in the form `![name arg1 arg2 ... argn]`.
As you have probably noticed from this, compiler directives, like functions,
can take arguments.  Like `ante` functions, these arguments must all
be compile-time constants.  There also exists a shorthand for compiler
directives that take no arguments, `!name`.

You may be thinking they sound very similar to functions, and you are right.
If we really wanted to, we could define a
compiler directive that adds two numbers during compile-time:

```ante
![add 1 2]  //=> 3
```

Although such a function would be better performed by an `ante` function.

```ante
let add = ante (+)

add 1 2  //=> 3
```

Generally, ante functions should be preferred when possible.  The main
advantage compiler directives have over ante functions is their ability
to precede function/variable/type declarations as modifiers.

# As Modifiers

Modifiers can appear before and modify any declaration (function, variable, or type),
any type, or any arbitrary expression.  Naturally, because compiler directives
are modifiers, they can also be used in these places as well.

The most common location for compiler directives is before declarations:
```ante
!inline
fun id: x = x

!packed
type T = bool a b, i32 i

!pure
![prereq (x > 0)]
fun fact: Int x =
    if x == 1 then 1
    else x * fact (x-1)
```

They can also appear before types and arbitrary expressions however:

```ante
fun reverse: !gc List l =
    foldl cons empty l


//Like ante functions, compiler directives can modify parse trees:
!class !nametypedecl
type Person =
    id: !unique Nat
    tasks: !gc List Str

    fun doWork: self
        match self.tasks with
        | t::ts -> print "Doing ${pop tasks}"
        | [] -> print "Nothing to do."

    fun get_name: self =
        "Bob. Probably."


let ntc = ![reinterpret_as Arr u8] Person(new 3, ["Work"])

//In most cases, compiler directives on arbitrary expressions
//could be better represented with a function.
let ntc = Person(new 3, ["Work"]).reinterpret_as Arr u8
```

# Creating New Compiler Directives

To be written.

# Builtin Compiler Directives

To be written.
