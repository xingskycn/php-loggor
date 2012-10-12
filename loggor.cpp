
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "zend_exceptions.h"
#include "ext/standard/info.h"
#include "php_loggor.hpp"
#include <jansson.h>

void (*old_error_cb)(int type, const char *error_filename,
                     const uint error_lineno, const char *format,
                     va_list args);

void loggor_error_cb(int type, const char *error_filename,
                  const uint error_lineno, const char *format,
                  va_list args);

void loggor_throw_exception_hook(zval *exception TSRMLS_DC);

static void insert_event(int, char *, uint, char * TSRMLS_DC);


/* {{{ loggor_module_entry
 */
zend_module_entry loggor_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"loggor",
	NULL,
	PHP_MINIT(loggor),
	PHP_MSHUTDOWN(loggor),
	PHP_RINIT(loggor),	
	PHP_RSHUTDOWN(loggor),
	PHP_MINFO(loggor),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_LOGGOR
ZEND_GET_MODULE(loggor)
#endif

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(loggor)
{
	/* Storing actual error callback function for later restore */
	old_error_cb = zend_error_cb;
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(loggor)
{
	/* Restoring saved error callback function */
	zend_error_cb = old_error_cb;
	return SUCCESS;
}
/* }}} */

PHP_RINIT_FUNCTION(loggor)
{
        /* Replacing current error callback function with loggor's one */
        zend_error_cb = loggor_error_cb;
        zend_throw_exception_hook = loggor_throw_exception_hook;
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(loggor)
{
	/* Restoring saved error callback function */
	zend_error_cb = old_error_cb;
	zend_throw_exception_hook = NULL;
	return SUCCESS;
}

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(loggor)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "loggor support", "loggor");
	php_info_print_table_end();
}
/* }}} */

/* {{{ void apm_error(int type, const char *format, ...)
   This function provides a hook for error */
void loggor_error_cb(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args)
{
	TSRMLS_FETCH();

	char *msg;
	va_list args_copy;
	zend_module_entry tmp_mod_entry;

	/* A copy of args is needed to be used for the old_error_cb */
	va_copy(args_copy, args);
	vspprintf(&msg, 0, format, args_copy);
	va_end(args_copy);
	
	if( true ) { // Enabled?

		/* We need to see if we have an uncaught exception fatal error now */
		if (type == E_ERROR && strncmp(msg, "Uncaught exception", 18) == 0) {

		} else {
			insert_event(type, (char *) error_filename, error_lineno, msg TSRMLS_CC);
		}
	}
	efree(msg);

	/* Calling saved callback function for error handling, unless xdebug is loaded */
	if (zend_hash_find(&module_registry, "xdebug", 7, (void**) &tmp_mod_entry) != SUCCESS) {
		old_error_cb(type, error_filename, error_lineno, format, args);
	}
}
/* }}} */


void loggor_throw_exception_hook(zval *exception TSRMLS_DC)
{
	if( true ) { // Enabled?
		zval *message, *file, *line;
		zend_class_entry *default_ce, *exception_ce;

		if (!exception) {
			return;
		}

		default_ce = zend_exception_get_default(TSRMLS_C);
		exception_ce = zend_get_class_entry(exception TSRMLS_CC);

		message = zend_read_property(default_ce, exception, "message", sizeof("message")-1, 0 TSRMLS_CC);
		file =    zend_read_property(default_ce, exception, "file",    sizeof("file")-1,    0 TSRMLS_CC);
		line =    zend_read_property(default_ce, exception, "line",    sizeof("line")-1,    0 TSRMLS_CC);

		insert_event(E_ERROR, Z_STRVAL_P(file), Z_LVAL_P(line), Z_STRVAL_P(message) TSRMLS_CC);
	}
}

/* Insert an event in the backend */
static void insert_event(int type, char * error_filename, uint error_lineno, char * msg TSRMLS_DC)
{
  // Create and pack object
  char * json_msg;
  json_t * obj = json_pack("{s:i, s:s, s:i, s:s}",
        "type", type,
        "file", error_filename,
        "line", error_lineno,
        "message", msg);
  json_msg = json_dumps(obj, JSON_ENCODE_ANY);
  
  // Handle json message
  php_log_err(json_msg TSRMLS_CC);
  
  // Free
  free(json_msg);
  free(obj);
}
