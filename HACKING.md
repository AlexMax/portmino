- 4 spaces, no tabs.
- All globals should begin with `g_`.
- All typedef types should end in `_t`.
- Any function that allocates and initializes a struct should have a name
  that ends with `_new`.  The inverse is `_delete`.
- Any function that initializes a struct inplace or initializes some global
  state should have a name that ends with `_init`.  The inverse is `_deinit`.
