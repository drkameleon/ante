---
title: "Operators"
date: 2017-11-15T14:03:27-05:00
menu: main
---

Ante supports a variety of common operators including arithmetic
operators, logical operators, assignment operators, among others.
The following is a quick run down on the purpose of each operator
proceeded by a table of associativity and precedence for each.

---

## Arithmetic
`+` Addition

`-` Subtraction (binary), Negation (unary)

`*` Multiplication

`/` Division

`%` Remainder

`^` Exponentiation

## Logical

`<` Less than

`>` Greather than

`<=` Less than or equal
 
`>=` Greater than or equal
 
`==` Is equal to, structural equality

`!=` Not equal to

`is` Is exactly equal to, referential equality

`and` Logical and, short-circuiting

`or` Logical or, short-circuiting

`not` Logical not

A note on `==` and `is`:

If two values contain the same values, for example if two
tuples contained the same elements within, they are structurally equal.
If we had two pointers to two separate structurally equal tuples those
pointers would be equal as well.  The pointers would not be the same value
however.  If one were to mutate the contents of its tuple the other tuple
would not necessarily change unless they point to the same value.  If two
pointers point to the same value then they are referentially equal, and
one pointer `is` the other because they point to the same value.

```ante
let a = new (5, "five")
let b = new (5, "five")
let c = a

a == b  //=> true
a == c  //=> true

a is b  //=> false
a is c  //=> true
```

## Assignment

`=` Assigns a value to a variable

`<binop>=` Assigns a value to a variable after performing
            the operation on the previous value and the expression.  ex.

```ante
var a = 3
a += 4  //a is now 7
```

## Bitwise

Bitwise operations are implemented as library functions.

```ante
import Bitwise

band 3 6  //=> 2
bxor 3 6  //=> 5
bor  3 6  //=> 7
bnot 0    //=> -1

//sl and sr are shift left and shift right respectively.
sl 1 3  //=> 8

//sr is arithmetic if its first operand is unsigned,
//and logical otherwise. lsr and asr are available as well.
sr 8 3  //=> 1
```

## Other

`in` Test if a value is in a collection

`++` Append

`..` Range, inclusive

`#` Subscript, array/collection access

`;` Sequencing

`.` Field accessor

`|>` Applies a value to a function on the rhs

`<|` Applies a value to a function on the lhs

`new` Allocates and stores a value

`@` Pointer dereference

`&` Address-of

---
# Operator Precedence


| Operator | Unary or Binary | Associativity | Precedence |
|:--------:|:---------------:|:-------------:|:----------:|
| .        | Binary          | Left          |     17     |
| &        | Unary           | Right         |     16     |
| new      | Unary           | Right         |     16     |
| -        | Unary           | Right         |     16     |
| @        | Unary           | Right         |     16     |
| #        | Binary          | Left          |     15     |
| ^        | Binary          | Right         |     14     |
| *        | Binary          | Left          |     13     |
| /        | Binary          | Left          |     13     |
| %        | Binary          | Left          |     13     |
| +        | Binary          | Left          |     12     |
| -        | Binary          | Left          |     12     |
| ..       | Binary          | Left          |     11     |
| ++       | Binary          | Left          |     10     |
| in       | Binary          | Left          |     9      |
| <        | Binary          | Left          |     8      |
| >        | Binary          | Left          |     8      |
| <=       | Binary          | Left          |     8      |
| >=       | Binary          | Left          |     8      |
| ==       | Binary          | Left          |     8      |
| !=       | Binary          | Left          |     8      |
| is       | Binary          | Left          |     8      |
| not      | Unary           | Right         |     7      |
| and      | Binary          | Left          |     6      |
| or       | Binary          | Left          |     5      |
| \|>      | Binary          | Left          |     4      |
| <\|      | Binary          | Right         |     3      |
| ;        | Binary          | Left          |     2      |
| =        | Binary          | Left          |     1      |
|=\<binop> | Binary          | Left          |     1      |

