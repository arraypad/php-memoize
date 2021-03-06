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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_memoize.h"
#include "php_memoize_storage.h"

memoize_storage_module memoize_storage_module_apc = {
	MEMOIZE_STORAGE_MODULE(apc)
};

/* {{{ MEMOIZE_GET_FUNC(apc) */
MEMOIZE_GET_FUNC(apc)
{
	int ret = FAILURE;
	zval *func, *key_zv;
	zval *params[1];

	MAKE_STD_ZVAL(key_zv);
	ZVAL_STRING(key_zv, key, 1);
	params[0] = key_zv;

	MAKE_STD_ZVAL(func);
	ZVAL_STRING(func, "apc_fetch", 1);
	ret = call_user_function(EG(function_table), NULL, func, *value, 1, params TSRMLS_CC);
	zval_ptr_dtor(&func);
	zval_ptr_dtor(&key_zv);

	if (ret == SUCCESS) {
		if (Z_TYPE_PP(value) == IS_BOOL && !Z_LVAL_PP(value)) {
			ret = FAILURE;
		}
	}

	return ret;
}
/* }}} */

/* {{{ MEMOIZE_SET_FUNC(apc) */
MEMOIZE_SET_FUNC(apc)
{
	int ret;
	zval *func, *key_zv, *expiry_zv, retval;
	zval *params[3];

	if (!ttl) {
		ttl = MEMOIZE_G(default_ttl);
	}

	MAKE_STD_ZVAL(key_zv);
	ZVAL_STRING(key_zv, key, 1);

	MAKE_STD_ZVAL(expiry_zv);
	ZVAL_LONG(expiry_zv, ttl);

	params[0] = key_zv;
	params[1] = value;
	params[2] = expiry_zv;

	MAKE_STD_ZVAL(func);
	ZVAL_STRING(func, "apc_store", 1);
	ret = call_user_function(EG(function_table), NULL, func, &retval, 3, params TSRMLS_CC);
	zval_ptr_dtor(&func);
	zval_ptr_dtor(&key_zv);
	zval_ptr_dtor(&expiry_zv);

	return ret;
}
/* }}} */
