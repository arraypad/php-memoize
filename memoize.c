/*
  +----------------------------------------------------------------------+
  | PHP Version 5														|
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2011 The PHP Group								|
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,	  |
  | that is bundled with this package in the file LICENSE, and is		|
  | available through the world-wide-web at the following url:		   |
  | http://www.php.net/license/3_01.txt								  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to		  |
  | license@php.net so we can mail you a copy immediately.			   |
  +----------------------------------------------------------------------+
  | Author: Arpad Ray <arpad@php.net>									|
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_smart_str.h"
#include "ext/standard/md5.h"
#include "php_memoize.h"
#include "Zend/zend_hash.h"

#include "ext/apc/apc_zend.h"
#include "ext/apc/apc_globals.h"


/* {{{ memoize_functions[]
 */
const zend_function_entry memoize_functions[] = {
	PHP_FE(memoize,	NULL)
	PHP_FE(memoize_call, NULL)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ memoize_module_entry
 */
zend_module_entry memoize_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"memoize",
	memoize_functions,
	PHP_MINIT(memoize),
	NULL,
	NULL,
	PHP_RSHUTDOWN(memoize),
	PHP_MINFO(memoize),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1",
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_MEMOIZE
ZEND_GET_MODULE(memoize)
#endif

/* {{{ ZEND_DECLARE_MODULE_GLOBALS(memoize) */
ZEND_DECLARE_MODULE_GLOBALS(memoize)

static void php_memoize_init_globals(zend_memoize_globals* memoize_globals TSRMLS_DC)
{
	memoize_globals->internal_functions = NULL;
}

static void php_memoize_shutdown_globals(zend_memoize_globals* memoize_globals TSRMLS_DC)
{
	/* done in rshutdown */
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(memoize)
{
	ZEND_INIT_MODULE_GLOBALS(memoize, php_memoize_init_globals, php_memoize_shutdown_globals);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(memoize)
{
	if (MEMOIZE_G(internal_functions)) {
		zend_hash_apply(MEMOIZE_G(internal_functions), (apply_func_t) memoize_fix_internal_functions TSRMLS_CC);
		zend_hash_destroy(MEMOIZE_G(internal_functions));
		FREE_HASHTABLE(MEMOIZE_G(internal_functions));
		MEMOIZE_G(internal_functions) = NULL;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(memoize)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "memoize support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ memoize_fix_internal_functions
	Restores renamed internal functions */
int memoize_fix_internal_functions(zend_internal_function *fe TSRMLS_DC)
{
	char *new_fname = NULL;

	/* remove renamed function */
	spprintf(&new_fname, 0, "%s%s", fe->function_name, MEMOIZE_FUNC_SUFFIX);
	zend_hash_del(EG(function_table), new_fname, strlen(new_fname));
	efree(new_fname);

	/* restore original function */
	zend_hash_update(EG(function_table), fe->function_name, strlen(fe->function_name), (void*)fe, sizeof(zend_function), NULL);

	return ZEND_HASH_APPLY_REMOVE;
}
/* }}} */

/* {{{ memoize_arguments_hash
	Returns an MD5 hash of the given arguments */
void memoize_arguments_hash(int argc, zval ***args, char *hash TSRMLS_DC)
{
	php_serialize_data_t args_data;
	smart_str args_str = {0};
	PHP_MD5_CTX md5ctx;
	int i;
	zval *args_array;

	if (argc == 0) {
		return;
	}

	/* construct php array from args */
	ALLOC_ZVAL(args_array);
	INIT_PZVAL(args_array);
	array_init_size(args_array, argc);
	for (i = 0; i < argc; i++) {
		zval *element = *args[i];
		zval_copy_ctor(element);
		add_next_index_zval(args_array, element);
	}

	/* serialize php array */
	PHP_VAR_SERIALIZE_INIT(args_data);
	php_var_serialize(&args_str, &args_array, &args_data TSRMLS_CC);
	PHP_VAR_SERIALIZE_DESTROY(args_data);
	zval_ptr_dtor(&args_array);

	/* hash serialized string */
	unsigned char raw_hash[16];
	PHP_MD5Init(&md5ctx);
	PHP_MD5Update(&md5ctx, args_str.c, args_str.len);
	PHP_MD5Final(raw_hash, &md5ctx);
	make_digest(hash, raw_hash);
	smart_str_free(&args_str);
}
/* }}} */

/* {{{ proto mixed memoize_call()
   Function to perform the actual memoized call */
PHP_FUNCTION(memoize_call)
{
	char *fname, *key = NULL;
	char hash[33] = "";
	int argc = 0;
	zval ***args = NULL;
	apc_cache_entry_t* entry;
	time_t t;
	apc_context_t ctxt = {0,};
	size_t key_len;
	zend_execute_data *ex = EG(current_execute_data);

	/* retrieve function name from entry */
	fname = ex->function_state.function->common.function_name;

	/* get parameters */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "*", &args, &argc) == FAILURE) {
		RETURN_FALSE;
	}

	/* construct hash key from memoize.fname.serialize(args) */
	memoize_arguments_hash(argc, args, hash TSRMLS_CC);
	key_len = spprintf(&key, 0, "%s%s%s", MEMOIZE_KEY_PREFIX, fname, hash);

	/* create apc pool */
	ctxt.pool = apc_pool_create(APC_UNPOOL, apc_php_malloc, apc_php_free, NULL, NULL TSRMLS_CC);
	if (!ctxt.pool) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to alocate memory for APC pool.");
		efree(key);
		if (args) {
			efree(args);
		}
		RETURN_FALSE;
	}
	ctxt.copy = APC_COPY_OUT_USER;
	ctxt.force_update = 0;

	/* look up key in apc */
	t = apc_time();
	entry = apc_cache_user_find(apc_user_cache, key, key_len, t TSRMLS_CC);
	if (entry) {
		/* deep-copy returned shm zval to emalloc'ed return_value */
		apc_cache_fetch_zval(return_value, entry->data.user.val, &ctxt TSRMLS_CC);
		apc_cache_release(apc_user_cache, entry TSRMLS_CC);
		apc_pool_destroy(ctxt.pool TSRMLS_CC);
	} else {
		/* destroy fetch pool immediately since _apc_store creates another */
		apc_pool_destroy(ctxt.pool TSRMLS_CC);

		/* call original function */
		zval *return_copy;
		char *new_fname = NULL;
		size_t new_fname_len = spprintf(&new_fname, 0, "%s%s", fname, MEMOIZE_FUNC_SUFFIX);
		zval new_fname_zv;
		ZVAL_STRINGL(&new_fname_zv, new_fname, new_fname_len, 0);
		MAKE_STD_ZVAL(return_copy);

		if (call_user_function(EG(function_table), NULL, &new_fname_zv, return_copy, argc, (argc ? *args : NULL) TSRMLS_CC) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to call memoized function %s.", fname);
		} else {
			/* store result in apc */
			_apc_store(key, key_len, return_copy, 0, 0 TSRMLS_CC);
		}

		COPY_PZVAL_TO_ZVAL(*return_value, return_copy);
		efree(new_fname);
	}

	efree(key);
	if (args) {
		efree(args);
	}
}
/* }}} */

/* {{{ proto bool memoize(string fname)
   Memoizes the given function */
PHP_FUNCTION(memoize)
{
	char *fname = NULL, *new_fname = NULL;
	int fname_len;
	size_t new_fname_len;
	zend_function *fe, *dfe, func, dfunc;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &fname, &fname_len) == FAILURE) {
		return;
	}

	if (!APCG(enabled)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "APC cache is disabled");
		RETURN_FALSE;
	}

	php_strtolower(fname, fname_len);

	/* find source function */
	if (zend_hash_find(EG(function_table), fname, fname_len + 1, (void**)&fe) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s() not found", fname);
		RETURN_FALSE;
	}

	/* find dest function */
	if (zend_hash_find(EG(function_table), "memoize_call", strlen("memoize_call") + 1, (void**)&dfe) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "memoize_call() not found");
		RETURN_FALSE;
	}

	/* rename source */
	func = *fe;
	if (fe->type == ZEND_INTERNAL_FUNCTION) {
		if (!MEMOIZE_G(internal_functions)) {
			ALLOC_HASHTABLE(MEMOIZE_G(internal_functions));
			zend_hash_init(MEMOIZE_G(internal_functions), 4, NULL, NULL, 0);
		}
		zend_hash_add(MEMOIZE_G(internal_functions), fname, fname_len + 1, (void*)fe, sizeof(zend_function), NULL);
	}
	new_fname_len = spprintf(&new_fname, 0, "%s%s", fname, MEMOIZE_FUNC_SUFFIX);
	if (zend_hash_add(EG(function_table), new_fname, new_fname_len + 1, &func, sizeof(zend_function), NULL) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error renaming %s()", fname);
		efree(new_fname);
		zend_function_dtor(fe);
		RETURN_FALSE;
	}
	function_add_ref(&func);
	efree(new_fname);

	/* copy dest entry with source name */
	dfunc = *dfe;
	function_add_ref(&dfunc);
	dfunc.common.function_name = fname;

	/* replace source with dest */
	if (zend_hash_update(EG(function_table), fname, fname_len + 1, &dfunc, sizeof(zend_function), NULL) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error replacing %s()", fname);
		zend_function_dtor(&func);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
