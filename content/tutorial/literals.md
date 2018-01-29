---
title: "Literals"
date: 2017-11-14T17:53:00-05:00
menu: main
---

Ante has four types of literal values: integers, floats, booleans, and strings.  Every literal,
except for strings, is a primitive type.

---
## Integers

Integer literals in Ante can be any primitive integer type, i8, i16, i32, i64, u8, u16, u32, u64, isz, or usz.  Without a type suffix,
an integer literal's default type is i32.  Integer literals have four parts: the optional base notation, the number itself, the 
optional exponent, and the optional type suffix.  The number itself is in the form

```
[0-9_]+
```

Underscores are allowed as separators in numeric literals and do not affect the value of the integer.
Negatives are represented by the unary - operator rather than in the literal itself.

The base notation comes before the number and can be one of 0x, 0o, or 0b, representing hexadecimal, octal, and binary bases
respectively.

After the number itself comes the optional exponent.  The exponent is in the form

```
[Ee][+-]?[0-9_]+
```

Finally, the type suffix of the number is an optional segment that specifies the literal's type.
Integer type suffixes begin with either an i or u, denoting whether or not the literal is signed, with u representing an unsigned integer.
After the sign specification comes the optional size specification which can be one of 8, 16, 32, 64, or sz, which represent the size in
bits of the integer.  A type suffix of i8 specifies an 8-bit signed integer, and a type suffix of usz specifies an unsigned integer
of pointer-size.  Literals with a sign specification but no size specification will be the default integer size.

---
## Floats

Floating-point literals come in three varieties according to their type: f16, f32, and f64, which correspond to an IEEE half, IEEE single,
and IEEE double respectively.  Float literals are distinguished from integer literals by the required . and numeric integer following it.
As such, 23. is not a float, the . in this case acts as a binary operator on 23 looking for a method declaration.  Like integer literals,
float literals may also contain an optional exponent and type suffix, however, float literals are always expressed in base ten, and as
such have no base notation.  Underscores are also permitted in float literals with no effect on the value.  A type suffix of a float
literal is either f16, f32, or f64.  If none are specified, the default is f64.

```ante
0.5      //IEEE double
2.4_f16  //IEEE half

43. 43   //Syntax error
43 .43   //Same as above

43.four  //method/field access on the integer 43
```

---
## Booleans

Boolean literals are either `true` or `false`, and are always of type `bool`.  `bool` values may
occupy anwhere from 1 to 8 bits depending on the target and optimization techniques used, but only 1 bit is able to be viewed or modified.
Boolean values are not integer types.

---
## Strings

String literals are denoted by matching double-quotes around a string of text.  Characters preceeded by a backslash will be interpretted
as an escape sequence.  Escape sequences not recognized by the compiler will generate an error informing the user of the unknown sequence.
The table below shows a list of all escape sequences used.


| Escape Sequence |  ASCII value   |    Translation      |
|:---------------:|:--------------:|:-------------------:|
|    \a           | 0x7            | Bell                |
|    \b           | 0x8            | Backspace character |
|    \f           | 0xC            | Form feed           |
|    \n           | 0xA            | Newline             |
|    \r           | 0xD            | Carriage return     |
|    \t           | 0x9            | Tab                 |
|    \v           | 0xB            | Vertical tab        |
|    \XXX         | XXX (in octal) | generic character   |

Note that the sequence \XXX is a backslash followed by one or more numeric characters.

### String Interpolation

String interpolation is the evaluation of an expression followed by the concatenation of the
result of that expression to the relevant part of the string.  String interpolation is performed
within the delimiters ${ and }.  To concatenate the expression into the string, an overload for
the ++ operator is searched for first.  Because the concatenation is left-associative, the operator
function `++_Str_<expr type>` is searched for first.  If there is no
appropriate overload found, then the compiler will attempt to cast the expression to a Str before
concatenation.  The result of the first concatenation is then concatenated with the end of the string
via the builtin function `++_Str_Str` if necessary.

Interpolated expressions are compiled normally and will generate compile-time errors if malformed
like any other expression.

```ante
let msg = "World!"

//Both args are strings and nothing follows the expression
//One concatenation is performed
print "Hello ${msg}"

//More text follows the expression, two concatenations are performed
print "Hello ${msg} Hello all!"


type Person = Str name, i32 age
fun Str.init: Person p = p.name

let p = Person("John", 32)

//Expression is not a Str, and no overload for ++ is found,
//expression is cast to a Str and then concatenated
print "Hello, ${p}"
```
