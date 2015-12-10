/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_rocksdb.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <rocksdb/c.h>

/* If you declare any globals in php_rocksdb.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(rocksdb)
*/

static int le_rocksdb;
static zend_class_entry *rocksdb_ce;
static zend_object_handlers rocksdb_handlers;

typedef struct _rocksdb_object{
	zend_object obj;
	rocksdb_t *db;
} rocksdb_object;

#define ROCKSDB_FETCH_OBJECT(zobj) (rocksdb_object *)zend_object_store_get_object((zobj) TSRMLS_CC)

//zend_object析构方法
void rocksdb_dtor(void *object TSRMLS_DC) {
	rocksdb_object *objval = (rocksdb_object *)object;
	if (objval->db != NULL) {
		rocksdb_close(objval->db);
		objval->db = NULL;
	}
	zend_object_std_dtor(&(objval->obj) TSRMLS_CC);
	efree(objval);//释放对像
}

//zend_object构造方法
static zend_object_value rocksdb_ctor(zend_class_entry *ce TSRMLS_DC) {
	rocksdb_object *objval = (rocksdb_object *)emalloc(sizeof(rocksdb_object));
	memset(objval, 0, sizeof(rocksdb_object));

	zend_object_value retval;
	zend_object_std_init(&(objval->obj), ce TSRMLS_DC);

	retval.handle = zend_objects_store_put(objval, NULL, (zend_objects_free_object_storage_t)rocksdb_dtor, NULL TSRMLS_CC);
	retval.handlers = &rocksdb_handlers;

	return retval;
}


//class method arg
ZEND_BEGIN_ARG_INFO(Rocksdb__construct_arginfo, 0)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()	

ZEND_BEGIN_ARG_INFO(Rocksdb_set_arginfo, 0)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(Rocksdb_get_arginfo, 0)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()
		

static PHP_METHOD(Rocksdb, __construct) {
	rocksdb_object *objval = ROCKSDB_FETCH_OBJECT(getThis());
	char *path;
	int path_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len) == FAILURE) {
		RETURN_FALSE;
	}

	if (path == NULL || path_len == 0) {
		RETURN_FALSE;
	}
	
	rocksdb_t *db;

	rocksdb_options_t *options = rocksdb_options_create();
	long cpus = sysconf(_SC_NPROCESSORS_ONLN);  // get # of online cores
	rocksdb_options_increase_parallelism(options, (int)(cpus));
	rocksdb_options_optimize_level_style_compaction(options, 0);

	rocksdb_options_set_create_if_missing(options, 1);
	
	char *err = NULL;
	db = rocksdb_open(options, path, &err);
	if (err != NULL) {
		rocksdb_options_destroy(options);
		RETURN_FALSE;
	}
	
	objval->db = db;
	rocksdb_options_destroy(options);
}

static PHP_METHOD(Rocksdb, del) {
	rocksdb_object *objval = ROCKSDB_FETCH_OBJECT(getThis());
	char *key;
	int key_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
		RETURN_FALSE;
	}

	if (key == NULL || key_len == 0) {
		RETURN_FALSE;
	}
	

	rocksdb_writeoptions_t *writeoptions = rocksdb_writeoptions_create();
	char *err = NULL;
	rocksdb_delete(objval->db, writeoptions, key, key_len, &err);
	rocksdb_writeoptions_destroy(writeoptions);

	if (err != NULL) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

static PHP_METHOD(Rocksdb, set) {
	rocksdb_object *objval = ROCKSDB_FETCH_OBJECT(getThis());
	char *key;
	int key_len;
	char *value;
	int value_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &key, &key_len, &value, &value_len) == FAILURE) {
		RETURN_FALSE;
	}
	
	if (key == NULL || key_len == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "key is empty!!!");
		RETURN_FALSE;
	}

	rocksdb_writeoptions_t *writeoptions = rocksdb_writeoptions_create();
	char *err = NULL;
	rocksdb_put(objval->db, writeoptions, key, key_len, value, value_len, &err);
	rocksdb_writeoptions_destroy(writeoptions);

	if (err != NULL) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

static PHP_METHOD(Rocksdb, get) {
	rocksdb_object *objval = ROCKSDB_FETCH_OBJECT(getThis());
	char *key;
	int key_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
		RETURN_FALSE;
	}
	
	if (key == NULL || key_len == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "key is empty!!!");
		RETURN_FALSE;
	}

	rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
	size_t len;
	char *err = NULL;
	char *value = rocksdb_get(objval->db, readoptions, key, key_len, &len, &err);
	
	rocksdb_readoptions_destroy(readoptions);
	
	if (err != NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, err);
		RETURN_FALSE;
	}
	if  (value == NULL) {
		RETURN_FALSE;
	}

	RETURN_STRING(value, 1);
}


static PHP_METHOD(Rocksdb, close) {
	rocksdb_object *objval = ROCKSDB_FETCH_OBJECT(getThis());
	if (objval->db != NULL) {
		rocksdb_close(objval->db);
		objval->db = NULL;
	}
}



/* {{{ php_rocksdb_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_rocksdb_init_globals(zend_rocksdb_globals *rocksdb_globals)
{
	rocksdb_globals->global_value = 0;
	rocksdb_globals->global_string = NULL;
}
*/
/* }}} */


static zend_function_entry rocksdb_methods[] = {
	PHP_ME(Rocksdb, __construct, Rocksdb__construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(Rocksdb, set, Rocksdb_set_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Rocksdb, get, Rocksdb_get_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Rocksdb, del, Rocksdb_get_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Rocksdb, close, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};




/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(rocksdb)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/

	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "Rocksdb", rocksdb_methods);
	rocksdb_ce = zend_register_internal_class(&ce TSRMLS_CC);
	rocksdb_ce->create_object = rocksdb_ctor;
	memcpy(&rocksdb_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	rocksdb_handlers.clone_obj = NULL;

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(rocksdb)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(rocksdb)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(rocksdb)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(rocksdb)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "rocksdb support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* {{{ rocksdb_functions[]
 *
 * Every user visible function must have an entry in rocksdb_functions[].
 */
const zend_function_entry rocksdb_functions[] = {
	PHP_FE_END	/* Must be the last line in rocksdb_functions[] */
};
/* }}} */

/* {{{ rocksdb_module_entry
 */
zend_module_entry rocksdb_module_entry = {
	STANDARD_MODULE_HEADER,
	"rocksdb",
	rocksdb_functions,
	PHP_MINIT(rocksdb),
	PHP_MSHUTDOWN(rocksdb),
	PHP_RINIT(rocksdb),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(rocksdb),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(rocksdb),
	PHP_ROCKSDB_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_ROCKSDB
ZEND_GET_MODULE(rocksdb)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
