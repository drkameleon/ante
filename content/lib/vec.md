---
title: "Vec"
date: 2018-03-28T14:30:00-05:00
menu: lib
---

The `Vec` module contains the vector type, a container for a
dynamically-allocated contiguous region of memory with constant time access
and the ability to resize.


# Types
---
### Vec 't

The main vector type exported by the module.

```ante
type Vec 't =
    data: ref 't
    len: usz
    cap: usz
```

#### Examples

Creating a mutable empty Vec and pushing to it:
```ante
v = mut empty Vec
v.push 2
v.push 4
```

Creating a Vec from an array:
```ante
v = Vec.of [1, 2, 3, 4]
```

Iterating through all elements of a Vec:
```ante
for elem in Vec.of [5, 6, 7] do
    print elem
```

Applying a function to every element of a Vec,
and receiving a new Vec containing the new elements:

```ante
v1 = Vec.of [1, 2, 3, 4]   //[1, 2, 3, 4]
v2 = v1.map (\x = x * 2)   //[2, 4, 6, 8]

square x = x * x
v3 = v2.map square         //[4, 16, 36, 64]
```

Combining each element of the vector in some way.
Eg. Summing them or finding the max element:

```ante
sum vec =
    fold vec 0 (+)

max_elem vec =
    fold vec None (\cur_max elem =
        match cur_max with
        | None -> elem
        | Some n -> max n elem)

//max_elem can alternatively be written using
//reduce, which enforces that the Vec is not empty
//and uses the first element as the initial value for cur_max
max_elem vec =
    reduce vec max
```

---
### VecIter 't

An iterator for a `Vec`.  These aren't typically
constructed directly.  Instead, they are made from
the into_iter function that is called automatically
when a Vec is used in a for loop.

```ante
type VecIter 't = view:(ref 't) idx:usz len:usz
```

#### Examples

Iterating through a Vec:

```ante
vec = Vec.of ["bob", "marissa", "sandra"]

iter = mut into_iter vec
while has_next iter do
    e = unwrap iter
    print e
    iter := next iter

//which is equivalent to the preferred form:
for e in vec do
    print e
```

---
# Traits Impls
---

### Iterable (Vec 't) (VecIter 't)

```ante
impl Iterable (Vec 't) (VecIter 't)
    into_iter (Vec 't) -> VecIter 't
```

Allows the usage of Vec in a for loop.

---
### Iterator (VecIter 't)

```ante
impl Iterator (VecIter 't) 't
    has_next VecIter -> bool

    unwrap (VecIter 't) -> 't

    next (VecIter 't) -> VecIter 't
```

Allows the usage of VecIter in a for loop.

---
# Functions
---

### empty

Initializes an empty vector of any element type.
The vector has a default capacity of 0 elements.

```ante
empty () -> Vec 't
```

#### Examples
```ante
v1 = Vec.empty ()
v2 = Vec.empty ()

//pushing to the Vec determines its element type
v1.push 10     //v1 : Vec i32
v2.push "ten"  //v2 : Vec Str

v1.push "test" //error! cannot push Str to Vec of i32
```

---
### of

Creates a vector of integers with all elements from the given iterable.

```ante
of i:'it -> Vec 'e given Iterable 'it 'e
```

#### Examples

```ante
v = Vec.of [1, 1, 2, 3, 5]

one_to_one_hundred = Vec.of (1..101)

evens = Vec.of <| (2,4)..101

copy = Vec.of v
```

---
### make

Creates a vector filled with count copies of the given argument e.
This function is useful to create multi-dimensional vectors or vectors
that need to be a certain length.

```ante
make count:usz elem:'e -> ret:(Vec 'e) where len ret = count
```

#### Examples

```ante
Vec.make 3 5.78  //=> [5.78, 5.78, 5.78]

Vec.make 4 (Vec.make 3 0)
/* =>  [[0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]] */
```

---
### is_empty

Returns true if the vector is empty.

```ante
is_empty v:Vec -> ret:bool given ret == (len v == 0)
```

#### Examples

```ante
Vec.empty () |> is_empty |> assert

v1 = Vec.of [1]
v2 = Vec.of [2, 3]
assert (not (Vec.empty v1))
assert (not (Vec.empty v2))
```

---
### indices

Returns a `Range` that contains all the indices from the given vector.

```ante
indices v:Vec -> r:Range \
    given r.start == 0   \
      and r.step == 1    \
      and r.end == len v
```

#### Examples

```ante
v = Vec.of ["zero", "one", "two"]

for i in indices v do
    print (v#i)

for (e, i) in (v, indices v) do
    print "element ${i} is ${e}"
```

---
### len

Returns the length of the vector.

```ante
len v:Vec = v.len
```

#### Examples

Checking if a given index is valid:
```ante
v = mut Vec.make (random u8) 3.14

if 50 < len v then
    print (v#50)
```

---
### capacity

Returns the capacity of the vector. This can be useful in determining when a
push will need to reallocate the vector's contents. The capacity is usually
a power of two, although if all the elements were allocated in one operation
(eg. in Vec.of or Vec.make) then the capacity will more generally be
initial_length * 2^n for some n, provided the initial_length is greater than zero.

```ante
capacity v:Vec = v.cap
```

#### Examples

```ante
v1 = mut Vec.empty ()
v1.push 1
v1.push 2
v1.push 3

v2 = mut Vec.of [1, 2, 3]

assert (capacity v1 == 4)  // = 2^2
assert (capacity v2 == 3)  // = len v2 * 2^0

v1.push 4
v3 = Vec.of v2
v3.push 4

assert (capacity v1 == 4)  // = 2^2
assert (capacity v3 == 6)  // = len v2 * 2^1
```

---
### push

Pushes an element onto the end of the vector.  Resizes the vector
automatically if the length is equal to the capacity.  When resizing
occurs, the capacity of the vector is doubled.

```ante
push (v: mut Vec 't) elem:'t -> unit given len v += 1
```

#### Examples

Appending to an existing vector:
```ante
v = mut Vec.empty ()  //=> []
v.push 3              //=> [3]
v.push 5              //=> [3, 5]
v.push 8              //=> [3, 5, 8]
```

Cannot push to an immutable vector:
```ante
v = Vec.empty ()
v.push 3   // error: v is immutable!
```

An immutable vector can be copied, and the copy can be pushed to:
```ante
v = Vec.of [1, 2]

cpy = mut Vec.of v  // same as copy v
cpy.push 3

print cpy  //=> [1, 2, 3]
print v    //=> [1, 2]
```

---
### push_all

Push all elements, copying from the given iterable to the end of the vector.

```ante
push_all (v: mut Vec 't) it:'it -> unit given Iterable 'it, len v += len it
```

#### Examples

```ante
v = mut Vec.of [1, 2, 3]

v.push_all [4, 5, 6]

print v  //=> [1, 2, 3, 4, 5, 6]
```

---
### reserve

Reserve space for numElems additional elements in the given vector.
The newly reserved elements will be uninitialized.  Note that this
function just allocates the space for new elements, it does not actually
fill it with any values or change the length of the vector.

```ante
reserve (v: mut Vec 't) numElems:usz -> unit given capacity v += numElems
```

#### Examples

```ante
buf = mut Vec.empty ()  //capacity = 0
buf.reserve 100         //capacity = 100

//no reallocations occur:
for e in 0..100 do
    buf.push e

//note that the following is preferred over the above
buf = Vec.of (0..100)
```

Newly allocated space is outside the length and is thus inaccessible initailly:

```ante
v = mut Vec.empty () //=> (buf = [], len = 0, cap = 0)
v.reserve 5          //=> (buf = [], len = 0, cap = 5)

v#0        //=> error: 0 >= len v
v.push 3   //=> (buf = [3], len = 1, cap = 5)
v#0        //=> ok: v#0 = 3
```

---
### pop

Pops the last element off and returns it.
This will never resize the vector.
```ante
pop (v: mut Vec 't) -> 't given len v > 0, len v -= 1
```

#### Examples

```ante
queue = mut Vec.of [3, 4]
queue.pop ()   //=> 4
queue          //=> [3]

queue.pop ()   //=> 3
queue.pop ()   //error: len queue > 0
queue          //=> []
```

---
### try_pop

Pops the last element off and returns it if it exists.
This will never resize the vector.
```ante
try_pop (v: mut Vec 't) -> Maybe 't
    given len v != 0 => len v -= 1
```

#### Examples

```ante
queue = mut Vec.of [3, 4]
queue.try_pop ()   //=> Some 4
queue              //=> [3]

queue.try_pop ()   //=> Some 3
queue.try_pop ()   //=> None
queue              //=> []
```


### remove_index

Removes the element at the given index and return it.
```ante
remove_index (v: mut Vec 't) idx:usz -> 't
    given idx < len v, len v -= 1
```

#### Examples

```ante
v = mut Vec.of (10..14)  //=> [10, 11, 12, 13]
v.remove_index 1         //=> 11
v                        //=> [10, 12, 13]

v.remove_index 4         //=> error: 4 >= len v
```

---
### remove_indices

Removes the given indices from the vector.
This function expects the indices to be in sorted order and
will error if any index is out of bounds.

```ante
remove_indices: (v: mut Vec) (indices: Vec usz) -> unit
    given is_sorted indices    \
      and max indices < len v  \
      and len v -= len indices
```

#### Examples

```ante
v = mut Vec.of [5, 6, 7, 8, 9]
v.remove_indices (Vec.of [1, 2, 4])
v  //=> [5, 7, 8]
```

---
### remove_first

Remove the first instance of the given element from
the vector or none if the element was not found.
Uses = to determine element equality.
returns the index where the element was found.

```ante
remove_first (v: mut Vec 't) elem:'t -> Maybe usz
    given elem in v => len v -= 1
```

#### Examples

```ante
v = mut Vec.of [1, 1, 0, 2, 3, 0, 0]
v.remove_first 0   //=> Some 2
v.remove_first 1   //=> Some 0
v                  //=> [1, 2, 3, 0, 0]

v.remove_first 2   //=> Some 1
v.remove_first 4   //=> None
v                  //=> [1, 3, 0, 0]
```

---
### remove_all

Remove all matching elements from the vector and
return the number of elements removed.
Uses = to determine element equality.

```ante
remove_all (v: mut Vec 't) elem:'t -> usz ret
    given len v -= ret
```

#### Examples

```ante
v = mut Vec.of [1, 2, 1, 5, 3, 1]
v.remove_all 1  //=> 3

assert <| v == Vec.of [2, 5, 3]
```
