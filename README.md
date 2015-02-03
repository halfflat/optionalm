# optionalm
Include-only library for C++ option types with monadic bindings.

## Overview

The std::option<T> class was proposed for inclusion into C++14, but was
ultimately rejected. (See N3672 proposal for details.) This class offers
similar functionality, namely a class that can represent a value (or
reference), or nothing at all.

In addition, this class offers monadic and monoidal bindings, allowing the
chaining of operations any one of which might represent failure with an unset
optional value.

One point of difference between the proposal N3672 and this implementation
is the lack of constexpr versions of the methods and constructors.

## Usage

An optional<T> value can be in one of two states: it can be _set_, where it
encapsulates a value of type T, or _unset_. In a boolean context, set
optional values are true, and unset ones are false.
```C++
    // Default constructed optional<int> is unset.
    optional<int> a;
    assert((bool)a==false);
    
    // Given a value, optional<T> is true in a boolean context.
    optional<int> b(3);
    assert((bool)b==true)
```
The associated value can be retrieved using the get() method (which throws
an exception if unset), or via operator\*() or operator->(), which do not
check for validity.
```C++
    optional<int> a(3);
    assert(3==a.get());
    assert(3==*a);
    
    optional<array<int,3>> x{{1,2,3}};
    cout << x->size() << "\n";
```
A chain of computations can be performed on an optional value, conditional
on that value being set. The bind method takes a functor and applies it
if the optional value is set, or returns an unset optional value if not.

       

