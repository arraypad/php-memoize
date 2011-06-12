dnl $Id$
dnl config.m4 for extension memoize

PHP_ARG_ENABLE(memoize-apc, whether to enable the APC storage module for memoize,
[  --enable-memoize-apc     Enable memoize APC module])

if test "$PHP_MEMOIZE_APC" != "no"; then
  PHP_NEW_EXTENSION(memoize_apc, memoize_apc.c, $ext_shared)

  PHP_ADD_EXTENSION_DEP(memoize_apc, memoize)
  PHP_ADD_EXTENSION_DEP(memoize_apc, apc)
fi
