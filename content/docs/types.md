---
title: "Types"
date: 2017-11-15T14:00:44-05:00
menu: main
---

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

Character types: `c8` `c16` `c32` `c64`

Other: `bool` `void`

Despite its name, Ante's `void` type is more analogous to a unit
type in other languages rather than the conventional void type.
This is due to it having  `()` as its single value rather than
most void types which have no values.

---
## Pointer Types

Pointer types are in the form of `<type>\*` and represent
a C-style pointer to the given type.

```ante
typeof (new 3)      //=> i32*
typeof (new new 3)  //=> i32**
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
typeof (2, 3)    //=> i32, i32
typeof ("hi", 5) //=> Str, i32

typeof (["three"], new 3)  //=> [1 Str], i32*
typeof (1u8, 2u16, 3u32)   //=> u8, u16, u32
```

A single element tuple is expressed with a trailing comma:

```ante
typeof (5,)   //=> i32,
```

---
## Function Types

The type of a function is the tuple of its parameters followed by `->` and
its return type.

```ante
fun add: i32 a, i32 b -> i32
    a + b

typeof add    //=> (i32, i32) -> i32
typeof puts   //=> (c8*,) -> i32
typeof printf //=> (c8*, ...) -> i32
```

---
## Record Types and Sum Types

Record types and sum types currently must be given a name after declaration
as they have no inline equivalent.  See the [type declarations](../type-declarations)
section for info on how to declare these types.

```ante
//record type
type Person = Str name, u8 age

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
fun print: 't x = ...

//p is any pointer type
fun print_ptr: 't* p = ...
```

Functions that accept a type variable anywhere in their parameters are
generic functions and are recompiled for every valid type variant of
arguments that are used.  For a given variable `x` of type `'t` the
expression `x + 1` is invalid because a type variable must be able to be
substituted with any type and not every type defines the `+` operator.
Resultingly, generic functions in Ante can be type checked sooner and
type checked only once for faster compilation.  If one wishes to have
a function generic over any function that implements addition, traits
should be used instead.

```ante
//invalid: + not defined for any type 't
fun inc: 't x =
    x + 1

//valid, + is defined for all Addable types
fun inc: Addable x =
    x + 1


fun extract_index: Arr 't a, usz idx -> Maybe 't
    if idx in indices a then
        Some (a#idx)
    else
        None


let int_arr = [1, 2, 3, 4]
let str_arr = ["one", "two", "three"]

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
fun print: Maybe m
    match m with
    | Some val -> print val
    | None -> print "None"

let mi = Some 3
let ms = Some "string"

//both Maybe i32 and Maybe Str are valid inputs
print mi
print ms
```

---
## Trait Types

The type of a [trait](../traits) is also its name.  The type of a type
satisfying multiple traits can be expressed as each trait's type
joined by `+`.

```ante
trait T1
trait T2

T1     //The type of the trait T1
T1+T2  //The type of the composition of T1 and T2
```

A function accepting a trait type as an argument is generic over any
type that satisfies that trait.

```ante
fun echo: Printable p
    print p

//compiles a function echo:i32 for integers
echo 3

//compiles a function echo:c8* for c8 pointers
echo (new 'h')
```

Functions with trait parameters are normally statically dispatched however
if desired they can be used with dynamic dispatch by using trait objects.
Trait objects can be created by wrapping the desired trait in the `Trait` type.


```ante
//EqList is a heterogenous list of any type that implements Eq
type EqList is List (Trait Eq)

let l = EqList(2, "three", (5usz, -2i8), 'h')

//take two parameters using dynamic dispatch for the = function.
fun f: Trait Eq a b =
    not(a = b)

f 2 3  //compile f:(Trait Eq, Trait Eq)

f "a" "a"  //f is not recompiled
```

Trait object types hide the actual value behind a pointer which contains both
the value itself and a vtable.

```ante
type Trait 't = ('t val, VTbl vtable)*
```
