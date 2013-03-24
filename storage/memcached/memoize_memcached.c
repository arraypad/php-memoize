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

#ifdef HAVE_LIBMEMCACHED
# include <ext/standard/php_smart_str.h>
# include <ext/standard/php_var.h>
# include <libmemcached/memcached.h>
# include <stdio.h>
# if defined(HAVE_INTTYPES_H)
#  include <inttypes.h>
# elif defined(HAVE_STDINT_H)
#  include <stdint.h>
# endif
#endif

memoize_storage_module memoize_storage_module_memcached = {
	MEMOIZE_STORAGE_MODULE(memcached)
};

/* {{{ Globals */
ZEND_DECLARE_MODULE_GLOBALS(memoize_memcached);

static PHP_GINIT_FUNCTION(memoize_memcached) 
{
	memoize_memcached_globals->user_connection = NULL;
#ifdef HAVE_LIBMEMCACHED
	memoize_memcached_globals->servers = NULL;
	memoize_memcached_globals->memc = NULL;
#endif
}
/* }}} */

/* {{{ libmemcached */
#ifdef HAVE_LIBMEMCACHED
static int _memoize_memcached_connect(TSRMLS_D) /* {{{ */
{
	char *servers, *server_last, *server_part;
	memcached_return status;
	char *server;
	int ret = SUCCESS;

	if (MEMOIZE_MEMCACHED_G(memc)) {
		return SUCCESS;
	}

	if (!MEMOIZE_MEMCACHED_G(servers)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "No memcached servers defined in memoize.memcached.servers");
		return FAILURE;
	}

	/* create memc */
	MEMOIZE_MEMCACHED_G(memc) = memcached_create(NULL);

	/* add servers */
	servers = estrdup(MEMOIZE_MEMCACHED_G(servers));
	server_part = php_strtok_r(servers, ",", &server_last);
	while (server_part != NULL) {
		server = php_trim(server_part, strlen(server_part), NULL, 0, NULL, 3 TSRMLS_CC);
		if (server) {
			char host[256], *server_dup;
			int has_port, port = 11211;

			has_port = sscanf(server, "%[^:]:%d", host, &port) == 2;
			server_dup = estrdup(has_port ? host : server);
			status = memcached_server_add(MEMOIZE_MEMCACHED_G(memc), server_dup, port);
			if (status != MEMCACHED_SUCCESS) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to add memcached server %s", server);
				ret = FAILURE;
			}
			efree(server_dup);
			efree(server);
		}
		server_part = php_strtok_r(NULL, ",", &server_last);
	}
	efree(servers);

	return ret;
}
/* }}} */

static int _memoize_memcached_get(char *key, zval **value TSRMLS_DC) /* {{{ */
{
	memcached_return_t rc;
	char *data;
	size_t data_len;
	uint32_t flags;
	php_unserialize_data_t var_hash;
	const unsigned char *p;

	if (_memoize_memcached_connect(TSRMLS_C) == FAILURE) {
		return FAILURE;
	}

	data = memcached_get(MEMOIZE_MEMCACHED_G(memc), key, strlen(key), &data_len, &flags, &rc);

	if (rc != MEMCACHED_SUCCESS) {
		return FAILURE;
	}

	p = (const unsigned char*)data;
	PHP_VAR_UNSERIALIZE_INIT(var_hash);
	if (!php_var_unserialize(value, &p, p + data_len, &var_hash TSRMLS_CC)) {
		free(data);
		PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Error unserializing data from memcached");
		return FAILURE;
	}

	free(data);
	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
	return SUCCESS;
}
/* }}} */

static int _memoize_memcached_set(char *key, zval *value, time_t expiry TSRMLS_DC) /* {{{ */
{
	memcached_return_t rc;
	php_serialize_data_t var_hash;
	smart_str buf = {0};

	if (_memoize_memcached_connect(TSRMLS_C) == FAILURE) {
		return FAILURE;
	}

	PHP_VAR_SERIALIZE_INIT(var_hash);
	php_var_serialize(&buf, &value, &var_hash TSRMLS_CC);
	PHP_VAR_SERIALIZE_DESTROY(var_hash);

	if (!buf.c) {
		return FAILURE;
	}

	rc = memcached_set(MEMOIZE_MEMCACHED_G(memc), key, strlen(key), buf.c, strlen(buf.c), expiry, (uint32_t)0);
	smart_str_free(&buf);
	if (rc == MEMCACHED_SUCCESS || rc == MEMCACHED_BUFFERED) {
		return SUCCESS;
	}

	return FAILURE;
}
/* }}} */
#endif
/* }}} */

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("memoize.memcached.servers", "", PHP_INI_ALL, OnUpdateString, servers, zend_memoize_memcached_globals, memoize_memcached_globals)
PHP_INI_END()
/* }}} */

/* {{{ PHP_MINIT_FUNCTION(memoize_memcached) */
PHP_MINIT_FUNCTION(memoize_memcached) 
{
	MEMOIZE_STORAGE_REGISTER(memcached);
	REGISTER_INI_ENTRIES();
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

#ifdef HAVE_LIBMEMCACHED
	if (MEMOIZE_MEMCACHED_G(memc)) {
		memcached_free(MEMOIZE_MEMCACHED_G(memc));
		MEMOIZE_MEMCACHED_G(memc) = NULL;
	}
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ MEMOIZE_GET_FUNC(memcached)
*/
MEMOIZE_GET_FUNC(memcached)
{
	int ret = FAILURE;
	zval *func, *key_zv;
	zval *params[1];

	if (MEMOIZE_MEMCACHED_G(user_connection)) {
		MAKE_STD_ZVAL(key_zv);
		ZVAL_STRING(key_zv, key, 1);

		params[0] = key_zv;

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

		return ret;
	}

#ifdef HAVE_LIBMEMCACHED
	return _memoize_memcached_get(key, value TSRMLS_CC);
#endif

	return ret;
}
/* }}} */

/* {{{ MEMOIZE_SET_FUNC(memcached) */
MEMOIZE_SET_FUNC(memcached)
{
	int ret = FAILURE;
	time_t expiry = 0;
	zval *func, *key_zv, *expiry_zv, retval, *params[3];
	long default_ttl = INI_INT("memoize.default_ttl");

	if (default_ttl) {
		expiry = time(NULL) + default_ttl;
	}

	if (MEMOIZE_MEMCACHED_G(user_connection)) {
		MAKE_STD_ZVAL(key_zv);
		ZVAL_STRING(key_zv, key, 1);

		MAKE_STD_ZVAL(expiry_zv);
		ZVAL_LONG(expiry_zv, expiry);

		params[0] = key_zv;
		params[1] = value;
		params[2] = expiry_zv;

		MAKE_STD_ZVAL(func);
		ZVAL_STRING(func, "set", 1);
		ret = call_user_function(NULL, &MEMOIZE_MEMCACHED_G(user_connection), func, &retval, 3, params TSRMLS_CC);
		zval_ptr_dtor(&func);
		zval_ptr_dtor(&key_zv);
		zval_ptr_dtor(&expiry_zv);

		return ret;
	}

#ifdef HAVE_LIBMEMCACHED
	return _memoize_memcached_set(key, value, expiry TSRMLS_CC);
#endif

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

	if (!instanceof_function(Z_OBJCE_P(obj), *ce TSRMLS_CC)) {
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
