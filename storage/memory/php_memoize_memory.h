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

#ifndef PHP_MEMOIZE_MEMORY_H
#define PHP_MEMOIZE_MEMORY_H

#define MEMOIZE_MEMORY_EXTVER "0.0.1-dev"

#include "php.h"
#include "php_ini.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef ZTS
# include "TSRM.h"
#endif

#include "ext/memoize/php_memoize.h"
#include "ext/memoize/php_memoize_storage.h"

/* Hook into memoize module */
extern memoize_storage_module memoize_storage_module_memory;
#define memoize_storage_module_memory_ptr &memoize_storage_module_memory

/* Normal PHP entry */
extern zend_module_entry memoize_memory_module_entry;
#define phpext_memoize_storage_memory_ptr &memoize_memory_module_entry

MEMOIZE_STORAGE_FUNCS(memory);

ZEND_BEGIN_MODULE_GLOBALS(memoize_memory)
	HashTable *store;
ZEND_END_MODULE_GLOBALS(memoize_memory)
extern ZEND_DECLARE_MODULE_GLOBALS(memoize_memory)

#ifdef ZTS
#define MEMOIZE_MEMORY_G(v) TSRMG(memoize_memory_globals_id, zend_memoize_memory_globals *, v)
#else
#define MEMOIZE_MEMORY_G(v) (memoize_memory_globals.v)
#endif

#endif

