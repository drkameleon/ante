---
title: "Build System"
date: 2017-11-15T14:33:28-05:00
menu: main
---

The `Build` module defines several useful functions for
compiling and linking applications.  It defines a DSL for
building programs centered around the creation of targets.
Each target represents a single output file and contains a list
of dependencies and a build action.  If any of its dependencies
are changed the build action is executed.

Although this build system is defined in Ante it can also be
used to build programs for abitrary languages and has built-in
support (automatic dependency management) for Ante, C, and C++.

---
## Target

```ante
ante type Target = Str name, Vec Target* depends, Maybe Expr action
```

Represents a build target.  Each Target can be referenced with the
`target` function and associate actions with `=`.  If the target's
list of dependencies are rebuilt the target will be rebuilt.

---
## target

```ante
ante fun target: Str name -> Target
```

Retrieve a reference to the given target.  If no target was found
matching the given name an empty target will be created.

### Examples
Get and create the target "hello":
```ante
target "hello" =
    print "Hello, build system!"
```

---
## =

```ante
ante fun (=): mut Target t, Expr action
```

Associates a target with an action to perform upon building.
This function will issue a compile-time error if the given
target already has an action associated.


### Examples
Compiling a C file to an executable:
```ante
import Build

target "hello" =
    cc "hello.c -o hello"
```

Compiling all C++ files in a directory to an object file and linking
them with the current ante program:
```ante
import Build, File

let cxxflags = "-DNDEBUG -Wall -O3"
let cppfiles = glob "*.cpp"
let objfiles = cppfiles.replace_all ".cpp" ".o"

default_target "program"

target "program" =
    link_with objfiles
    compile "program.an"

target objfiles =
    cxx "${cxxflags} ${target} ${deps}"
```

Note that most of the time when compiling with C/C++ it is easier to use
the C/C++ FFI support provided by the modules `Cpp` and `C`.  These modules
overload imports of .cpp and .c files to auto-import symbols and link with
libraries automatically.  It will still be necessary to have a build
file to build the C++ or C programs if they are not built already however.

---
## run

```ante
ante fun run: Target target

ante fun run: Str target_name
```

Executes the given target if any of its dependencies have been changed
since the last time it was executed.

### Examples
Run a target conditionally:
```ante
if argv#1 == "build" then
    run "build"
else
    run "build_and_run"
```

---
## default_target

```ante
ante fun default_target: Str target_name
```

Sets a target as the default target when the program is run with no
arguments.  If a target with the given name does not exist when the
program is run, an error will be issued during compile-time.


### Examples
```ante
//In file build.an
default_target "debug"

//build debug with "ante build.an"
target "debug" =
    option "-debug"
    compile "file.an"

//build release with "ante build.an -argv release"
target "release" =
    option "-release"
    compile "file.an"
```

---
## depend_on

```ante
ante fun depend_on: mut Target t, Vec Str depends

ante fun depend_on: mut Target t, Str depend
```

Appends a list of files or targets to the dependencies of the given target.
When used within the action clause of a target the Target argument is implicitly
given.

`depend_on` usually does not need to be manually called unless a build command
does not automatically detect and add dependencies or an additional file that
is not explicitely used should be depended on.


### Examples
```ante
let t = target "build"

t.depend_on ["file1.an", "test"]

target "test" =
    //same as (target "test").depend_on "tests.txt"
    depend_on "tests.txt"
    compile "unittests.an"
```

---
## compile

```ante
ante fun compile: Str file -> Maybe Str

ante fun compile: Str file, Arr Str args -> Maybe Str

ante fun compile: Str file, Arr CompilerFlag args -> Maybe Str
```

Compile an ante source file 

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
mut timer = Timer()
before Compilation (start timer)

before Linking (
    stop timer
    print "All done compiling!"
    print "Only took ${timer}")
```

Building a parser with bison:
```ante
before Compilation (
    system "bison syntax.y -oparser.c"
    cc "parser.c -o parser.o")
```

Note: if the flag Compilation is passed and the call to
`before` is encountered while the program is already in the
code generation phase then the body will be executed immediately.

---
## link_with

```ante
ante fun link_with: Str lib
```

Adds the given library path to the list of files to
link with.  The given string can be either an
object file or a system library name such as libc.

If used within the action clause of a target then the given
library or list of files is automatically added to the
depends list of the current target.

#### Examples

```ante
link_with "libc"

cc "file.c -o file.o"
link_with "file.o"
```

---
## cc

```ante
ante fun cc: Str cmd -> i32
```

Evokes the System's default C compiler with
the given command as an argument.

If used within the action clause of a target this
function will automatically add the implicit dependencies
of the compiled file by analyzing the files opened by the
invoked compiler.

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

If used within the action clause of a target this
function will automatically add the implicit dependencies
of the compiled file by analyzing the files opened by the
invoked compiler.

Returns the status of the C++ compiler on success

If no C++ compiler was found, a compilation error
will be issued in the form "No default C++ compiler found"
