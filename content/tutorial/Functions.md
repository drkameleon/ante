---
title: "Functions"
date: 2017-11-15T14:07:26-05:00
menu: main
---

# Function Calls

The syntax for calling a function is `f arg1 arg2 ... argn`.  The arguments are
separated by spaces and are each expressions, although the only operator with
a higher precedence than a function call is `.` to access fields.  For example,
to call the print function with the parameters 2 and "three":

```ante
print 2 "three"
```

## UFCS

Universal Function Call Syntax is fully supported on every function.  That is to
say that the call `f arg1 arg2 ... argN` is the same as calling the method f with
arg1 as the object: `arg1.f arg2 arg3 ... argN`.  If `arg1` has a field `f` then
the f will refer to the field rather than the global function meaning UFCS cannot
be used if the desired function has the same name as a field of its first parameter.

```ante
2.print ()        //UFCS!
2.print "three"   //UFCS!

type Bad = i32 print
let b = Bad 3

print b     //Prints b
b.print()   //Error: print is not a function, it is an i32!
```

## Mangling

Function names in Ante are mangled to enable overloading for
various parameter types.

```ante
print 2          // call print: i32

print "Hi"       // call print: Str

print (2, "Hi")  // call print: (i32, Str)
```

In the case that multiple functions match a given parameter tuple, the
definition that requires the least amount of typevar bindings is used.

```ante
fun print: 'a val = ...

fun print: i32 int = ...

print "hi"  // call print: 'a  with 'a = Str
print 4321  // call print: i32



fun print: 'a,'b pair = ...

fun print: i32,'b pair = ...

print ("Hello", 300) // call print: ('a,'b) with
                     // 'a = Str, 'b = i32

print (2, 3)  //call print: (i32,'b) with 'b = i32
```

If multiple functions equally match a given call an appropriate error is issued.

```ante
fun print: 'a,i32 pair = ...

fun print: i32,'b pair = ...

print (2, 3)

/* Output:
file.an: 5,1-5    error: Multiple equally-matching candidates found for call to print with args ((i32, i32))
print (2, 3)
^^^^^

file.an: 1,5-9    note: Candidate function with params (('a, i32))
fun print: 'a,i32 pair = ...
    ^^^^^

file.an: 3,5-9    note: Candidate function with params ((i32, 'b))
fun print: i32,'b pair = ...
    ^^^^^

Compilation aborted.
*/
```

# Function Definitions

Normal function definitions that take a series of parameters and
have a return type are in the form:

```
[modifiers] fun <name>: <params> -> <type>
    <block>
```

Optionally, if the function returns `void` the return type may
be omitted entirely:

```
[modifiers] fun <name>: <params>
    <block>
```

```ante
//Examples

//Take no parameters and return void
fun sayHi:
    print "hi"

//Take one i32 and return an i32
fun inc: i32 x -> i32
    x + 1

//Take two strings and return a string
fun concat: Str a, Str b -> Str
    a ++ b

//Take no parameters and return an i32
fun one: -> i32
    1

pub fun add: i32 a, i32 b -> i32
    a + b

//Modifiers and compiler directives can be all on one line
!inline !noignore pri ante fun five: -> i32
    5

//but the preferred practice is to separate them.
//Consider a redesign if the modifiers are excessive.
!inline !noignore
pri ante
fun five: -> i32
    5

```

## Parameter Shorthand

If multiple parameters of a given type are taken in a row by
a function definition (or a field of a type definition) the type
may be omitted for the second parameter along with the comma.

```ante
fun concat: Str a b -> Str
    a ++ b
```

## Inferred Return Type

If the return type of a function is omitted and replaced with an `=`
Ante will infer the return type of a function from its body.

```ante
fun concat: Str a b =
    a ++ b
```

## Lambdas

A lambda is an anonymous function, a function with no name.
To create a lambda, simply omit the function's name.

```ante
fun Str a b =
    a ++ b
```

Lambdas are often used in conjunction with higher order functions when
defining a named function to use once would make little sense or when
a closure is needed.

```ante
let l = List(0, 3, 4, 9, 10)

//To double every element of a list
map l (fun n = 2 * n)

//_ can be used to create a lambda with 1 argument, the _
l.map (2 * _)

//get a list of only "hi" and "hello"
["hi", "yo", "hello", "bye"]
|> filter (_ in ["hi", "hello"])
```

## External Function Declarations

Functions external to the linking unit, usually non-Ante functions,
can be declared with a `;` after the function header.

```ante
fun printf: c8* cStr, ... -> i32;

printf "My favorite number is %d\n" 11
```

## Operator Overloads

Operators can be overloaded for any given set of types by creating
a function with the name being the operator surrounded in parenthesis.
Normal rules for function overloading apply.

```ante
fun (==): File l r =
    abspath l == abspath r


//Get a list of files in a directory with names between the
//names of the Files l and r
fun (..): File l r -> List File
    let (dl, dr) = (dir l, dir r)
    if dl == dr then
        dl.filter (_.name in l.name .. r.name)
    else
        []

//given a directory of with the structure:
// ante.an  javascript.js  haskell.hs  README.md  d-lang.d
File "ante.an" .. File "haskell.hs" ==
    List(File "ante.an", File "haskell.hs", File "d-lang.d")
```
