---
title: "Loops"
date: 2017-11-15T14:15:21-05:00
menu: main
---

Ante provides two forms of loops: while and for.

---
# While

While loops are the most basic form of loop, they
continue executing their body while the given
condition remains true.  The syntax is as follows:

```
while <expr> do <expr>
```

The first expression is the loop condition and must
evaluate to a `bool`.  The second expression is the
body to loop over, and is usually a block.  While loops
are often used when you are unsure precisely how many
times you should loop.  In the following example, a while
loop is used to continue asking the user to input until
they input something other than "example".

```ante
mut inp = input "Enter anything except 'example': "
while inp == "example" do
    inp = input "No no no, DON'T enter 'example': "
```

### Do While

In many languages, the above could be simplified with a 'do-while'
loop that always runs at least once and then checks the loop
condition to see if it should jump back to the top.
While Ante has no concept of do while loops, this is no problem
in practice as the same effect can be achieved by sequencing
the loop body with the loop's condition inside the while condition
and having an empty loop body.  Here is the previous example using
that form.

```ante
while
    let inp = input "Enter anything except 'example': "
    inp == "example"
do ()
```

### Break and Continue

In many cases, it is necessary to either break out of a loop or
continue execution from the beginning within the body of a loop.
The `break` and `continue` keywords serve this purpose.

`break` jumps to the end of a loop unconditionally.  For this
reason it is often used in conjunction with `while true do ...`
loops that cannot be refactored to include a proper condition.

```ante
while true do
    print "Help, I'm stuck in an infinite loop!"
    break //oh, nevermind
```

`continue` jumps to the beginning of the loop.  It is worth noting
that after this jump occurs, the loop condition is checked again and
if that is not met the loop will end normally.  This ensures execution
never continues past the normal end of a loop.

```ante
//print even numbers from 1 to 100
mut i = 0
while i <= 100 do
    i += 1

    //continue if i is odd
    if i % 2 != 0 then
        continue

    print i
```

Both `break` and `continue` can optionally take an argument of how many
nested loops to jump out of.  For example, to break out of two nested
loops one could `break 2`.  It is a compile-time error to jump out of
an invalid number of loops.  The default behaviour of break/continue without any
arguments is `break 1` and `continue 1` respectively.

```ante
mut i = 1
while true do
    mut j = 1
    //print multiplication table's column for i
    while true do
        print "${i} * ${j} = ${i * j}"

        //Jump out of both loops if i=50 and j=58
        if i == 50 and j == 58 then
            break 2
```

---
# For

In contrast with while loops, for loops are used for iterating over a set
of something.  Specifically, for loops can iterate over anything that is
`Iterable`.  In practice, this includes things like arrays, ranges, and most
containers, although any type can be iterated over if it implements either
the `Iterable` or `Iterator` traits.

Here is the print even numbers example using a for loop:
```ante
//print even numbers from 1 to 100
for i in 1..100 do
    if i % 2 == 0 then
        print i
```

Even better, using a range with a step of 2:
```ante
for i in (2,4)..100 do
    print i
```


## Iterable

```ante
fun into_iter: Iterable i -> Iterator
```

Types that are iterable define a single function, `into_iter`
that simply returns an iterator for that type.  Types that
are iterable include `Vec`, `Arr`, `Map`, and most other container types.

## Iterator

```ante
fun has_next: Iterator i -> bool

fun next: Iterator i -> Iterator

fun unwrap: Iterator i -> 't
```

Iterators manage the actual iteration of the for loop.  In Ante,
this is done  by having every for loop be translated under the hood
into a while loop using the functions above.  For example, the following loop:

```ante
for e in iterator do ...
```

Is equivalent to:
```ante
mut it = iterator
while has_next it do
    let e = unwrap it
    ...
    it = next it
```

Similarly, looping over an `Iterable` just requires `into_iter` to
be called to get the iterator used:
```ante
for e in iterable do ...
```

Is equivalent to:
```ante
mut it = into_iter iterable
while has_next it do
    let e = unwrap it
    ...
    it = next it
```

