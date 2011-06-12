dnl $Id$
dnl config.m4 for extension memoize

PHP_ARG_ENABLE(memoize, whether to enable memoize support,
[  --enable-memoize           Enable memoize support])

if test "$PHP_MEMOIZE" != "no"; then
  PHP_NEW_EXTENSION(memoize, memoize.c, $ext_shared)
  PHP_INSTALL_HEADERS([ext/memoize], [php_memoize.h php_memoize_storage.h])
fi
