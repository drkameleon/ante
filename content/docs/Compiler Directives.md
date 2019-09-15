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
Like functions, compiler directives can take arguments.  However, like `ante`
functions, these arguments must all be compile-time constants.  There also
exists a shorthand for compiler directives that take no arguments, `!name`.

You may be thinking they sound very similar to functions, and you are right.
If we really wanted to, we could define a
compiler directive that adds two numbers during compile-time:

```ante
![add 1 2]  //=> 3
```

Although such a function would be better performed by an `ante` function.

```ante
add = ante (+)

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
id x = x

!packed
type T = a:bool b:bool i:i32

!pure
![prereq (x > 0)]
fact x:Int =
    if x == 1 then 1
    else x * fact (x-1)
```

They can also appear before types and arbitrary expressions however:

```ante
reverse l:(!gc List) =
    foldl cons Nil l


//Like ante functions, compiler directives can modify parse trees:
!class
type Person =
    id: !unique Nat
    tasks: !gc List Str

    doWork self =
        match self.tasks with
        | t::ts -> print "Doing ${pop self.tasks}"
        | [] -> print "Nothing to do."

    get_name self =
        "Bob. Probably."


ntc = ![reinterpret_as Arr u8] Person(new 3, ["Work"])

//In most cases, compiler directives on arbitrary expressions
//could be better represented with a function.
ntc = Person(new 3, ["Work"]) |> reinterpret_as Arr u8
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
test fd:FuncDecl a:Args val:'t =
    if fd a != val then
        Ante.error "${fd.name} ${a} != ${val}"


![test 1 1]
![test 5 120]
fact x =
    if x == 1 then 1
    else x * fact (x-1)
```

The following example implements the function typeof on
any type that includes `!typeid` in its definition.

```ante
//map types to their type ids
type_tbl = mut global empty Vec

trait TypeId 't
    typeof 't -> Type 't

!compiler_directive
typeid (td: mut TypeDecl 't) =
    //inject a hidden field with a default value of len type_tbl
    td.inject_hidden "type_id:u64" default:(len type_tbl)
    type_tbl.push td

    Ante.implement (TypeId 't) block
        typeof x:'t = type_tbl#(td.get_hidden "type_id")


!typeid type Person = name:Str job:Str age:u8

!typeid type Animal = name:Str species:Str age:u8


p = Person("John", "Programmer", 32)
a = Animal("Spot", "Dog", 3)
do_thing p  //=> "Do whatever it is Programmers do!"
do_thing a  //=> "Go fetch!"

do_thing val =
    match typeof val with
    | Person -> "Do whatever it is ${val.job}s do"
    | Animal where val.species = "Dog" -> "Go fetch!"
    | _ -> "Wait, O unknown value"
```

---
# Builtin Compiler Directives
---
## !inline

Inlines a function at its callsite whenever possible.

```ante
!inline
add a b = a + b

!inline
print s:Str =
    puts s.cStr
```
---
## !compiler_directive

Creates a new compiler directive with the given function
as its implementation.

```ante
!compiler_directive
debug v:'t = print v

!debug
!compiler_directive
test fd:FuncDecl a:Args =
    print "test <| ${fd} ${a} = " (fd a)
```

---
## !implicit

If used on a cast function, marks the function as an implicit cast.
If used on a type, enables implicit casts to that type.

```ante
type Vec 't = ...
type Arr 't = ...

impl To (Arr 't) (Vec 't)
    !implicit to _ vec = ...

print_arr (arr: !implicit Arr) =
    print "Array" arr "of length" arr.len

a = Arr.of (1, 5, 9)
v = Vec.of (1, 3, 7)

print_arr a  //Arr is already an Arr
print_arr v  //Vec is converted to Arr
             //without calling (to Arr v), (Arr v), or (v as Arr)
```

---
## !lazy

Marks a type as lazy.  This type is guarenteed not to evaluate its
value until forced into a non-lazy type.  Furthermore, the value
is never double-evaluated.

```ante
type ThingNum = One | Two | Three

do_thing thing_num (thing1:!lazy 't) (thing2:!lazy 't) (thing3:!lazy 't) -> 't =
    match thing_num with
    | One -> thing1
    | Two -> thing2
    | Three -> thing3

t1 = !lazy print "one"

//only print "two"
do_thing Two t1 (print "two") (print "three")
```

---
## !noinit

Declares a variable without an initialization.  Variables declared
with noinit will issue an error if they are used in an expression when
the compiler cannot guarentee that they have been initialized.  Noinit
vars are usually used paired with c functions that expect to initialize
the values themselves.

```ante
type Mpz = ref unit

//Declare mpz_init to accept possibly uninitialized values
mpz_init out:(!noinit mut Mpz) -> unit

//Create a wrapper around the c-style initialization
module Mpz
    init () =
        t = !noinit mut Mpz
        mpz_init t
        t
```

---
## !raw

Explicitly declares an unmanaged pointer.  If the `GC` or `Ownership`
modules are imported this variable or type will remain manually
managed.

```ante
alloc_int i:Int -> !raw ref Int =
    new i

mem = !raw malloc 8
int = alloc_int 10

free mem
free int
```
