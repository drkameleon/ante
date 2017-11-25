---
title: "If"
date: 2017-11-15T14:14:02-05:00
menu: main
---

In Ante if is an expression that can
conditionally do one thing if its condition
is true and otherwise do something else if its
condition is false.  For example, if you wanted
to print "good afternoon" only if the time on the
clock is after 12:00 then you could write:

```ante
if Time.is_after "12:00" then
    print "good afternoon"
```

An else branch can also be added to do something
if the given condition was false.  In this case,
if it is not after 12:00, then it must be the
morning.

```ante
if Time.is_after "12:00" then
    print "good afternoon"
else
    print "good morning"
```

This is somewhat unsatisfactory as it does not account
for nighttime.  If expressions can, however, be made
to include more than two branches with the addition of
one or more elif branchs.  Elif branches are matched if
and only if its condition is true and all the above if
and elif conditions were false.

```ante
//If the current time is between 12:00 and 19:00 (12pm and 7pm)
if Time.now() in 12..19 then
    print "good afternoon"
elif Time.is_after 5 then
    print "good morning"
else
    print "good night!"
```

### Return Value

The return value of an if statement is the return value of the
matched branch.  For this reason, the return type of all branches within
an if expression must match type.  In the case of an if without a
corresponding else, no value is returned if the condition is false,
so the overall return value is always void.

```ante
let int =
    if 5 > 2 then 10
    else 5

let greeting = 
    if Time.now() in 12..19 then
        "good afternoon"
    elif Time.is_after 5 then
        "good morning"
    else
        "good night!"

//returns void even if the condition is always true
if true then
    2

//error, return type of else branch does not match the then branch
let t = if len greeting > 11 then "" else 3
```
