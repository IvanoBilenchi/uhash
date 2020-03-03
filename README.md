## uHash - a type-safe, generic C hash table.

### Author

[Ivano Bilenchi](https://ivanobilenchi.com)

### Description

Implementation of a type-safe, generic hash table data structure written in C.
It's a fork of [klib](https://github.com/attractivechaos/klib)'s khash, aimed
at extending its API and providing additional configuration options.

### Features

- Hash table primitives (`uhash_get`, `uhash_put`, `uhash_delete`, `uhash_contains`, ...)
- Iteration macros (`uhash_foreach`, `uhash_foreach_key`, `uhash_foreach_value`)
- Map-specific high-level API (`uhmap_get`, `uhmap_set`, `uhmap_remove`, ...)
- Set-specific high-level API (`uhset_insert`, `uhset_remove`, `uhset_is_superset`, ...)

### Usage

If you are using [CMake](https://cmake.org) as your build system, you can add `uHash` as
a subproject, then link against the `uhash` target. Otherwise, in general you just need
the [uhash.h](include/uhash.h) header.

### Documentation

Documentation for the project is provided in form of docstrings in the *Public API* section
of [uhash.h](include/uhash.h). You can also generate HTML docs via CMake, though you will
also need [Doxygen](http://www.doxygen.nl). For usage examples, see [test.c](test/test.c).

### CMake targets

- `uhash`: interface library target, which you can link against.
- `uhash-docs`: generates documentation via Doxygen.
- `uhash-test`: generates the test suite.
- `uhash-bench`: generates benchmarks.

### License

`uHash` is available under the MIT license. See the [LICENSE](./LICENSE) file for more info.
