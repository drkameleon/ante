---
title: "Prelude"
date: 2018-03-28T14:30:00-05:00
menu: main
---

The `Prelude` module contains commonly used functions and types
that are imported implicitly into every module.


# Types
---
### Str

A string type that owns its contents.

Strings in Ante are null terminated and contain a length field.

```ante
type Str = c8* cStr, usz len
```

#### Examples
```ante
let s1 = "Hello"
let s2 = Str("Hello".cStr, 5usz)
let s3 = Str(strdup "hi", 2usz)
```

---
### Maybe 't

Represents either the presence of a value with `Some`,
or the absence of a value with `None`.

```ante
type Maybe 't =
   | Some 't
   | None
```

`Maybe` is often used throughout the standard library to
represent failure.  For example, if an allocation can fail
`None` is returned instead of a null pointer.  Unlike null
pointers and error codes, the Maybe type forces the error
case to be checked via a match expression before its value
is used.

#### Examples
```ante
//Parsing integers can fail if the string is
//non-numeric so represent failure with None
fun parse_int: Str s -> Maybe i32
    if not is_numeric s then
        return None
    ...
    Some int

assert <| parse_int "54" == Some 54

//print None
match parse_int "non-numeric string" with
| Some n -> print n
| None -> print "None"
```

---
### Range

Represents a lazy range of values from start
to end incrementing by step each time.  This range
is start-inclusive and end-exclusive.

```ante
type Range = i32 start end step
```

#### Examples
```ante
//print 0 to 9 inclusive
let r = Range(0, 10, 1)
for x in r do print x

//corresponding C code:
//for(int i = 0; i < 10; i += 1){
//    printf("%d\n", i);
//}

//Ranges can also be constructed with ..
let arr = [2, 3, 5, 6, 8]
for i in 0 .. arr.len do
    print (arr#i)
```

---
### InFile

A file that can be read from.

```ante
type InFile = File
```

#### Examples
```ante
let f = InFile "file.txt"

let line1 = f.next_line ()
let line2 = f.next_line ()

for line in InFile "file2.txt" do
    print line
```

---
### OutFile

A file that can be written to.
If the file does not yet exist, a new one will be created.
An already existing file will be overwritten by default
or appended to if the append flag was passed when the OutFile
was made.

```ante
type OutFile = File
```

#### Examples
```ante
let f = OutFile "out.txt"
f.write "Hello!\n"
fclose f

let f_append = OutFile "out.txt" Append 
f_append.write "Hello again!"
```

---
## C Types

Wrappers for some common types in C.

---
### File

```ante
type File = void*
```

---
### FilePos

```ante
type FilePos = void*
```


---
# Traits
---
### Iterable

Types that implement either `Iterable` or `Iterator`
are able to be used in for loops as an iterator.

Types that are `Iterable` are not directly iterated
over but provide a function that returns an object
that can iterate over them.  This is useful for
types that would be too expensive to be copied or
want to prevent mutation and would rather provide
an immutable view that can iterate over them.

```ante
trait Iterable
    fun into_iter: Iterable i -> Iterator
```

---
### Iterator

Types that implement either `Iterable` or `Iterator`
are able to be used in for loops as an iterator.

Iterators are immutable and return a new Iterator
on each run of the loop.

```ante
trait Iterator
    fun next: Iterator i -> Iterator
    fun unwrap: Iterator i -> 't
    fun has_next: Iterator i -> bool
```

Implemented by `Range` and `InFile` in the prelude.

An iterator in a for loop is implicitly translated by
the compiler into a while loop using the functions above.
All for loops in the form:

```ante
for e in iterator do
    ...
```

Are equivalent to the following while loop:
```ante
mut i = iterator
while has_next i do
    let e = unwrap i
    ...
    i = next i
```

#### Example Implementation

```ante
type BackwardsRange = i32 start end

ext BackwardsRange : Iterator
    fun next: BackwardsRange r =
        BackwardsRange(r.start-1, r.end)

    fun unwrap: BackwardsRange r =
        r.start

    fun has_next: BackwardsRange r =
        r.start > r.end
```

---
# Functions
---
### ==

Compares the structural equality of two values.

```ante
fun (==): Maybe l r -> bool
```

```ante
!inline fun (==): 't* l r -> bool
```

```ante
fun (==): void* l r =
    Ante.error "Cannot call == on a void*!  Use 'is' to compare addresses!"
```

```ante
fun (==): Str l r -> bool
```

---
### !=

Equal to `not (a == b)`

```ante
!inline fun (!=): Str l r -> bool
```

---
### is

Compare the identity (referential equality) of two values.

```ante
!inline fun (is): Str l r -> bool
```

---
### Ante.debug

Dumps the type and value of an expression to stdout during compile-time

```ante
ante fun Ante.debug: 't t
```

---
### Ante.sizeof

Returns the size of a type in Bytes.  Accepts types or values as an argument

```ante
ante fun Ante.sizeof: 't t -> usz
```

---
### Ante.store

Store values during compile-time to be retrieved later.
Storing a value with `Ante.store` stores it in an internal table
in the compiler separate to the scope table of variables.  This results
in variables stored with this function being able to be retrieved later
at a possibly different global scope.

Values stored with this function will not be able to be retrieved through
any way except for the function `Ante.lookup`.

```ante
ante fun Ante.store: c8* name, 't val
```

#### Example

```ante
ante fun store_val:
    let pi = 3.14159265
    Ante.store "my val" v

store_val ()

Ante.lookup "my val" ?
    Some n -> print n
    None -> Ante.error "my val not found!"
```

---
### Ante.lookup

Lookup variables stored with Ante.store

Ante.lookup will never reference variables not stored with Ante.store.

```ante
ante fun Ante.lookup: c8* name -> Maybe 't
```

---
### Ante.error

Issues a compilation error with the given message.

This function will mark the function that issued it as having errored
and will cease compilation of the function.

This function should only be used in compile-time code.  If used in
normal code that contains values not known during compile-time (non-ante values),
this function will always be compiled and thus always issue an error even
if it is behind an else branch that may or may not execute.

```ante
ante fun Ante.error: c8* msg
```

#### Example
```ante
type Context =
    bool debug

//load config during compile-time
//useful to change compilation options with a cfg file automatically
ante fun load_config: Str config_name -> Context
    let f = InFile.open config_name ?
        None -> Ante.error "Config file ${config_name} not found"

    let res = f.find_regex "debug = (true|false)"
    bool.parse res ?
        Some b -> Context b
        None -> Ante.error "Config must set debug to either true or false"


let cfg = load_config "config.cfg"
Ante.set_mode (if cfg.debug then Debug else Release)
```

---
### Ante.emitIR

Emits the llvm intermediate representation of the current module to stdout.

```ante
ante fun Ante.emitIR:
```

---
### Ante.forget

Instruct the compiler to forget about a function and remove it from the
list of functions able to be called.  This will not affect any call instances
that were already compiled and already refer to this function.

Useful in the repl to redefine functions

```ante
ante fun Ante.forget: c8* function_name
```

#### Example (repl)

```ante
: fun get_num := 4
: get_num ()
4
: fun get_num := 5
(unknown file): 3,8-14  error: Function get_num was redefined

: Ante.forget "get_num"
: fun get_num := 6
: get_num ()
6
```

---
### printne

Prints a value with no endline.

```ante
!inline fun i8.printne: i8 x
```

```ante
!inline fun i16.printne: i16 x
```

```ante
!inline fun i32.printne: i32 x
```

```ante
!inline fun i64.printne: i64 x
```

```ante
!inline fun isz.printne: isz x
```

```ante
!inline fun u8.printne: u8 x
```

```ante
!inline fun u16.printne: u16 x
```

```ante
!inline fun u32.printne: u32 x
```

```ante
!inline fun u64.printne: u64 x
```

```ante
!inline fun usz.printne: usz x
```

```ante
!inline fun f16.printne: f16 x
```

```ante
!inline fun f32.printne: f32 x
```

```ante
!inline fun f64.printne: f64 x
```

```ante
!inline fun c8.printne: c8 x
```

```ante
!inline fun bool.printne: bool b
```

```ante
!inline fun printne: c8* s
```

```ante
!inline fun printne: Str s
```

```ante
!inline fun printne: 't x
```

The unspecialized `printne:'t` will attempt to cast its argument to a
Str before outputting the Str.

---
### print

Print a value with a trailing newline.

Print uses printne internally and thus only printne needs to
be implemented for a type to be printable.  If a type is not
directly printable, `print:'t` will attempt to convert it
to a `Str` before printing.

```ante
!inline fun print: 't x
```

```ante
!inline fun Str.print: Str s
```

---
### c8\*.init

Performs the cast of a Str to a c8\* c-string.

This cast returns a non-owning reference to
the cStr field of the string.

```ante
!implicit !inline fun c8*.init: Str s -> c8*
```

---
### Str.reverse

Returns a new string that is the reverse
of the given string.

```ante
fun Str.reverse: Str s -> Str
```

#### Example

```ante
assert <| reverse "hello" == "olleh"
```

---
### Str.init

Create a string from the given value.

```ante
fun Str.init: i64 i -> Str
```

```ante
fun Str.init: u64 i -> Str
```

```ante
!inline fun Str.init: i8 x -> Str
```

```ante
!inline fun Str.init: i16 x -> Str
```

```ante
!inline fun Str.init: i32 x -> Str
```

```ante
!inline fun Str.init: isz x -> Str
```

```ante
!inline fun Str.init: u8 x -> Str
```

```ante
!inline fun Str.init: u16 x -> Str
```

```ante
!inline fun Str.init: u32 x -> Str
```

```ante
!inline fun Str.init: usz x -> Str
```

```ante
!inline fun Str.init: c8* cStr -> Str
```

#### Examples

```ante
let five = Str.init 5

let two_thousand = Str.init 2000_u16

//T.init functions are cast functions and
//thus the .init can be omitted
let fifty = Str 50usz

let hi = Str (strdup "hi")
```

---
### u64.parse

Parses an unsigned integer from a string.

If the string is non-numeric or contains
a floating-point or negative number this function
will return None.  Otherwise, it will return Some value.

```ante
fun u64.parse: Str s -> Maybe u64
```

#### Example

```ante
assert <| u64.parse "53" == Some 53_u64

assert <| u64.parse "-5" == None
```

---
### i64.parse

Parses a signed integer from a string.

If the string is non-numeric or contains
a floating-point or negative number this function
will return None.  Otherwise, it will return Some value.

```ante
fun i64.parse: Str s -> Maybe i64
```

#### Example

```ante
assert <| u64.parse "-1000" == Some(-1000_i64)

assert <| u64.parse "23.5" == None
```

---
### in

Returns true if a is in the range r

```ante
fun (in): i32 a, Range r -> bool
```

#### Examples

```ante
3 in 1..5  //=> true

4 in 2..4  //=> false

0 in 0..5  //=> true
```

---
### .\.

Creates an end-exclusive range with a step of 1.

```ante
fun (..): i32 start end -> Range
```

Creates an end-exclusive range with a step equal
to the difference of the first two elements in the range.

```ante
fun (..): i32,i32 first_two, i32 end -> Range
```

#### Examples

```ante
Vec(1..4)  //=> [1, 2, 3]

Vec( (0,5)..25 )  //=> [0, 5, 10, 15, 20]

Vec( (5,4)..0 )  //=> [5, 4, 3, 2, 1]
```

---
### ++

The append operator.  Appends two strings together
and returns a new string leaving the two given strings intact.

```ante
fun (++): Str s1 s2 -> Str
```

#### Examples

```ante
let s = "hello " ++ "world"
assert (s == "hello world")
```

---
### \#

Retrieves the character at the given index from
the string.

```ante
!inline fun (#): Str s, i32 index -> c8
```

Replace the original character at index i with
a new character.

```ante
!inline fun (#): mut Str s, i32 i, c8 c
```

---
### InFile.init

Creates an `InFile` from the given file name.

```ante
!inline fun InFile.init: Str fName -> InFile
```

#### Example

```ante
let f = InFile "file.txt"
```

---
### OutFile.init

Creates an `InFile` from the given file name.

If the file does not exist yet, a new one will be created.

```ante
!inline fun OutFile.init: Str fName -> OutFile
```

#### Example

```ante
let f = OutFile "out.txt"
```

---
### OutFile.write

Write the given text to a file.  If the file
already exists it will be overwritten.

```ante
!inline fun OutFile.write: OutFile f, c8* cStr
```

```ante
!inline fun OutFile.write: OutFile f, c8 c
```

```ante
!inline fun OutFile.write: OutFile f, Str s
```

#### Examples

```ante
let f = OutFile "out.txt"

f.write "Hello"
f.write ' '
f.write "World"
```

---
### InFile.next_line

Returns the next line from a file up to
but not including the newline character.  If
the end of the file is reached an empty string
will be returned on all successive calls.

```ante
fun InFile.next_line: InFile f -> Str
```

```ante
//Given the file "in.txt" containing:
//hi
//hello there

let f = InFile "in.txt"
next_line f  //=> "hi"
next_line f  //=> "hello there"
next_line f  //=> ""
next_line f  //=> ""
```

---
### input

Print a message without a trailing newline to stdout
then get and return user input.

```ante
fun input: c8* msg -> Str
```

#### Example

```ante
let s = input "Enter your name: "
// s now contains input from stdin
```

---
## C Functions
---
### printf

```ante
fun printf: c8* fmt, ... -> i32
```

---
### puts

```ante
fun puts: c8* str -> i32
```

---
### putchar

```ante
fun putchar: c8 char
```

---
### getchar
```ante
fun getchar: -> c8
```

---
### exit

```ante
fun exit: i32 status
```

---
### malloc

```ante
fun malloc: usz size -> void*
```

---
### calloc

```ante
fun calloc: usz num size -> void*
```

---
### realloc

```ante
fun realloc: void* ptr, usz size -> void*
```

---
### free

```ante
fun free: void* mem
```

---
### memcpy

```ante
fun memcpy: void* dest src, usz bytes -> void* /*dest*/
```

---
### system

```ante
fun system: c8* cmd -> i32
```

---
### strlen

```ante
fun strlen: c8* str -> usz
```

---
### fopen

```ante
fun fopen: c8* fName, c8* mode -> File
```

---
### fclose

```ante
fun fclose: File file
```

---
### fputs

```ante
fun fputs: c8* str, OutFile file
```

---
### fputc

```ante
fun fputc: c8 char, OutFile file
```

---
### fgetc

```ante
fun fgetc: InFile file -> c8
```

---
### fgets

```ante
fun fgets: c8* str, i32 numBytes, InFile file -> c8*
```

---
### ungetc

```ante
fun ungetc: c8 c, InFile file -> i32
```

---
### fgetpos

```ante
fun fgetpos: File f, FilePos fp
```

---
### ftell

```ante
fun ftell: File f -> i64
```

---
### fsetpos

```ante
fun fsetpos: File f, FilePos fp
```

---
### fseek

```ante
fun fseek: File f, i64 offset, i32 origin
```

---
### feof

```ante
fun feof: InFile f -> bool
```

---
### ferror

```ante
fun ferror: File f -> bool
```
