---
title: "Modules"
date: 2017-11-15T14:11:33-05:00
menu: main
---

Modules provide a method to separate implementation from interface.
They may contain types, traits, functions, and global variables, although
the latter is discouraged.  Before importing a module, the modules names
can be accessed through indexing the hierarchy of modules with the `.` operator.
For example, accessing a function `f` in the module `SubModule` which itself
is within the module `Module` can be done with `Module.SubModule.f`.  This
is the primary method to access other modules, with the one exception that
the current module has all of its names already imported, thus if the current
module is the `SubModule` above then the function can be accessed with just `f`.

---
## Directory Structure

By default, every file is wrapped in a module whose name matches the
file's name but in PascalCase.  It is standard practice to have file
names be in lowercase or snake\_case.  If the name is not translated
correctly into PascalCase it can be changed within a module expression.

Ante converts a directory of source files into a hierarchy of modules
with the following rules:

0. All paths are relative from the file containing the program's start point.
1. Folder names are converted into module names
2. Files inside folders are converted into submodules of the folder module
3. If a file's name matches the folder it is in, it is the implementation
for the folder's module


For example the following directory tree will produce the corresponding
modules on the right.

```
.
|- driver.an       -> Driver
|- helper.an       -> Helper
|- folder          -> Folder
   |- folder.an    -> Folder
   |- helper.an    -> Folder.Helper
   |- subfolder    -> Folder.Subfolder
      |- file.an   -> Folder.Subfolder.File
```

---
## Imports

Importing a module brings all of its names into scope and is not required
to use the module.  A module's names can still be accessed with the
`Module.name` syntax afterward.

If an imported module's names conflict with a module already imported
then an error is issued.  In this case, the error can be resolved by
either renaming one module, importing only the desired functions, or
not importing the module at all.

```ante
import Math

Math.sqrt 16  //=> 4.0
sqrt 16  //=> 4.0
```

Import expressions actually accept a variety of arguments from lists of
modules to strings and lists of strings.  They can also be overloaded like
other operators to import special files in different ways.  The arguments
to import must be compile-time constants, and if they are not then import attempts
to execute them as such (thus `import` is equivalent to `import ante`)

```ante
//import a file from a string
import "../file_in_parent_dir.an"

//import multiple files
import "file1.an", "file2.an", File3, File4

//import arbitrary string expressions:
import "quile.an".replace "qu" "f"

import many_files()

//Ask for which file to import during compile-time
import input "Input the file to import: "
```

Overloading import for different types is done like any other operator.
The following will import a c source file, assuming the functions parse
and decls exist.

```ante
type C = Str file lib

//import a c src file
fun (import): C src
    //import directly from the ast
    import parse src
    Ante.Compiler.link_with src.lib

import C("file.h", "libname")
```

---
## Module Expressions

The module expression is a special expression that is run before
the main body of the file is executed.

A module expression can optionally include a module name.  If one
is included it is taken to be the name of a submodule that is
contained within the expression.  If no name is specified, then
the module expression refers to the current module.

```ante
//in file File.an

//four can use inc that is declared later because module
//expressions are executed before the main body of a file
let four = inc 3

let six = Util.double 3

module
    let inc = (+1)


module Util
    let double = (*2)
```

The fact that module expressions are run before the main body of
a file may seem odd at first but it is helpful when managing
imports and exports.  Typically stating the imports and
exports of a module is done at the top of a file using functions
such as `export` that would typically not be run when the compiler
is only looking for function/type/trait declarations to import.
Putting these functions within a module expression would cause them
to run and expand into the desired function declarations to import.
In this way, a module expression can be thought of as an `ante` block
that is run while importing a file.

```ante
module
    //Every module that imports this module
    //will now also import Math
    import Math

    //export function is run while importing this file
    export inc

let inc = (+1)
```

---
## Visibility

The declarations visible to external modules are handled through the
visibility modifiers (see the [modifiers](../modifiers) section).

Alternatively, functions can be used to manually export functions.
In the following example, public exports are created with the export
function.

```ante
module
    let add = (+)
    let sub = (-)
    let mul = (*)
    let div = (/)
    export add sub mul div

    //not exported
    let rem = (%)
```

---
## Type Extensions

Type extensions can be used to extend modules as well as types.  Adding
new functions to a module is as simple as creating an extension with those
functions.

```ante
ext Math
    fun fact: Nat n =
        if n == 0 then 1
        else n * fact (n - 1)

    fun zero := 0
```
