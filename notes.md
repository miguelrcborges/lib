Here I plan to save some notes of what annoys me when working with the C language, 
so later on I try to work on my own programming language, find a way to address
this annoyances.


# No way to embed compile time values into a struct or generate types on the fly

It's hard to explain this properly, so I will just give a specific example.
Let's say I want to create a vector type, base to do SIMD operations.
(SIMD not being implemented manually in this example)

I want there to be some restrictions when adding vectors. For example, vectors
must be on the same size in order to add one with each other.

There would be two ways to verify this at compile time: create a type for
each vector length or have a compile time variable correspondent to the length
of the vector and ensure that, in every call, those values match.

It would be possible to do this with C++ templates. However, it would 
[generate different functions for every possible length](https://godbolt.org/z/1TY6enKje).

I don't know how ergonomically this could be in the language. There could be cases
where a Vector would be dynamically generated and, consequently, size not being
known at compile time. Such cases would still require a length check at runtime.
Maybe the ideal would be doing the length check at both compile time and runtime,
in order to avoid generating twice the same function (one with the runtime check
and one without).
