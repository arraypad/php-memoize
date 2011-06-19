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

#ifndef _PHP_MEMOIZE_STORAGE_H_
#define _PHP_MEMOIZE_STORAGE_H_

#include "php_memoize.h"

/* Storage function arguments */
#define MEMOIZE_GET_ARGS	char *key, zval **value TSRMLS_DC
#define MEMOIZE_SET_ARGS	char *key, zval *value TSRMLS_DC

/* {{{ typedef struct _php_memoize_storage_module */
typedef struct _memoize_storage_module {
	char *name;
	int (*get)(MEMOIZE_GET_ARGS);
	int (*set)(MEMOIZE_SET_ARGS);
} memoize_storage_module;
/* }}} */

/* Storage function signatures */
#define MEMOIZE_GET_FUNC(mod_name)	int memoize_storage_get_##mod_name(MEMOIZE_GET_ARGS)
#define MEMOIZE_SET_FUNC(mod_name)	int memoize_storage_set_##mod_name(MEMOIZE_SET_ARGS)

/* Define storage functions */
#define MEMOIZE_STORAGE_FUNCS(mod_name) \
	MEMOIZE_GET_FUNC(mod_name); \
	MEMOIZE_SET_FUNC(mod_name); 

/* Define the func ptrs for the specific module */
#define MEMOIZE_STORAGE_MODULE(mod_name) \
	#mod_name, memoize_storage_get_##mod_name, memoize_storage_set_##mod_name
	
/* Register the storage module. called from MINIT in module */
#define MEMOIZE_STORAGE_REGISTER(mod_name) \
	memoize_register_storage_module(memoize_storage_module_##mod_name##_ptr)

#endif
