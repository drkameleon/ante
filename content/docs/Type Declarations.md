---
title: "Type Declarations"
date: 2017-11-15T14:01:44-05:00
menu: main
---

Declaring a new type in Ante is accomplished with the `type` keyword.
When a new type is declared it is considered to be different than the type(s)
it contains.  Resultingly, if a function requires a Str but a Name is
given an error will be issued even if Name contains only a single Str.
This feature provides additional type safety by protecting against the use
of a value on a function that was defined with a different type in mind that
may have different semantics than the newly-defined type.  In situations
where this is not desirable, type aliases can be used.

```ante
type Name = Str

fun print: Str s = ...

let n = Name "John"
print n  //=> error, print:Name not found
```

---
## Type Aliases

An alias for a type can be used in any instance
the original type is used, it is exactly equal to the
original type.  The syntax for an alias declaration is
the same as the syntax for a type declaration but with
the `=` replaced with `is`.

```ante
type AnotherNameForStr is Str
type Coord is i32,i32

let s = AnotherNameForStr "Greetings!"
let c = Coord(2, 3)

fun print: Str s = ...
fun print: i32,i32 c = ...

//ok!
print s
print c
```

Types declared with `=` are considered separate types, but
types declared with `is` are always exactly equal to the types
on the right hand side of the declaration.  In this way, the
distinction between type declaration with `=` and `is` is
similar to the distinction between `=` and `is` when comparing
two values.

---
## Structs and Records

If desired a types contained types can each be named and later
accessed by this name.  These contained types would then be
called the fields and the overall type would be called either
a struct or a record type.

```ante
type Person = Str name, u8 age

let j = Person("John", 24)
print j.name
```

This can also be used on types with just a single field:

```ante
type MyStr = Str s

let str = MyStr "How are you doing today?"
print str.s
```

---
## Tagged Unions

Tagged union types are types that can be one of several
possible cases.  When declaring a tagged union each type
must be separated with a '|'.  Additionally, each case
must be given a name to distinguish it from the other
cases.

```ante
type Day = Mon | Tue | Wed | Thurs | Fri | Sat | Sun
```

These types can also have associated values:

```ante
//Both Absolute and Relative Paths have an associated Dir
type Path =
   | Absolute Dir
   | Relative Dir

//If made with the S constructor, this type has
//a contained Str.  Otherwise, it has an Int
type StrOrInt =
   | S Str
   | I Int
```

The only way to extract values from a tagged union type
is to use pattern matching across each possible case.

```ante
//The Maybe type represents having either a value or the
//absence of a value and is often used to represent failure.
type Maybe 't =
   | Some 't
   | None

let val = Some 3

//to print the underlying value if it exists:
match val with
| Some int -> print int
| None -> print "No integer found!"
```
