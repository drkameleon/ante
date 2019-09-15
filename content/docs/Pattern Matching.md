---
title: "Pattern Matching"
date: 2017-11-15T14:14:23-05:00
menu: main
---

Pattern matching is a powerful tool that compares a value
against a several patterns until it finds one that matches.
These patterns can range from simple literal values to a
complex assortment and combination of other patterns,
most of which are covered below.

Perhaps most importantly, pattern Matches with are completeness
checked, so all possibilities for a given value must be covered.
For example, a match against an integer x with a single
pattern `0` (the equivalent of if x = 0) is rejected
as it does not have a case for when x != 0.

---
# Patterns

Every match expression must match a value against
a list of patterns that are to be matched in order of
declaration.  There are several basic types of patterns:
type cast patterns, operator patterns, destructures, clauses.
Additionally, each type of pattern can be used within other patterns.

---
## Type Cast Pattern

Type cast patterns are in the form `Type val` and are matched
if val's type is Type.  This is how each possible value of a
tagged-union types is matched against.  For example, the type
`Maybe 't` is one of two values: `None` or `Some 't`.  In pattern
matching syntax, this translates to:

```ante
//typeof t = Maybe i32
t = Some 2

match t with
| None -> print "none :("
| Some val -> print val
```

---
## Operator Patterns

A useful operator for patterns is the range operator `..` which can
be used to test if a value is in a given range.

```ante
match 103 with
| -9..9 -> "1 digit"
| -99..99 -> "2 digits"
| _ -> "> 2 digits"
```

The append operator can also be used to check if a value has a
given prefix or suffix:

```ante
match "my string" with
| "my " ++ item -> "I have a " ++ item
| "your " ++ item -> "You have a " ++ item
| _ -> "Someone has something.  Probably."
```

---
## Clauses

Adding a clause in a pattern causes the pattern to only be
matched when the clause is true.  Clauses use the `given`
keyword and are always boolean expressions.  A clause also
has access to the bindings from the pattern it acts upon.

```ante
match 5 with
| small given small < 3 -> "small!"
| big   given big  > 10 -> "big!"
| _ -> "moderate."


match (3, "four") with
| (x, "four") given x > 5 -> ...
| (3, s) given len s > 3 -> ...
| _ -> ...
```

Much of the time a clause can be replaced with an inline
boolean expression.

```ante
match 5 with
| _ < 3 -> "small!"
| _ > 10 -> "big!"
| _ -> "moderate."


match (3, "four") with
| (x > 5, "four") -> ...
| (3, len s > 3) -> ...
| _ -> ...
```

In match expressions any unbound identifier is bound to a value
based on its position in the pattern.  For example, `s` above refers
to the second index of the tuple type `(i32, Str)`, and thus is a
`Str` type.  `len` is already bound to a function, so it is not
rebound.  The explicit `given` clause can help prevent the ambiguity
that arises if it is unclear if `len` or `s` are already bound.


---
## Multiple Patterns

`|` can be placed between two patterns so that its
branch is matched if either of those patterns are true.
It is essentially the `or` operator for patterns.

```ante
fib n =
    match n with
    | 0 | 1 -> 1
    | _ -> fib (n-1) + fib (n-2)
```

---
## Parameter Patterns

Function parameters themselves are also patterns.  This is best
shown via example.  The fib function above can be rewritten as follows:

```ante
fib (0|1) = 1
  | n = fib (n-1) + fib (n-2)
```

---
# Return Value

A match expression shares much of the same semantics of an
if expression in that the types of its branches must match
and the value returned is the value of the matched branch.

```ante
assert (3 == match "hi" with
     | "yo" -> 1
     | "hi" -> 3
     | _    -> 5)
```
