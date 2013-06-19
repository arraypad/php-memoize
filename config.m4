dnl $Id$
dnl config.m4 for extension memoize

PHP_ARG_ENABLE(memoize, whether to enable memoize support,
[  --enable-memoize           Enable memoize support])

PHP_ARG_ENABLE(memoize-apc, whether to enable the APC storage module for memoize,
[  --enable-memoize-apc     Enable memoize APC module], no, no)

PHP_ARG_WITH(memoize-memcached, whether to enable the memcached storage module for memoize,
[  --with-memoize-memcached[=libmemcached_prefix]     Enable memoize memcached storage module], no, no)

PHP_ARG_ENABLE(memoize-memory, whether to disable the built in memory storage module for memoize,
[  --disable-memoize-memory     Disable the memoize memory storage module], yes, no)

if test "$PHP_MEMOIZE" != "no"; then
	SOURCES="memoize.c"

	if test "$PHP_MEMOIZE_APC" != "no"; then
		PHP_DEF_HAVE(memoize_apc)
		SOURCES="$SOURCES memoize_apc.c"
		PHP_ADD_EXTENSION_DEP(memoize, apc)
	fi

	if test "$PHP_MEMOIZE_MEMCACHED" != "no"; then
		PHP_DEF_HAVE(memoize_memcached)
		SOURCES="$SOURCES memoize_memcached.c"
		PHP_ADD_EXTENSION_DEP(memoize, memcached, false)

		if test "$PHP_MEMOIZE_MEMCACHED" != "yes"; then
			if test ! -r "$PHP_MEMOIZE_MEMCACHED/include/libmemcached-1.0/memcached.h"; then
				AC_MSG_ERROR([Can't find libmemcached headers under "$PHP_MEMOIZE_MEMCACHED"])
			fi
		else
			for i in /usr /usr/local; do
				if test -r "$i/include/libmemcached-1.0/memcached.h"; then
					PHP_MEMOIZE_MEMCACHED=$i
					break
				fi
			done
		fi

		AC_MSG_CHECKING([for libmemcached location])
		if test "$PHP_MEMOIZE_MEMCACHED" = "yes"; then
			AC_MSG_RESULT([libmemcached 1.0.x not found. The memcached storage module for memoize will only be usable with the memcached extension via memoize_memcached_set_connection. Use --with-memoize-memcached=<DIR> to specify the prefix where libmemcached headers and library are located])
		else
			AC_MSG_RESULT([Using libmemcached prefix: $PHP_MEMOIZE_MEMCACHED])
			PHP_ADD_INCLUDE($PHP_MEMOIZE_MEMCACHED/include)
			PHP_ADD_LIBRARY_WITH_PATH(memcached, $PHP_MEMOIZE_MEMCACHED/$PHP_LIBDIR, MEMOIZE_SHARED_LIBADD)
			PHP_SUBST(MEMOIZE_SHARED_LIBADD)
			PHP_DEF_HAVE(memoize_libmemcached)
		fi
	fi

	if test "$PHP_MEMOIZE_MEMORY" != "no"; then
		PHP_DEF_HAVE(memoize_memory)
		SOURCES="$SOURCES memoize_memory.c"
	fi

	PHP_NEW_EXTENSION(memoize, $SOURCES, $ext_shared)
	PHP_INSTALL_HEADERS([ext/memoize], [php_memoize_storage.h])
fi
