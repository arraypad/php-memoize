dnl config.m4 for extension memoize_memory

PHP_ARG_ENABLE(memoize-memory, whether to enable the memory storage module for memoize,
[  --enable-memoize-memory     Enable memoize memory storage module])

if test "$PHP_MEMOIZE_MEMORY" != "no"; then
  PHP_NEW_EXTENSION(memoize_memory, memoize_memory.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(memoize_memory, memoize)
fi
