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
	PHP_MSHUTDOWN(memoize),
	NULL,
	PHP_RSHUTDOWN(memoize),
	PHP_MINFO(memoize),
	"0.1",
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_MEMOIZE
ZEND_GET_MODULE(memoize)
#endif

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("memoize.cache_namespace", "", PHP_INI_ALL, OnUpdateString, cache_namespace, zend_memoize_globals, memoize_globals)
PHP_INI_END()
/* }}} */

/* {{{ ZEND_DECLARE_MODULE_GLOBALS(memoize) */
ZEND_DECLARE_MODULE_GLOBALS(memoize)

static void php_memoize_init_globals(zend_memoize_globals* memoize_globals TSRMLS_DC)
{
	memoize_globals->internal_functions = NULL;
	memoize_globals->cache_namespace = NULL;
}

static void php_memoize_shutdown_globals(zend_memoize_globals* memoize_globals TSRMLS_DC)
{
	/* internal_function done in rshutdown */

	memoize_globals->cache_namespace = NULL;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(memoize)
{
	ZEND_INIT_MODULE_GLOBALS(memoize, php_memoize_init_globals, php_memoize_shutdown_globals);
	REGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(memoize)
{
	UNREGISTER_INI_ENTRIES();
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

	zend_hash_apply(EG(function_table), (apply_func_t) memoize_remove_handler_functions TSRMLS_CC);

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
int memoize_fix_internal_functions(memoize_internal_function *mem_func TSRMLS_DC)
{
	zend_internal_function *fe = (zend_internal_function*)&mem_func->function;
	char *new_fname = NULL, *fname = zend_str_tolower_dup(fe->function_name, strlen(fe->function_name));

	/* remove renamed function */
	spprintf(&new_fname, 0, "%s%s", fname, MEMOIZE_FUNC_SUFFIX);
	zend_hash_del(mem_func->function_table, new_fname, strlen(new_fname) + 1);
	efree(new_fname);

	/* restore original function */
	zend_hash_update(mem_func->function_table, fname, strlen(fname) + 1, (void*)fe, sizeof(zend_function), NULL);

	efree(fname);
	return ZEND_HASH_APPLY_REMOVE;
}
/* }}} */

/* {{{ memoize_remove_handler_functions
	Try to clean up before zend mm  */
int memoize_remove_handler_functions(zend_function *fe TSRMLS_DC)
{
	if (fe->type == ZEND_INTERNAL_FUNCTION && fe->internal_function.handler == &ZEND_FN(memoize_call) && !strstr(fe->common.function_name, "memoize_call")) {
		return ZEND_HASH_APPLY_REMOVE;
	}

	return ZEND_HASH_APPLY_KEEP;
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
	Z_TYPE_P(args_array) = IS_ARRAY;
	ALLOC_HASHTABLE(Z_ARRVAL_P(args_array));
	zend_hash_init(Z_ARRVAL_P(args_array), argc, NULL, NULL, 0);
	for (i = 0; i < argc; i++) {
		add_next_index_zval(args_array, *args[i]);
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
	zend_function *fe;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "*", &args, &argc) == FAILURE) {
		RETURN_FALSE;
	}

	/* retrieve function name from entry */
	fe = EG(current_execute_data)->function_state.function;
	fname = estrdup(fe->common.function_name);

	if (strlen(fname) == strlen("memoize_call") && !memcmp(fname, "memoize_call", strlen(fname))) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot call memoize_call() directly");
		efree(fname);
		if (argc) {
			efree(args);
		}
		RETURN_FALSE;
	}

	/* create apc pool */
	ctxt.pool = apc_pool_create(APC_UNPOOL, apc_php_malloc, apc_php_free, NULL, NULL TSRMLS_CC);
	if (!ctxt.pool) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to allocate memory for APC pool.");
		efree(fname);
		efree(key);
		if (argc) {
			efree(args);
		}
		RETURN_FALSE;
	}
	ctxt.copy = APC_COPY_OUT_USER;
	ctxt.force_update = 0;

	/* construct hash key from memoize.fname.serialize(args) */
	memoize_arguments_hash(argc, args, hash TSRMLS_CC);
	key_len = spprintf(&key, 0, "%s%s%s%s", MEMOIZE_KEY_PREFIX, MEMOIZE_G(cache_namespace), fname, hash);

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

		/* create callable for original function */
		char *new_fname = NULL;
		zval **obj_pp = NULL;
		size_t new_fname_len = spprintf(&new_fname, 0, "%s%s", fname, MEMOIZE_FUNC_SUFFIX);
		zval *callable;
		MAKE_STD_ZVAL(callable);
		if (fe->common.scope) {
			/* static method */
			array_init_size(callable, 2);
			if (EG(current_execute_data)->object) {
				obj_pp = &EG(current_execute_data)->object;
				add_next_index_zval(callable, *obj_pp);
			} else {
				add_next_index_stringl(callable, fe->common.scope->name, strlen(fe->common.scope->name), 1);
			}
			add_next_index_stringl(callable, new_fname, new_fname_len, 0);
		} else {
			/* function */
			ZVAL_STRINGL(callable, new_fname, new_fname_len, 0);
		}

		/* call original function */
		zval *return_copy;
		MAKE_STD_ZVAL(return_copy);
		if (call_user_function(&fe->common.scope->function_table, obj_pp, callable, return_copy, argc, (argc ? *args : NULL) TSRMLS_CC) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to call memoized function %s.", fname);
		} else {
			/* store result in apc */
			_apc_store(key, key_len, return_copy, 0, 0 TSRMLS_CC);
		}
		zval_ptr_dtor(&callable);

		COPY_PZVAL_TO_ZVAL(*return_value, return_copy);
	}

	efree(fname);
	efree(key);
	if (argc) {
		efree(args);
	}
}
/* }}} */

/* {{{ proto bool memoize(string fname)
   Memoizes the given function */
PHP_FUNCTION(memoize)
{
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;
	char *fname = NULL, *new_fname = NULL;
	int fname_len;
	size_t new_fname_len;
	zend_function *fe, *dfe, func, *new_dfe;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f", &fci, &fci_cache) == FAILURE) {
		return;
	}

	if (!APCG(enabled)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "APC cache is disabled");
		RETURN_FALSE;
	}

	if (Z_TYPE_P(fci.function_name) == IS_ARRAY) {
		zval **fname_zv;
		HashTable *callable = Z_ARRVAL_P(fci.function_name);
		if (zend_hash_index_find(callable, 1, (void **)&fname_zv) == FAILURE) {
			/* should be impossible, array callables always have this index */
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "malformed callback");
			return;
		}
		fname = Z_STRVAL_PP(fname_zv);
		fname_len = Z_STRLEN_PP(fname_zv);
	} else {
		fname = Z_STRVAL_P(fci.function_name);
		fname_len = Z_STRLEN_P(fci.function_name);

		if (fname_len == strlen("memoize") && !memcmp(fname, "memoize", fname_len)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot memoize memoize()!");
			RETURN_FALSE;
		}
	}

	php_strtolower(fname, fname_len);

	/* find source function */
	if (zend_hash_find(fci.function_table, fname, fname_len + 1, (void**)&fe) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s() not found", fname);
		RETURN_FALSE;
	}

	if (fe->common.return_reference) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot cache functions which return references");
		RETURN_FALSE;
	}

	func = *fe;
	function_add_ref(&func);

	/* find dest function */
	if (zend_hash_find(EG(function_table), "memoize_call", strlen("memoize_call") + 1, (void**)&dfe) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "memoize_call() not found");
		RETURN_FALSE;
	}
	
	/* copy dest entry with source name */
	new_dfe = emalloc(sizeof(zend_function));
	memcpy(new_dfe, dfe, sizeof(zend_function));
	new_dfe->common.scope = fe->common.scope;
	new_dfe->common.fn_flags = fe->common.fn_flags;
	new_dfe->common.function_name = fe->common.function_name;

	/* replace source with dest */
	if (zend_hash_update(fci.function_table, fname, fname_len + 1, new_dfe, sizeof(zend_function), NULL) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error replacing %s()", fname);
		zend_function_dtor(&func);
		efree(new_dfe);
		RETURN_FALSE;
	}
	efree(new_dfe);

	/* rename source */
	if (func.type == ZEND_INTERNAL_FUNCTION) {
		if (!MEMOIZE_G(internal_functions)) {
			ALLOC_HASHTABLE(MEMOIZE_G(internal_functions));
			zend_hash_init(MEMOIZE_G(internal_functions), 4, NULL, NULL, 0);
		}

		memoize_internal_function mem_func;
		mem_func.function = func;
		mem_func.function_table = fci.function_table;
		zend_hash_add(MEMOIZE_G(internal_functions), fname, fname_len + 1, (void*)&mem_func, sizeof(memoize_internal_function), NULL);
	}

	new_fname_len = spprintf(&new_fname, 0, "%s%s", fname, MEMOIZE_FUNC_SUFFIX);
	if (zend_hash_add(fci.function_table, new_fname, new_fname_len + 1, &func, sizeof(zend_function), NULL) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error renaming %s()", fname);
		efree(new_fname);
		zend_function_dtor(&func);
		RETURN_FALSE;
	}
	efree(new_fname);

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
