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
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_MEMOIZE_H
#define PHP_MEMOIZE_H

#define MEMOIZE_EXTVER "0.0.1-dev"

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
PHP_RSHUTDOWN_FUNCTION(memoize);
PHP_MINFO_FUNCTION(memoize);

PHP_FUNCTION(memoize);
PHP_FUNCTION(memoize_call);

PHPAPI int memoize_register_storage_module(memoize_storage_module *ptr);

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
	char *cache_namespace;
	char *storage_module;
ZEND_END_MODULE_GLOBALS(memoize)
extern ZEND_DECLARE_MODULE_GLOBALS(memoize)

#ifdef ZTS
#define MEMOIZE_G(v) TSRMG(memoize_globals_id, zend_memoize_globals *, v)
#else
#define MEMOIZE_G(v) (memoize_globals.v)
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
