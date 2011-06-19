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

#include "php_memoize_memory.h"
#include "ext/standard/info.h"

memoize_storage_module memoize_storage_module_memory = {
	MEMOIZE_STORAGE_MODULE(memory)
};

/* {{{ Globals */
ZEND_DECLARE_MODULE_GLOBALS(memoize_memory);

static PHP_GINIT_FUNCTION(memoize_memory) 
{
	memoize_memory_globals->store = NULL;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION(memoize_memory) */
PHP_RINIT_FUNCTION(memoize_memory) 
{
	MEMOIZE_STORAGE_REGISTER(memory);
	ALLOC_HASHTABLE(MEMOIZE_MEMORY_G(store));
	return zend_hash_init(MEMOIZE_MEMORY_G(store), 4, NULL, (dtor_func_t)ZVAL_PTR_DTOR, 0);
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION(memoize_memory) */
PHP_RSHUTDOWN_FUNCTION(memoize_memory) 
{
	zend_hash_destroy(MEMOIZE_MEMORY_G(store));
	FREE_HASHTABLE(MEMOIZE_MEMORY_G(store));
	return SUCCESS;
}
/* }}} */

/* {{{ MEMOIZE_GET_FUNC(memory)
*/
MEMOIZE_GET_FUNC(memory)
{
	zval **entry;
	if (zend_hash_find(MEMOIZE_MEMORY_G(store), key, strlen(key) + 1, (void **)&entry) == SUCCESS) {
		zval_ptr_dtor(value);
		*value = *entry;
		Z_ADDREF_PP(value);
		return SUCCESS;
	}

	return FAILURE;
}
/* }}} */

/* {{{ MEMOIZE_SET_FUNC(memory) */
MEMOIZE_SET_FUNC(memory)
{
	zval *tmp = value;
	Z_ADDREF_P(tmp);
	return zend_hash_update(MEMOIZE_MEMORY_G(store), key, strlen(key) + 1, (void *)&tmp, sizeof(zval *), NULL);
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION(memoize_memory) */
PHP_MINFO_FUNCTION(memoize_memory)
{	
	php_info_print_table_start();
	php_info_print_table_row(2, "memoize_memory storage", "enabled");
	php_info_print_table_row(2, "memoize_memory version", MEMOIZE_MEMORY_EXTVER);
	php_info_print_table_end();
}
/* }}} */

static zend_function_entry memoize_memory_functions[] = {
	{NULL, NULL, NULL}
};

zend_module_entry memoize_memory_module_entry = {
	STANDARD_MODULE_HEADER,
	"memoize_memory",
	memoize_memory_functions,
	NULL,
	NULL,
	PHP_RINIT(memoize_memory),
	PHP_RSHUTDOWN(memoize_memory),
	PHP_MINFO(memoize_memory),
	MEMOIZE_MEMORY_EXTVER,
	PHP_MODULE_GLOBALS(memoize_memory),
	PHP_GINIT(memoize_memory),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_MEMOIZE_MEMORY
ZEND_GET_MODULE(memoize_memory)
#endif
