---
title: "Functions"
date: 2017-11-15T14:07:26-05:00
menu: main
---
---
# Function Calls

The syntax for calling a function is `f arg1 arg2 ... argn`.  The arguments are
separated by spaces and are each expressions, although the only operator with
a higher precedence than a function call is `.` to access fields.  For example,
to call the print function with the parameters 2 and "three":

```ante
print 2 "three"
```

---
## UFCS

Universal Function Call Syntax is fully supported on every function.  That is to
say that the call `f arg1 arg2 ... argN` is the same as calling the method f with
arg1 as the object: `arg1.f arg2 arg3 ... argN`.  If `arg1` has a field `f` then
the f will refer to the field rather than the global function meaning UFCS cannot
be used if the desired function has the same name as a field of its first parameter.

```ante
print 2           //UFCS!
2.print ()        //UFCS!

type Bad = i32 print
b = Bad 3

print b     //Prints b
b.print()   //Error: print is not a function, it is an i32!
```

---
## Mangling

Function names in Ante are NOT mangled, so multiple functions in the same
module with the same name are disallowed.  For an alternative to function
overloading, traits can be implemented for multiple types with different
implementations for each.

---
# Function Definitions

Normal function definitions that take a series of parameters and
have a return type are in the form:

```
[modifiers] <name> <params> -> <return type> =
    <expr>
```

If a function should have its return type inferred, the return type and arrow can be omitted.

```ante
//Examples

//Take a unit value as a parameter and return unit
sayHi () -> unit =
    print "hi"

//Take one i32 and return an i32
inc x:i32 -> i32 =
    x + 1

//Take two strings and return a string
concat a:Str b:Str -> Str =
    a ++ b

//Take no parameters and infer return type to be i32
one () = 1

//Make the function private, and infer parameter and return types
pri add a b =
    a + b

//Modifiers and compiler directives can be all on one line
!inline pro ante five () -> i32 =
    5

//but the preferred practice is to separate them.
//Consider a redesign if the modifiers are excessive.
!inline
pro ante
five () -> i32 =
    5

```

---
## Parameter Type Shorthand

The full syntax for parameters is (var:Type) or var:(Type) but if the type
is not a generic type (and thus has no spaces in it), the parenthesis can
be omitted.

```ante
// v's type needs parenthesis, but i does not
push (v: Vec i32) i:i32 -> Str
```

---
## Lambdas

A lambda is an anonymous function, a function with no name.
To create a lambda, use `\\` instead of the function's name.

```ante
\a b = a ++ b
```

Lambdas are often used in conjunction with higher order functions when
defining a named function to use once would make little sense or when
a closure is needed.

```ante
l = List.of (0, 3, 4, 9, 10)

//double every element of a list
map l (\n = 2 * n)

//_ can be used to create a lambda with 1 argument
l.map (2 * _)

//get a list of only "hi" and "hello"
["hi", "yo", "hello", "bye"]
|> filter (_ in ["hi", "hello"])
```

---
## External Function Declarations

Functions external to the linking unit, usually non-Ante functions,
can be declared by omitting the body of the function.

```ante
printf (cStr:ref c8) ... -> i32

printf "My favorite number is %d\n" 11
```

---
## Operator Overloads

Operators can be overloaded for any given set of types by implementing
their corresponding trait and creating
a function with the name being the operator surrounded in parenthesis.

```ante
impl Eq File
    (==) l r =
        abspath l == abspath r


//Get a list of files in a directory with names between the
//names of the Files l and r
impl Range File File
    type Ret = List File

    (..) l:File r:File -> List File =
        (dl, dr) = (dir l, dir r)
        if dl == dr then
            dl.filter (_.name in l.name .. r.name)
        else
            []

//given a directory of with the structure:
// ante.an  javascript.js  haskell.hs  README.md  d-lang.d
File "ante.an" .. File "haskell.hs" =
    List(File "ante.an", File "haskell.hs", File "d-lang.d")
```
