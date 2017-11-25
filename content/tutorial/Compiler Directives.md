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

---
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
        | t::ts -> print "Doing ${pop self.tasks}"
        | [] -> print "Nothing to do."

    fun get_name: self =
        "Bob. Probably."


let ntc = ![reinterpret_as Arr u8] Person(new 3, ["Work"])

//In most cases, compiler directives on arbitrary expressions
//could be better represented with a function.
let ntc = Person(new 3, ["Work"]).reinterpret_as Arr u8
```
---
# Creating New Compiler Directives

To create your own compiler directive, tag a function
with `!compiler_directive`.  As with functions, the parameters of
a compiler directive determine what it can be used with.  If
the newly created compiler directive should be used with function
declarations its first parameter should be a `FuncDecl`, likewise if it
is to be used with a type declaration or a variable declaration
its first parameter should be a `TypeDecl` or `VarDecl` respectively.
If it can operate on any declaration, it should take a `Decl`.

```ante
!compiler_directive
fun test: FuncDecl fd, Args a, 't val
    if fd a != val then
        Ante.error "${fd.name} ${a} != ${val}"


![test 1 1]
![test 5 120]
fun fact: i32 x =
    if x == 1 then 1
    else x * fact (x-1)
```

The following example implements the function typeof on
any type that includes `!typeid` in its definition.

```ante
//map types to their type ids
var type_tbl = Vec Type

!compiler_directive
fun typeid: mut TypeDecl td
    //inject a hidden field with a default value of len type_tbl
    td.inject_hidden "u64 type_id" default:(len type_tbl)
    type_tbl.push td

    td.define "typeof" (fun self =
        type_tbl#td.get_hidden "type_id")


!typeid type Person = Str name job, u8 age

!typeid type Animal = Str name species, u8 age


let p = Person("John", "Programmer", 32)
let a = Animal("Spot", "Dog", 3)
do_thing p  //=> "Do whatever it is Programmers do!"
do_thing a  //=> "Go fetch!"

fun get_task_for: 't val
    match typeof val with
    | Person -> "Do whatever it is ${val.job}s do"
    | Animal where val.species == "Dog" -> "Go fetch!"
    | _ -> "Wait, oh unknown value"
```

---
# Builtin Compiler Directives
---
## inline

Inlines a function at its callsite whenever possible.

```ante
!inline
fun add: i32 a b = a + b

!inline
fun print: Str s
    puts s.cStr
```
---
## compiler_directive

Creates a new compiler directive with fd as its implementation.

```ante
!compiler_directive
fun debug: 't v = print v

!debug
!compiler_directive
fun test: FuncDecl fd, Args a
    print "test <| ${fd} ${a} = " (fd a)
```

---
## noinit

Declares a variable without an initialization.  Variables declared
with noinit will issue an error if they are used in an expression when
the compiler cannot guarentee that they have been initialized.  Noinit
vars are usually used paired with c functions that expect to initialize
the values themselves.

```ante
type Mpz = void*
fun mpz_init: mut Mpz out;

fun Mpz.init: -> Mpz
    !noinit var Mpz t
    mpz_init t
    t
```

---
## raw

Explicitly declares an unmanaged pointer.  If the `GC` or `Ownership`
modules are imported this variable or type will remain manually
managed.

```ante
fun alloc_int: Int i -> !raw Int*
    new i

let !raw mem = malloc 8
let int = alloc_int 10

free mem
free int
```
