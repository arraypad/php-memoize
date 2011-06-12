dnl $Id$
dnl config.m4 for extension memoize

PHP_ARG_ENABLE(memoize-apc, whether to enable the APC storage module for memoize,
[  --enable-memoize-apc     Enable memoize APC module])

if test "$PHP_MEMOIZE_APC" != "no"; then
 
  AC_MSG_CHECKING([for APC])
  if test "$PHP_APC" = "yes" && test "$PHP_APC_SHARED" == "no"; then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_ERROR([memoize_apc is only supported if both APC and memoize are compiled statically])
  fi

  PHP_NEW_EXTENSION(memoize/storage/apc, memoize_apc.c, $ext_shared)

  PHP_ADD_EXTENSION_DEP(memoize/storage/apc, memoize)
  PHP_ADD_EXTENSION_DEP(memoize/storage/apc, apc)
fi
