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

memoize_storage_module memoize_storage_module_memory = {
	MEMOIZE_STORAGE_MODULE(memory)
};

/* {{{ MEMOIZE_GET_FUNC(memory)
*/
MEMOIZE_GET_FUNC(memory)
{
	zval **entry;
	if (zend_hash_find(MEMOIZE_G(store), key, strlen(key) + 1, (void **)&entry) == SUCCESS) {
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
	Z_ADDREF_P(value);
	return zend_hash_update(MEMOIZE_G(store), key, strlen(key) + 1, (void *)&value, sizeof(zval *), NULL);
}
/* }}} */
