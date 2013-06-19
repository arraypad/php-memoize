/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2011 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Arpad Ray <arpad@php.net>                                    |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_MEMOIZE_H
#define PHP_MEMOIZE_H

#define MEMOIZE_EXTVER "0.1.0-dev"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php_memoize_storage.h"

extern zend_module_entry memoize_module_entry;
#define phpext_memoize_ptr &memoize_module_entry

#ifdef PHP_WIN32
#	define PHP_MEMOIZE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_MEMOIZE_API __attribute__ ((visibility("default")))
#else
#	define PHP_MEMOIZE_API 
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(memoize);
PHP_MSHUTDOWN_FUNCTION(memoize);
PHP_RINIT_FUNCTION(memoize);
PHP_RSHUTDOWN_FUNCTION(memoize);
PHP_MINFO_FUNCTION(memoize);

PHP_FUNCTION(memoize);
PHP_FUNCTION(memoize_call);
PHP_FUNCTION(memoize_has_storage);

#ifdef HAVE_MEMOIZE_MEMCACHED
	PHP_FUNCTION(memoize_memcached_set_connection);
#endif

extern PHP_MEMOIZE_API int memoize_register_storage_module(memoize_storage_module *ptr);

typedef struct {
	HashTable *function_table;
	zend_function function;
} memoize_internal_function;

int memoize_fix_internal_functions(memoize_internal_function *fe TSRMLS_DC);
int memoize_remove_handler_functions(zend_function *fe TSRMLS_DC);

#define MEMOIZE_KEY_PREFIX "_memoizd"
#define MEMOIZE_FUNC_SUFFIX "$memoizd"

ZEND_BEGIN_MODULE_GLOBALS(memoize)
	HashTable *internal_functions;
	HashTable *ttls;
	char *cache_namespace;
	char *storage_module;
	long default_ttl;

#ifdef HAVE_MEMOIZE_MEMORY
	HashTable *store;
#endif

#ifdef HAVE_MEMOIZE_MEMCACHED
	zval *user_connection;
#endif

#ifdef HAVE_MEMOIZE_LIBMEMCACHED
	char *servers;
	struct memcached_st *memc;
#endif
ZEND_END_MODULE_GLOBALS(memoize)

ZEND_EXTERN_MODULE_GLOBALS(memoize);

#ifdef HAVE_MEMOIZE_MEMORY
	extern memoize_storage_module memoize_storage_module_memory;
#	define memoize_storage_module_memory_ptr &memoize_storage_module_memory
	MEMOIZE_STORAGE_FUNCS(memory);
#endif

#ifdef HAVE_MEMOIZE_APC
	extern memoize_storage_module memoize_storage_module_apc;
#	define memoize_storage_module_apc_ptr &memoize_storage_module_apc
	MEMOIZE_STORAGE_FUNCS(apc);
	extern PHP_FUNCTION(memoize_memcached_set_connection);
#endif

#ifdef HAVE_MEMOIZE_MEMCACHED
	extern memoize_storage_module memoize_storage_module_memcached;
#	define memoize_storage_module_memcached_ptr &memoize_storage_module_memcached
	MEMOIZE_STORAGE_FUNCS(memcached);
#	ifdef HAVE_MEMOIZE_LIBMEMCACHED
#		include <libmemcached/memcached.h>
#	endif
#endif

#ifdef ZTS
#define MEMOIZE_G(v) TSRMG(memoize_globals_id, zend_memoize_globals *, v)
#else
#define MEMOIZE_G(v) (memoize_globals.v)
#endif

#if ZEND_MODULE_API_NO >= 20100525
#define MEMOIZE_RETURNS_REFERENCE(fe) (fe->common.fn_flags & ZEND_ACC_RETURN_REFERENCE)
#else
#define MEMOIZE_RETURNS_REFERENCE(fe) (fe->common.return_reference)
#endif


#endif	/* PHP_MEMOIZE_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
