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
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_smart_str.h"
#include "ext/standard/md5.h"
#include "php_memoize.h"

/* {{{ memoize_functions[]
 */
const zend_function_entry memoize_functions[] = {
	PHP_FE(memoize,	NULL)
	PHP_FE(memoize_call, NULL)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ PHP_GINIT_FUNCTION(memoize) */
ZEND_EXTERN_MODULE_GLOBALS(memoize);
static PHP_GINIT_FUNCTION(memoize)
{
	memoize_globals->internal_functions = NULL;
	memoize_globals->cache_namespace = NULL;
	memoize_globals->storage_module = NULL;
	memoize_globals->default_ttl = 0;
}
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
	MEMOIZE_EXTVER,
	PHP_MODULE_GLOBALS(memoize),
	PHP_GINIT(memoize),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_MEMOIZE
ZEND_GET_MODULE(memoize)
#endif

/* {{{ Storage modules
 */

#define MEMOIZE_MAX_MODULES 10

static memoize_storage_module *memoize_storage_modules[MEMOIZE_MAX_MODULES + 1] = { NULL };

static int _memoize_find_storage_module(memoize_storage_module **ret TSRMLS_DC)
{
	int i;

	if (!MEMOIZE_G(storage_module)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "No storage module selected");
		return FAILURE;
	}

	for (i = 0; i < MEMOIZE_MAX_MODULES; i++) {
		if (memoize_storage_modules[i] && !strcasecmp(MEMOIZE_G(storage_module), memoize_storage_modules[i]->name)) {
			*ret = memoize_storage_modules[i];
			break;
		}
	}

	if (!*ret) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Couldn't load memoize storage module '%s'", MEMOIZE_G(storage_module));
		return FAILURE;
	}

	return SUCCESS;
}

PHP_MEMOIZE_API int memoize_register_storage_module(memoize_storage_module *ptr)
{
	int i, ret = FAILURE;

	for (i = 0; i < MEMOIZE_MAX_MODULES; i++) {
		if (!memoize_storage_modules[i]) {
			memoize_storage_modules[i] = ptr;
			ret = SUCCESS;
			break;
		}
	}

	return ret;
}
/* }}} */

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("memoize.cache_namespace", "", PHP_INI_ALL, OnUpdateString, cache_namespace, zend_memoize_globals, memoize_globals)
	STD_PHP_INI_ENTRY("memoize.storage_module", "memory", PHP_INI_ALL, OnUpdateString, storage_module, zend_memoize_globals, memoize_globals)
	STD_PHP_INI_ENTRY("memoize.default_ttl", "3600", PHP_INI_ALL, OnUpdateLong, default_ttl, zend_memoize_globals, memoize_globals)
PHP_INI_END()
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(memoize)
{
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
	php_info_print_table_row(2, "memoize version", MEMOIZE_EXTVER);
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
	
#define MEMOIZE_IS_HANDLER(fe)	(fe->type == ZEND_INTERNAL_FUNCTION && \
		fe->internal_function.handler == &ZEND_FN(memoize_call) && \
		!strstr(fe->common.function_name, "memoize_call"))

/* {{{ memoize_remove_handler_functions
	Try to clean up before zend mm  */
int memoize_remove_handler_functions(zend_function *fe TSRMLS_DC)
{
	if (MEMOIZE_IS_HANDLER(fe)) {
		return ZEND_HASH_APPLY_REMOVE;
	}

	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

/* {{{ memoize_arguments_hash
	Returns an MD5 hash of the given arguments */
void memoize_arguments_hash(int argc, char *fname, zval ***args, zval **object, char *hash TSRMLS_DC)
{
	php_serialize_data_t args_data;
	unsigned char raw_hash[16];
	smart_str args_str = {0};
	PHP_MD5_CTX md5ctx;
	int i;
	zval *args_array;

	/* construct php array from args */
	MAKE_STD_ZVAL(args_array);
	array_init_size(args_array, argc + 2 + (object != NULL));
	add_next_index_string(args_array, MEMOIZE_G(cache_namespace), 1);
	add_next_index_string(args_array, fname, 1);
	if (object) {
		Z_ADDREF_PP(object);
		add_next_index_zval(args_array, *object);
	}
	for (i = 0; i < argc; i++) {
		zval **arg_pp = args[i];
		Z_ADDREF_PP(arg_pp);
		add_next_index_zval(args_array, *arg_pp);
	}

	/* serialize php array */
	PHP_VAR_SERIALIZE_INIT(args_data);
	php_var_serialize(&args_str, &args_array, &args_data TSRMLS_CC);
	PHP_VAR_SERIALIZE_DESTROY(args_data);
	zval_ptr_dtor(&args_array);

	/* hash serialized string */
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
	zval ***args = NULL, *temp_value = NULL;
	size_t key_len;
	zend_function *fe;
	memoize_storage_module *mod = NULL;

	if (_memoize_find_storage_module(&mod TSRMLS_CC) == FAILURE) {
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "*", &args, &argc) == FAILURE) {
		RETURN_FALSE;
	}

	fe = EG(current_execute_data)->function_state.function;

	if (strlen(fe->common.function_name) == strlen("memoize_call") && !memcmp(fe->common.function_name, "memoize_call", strlen("memoize_call"))) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot call memoize_call() directly");
		if (argc) {
			efree(args);
		}
		RETURN_FALSE;
	}

	fname = estrdup(fe->common.function_name);

	/* construct hash key */
	memoize_arguments_hash(argc, fname, args, EG(current_execute_data)->object ? &EG(current_execute_data)->object : NULL, hash TSRMLS_CC);
	key_len = spprintf(&key, 0, "%s%s", MEMOIZE_KEY_PREFIX, hash);

	/* look up key in storage mod */
	if (mod->get(key, return_value_ptr TSRMLS_CC) == FAILURE) {
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
				Z_ADDREF_PP(obj_pp);
				add_next_index_zval(callable, *obj_pp);
			} else {
				add_next_index_stringl(callable, fe->common.scope->name, strlen(fe->common.scope->name), 1);
			}
			add_next_index_stringl(callable, new_fname, new_fname_len, 0);
		} else {
			/* function */
			ZVAL_STRINGL(callable, new_fname, new_fname_len, 0);
		}

		/* ensure we have a zval for the return value even if it isn't used */ 
		if (!return_value_used) {
			MAKE_STD_ZVAL(temp_value);
			return_value_ptr = &temp_value;
		}

		/* call original function */
		if (call_user_function(&fe->common.scope->function_table, obj_pp, callable, *return_value_ptr, argc, (argc ? *args : NULL) TSRMLS_CC) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to call memoized function %s.", fname);
		} else {
			/* store result in storage mod */
			mod->set(key, *return_value_ptr TSRMLS_CC);
		}
		zval_ptr_dtor(&callable);

		/* free the return value if it's not being returned */
		if (temp_value) {
			zval_ptr_dtor(&temp_value);
		}
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
	zval *callable;
	char *fname = NULL, *new_fname = NULL;
	int fname_len;
	size_t new_fname_len;
	zend_function *fe, *dfe, func, *new_dfe;
	HashTable *function_table = EG(function_table);
	memoize_storage_module *mod = NULL;

	if (_memoize_find_storage_module(&mod TSRMLS_CC) == FAILURE) {
		RETURN_FALSE;
	}

	/* parse argument */

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &callable) == FAILURE) {
		return;
	}
	
	if (Z_TYPE_P(callable) == IS_ARRAY) {
		zval **fname_zv, **obj_zv;
		if (zend_hash_num_elements(Z_ARRVAL_P(callable)) == 2) {
			zend_hash_index_find(Z_ARRVAL_P(callable), 0, (void **)&obj_zv);
			zend_hash_index_find(Z_ARRVAL_P(callable), 1, (void **)&fname_zv);
		}

		if (obj_zv && fname_zv && (Z_TYPE_PP(obj_zv) == IS_OBJECT || Z_TYPE_PP(obj_zv) == IS_STRING) && Z_TYPE_PP(fname_zv) == IS_STRING) {
			/* looks like a valid callback */
			zend_class_entry *ce, **pce;

			if (Z_TYPE_PP(obj_zv) == IS_OBJECT) {
				ce = Z_OBJCE_PP(obj_zv);
			} else if (Z_TYPE_PP(obj_zv) == IS_STRING) {
				if (zend_lookup_class(Z_STRVAL_PP(obj_zv), Z_STRLEN_PP(obj_zv), &pce TSRMLS_CC) == FAILURE) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Class %s() not found", Z_STRVAL_PP(obj_zv));
					RETURN_FALSE;
				}
				ce = *pce;
			}

			function_table = &ce->function_table;

			fname = zend_str_tolower_dup(Z_STRVAL_PP(fname_zv), Z_STRLEN_PP(fname_zv));
			fname_len = Z_STRLEN_PP(fname_zv);

			if (zend_hash_exists(function_table, fname, fname_len + 1) == FAILURE) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Method %s() not found", fname);
				efree(fname);
				RETURN_FALSE;
			}
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Argument is not a valid callback");
			return;
		}
	} else if (Z_TYPE_P(callable) == IS_STRING) {
		fname = zend_str_tolower_dup(Z_STRVAL_P(callable), Z_STRLEN_P(callable));
		fname_len = Z_STRLEN_P(callable);

		if (fname_len == strlen("memoize") && !memcmp(fname, "memoize", fname_len)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot memoize memoize()!");
			efree(fname);
			RETURN_FALSE;
		}
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Argument must be a string (function name) or an array (class or object and method name)");
		RETURN_FALSE;
	}


	/* find source function */
	if (zend_hash_find(function_table, fname, fname_len + 1, (void**)&fe) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Function %s() not found", fname);
		efree(fname);
		RETURN_FALSE;
	}

	if (MEMOIZE_IS_HANDLER(fe)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s() is already memoized", fe->common.function_name);
		efree(fname);
		RETURN_FALSE;
	}


	if (MEMOIZE_RETURNS_REFERENCE(fe)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot cache functions which return references");
		efree(fname);
		RETURN_FALSE;
	}

	func = *fe;
	function_add_ref(&func);

	/* find dest function */
	if (zend_hash_find(EG(function_table), "memoize_call", strlen("memoize_call") + 1, (void**)&dfe) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "memoize_call() not found");
		efree(fname);
		RETURN_FALSE;
	}
	
	/* copy dest entry with source name */
	new_dfe = emalloc(sizeof(zend_function));
	memcpy(new_dfe, dfe, sizeof(zend_function));
	new_dfe->common.scope = fe->common.scope;
	new_dfe->common.fn_flags = fe->common.fn_flags;
	new_dfe->common.function_name = fe->common.function_name;

	/* replace source with dest */
	if (zend_hash_update(function_table, fname, fname_len + 1, new_dfe, sizeof(zend_function), NULL) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error replacing %s()", fname);
		zend_function_dtor(&func);
		efree(fname);
		efree(new_dfe);
		RETURN_FALSE;
	}
	efree(new_dfe);

	/* rename source */
	if (func.type == ZEND_INTERNAL_FUNCTION) {
		memoize_internal_function mem_func;

		if (!MEMOIZE_G(internal_functions)) {
			ALLOC_HASHTABLE(MEMOIZE_G(internal_functions));
			zend_hash_init(MEMOIZE_G(internal_functions), 4, NULL, NULL, 0);
		}

		mem_func.function = func;
		mem_func.function_table = function_table;
		zend_hash_add(MEMOIZE_G(internal_functions), fname, fname_len + 1, (void*)&mem_func, sizeof(memoize_internal_function), NULL);
	}

	new_fname_len = spprintf(&new_fname, 0, "%s%s", fname, MEMOIZE_FUNC_SUFFIX);
	if (zend_hash_add(function_table, new_fname, new_fname_len + 1, &func, sizeof(zend_function), NULL) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error renaming %s()", fname);
		efree(new_fname);
		zend_function_dtor(&func);
		RETURN_FALSE;
	}
	efree(new_fname);
	efree(fname);

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
