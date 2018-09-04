---
title: "Blocks"
date: 2017-11-15T14:12:44-05:00
menu: main
---

Ante is an indentation sensitive language.
The beginning and end of blocks are marked by an increase in
indentation and a decrease in indentation respectively and the
indentation before a line of non-comment code is called
significant whitespace.  Ante's policy on significant whitespace is
to keep it as intuitive as possible by preventing inconsistensies that
would otherwise hamper code readability.  For example, indentations of
a single space in size are considered to be a mistake and will trigger
an appropriate error message.  Additionally, significant whitespace is
comprised solely of spaces, tabs are prohibited to keep indentation
consistent and to prevent the intermingling of tabs and spaces while
keeping the ability to vertically align blocks to other code components.

```ante
//error, random indents are prohibited
let t = 32
    let x = 5

//error, indents must be at least two spaces in size
if true then
 print "hello"
```
---
## Block Keyword

Ante does not allow for stray indents in unexpected areas.  In fact,
indents are usually only allowed following a function header, any of
the if expression or loop expression keywords, or any binary operator
(most notably `=` and `->`).  In the cases where a new block in the
middle of a statement list is desired, the `block` keyword
can be used to introduce a new scope.

```ante
let x = 42
block
    let x = 3
    let y = 5

print x  //=> prints 42
print y  //=> error, y is not in scope
```

---
## Line Continuation

If a line is too long, it can be split into two with a `\` preceeding
the newline.

```ante
some_very_long_function_name other_similarly_long_argument_name_1 \
                             other_similarly_long_argument_name_2 \
                             other_similarly_long_argument_name_3
```

The spaces preceeding `other_similarly_long_argument_name` 2 and 3 are
not considered to be the start of a line and thus are not significant whitespace.

---
## Indentation Ignored within (), [], and {}

Changes to indentation are ignored while between an opening
parenthesis, bracket, or brace.  This is particularly useful
in long initialization expressions.

```ante
let matrix = [[1, 0, 0, 0],
              [0, 1, 0, 0],
              [0, 0, 1, 0],
              [0, 0, 0, 1]]

let map = ["one" -> 1,
           "two" -> 2,
           "three" -> 3]
```

### Braces can replace indents

If explicit indentation is preferred, `{` and `}` can be used for this
purpose similarly to their counterparts in c-like languages.  Within
blocks delimited by `{` and `}` all indentation is ignored (and consequently
any contained sub-blocks must also use `{` and `}`).  Most of the time,
changes in scope are better and more tersely expressed through indentation.
For this reason, it is not idiomatic to use braces for indentation unless they
are needed in rare cases such as posting source code to a website that may
strip spaces or inputting a one-liner in the terminal.

```ante
fun fact: u32 x -> u32 {
    if x = 1 then{
        1
    }else{
        x * fact (x-1)
    }
}

let s = match fact 5 with
| 120 -> { "fact 5 = 120" }
| n -> { "fact 5 should not equal ${n}" }

type Shape = {
   | Circle f64
   | Square f64
   | Rectangle f64,f64
   | Triangle f64,f64
}

fun area: Shape s = {
    match s with
    | Circle r -> Math.pi * r^2
    | Square l -> l^2
    | Rectangle (w,h) -> w * h
    | Triangle (b,h) -> 0.5 * b * h
}

//invalid
//{
//    let x = 3
//}

//The block keyword is still required for mid-block changes in scope
block {
    let x = 3
}
```
