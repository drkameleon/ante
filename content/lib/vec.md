---
title: "Vec"
date: 2018-03-28T14:30:00-05:00
menu: lib
---

The `Vec` module contains the vector type, a container for a
contiguous region of memory with constant time access.


# Types
---
### Vec 't

```ante
type Vec 't = 't* data, usz len cap
```

---
### VecIter 't

An iterator for a `Vec`.

```ante
type VecIter 't = 't* view, usz idx len
```

---
# Traits
---

### Vec : Iterable

```ante
ext Vec : Iterable
    fun into_iter: Vec 't v -> VecIter 't
```

---
### VecIter : Iterator

```ante
ext VecIter : Iterator
    fun has_next: VecIter v -> bool

    fun unwrap: VecIter 't v -> 't

    fun next: VecIter 't v -> VecIter 't
```

---
# Functions
---
    
### Vec.init

Initializes a vector of the given type.
This vector has a default capacity of 4 elements.

```ante
fun init: -> Vec 't
```

Create a vector of integers with all elements from the given range.

```ante
fun init: Range r -> Vec i32
```

---
### is_empty

Returns true if the vector is empty.

```ante
fun is_empty: Vec v -> bool
```

---
### indices

Returns a `Range` that contains all the indices from the given vector.

```ante
fun indices: Vec v -> Range
```

---
### len

Returns the length of the vector.

```ante
fun len: Vec v -> usz
```

---
### capacity

Returns the capacity of the vector.

```ante
fun capacity: Vec v = v.cap
```

---
### fill

Fill the given vector with all items from the Range

```ante
fun fill: mut Vec i32 v, Range r -> Vec i32
```

---
### reserve

Reserve numElements additional elements in the given vector.
The newly reserved elements will be uninitialized.
```ante
fun reserve: mut Vec 't v, usz numElems
```

---
### push

Pushs an element onto the end of the vector.  Resizes if necessary.

```ante
fun push: mut Vec 't v, 't elem
```

---
### pop

Pops the last element off and return it if it exists.
This will never resize the vector.
```ante
fun pop: mut Vec 't v -> Maybe 't
```

---
### remove_index

Removes the element at the given index and return it.
This will error if the index is out of bounds.
```ante
fun remove_index: mut Vec 't v, i32 idx -> 't
```

---
### remove_indices

Removes the given indices from the vector.
This function expects the indices to be in sorted order and
will error if any index is out of bounds.

```ante
fun remove_indices: mut Vec 't v, Vec i32 indices
```

---
### remove_first

remove the first instance of the given element from
the vector or none if the element was not found.
Uses = to determine element equality.
returns the index where the element was found.

```ante
fun remove_first: mut Vec 't v, 't elem -> Maybe i32
```

---
### remove_all

Remove all matching elements from the vector and
return the number of elements removed.
Uses = to determine element equality.

```ante
fun remove_all: mut Vec 't v, 't elem -> usz
```

#### Examples

```ante
mut v = Vec[1, 2, 1, 5, 3, 1]
v.remove_all 1  //=> 3

assert <| v = Vec[2, 5, 3]
```
