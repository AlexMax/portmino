- 4 spaces, no tabs.
- All globals should begin with `g_`.
- All typedef types should end in `_t`.
- Any function that allocates and initializes a struct should have a name
  that ends with `_new`.  The inverse is `_delete`.
- Any function that initializes a struct inplace or initializes some global
  state should have a name that ends with `_init`.  The inverse is `_deinit`.
- Code should be written in a widely-understood dialect of C.  In practice,
  this means sticking with C89, but with a few widely-implemented or
  easy-to-shim additional features, such as:
  - C++-style comments (`//`).
  - Variable declarations in places other than the top of a scope.
  - `<stdint.h>` and `<stdbool.h>` types.
  - Length-limited string functions such as `snprintf`.
  - `restrict` type qualifier.
