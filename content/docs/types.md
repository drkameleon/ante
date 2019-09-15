---
title: "Types"
date: 2017-11-15T14:00:44-05:00
menu: main
---

Ante has an algebraic type system with refinement types.  In
other words, Ante supports sum types (tagged unions), product types, as well
as types with an added boolean condition.

Types in Ante are lexically distinct from identifiers.  All
user-defined types are in the form `[A-Z][A-Za-z]*`, and
all other types are in the forms described below.

---
## Primitive Types

Most primitive types are in the form of `[a-z]\d+`.
Where the letter specifies its general type (signed integer,
unsigned integer, floating-point, or char), and the number
afterwards specifies its size in bits.  The size may also
be `sz` which indicates that the type is the size of a pointer
on the current platform.

Signed integer types: `i8` `i16` `i32` `i64` `isz`

Unsigned integer types: `u8` `u16` `u32` `u64` `usz`

Floating-point types: `f16` `f32` `f64`

Character type: `c8`

Other: `bool` `unit`

---
## Pointer Types

Pointer types are in the form of `ref <type>` and represent
a C-style pointer to the given type.

```ante
typeof (new 3)      //=> ref i32
typeof (new new 3)  //=> ref ref i32
```

Pointer types are unmanaged by default.  It is strongly reccomended
to choose a memory management strategy early on by importing either
the standard `GC` module or the `Ownership` module.

---
## Array Types

Array types are in the form `\[<size> <type>\]` where
the size is optional.  If the size is included then the
array is a constant array without a length field.  If it
is excluded it is a possibly dynamic array with a separate
length field.

```ante
typeof [1, 2]  //=> [2 i32]
typeof []      //=> [0 't]

typeof <| [1..10]#(2..4)        //=> [3 i32]
typeof <| [1..10].slice (2..4)  //=> [i32]
```

---
## Tuple Types

A tuple type is an aggregate type that is contiguous and heterogenous.
Its type is a combination of each of its element types separated with a comma.

```ante
typeof (2, 3)    //=> (i32, i32)
typeof ("hi", 5) //=> (Str, i32)

typeof (["three"], new 3)  //=> ([1 Str], ref i32)
typeof (1u8, 2u16, 3u32)   //=> (u8, u16, u32)
```

A single element tuple is expressed with a trailing comma:

```ante
typeof (5,)   //=> (i32,)
```

---
## Function Types

The type of a function is its space-delimited parameter types followed by `->` and
its return type.

```ante
add a:i32 b:i32 -> i32 =
    a + b

typeof add    //=> i32 i32 -> i32
typeof puts   //=> (ref c8) -> i32
typeof printf //=> (ref c8) ... -> i32
```

---
## Record Types and Sum Types

Record types and sum types currently must be given a name after declaration
as they have no inline equivalent.  See the [type declarations](../type-declarations)
section for info on how to declare these types.

```ante
//record type
type Person = name:Str age:u8

typeof Person("George", 3)  //=> Person

//Sum type
type Day = Mon | Tue | Wed | Thurs | Fri | Sat | Sun

typeof Tue  //=> Day
```

---
## Type Variables

Type variables can stand in for any type and are the building block
for generic types.  Their syntax is an apostrophe followed by an
identifier.

```ante
//x is any type
id x:'t = x

//p is any pointer type
print_ptr p:(ref 't) = ...
```

Functions that accept a type variable anywhere in their parameters are
generic functions and are recompiled for every valid type variant of
arguments that are used.  For a given variable `x` of type `'t` the
expression `x + 1` is invalid because a type variable must be able to be
substituted with any type and not every type defines the `+` operator.
Resultingly, generic functions in Ante can be type checked sooner and
type checked only once for faster compilation.  If one wishes to have
a function generic over any function that implements addition, traits
can be used to specify this behaviour is required.

```ante
//invalid: + not defined for any type 't
//note: while the given type signature is invalid,
//      there is no error since the compiler
//      automatically infers the missing
//      `given Add 't` clause
inc x:'t =
    x + 1

//valid, (this is what the compiler will infer for the above)
inc x:'t -> 't given Add 't =
    x + x


extract_index (a: Arr 't) idx:usz -> Maybe 't
    if idx in indices a then
        Some (a#idx)
    else
        None


int_arr = [1, 2, 3, 4]
str_arr = ["one", "two", "three"]

//compile a variant just for 't = i32
extract_index int_arr 2

//compile a variant just for 't = Str
extract_index str_arr 1
```

---
## Generic Types

Generic types are types that themselves take other types of parameters.
For example, the `Maybe` type is generic over the type of value it holds.
The arguments of a generic type are specified after the type and are
separated with spaces.  The generic type with no type arguments is
referred to as the parent type (`Maybe` in this case), and each version
with type arguments, eg. `Maybe i32` is a variant of the parent type.
Two generic variants are equal if their parent types are equal and their
generic arguments are equal.

```ante
typeof (Some 3)  //=> Maybe i32
typeof [2, 3]    //=> Vec i32
typeof (Err 0)   //=> Result 't i32
```

If a function omits the generic arguments of a parameter then the resulting
type is the parent type, and is generic for any variant of the parent type.

```ante
//print will accept any variant of Maybe
unwrap m:Maybe =
    match m with
    | Some val -> val
    | None -> panic "Tried to unwrap None value"

mi = Some 3
ms = Some "string"

//both Maybe i32 and Maybe Str are valid inputs
print mi
print ms
```

---
## Refinement Types

Refinement types consist of a normal type, and a boolean condition after delimited
by curly brackets.  This condition is called a refinement.  Refinement types "depend"
on values and are thus a limited form of dependent types, intentionally limited to
a subset that is automatically solvable by a SMT solver so that no proofs need to
be made by the programmer.

In practice, refinement types are useful for changing many runtime errors into
compile-time errors.  The classic example is preventing array/memory indexing
errors by refining an index to be less than the length of the array:

```ante
get (arr: Array 't) idx:usz -> 't
    given idx < len arr =
    ...

a = Arr(2, 4, 6)

get a 0  //=> 2
get a 3  //=> compile-time error: 3 >= len a
```

Preventing division by 0:

```ante
(/) a:Num b:Num -> ret:Num given b != 0
```

Refinement expressions are limited to use only the following operators, in
addition to uninterpreted functions.  In this context, an uninterpreted
function is not evaluated.  Instead, it is type checked and the type of its
result must satisfy the boolean expression.

Operators allowed:

```
=  
<  
<=
>
>=
=>               // Logical implication
and
or 
not
+  
-  
*                // Multiplication by constants only
.                // Member access
#                // Tuple member access only
f arg1 ... argN  // Uninterpreted function call     
```
