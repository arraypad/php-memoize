dnl config.m4 for extension memoize_memcached

PHP_ARG_ENABLE(memoize-memcached, whether to enable the APC storage module for memoize,
[  --enable-memoize-memcached     Enable memoize APC module])

if test "$PHP_MEMOIZE_APC" != "no"; then
  PHP_NEW_EXTENSION(memoize_memcached, memoize_memcached.c, $ext_shared)

  PHP_ADD_EXTENSION_DEP(memoize_memcached, memoize)
  PHP_ADD_EXTENSION_DEP(memoize_memcached, memcached)
fi
