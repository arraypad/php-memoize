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

#include "php_memoize_memcached.h"
#include "ext/standard/info.h"

memoize_storage_module memoize_storage_module_memcached = {
	MEMOIZE_STORAGE_MODULE(memcached)
};

/* {{{ Globals */
ZEND_DECLARE_MODULE_GLOBALS(memoize_memcached);

static PHP_GINIT_FUNCTION(memoize_memcached) 
{
	memoize_memcached_globals->user_connection = NULL;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION(memoize_memcached) */
PHP_MINIT_FUNCTION(memoize_memcached) 
{
	MEMOIZE_STORAGE_REGISTER(memcached);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION(memoize_memcached) */
PHP_RSHUTDOWN_FUNCTION(memoize_memcached) 
{
    if (MEMOIZE_MEMCACHED_G(user_connection)) {
        zval_ptr_dtor(&MEMOIZE_MEMCACHED_G(user_connection));
        MEMOIZE_MEMCACHED_G(user_connection) = NULL;
    }

	return SUCCESS;
}
/* }}} */

/* {{{ MEMOIZE_GET_FUNC(memcached)
*/
MEMOIZE_GET_FUNC(memcached)
{
    int ret = FAILURE;

    if (MEMOIZE_MEMCACHED_G(user_connection)) {
        zval *func, *key_zv;
        MAKE_STD_ZVAL(key_zv);
        ZVAL_STRING(key_zv, key, 1);

        zval *params[1] = {key_zv};

        MAKE_STD_ZVAL(func);
        ZVAL_STRING(func, "get", 1);
        ret = call_user_function(NULL, &MEMOIZE_MEMCACHED_G(user_connection), func, *value, 1, params TSRMLS_CC);
        zval_ptr_dtor(&func);
        zval_ptr_dtor(&key_zv);

        if (ret == SUCCESS) {
            if (Z_TYPE_PP(value) == IS_BOOL && !Z_LVAL_PP(value)) {
                ret = FAILURE;
            }
        }
    }

    return ret;
}
/* }}} */

/* {{{ MEMOIZE_SET_FUNC(memcached) */
MEMOIZE_SET_FUNC(memcached)
{
    int ret = FAILURE;

    if (MEMOIZE_MEMCACHED_G(user_connection)) {
        zval *func, *key_zv, retval;
        MAKE_STD_ZVAL(key_zv);
        ZVAL_STRING(key_zv, key, 1);

        zval *params[2] = {key_zv, value};

        MAKE_STD_ZVAL(func);
        ZVAL_STRING(func, "set", 1);
        ret = call_user_function(NULL, &MEMOIZE_MEMCACHED_G(user_connection), func, &retval, 2, params TSRMLS_CC);
        zval_ptr_dtor(&func);
        zval_ptr_dtor(&key_zv);
    }

    return ret;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION(memoize_memcached) */
PHP_MINFO_FUNCTION(memoize_memcached)
{	
	php_info_print_table_start();
	php_info_print_table_row(2, "memoize_memcached storage", "enabled");
	php_info_print_table_row(2, "memoize_memcached version", MEMOIZE_MEMCACHED_EXTVER);
	php_info_print_table_end();
}
/* }}} */

/* {{{ PHP_FUNCTION(memoize_memcached_set_connection) */
PHP_FUNCTION(memoize_memcached_set_connection)
{
    zval *obj;
    zend_class_entry **ce;

    if (zend_hash_find(EG(class_table), "memcached", strlen("memcached") + 1, (void **)&ce) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot find Memcached class");
        RETURN_FALSE;
    }

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &obj) == FAILURE) {
		RETURN_FALSE;
	}

    if (!instanceof_function(Z_OBJCE_P(obj), *ce)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Connection must be an instance of the Memcached class");
        RETURN_FALSE;
    }

    Z_ADDREF_P(obj);
    MEMOIZE_MEMCACHED_G(user_connection) = obj;
    RETURN_TRUE;
}
/* }}} */

static zend_function_entry memoize_memcached_functions[] = {
	PHP_FE(memoize_memcached_set_connection,	NULL)
	{NULL, NULL, NULL}
};

zend_module_entry memoize_memcached_module_entry = {
	STANDARD_MODULE_HEADER,
	"memoize_memcached",
	memoize_memcached_functions,
	PHP_MINIT(memoize_memcached),
	NULL,
	NULL,
	PHP_RSHUTDOWN(memoize_memcached),
	PHP_MINFO(memoize_memcached),
	MEMOIZE_MEMCACHED_EXTVER,
	PHP_MODULE_GLOBALS(memoize_memcached),
	PHP_GINIT(memoize_memcached),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_MEMOIZE_MEMCACHED
ZEND_GET_MODULE(memoize_memcached)
#endif
