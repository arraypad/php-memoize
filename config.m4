dnl $Id$
dnl config.m4 for extension memoize

PHP_ARG_ENABLE(memoize, whether to enable memoize support,
[  --enable-memoize           Enable memoize support])

if test "$PHP_MEMOIZE" != "no"; then

  if test "$PHP_APC" = "no"; then
    AC_MSG_ERROR([APC is not enabled! Add --enable-apc to your configure line.])
  fi

  ifdef([PHP_CHECK_APC_INCLUDES],
  [
    PHP_CHECK_APC_INCLUDES
  ],[
    AC_MSG_CHECKING([for APC includes])
    if test -f $abs_srcdir/include/php/ext/apc/apc.h; then
      apc_inc_path=$abs_srcdir/ext
    elif test -f $abs_srcdir/ext/apc/apc.h; then
      apc_inc_path=$abs_srcdir/ext
    elif test -f $prefix/include/php/ext/apc/apc.h; then
      apc_inc_path=$prefix/include/php/ext
    else
      AC_MSG_ERROR([Cannot find apc.h.])
    fi
    AC_MSG_RESULT($apc_inc_path)
  ])


  PHP_NEW_EXTENSION(memoize, memoize.c, $ext_shared,,-I$apc_inc_path -I)
  ifdef([PHP_ADD_EXTENSION_DEP],
  [
    PHP_ADD_EXTENSION_DEP(memoize, apc)
  ])
fi
