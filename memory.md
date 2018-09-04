---
title: "Gradual Memory Management"
date: 2018-06-26T22:29:00-05:00
menu: main
---


Pointer type as a trait

```ante
trait PtrTo 't
    fun (@): self -> 't

    fun addr: self -> usz

    fun delete: !out self
```
