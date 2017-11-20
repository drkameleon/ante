---
title: "Modifiers"
date: 2017-11-15T14:10:13-05:00
menu: main
---

pub, pri, pro,
const
raw
noinit
global
ante


Modifiers can appear before and modify any declaration (function, variable, or type),
any type, or any arbitrary expression.

For example, the `ante` modifier used before a function marks the function's parameters
and its output as compile-time constants and the function itself to be executed during
compile-time.  Before a type declaration, it signifies the value of the type should
always be known during compile-time.  Before a variable it signifies the variable should
be known during compile-time, and before an expression it will evaluate the expression
during compile-time.  Consider the type of an integer literal in Ante:

```ante
ante type IntLit = Int
```

Note that because modifiers can also apply to types directly the above is equivalent to:

```ante
type IntLit = ante Int
```

That is, an integer literal is an arbitrarily sized integer that is always known during
compile time.  This enables the compiler to perform implicit conversions on integers
safely without any runtime confusion or slowdowns.  For example, in time-sensitive code,
a function may take in an `i32` and Ante will convert the `IntLit` to an `i32` provided
it is small enough without ever needing to store the arbitrarily sized integer during runtime.
For the reverse, if an arbitrarily sized integer is needed, it will be converted into an
`Int` and simply lose its `ante` modifier.  Implicit conversions are a strong tool that
can easily lead to hard to find mistakes if its semantics aren't clear.  For this reason,
implicit conversions are defined manually for a given type.
