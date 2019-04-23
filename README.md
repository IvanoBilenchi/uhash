## uHash - a type-safe, generic C hash table.

### Author

[Ivano Bilenchi](https://ivanobilenchi.com)


### Description

Implementation of a type-safe, generic hash table data structure written in C.
It's a fork of [klib](https://github.com/attractivechaos/klib)'s khash, aimed at extending its API and providing additional configuration options.


### Documentation

Documentation for the project is provided in form of docstrings in the *Public API* section of [uhash.h](include/uhash.h). HTML and LaTeX docs can be generated via [Doxygen](http://www.doxygen.nl). For usage examples, see [test.c](test/test.c).

**Run tests:** `cd test && make run`

**Generate docs:** `doxygen Doxyfile`


### Features

- Hash table primitives (`uhash_get`, `uhash_put`, `uhash_delete`, `uhash_contains`, ...)
- Iteration macros (`uhash_foreach`, `uhash_foreach_key`, `uhash_foreach_value`)
- Map-specific high-level API (`uhmap_get`, `uhmap_set`, `uhmap_remove`, ...)
- Set-specific high-level API (`uhset_insert`, `uhset_remove`, `uhset_is_superset`, ...)


### License

`uHash` is available under the MIT license. See the [LICENSE](./LICENSE) file for more info.
