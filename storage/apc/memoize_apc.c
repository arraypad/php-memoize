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

#include "php_memoize_apc.h"
#include "ext/standard/info.h"

#include "ext/apc/apc_zend.h"
#include "ext/apc/apc_globals.h"

memoize_storage_module memoize_storage_module_apc = {
	MEMOIZE_STORAGE_MODULE(apc)
};

/* {{{ PHP_MINIT_FUNCTION(memoize_apc) */
PHP_MINIT_FUNCTION(memoize_apc) 
{
	if (!APCG(enabled)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "APC cache is disabled");
		return FAILURE;
	}

	MEMOIZE_STORAGE_REGISTER(apc);
	return SUCCESS;
}
/* }}} */

/* {{{ MEMOIZE_GET_FUNC(apc) */
MEMOIZE_GET_FUNC(apc)
{
	apc_cache_entry_t* entry;
	apc_context_t ctxt = {0,};
	time_t t;

	/* create apc pool */
	ctxt.pool = apc_pool_create(APC_UNPOOL, apc_php_malloc, apc_php_free, NULL, NULL TSRMLS_CC);
	if (!ctxt.pool) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to allocate memory for APC pool.");
		return FAILURE;
	}
	ctxt.copy = APC_COPY_OUT_USER;
	ctxt.force_update = 0;

	/* try to get value */
	t = apc_time();
	entry = apc_cache_user_find(apc_user_cache, key, strlen(key), t TSRMLS_CC);
	if (entry) {
		/* deep-copy returned shm zval to emalloc'ed value */
		apc_cache_fetch_zval(*value, entry->data.user.val, &ctxt TSRMLS_CC);
		apc_cache_release(apc_user_cache, entry TSRMLS_CC);
		apc_pool_destroy(ctxt.pool TSRMLS_CC);

		return SUCCESS;
	}

	apc_pool_destroy(ctxt.pool TSRMLS_CC);
	return FAILURE;
}
/* }}} */

/* {{{ MEMOIZE_SET_FUNC(apc) */
MEMOIZE_SET_FUNC(apc)
{
	return _apc_store(key, strlen(key), value, 0, 0 TSRMLS_CC);
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION(memoize_apc) */
PHP_MINFO_FUNCTION(memoize_apc)
{	
	php_info_print_table_start();
	php_info_print_table_row(2, "memoize_apc storage", "enabled");
	php_info_print_table_row(2, "memoize_apc version", MEMOIZE_APC_EXTVER);
	php_info_print_table_end();
}
/* }}} */

static zend_function_entry memoize_apc_functions[] = {
	{NULL, NULL, NULL}
};

zend_module_entry memoize_apc_module_entry = {
	STANDARD_MODULE_HEADER,
	"memoize_apc",
	memoize_apc_functions,
	PHP_MINIT(memoize_apc),
	NULL,
	NULL,
	NULL,
	PHP_MINFO(memoize_apc),
	MEMOIZE_APC_EXTVER,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_MEMOIZE_APC
ZEND_GET_MODULE(memoize_apc)
#endif
