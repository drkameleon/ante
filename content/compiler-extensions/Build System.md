---
title: "Build System"
date: 2017-11-15T14:33:28-05:00
menu: main
---

The `Build` module defines several useful functions for
compiling and linking applications.  Its interface is as follows:


---
## option

```ante
ante fun option: Str opt
```

Equivalent to passing the string opt while evoking the 
compiler, eg. a call to `option "-O3"` is equivalent
to compiling a module with `ante -O3`

If the given option is not recognized a compilation error
will be issued with the message "Unrecognized option ${opt}"

```ante
ante fun option: CompilerFlag f
```

Enables the compiler flag f.

If the given CompilerFlag requires an argument, such
as the flag OptLevel, a compilation error will be
issued with the message "Flag ${f} requires an argument but none were passed"


```ante
ante fun option: CompilerFlag f, Str value
```

Sets the compiler flag f with the Str value as its argument.

If the given CompilerFlag does not require an argument, such
as the flag Release, a compilation error will be issued with
the message "Flag ${f} requires no arguments but one was passed"

---

## before

```ante
ante fun before: Compiler.Phase p, !implicit void->void f
```

Executes the given function before the given phase begins.

#### Examples
Timing compilation:
```ante
var timer = Timer()
before Compilation (start timer)

before Linking (
    stop timer
    print "All done compiling!"
    print "Only took ${timer}")
```

Building a parser with bison:
```ante
before Compilation (
    sh "bison syntax.y -oparser.c"
    cc "parser.c -oparser.o")
```

---
## sh

```ante
ante fun sh: Str cmd -> i32
```

Executes the given command in the shell and returns
the return status of that command.

#### Examples

```ante
sh "echo Hello"

sh "yacc my_grammar.y"
```

---
## cc

```ante
ante fun cc: Str cmd -> i32
```

Evokes the System's default C compiler with
the given command as an argument.

Returns the status of the C compiler on success

If no C compiler was found, a compilation error
will be issued in the form "No default C compiler found"

---
## cxx

```ante
ante fun cxx: Str cmd -> i32
```

Evokes the System's default C++ compiler with
the given command as an argument.

Returns the status of the C++ compiler on success

If no C++ compiler was found, a compilation error
will be issued in the form "No default C++ compiler found"
