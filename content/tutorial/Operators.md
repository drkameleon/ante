---
title: "Operators"
date: 2017-11-15T14:03:27-05:00
menu: main
---

# Binary

## Arithmetic
\+
\-
*
/
%

## Logical
<
>
<=
 >=
 ==
!=
is
and
or

## Assignment
=
\<binop>=

## Bitwise
implemented as library functions.
Bitwise.xor a b

## Other
in
++
..
#
;
.
|>
<|

# Unary
not
new
\-
@
&

# Operator Precedence


| Operator | Unary or Binary | Associativity | Precedence |
|:--------:|:---------------:|:-------------:|:----------:|
| .        | Binary          | Left          |     17     |
| &        | Unary           | Right         |     16     |
| new      | Unary           | Right         |     15     |
| -        | Unary           | Right         |     15     |
| @        | Unary           | Right         |     15     |
| #        | Binary          | Left          |     14     |
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

