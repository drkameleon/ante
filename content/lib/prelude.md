---
title: "Prelude"
date: 2018-03-28T14:30:00-05:00
menu: lib
---

The `Prelude` module contains commonly used functions and types
that are imported implicitly into every module.


# Types
---
### Str

A string type that owns its contents.

Strings in Ante are null terminated and contain a length field.

```ante
type Str = cStr:(ref c8) len:usz
```

#### Examples
```ante
s1 = "Hello"
s2 = Str("Hello".cStr, 5usz)
s3 = Str(strdup "hi", 2usz)
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
parse_int s:Str -> Maybe i32 =
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
### LazyRange

Represents a lazy range of values from start
to end incrementing by step each time.  This range
is start-inclusive and end-exclusive.

```ante
type LazyRange =
    start: i32
    end: i32
    step: i32
```

#### Examples
```ante
//print 0 to 9 inclusive
r = Range(0, 10, 1)
for x in r do print x

//corresponding C code:
//for(int i = 0; i < 10; i += 1){
//    printf("%d\n", i);
//}

//Ranges can also be constructed with ..
arr = [2, 3, 5, 6, 8]
for i in 0 .. len arr / 2 do
    print arr#i + arr#(i*2)
```

---
### InFile

A file that can be read from.

```ante
type InFile = File
```

#### Examples
```ante
f = mut InFile "file.txt"

line1 = f.next_line ()
line2 = f.next_line ()

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
f = OutFile "out.txt"
f.write "Hello!\n"
fclose f
```

---
## C Types

Wrappers for some common types in C.

---
### File

```ante
type File = ref unit
```

---
### FilePos

```ante
type FilePos = ref unit
```


---
# Traits
---
### Empty 't

Returns an empty version of the given type.  This is often implemented
by containers to give a default, empty version of them.

```ante
trait Empty 't
    empty (Type 't) -> 't
```

#### Examples

Creating an empty Vec of any element type.

```ante
vec = empty Vec   // Creates an empty Vec 't
push vec "hello"  // 't is inferred to be Str
```

Creating an empty Vec of a given element type.

```ante
intvec = empty (Vec i32)
push intvec "hello"  //error here, cant push Str to Vec i32
```

`empty` is implemented by many containers, for example, there
are also impls for `Empty HashMap` and `Empty Set`. 

```ante
a = empty HashMap
b = empty Set
```

---
### Operator Functions

The prelude defines many traits for various operators, implement
these for your type to emulate operator overloading.

```ante
trait Add 'n
    (+) 'n 'n -> 'n

trait Sub 'n
    (-) 'n 'n -> 'n

trait Mul 'n
    (*) 'n 'n -> 'n

trait Div 'n
    (/) 'n 'n -> 'n

trait Mod 'n
    (%) 'n 'n -> 'n

trait Pow 'n
    (^) 'n 'n -> 'n

trait Cmp 'n
    (<) 'n 'n -> bool
    (>) 'n 'n -> bool
    (<=) l:'n r:'n = not (l > r)
    (>=) l:'n r:'n = not (l < r)

trait Eq 't
    (==) 't 't -> bool
    (!=) l:'t r:'t = not (l == r)

trait Is 't
    (is) 't 't -> bool

trait Append 't
    (++) 't 't -> 't

trait Extract 'col 'index 'elem given 'col 'index -> 'elem
    (#) 'col 'index -> 'elem

trait Insert 'col 'index 'elem
    (#) (mut 'col) 'index 'elem -> unit

trait In 'elem 'collection
    (in) 'elem 'collection -> bool

trait Range 'l 'r 'result given 'l 'r -> 'result
    (..) 'l 'r -> 'result

trait Deref 't 'res given 't -> 'res
    (@) 't -> 'res

trait Neg 't
    neg 't -> 't

trait Not 't
    (not) 't -> 't
```

---
### Print 't

Print the given argument to stdout.  Note that only printne (print with no endline)
needs to be implemented, print is already defined for every type that implements
Print 't.

```ante
trait Print 't
    printne 't -> unit
```

#### Examples

Since most primitive types are printable, implementing Print for your own
type usually consists of deconstructing your type and printing its contents.

```ante
type Person =
    age: u8
    name: Str

impl Print Person
    printne p = printne "${p.name} is ${p.age} years old"
```

---
### To 'a 'b

Casts a value from one type 'a to another type 'b.  The first argument to `to` is the value
to be casted, and the second is the type to cast to.  Note that implementing this
also overloads the type cast operator for the given type combination.

```ante
trait To /*from*/'a  /*to*/'b
    to 'a (Type 'b) -> 'b
```

#### Examples

Enabling casting a Person to an Animal

```ante
type Person = name:Str
type Animal = name:Str species:Str

impl To Animal Person
    to _ p = Animal with
        name = p.name
        species = "homo sapiens sapiens"
```

#### Similar Functions

`to` is meant to be an unfailable cast, for a cast that can fail, you
can implement `TryCast` and use `try_cast` instead.

```ante
trait TryCast 'a 'b
    try_cast 'a (Type 'b) -> Maybe 'b

"hello!"
|> try_cast i32     // Returns Maybe i32
|> print
```

If you don't want to specify the type when casting, you can use
`autocast` to try to have the type system inference the type for
you.  This function is automatically implemented for each `To` impl.
Note that this isn't always possible as there may not be
enough information to decide the return type.

```ante
autocast 'a -> 'b given To 'a 'b

vec = autocast [1, 2, 4, 6]  // vec : 'u
Vec.push vec 32              // vec : Vec i32
```

---

### Iterable 't

Types that implement either `Iterable` or `Iterator`
are able to be used in for loops as an iterator.

Types that are `Iterable` are not directly iterated
over but provide a function that returns an object
that can iterate over them.  This is useful for
types that would be too expensive to be copied or
want to prevent mutation and would rather provide
an immutable view that can iterate over them.

```ante
trait Iterable 't 'it given 't -> 'it
    into_iter 'i -> 'it
```

---
### Iterator 't

Types that implement either `Iterable` or `Iterator`
are able to be used in for loops as an iterator.

Iterators are immutable and return a new Iterator
on each run of the loop.

```ante
trait Iterator 'it 'elem given 'it -> 'elem
    next 'it -> 'it
    unwrap 'it -> 'elem
    has_next 'it -> bool
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
i = mut iterator
while has_next i do
    e = unwrap i
    ...
    i := next i
```

#### Example Implementation

```ante
type BackwardsRange = start:i32 end:i32

impl Iterator BackwardsRange i32
    next r =
        BackwardsRange(r.start-1, r.end)

    unwrap r = r.start

    has_next r =
        r.start > r.end
```

---
# Functions
---

### Ante.debug

Dumps the type and value of an expression to stdout during compile-time

```ante
ante debug 't -> unit
```

---
### Ante.sizeof

Returns the size of a type in Bytes.  Accepts types or values as an argument

```ante
ante sizeof 't -> usz
```

#### Examples

```ante
sizeof i32 == 4
sizeof usz == sizeof (ref 't)
sizeof (i8, u16) == 3
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
ante error (msg: Str) -> 'never_returns
```

#### Example
```ante
type Context = debug:bool

//load config during compile-time
//useful to change compilation options with a cfg file automatically
ante load_config config_name:Str -> Context
    f = InFile.open config_name ?
        None -> Ante.error "Config file ${config_name} not found"

    res = f.find_regex "debug = (true|false)"
    match try_parse res bool with
    | Some b -> Context b
    | None -> Ante.error "Config must set debug to either true or false"


cfg = load_config "config.cfg"
Ante.set_mode (if cfg.debug then Debug else Release)
```

---
### Ante.emit_ir

Emits the llvm intermediate representation of the current module to stdout.

```ante
ante emit_ir unit -> unit
```

---
### Ante.forget

Instruct the compiler to forget about a function and remove it from the
list of functions able to be called.  This will not affect any call instances
that were already compiled and already refer to this function.

Useful in the repl to redefine functions

```ante
ante forget (function_name: ref c8) -> unit
```

#### Example (repl)

```ante
: get_num () = 4
: get_num ()
4
: get_num () = 5
(unknown file): 3,8-14  error: Function get_num was redefined

: Ante.forget "get_num"
: get_num () = 6
: get_num ()
6
```

---
### print

Print a value with a trailing newline.

Print uses printne internally and thus only printne needs to
be implemented for a type to be printable.

```ante
print 't -> unit given Print 't
```

#### Examples

Formatting a string via string interpolation then printing it to stdout.

```ante
print "My favourite number is ${random i32}!"
```

---
### Str.reverse

Returns a new string that is the reverse
of the given string.

```ante
reverse s:Str -> Str
```

#### Example

```ante
reverse "hello" == "olleh"
```

---
### u64.parse

Parses an unsigned integer from a string.

If the string is non-numeric or contains
a floating-point or negative number this function
will return None.  Otherwise, it will return Some value.

```ante
parse s:Str -> Maybe u64
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
parse s:Str -> Maybe i64
```

#### Example

```ante
assert <| u64.parse "-1000" == Some(-1000_i64)

assert <| u64.parse "23.5" == None
```

---
### OutFile.write

Write the given text to a file.  If the file
already exists it will be overwritten.

```ante
write (f: OutFile) (s: 's) -> unit given To 's Str
```

#### Examples

```ante
f = OutFile "out.txt"

f.write "Hello"
f.write ' '
f.write 32
```

---
### InFile.next_line

Returns the next line from a file up to
but not including the newline character.  If
the end of the file is reached None
will be returned on all successive calls.

```ante
next_line f:InFile -> Maybe Str
```

```ante
//Given the file "in.txt" containing:
//hi
//hello there

f = InFile "in.txt"
next_line f  //=> Some "hi"
next_line f  //=> Some "hello there"
next_line f  //=> None
next_line f  //=> None
```

---
### input

Print a message without a trailing newline to stdout
then get and return a string containing user input
up until a newline character is received.

```ante
input (msg:Str) -> Str
```

#### Example

```ante
s = input "Enter your name: "
// s now contains input from stdin
```

---
## C Functions
---
### printf

```ante
printf (fmt: ref c8) ... -> i32
```

---
### puts

```ante
puts (ref c8) -> i32
```

---
### putchar

```ante
putchar c8 -> i32
```

---
### getchar
```ante
getchar unit -> c8
```

---
### exit

```ante
exit (status: i32) -> 'never_returns
```

---
### malloc

```ante
malloc size:usz -> ref 't
```

---
### calloc

```ante
calloc num:usz size:usz -> ref 't
```

---
### realloc

```ante
realloc (ptr: ref 't) size:usz -> ref 't
```

---
### free

```ante
free (ref 't) -> unit
```

---
### memcpy

```ante
memcpy (dest: ref 't) (src: ref 't) bytes:usz -> (dest: ref 't)
```

---
### system

```ante
system (cmd: ref c8) -> i32
```

---
### strlen

```ante
strlen (ref c8) -> usz
```

---
### fopen

```ante
fopen (fName: ref c8) (mode: ref c8) -> File
```

---
### fclose

```ante
fclose File -> i32
```

---
### fputs

```ante
fputs (ref c8) OutFile -> i32
```

---
### fputc

```ante
fputc c8 OutFile -> i32
```

---
### fgetc

```ante
fgetc InFile -> c8
```

---
### fgets

```ante
fgets (dest_str:ref c8) num_bytes:i32 InFile -> ref c8
```

---
### ungetc

```ante
ungetc c8 InFile -> i32
```

---
### fgetpos

```ante
fgetpos File FilePos -> unit
```

---
### ftell

```ante
ftell File -> i64
```

---
### fsetpos

```ante
fsetpos File FilePos -> unit
```

---
### fseek

```ante
fseek File offset:i64 origin:i32 -> unit
```

---
### feof

```ante
feof InFile -> bool
```

---
### ferror

```ante
ferror File -> bool
```
