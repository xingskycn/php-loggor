
extern "C" {
#include "php.h"
#include "php_ini.h"
#include "zend_exceptions.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "ext/standard/basic_functions.h"
#include "php_loggor.hpp"
#include <jansson.h>

#include <stdio.h>
#include <string.h>

/* gettimeofday */
#ifdef PHP_WIN32
# include "win32/time.h"
#else
# include "main/php_config.h"
#endif

#if HAVE_SYS_SOCKET_H
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
}

/* {{{ Declarations --------------------------------------------------------- */

ZEND_FUNCTION(loggor_error_log);

ZEND_DECLARE_MODULE_GLOBALS(loggor)
static PHP_MINIT_FUNCTION(loggor);
static PHP_MSHUTDOWN_FUNCTION(loggor);
static PHP_RINIT_FUNCTION(loggor);
static PHP_RSHUTDOWN_FUNCTION(loggor);
static PHP_MINFO_FUNCTION(loggor);
static PHP_GINIT_FUNCTION(loggor);

zend_function *old_error_log_fe = NULL;

void (*old_error_cb)(int type, const char *error_filename,
                     const uint error_lineno, const char *format,
                     va_list args);

void loggor_error_cb(int type, const char *error_filename,
                  const uint error_lineno, const char *format,
                  va_list args);

void loggor_throw_exception_hook(zval *exception TSRMLS_DC);

static void insert_event(int, char *, uint, char * TSRMLS_DC);

const char * loggor_error_type_simple(int type);
const char * loggor_error_type(int type);
const char * loggor_error_type_const(int type);

static void loggor_override_error_log();

/* }}} ---------------------------------------------------------------------- */
/* {{{ Function Entry ------------------------------------------------------- */

ZEND_BEGIN_ARG_INFO_EX(arginfo_loggor_error_log, 0, 0, 1)
    ZEND_ARG_INFO(0, message)
    ZEND_ARG_INFO(0, message_type)
    ZEND_ARG_INFO(0, destination)
    ZEND_ARG_INFO(0, extra_headers)
ZEND_END_ARG_INFO()

static const zend_function_entry loggor_functions[] = {
    ZEND_FE(loggor_error_log, arginfo_loggor_error_log)
    ZEND_FE_END
};

/* }}} ---------------------------------------------------------------------- */
/* {{{ Module Entry --------------------------------------------------------- */

zend_module_entry loggor_module_entry = {
  STANDARD_MODULE_HEADER,
  PHP_LOGGOR_NAME,              /* Name */
  loggor_functions,             /* Functions */
  PHP_MINIT(loggor),            /* MINIT */
  PHP_MSHUTDOWN(loggor),        /* MSHUTDOWN */
  PHP_RINIT(loggor),            /* RINIT */
  PHP_RSHUTDOWN(loggor),        /* RSHUTDOWN */
  PHP_MINFO(loggor),            /* MINFO */
  PHP_LOGGOR_VERSION,           /* Version */
  PHP_MODULE_GLOBALS(loggor),   /* Globals */
  PHP_GINIT(loggor),            /* GINIT */
  NULL,
  NULL,
  STANDARD_MODULE_PROPERTIES_EX
};  

#ifdef COMPILE_DL_LOGGOR 
extern "C" {
  ZEND_GET_MODULE(loggor)
}
#endif

/* }}} ---------------------------------------------------------------------- */
/* {{{ INI Settings --------------------------------------------------------- */

PHP_INI_BEGIN()
  STD_PHP_INI_BOOLEAN("loggor.enabled",     "1", PHP_INI_ALL, OnUpdateBool,   enabled,     zend_loggor_globals, loggor_globals)
  STD_PHP_INI_ENTRY  ("loggor.type_format", "3", PHP_INI_ALL, OnUpdateLong,   type_format, zend_loggor_globals, loggor_globals)
  STD_PHP_INI_BOOLEAN("loggor.php.enabled", "1", PHP_INI_ALL, OnUpdateBool,   php_enabled, zend_loggor_globals, loggor_globals)
  STD_PHP_INI_BOOLEAN("loggor.udp.enabled", "0", PHP_INI_ALL, OnUpdateBool,   udp_enabled, zend_loggor_globals, loggor_globals)
  STD_PHP_INI_ENTRY  ("loggor.udp.host",    "",  PHP_INI_ALL, OnUpdateString, udp_host,    zend_loggor_globals, loggor_globals)
  STD_PHP_INI_ENTRY  ("loggor.udp.port",    "",  PHP_INI_ALL, OnUpdateString, udp_port,    zend_loggor_globals, loggor_globals)
PHP_INI_END()

/* }}} ---------------------------------------------------------------------- */
/* {{{ Module Hooks --------------------------------------------------------- */

static PHP_MINIT_FUNCTION(loggor)
{
  REGISTER_INI_ENTRIES();
  
  REGISTER_LONG_CONSTANT("LOGGOR_TYPE_INT", LOGGOR_TYPE_INT, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("LOGGOR_TYPE_CONST", LOGGOR_TYPE_CONST, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("LOGGOR_TYPE_SIMPLE", LOGGOR_TYPE_SIMPLE, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("LOGGOR_TYPE_STRING", LOGGOR_TYPE_STRING, CONST_CS | CONST_PERSISTENT);
  
  // Storing actual error callback function for later restore
  old_error_cb = zend_error_cb;
  
  // Override the error_log function
  loggor_override_error_log();
  
  return SUCCESS;
}

static PHP_MSHUTDOWN_FUNCTION(loggor)
{
  // Restoring saved error callback function 
  zend_error_cb = old_error_cb;
  return SUCCESS;
}

static PHP_RINIT_FUNCTION(loggor)
{
  // Replacing current error callback function with loggor's one
  zend_error_cb = loggor_error_cb;
  zend_throw_exception_hook = loggor_throw_exception_hook;
  return SUCCESS;
}

static PHP_RSHUTDOWN_FUNCTION(loggor)
{
  // Restoring saved error callback function
  zend_error_cb = old_error_cb;
  zend_throw_exception_hook = NULL;
  return SUCCESS;
}

static PHP_MINFO_FUNCTION(loggor)
{
  php_info_print_table_start();
  php_info_print_table_header(2, "loggor support", "loggor");
  php_info_print_table_row(2, "Version", PHP_LOGGOR_VERSION);
  php_info_print_table_row(2, "Released", PHP_LOGGOR_RELEASE);
  php_info_print_table_row(2, "Revision", PHP_LOGGOR_BUILD);
  php_info_print_table_row(2, "Authors", PHP_LOGGOR_AUTHORS);
  php_info_print_table_end();
  
  DISPLAY_INI_ENTRIES();
}

static PHP_GINIT_FUNCTION(loggor)
{
  loggor_globals->enabled = 1;
  loggor_globals->type_format = 3;
  loggor_globals->php_enabled = 1;
  loggor_globals->udp_enabled = 0;
  loggor_globals->udp_host = NULL;
  loggor_globals->udp_port = NULL;
}

/* }}} ---------------------------------------------------------------------- */
/* {{{ Functions ------------------------------------------------------------ */

ZEND_FUNCTION(loggor_error_log)
{
  char *message, *opt = NULL, *headers = NULL;
  int message_len, opt_len = 0, headers_len = 0;
  int opt_err = 0, argc = ZEND_NUM_ARGS();
  long erropt = 0;

  if( zend_parse_parameters(argc TSRMLS_CC, "s|lss", &message, &message_len, 
          &erropt, &opt, &opt_len, &headers, &headers_len) == FAILURE ) {
    return;
  }

  if( argc > 1 ) {
    opt_err = erropt;
  }

  if( opt_err == 3 && opt ) {
    if( strlen(opt) != opt_len ) {
      RETURN_FALSE;
    }
  }
  
  // End stolen from error_log
  char * error_filename = NULL;
  uint error_lineno = 0;
  char * msg = (char *) emalloc(sizeof(char) * (message_len + 1));
  memcpy(msg, message, message_len);
  *(msg + message_len) = 0;
  if( zend_is_compiling(TSRMLS_C) ) {
    error_filename = zend_get_compiled_filename(TSRMLS_C);
    error_lineno = zend_get_compiled_lineno(TSRMLS_C);
  } else if (zend_is_executing(TSRMLS_C)) {
    error_filename = zend_get_executed_filename(TSRMLS_C);
    error_lineno = zend_get_executed_lineno(TSRMLS_C);
  } else {
    error_filename = NULL;
    error_lineno = 0;
  }
  insert_event(E_NOTICE, error_filename, error_lineno, msg);
  // Resume stolen from error_log

  if( _php_error_log_ex(opt_err, message, message_len, opt, headers TSRMLS_CC) == FAILURE ) {
    RETURN_FALSE;
  }
  
  RETURN_TRUE;
}

/* }}} ---------------------------------------------------------------------- */

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

  if( LOGGOR_G(enabled) ) {
    // We need to see if we have an uncaught exception fatal error now
    if (type == E_ERROR && strncmp(msg, "Uncaught exception", 18) == 0) {

    } else {
      insert_event(type, (char *) error_filename, error_lineno, msg TSRMLS_CC);
    }
  }
  efree(msg);

  /* Calling saved callback function for error handling, unless xdebug is loaded */
  if( zend_hash_find(&module_registry, "xdebug", 7, (void**) &tmp_mod_entry) != SUCCESS ) {
    old_error_cb(type, error_filename, error_lineno, format, args);
  }
}

void loggor_throw_exception_hook(zval *exception TSRMLS_DC)
{
  if( LOGGOR_G(enabled) ) {
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

static void insert_event(int type, char * error_filename, uint error_lineno, char * msg TSRMLS_DC)
{
  char * json_msg;
  struct timeval ts;
  json_t * obj;
  
  // Get timestamp
  gettimeofday(&ts, NULL);
  
  // Create and pack object
  obj = json_pack("{s:s, s:i, s:s, s:f}",
        "file", error_filename,
        "line", error_lineno,
        "message", msg,
        "time", (double) ts.tv_sec + USEC_TO_SEC(ts.tv_usec));
  
  // Format and add type
  switch( LOGGOR_G(type_format) ) {
    case LOGGOR_TYPE_INT:
      json_object_set_new(obj, "type", json_integer(type));
      break;
    case LOGGOR_TYPE_CONST:
      json_object_set_new(obj, "type", json_string(loggor_error_type_const(type)));
      break;
    case LOGGOR_TYPE_SIMPLE:
      json_object_set_new(obj, "type", json_string(loggor_error_type_simple(type)));
      break;
    case LOGGOR_TYPE_STRING:
    default:
      json_object_set_new(obj, "type", json_string(loggor_error_type(type)));
      break;
  }
  
  // Add hostname if available
#if HAVE_SYS_SOCKET_H
  char hostname_buf[255];
  if( gethostname(hostname_buf, sizeof(hostname_buf) - 1) == 0 ) {
    json_object_set_new(obj, "hostname", json_string(hostname_buf));
  }
#endif
  
  // Convert to string
  json_msg = json_dumps(obj, JSON_SORT_KEYS);
  
  // PHP Default
  if( LOGGOR_G(php_enabled) ) {
    php_log_err(json_msg TSRMLS_CC);
  }
  
  // UDP
#if HAVE_SYS_SOCKET_H
  if( LOGGOR_G(udp_enabled) ) {
    struct sockaddr_in sock;
    int s, i;
    char * udp_host = LOGGOR_G(udp_host);
    int udp_port = atoi(LOGGOR_G(udp_port));

    if( (s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1 ) {
      // Could not open UDP socket
      fprintf(stderr, "socket() failed\n");
    } else {
      memset((char *) &sock, 0, sizeof(sock));
      sock.sin_family = AF_INET;
      sock.sin_port = htons(udp_port);
      if( inet_aton(udp_host, &sock.sin_addr) == 0 ) {
        // Could not aton?
        fprintf(stderr, "inet_aton() failed\n");
      } else {
        if( sendto(s, (const char *) json_msg, strlen(json_msg), 0, (struct sockaddr*) &sock, sizeof(sock)) == -1 ) {
          // Could not send packet
          fprintf(stderr, "sendto() failed\n");
        } else {
          // Ok
        }
      }
      close(s);
    }
  }
#endif
  
  // Free
  free(json_msg);
  free(obj);
}

const char * loggor_error_type_simple(int type)
{
	switch (type) {
		case E_ERROR:
		case E_CORE_ERROR:
		case E_COMPILE_ERROR:
		case E_USER_ERROR:
			return "fatal-error";
			break;
#if PHP_VERSION_ID >= 50200
		case E_RECOVERABLE_ERROR:
			return "catchable-fatal-error";
			break;
#endif
		case E_WARNING:
		case E_CORE_WARNING:
		case E_COMPILE_WARNING:
		case E_USER_WARNING:
			return "warning";
			break;
		case E_PARSE:
			return "parse-error";
			break;
		case E_NOTICE:
		case E_USER_NOTICE:
			return "notice";
			break;
		case E_STRICT:
			return "strict-standards";
			break;
#if PHP_VERSION_ID >= 50300
		case E_DEPRECATED:
		case E_USER_DEPRECATED:
			return "deprecated";
			break;
#endif
		case 0:
			return "xdebug";
			break;
		default:
			return "unknown-error";
			break;
	}
}

const char * loggor_error_type(int type)
{
	switch (type) {
		case E_ERROR:
		case E_CORE_ERROR:
		case E_COMPILE_ERROR:
		case E_USER_ERROR:
			return "Fatal error";
			break;
#if PHP_VERSION_ID >= 50200
		case E_RECOVERABLE_ERROR:
			return "Catchable fatal error";
			break;
#endif
		case E_WARNING:
		case E_CORE_WARNING:
		case E_COMPILE_WARNING:
		case E_USER_WARNING:
			return "Warning";
			break;
		case E_PARSE:
			return "Parse error";
			break;
		case E_NOTICE:
		case E_USER_NOTICE:
			return "Notice";
			break;
		case E_STRICT:
			return "Strict standards";
			break;
#if PHP_VERSION_ID >= 50300
		case E_DEPRECATED:
		case E_USER_DEPRECATED:
			return "Deprecated";
			break;
#endif
		default:
			return "Unknown error";
			break;
	}
}

const char * loggor_error_type_const(int type)
{
	switch (type) {
		case E_ERROR:
			return "E_ERROR";
			break;
		case E_CORE_ERROR:
			return "E_CORE_ERROR";
			break;
		case E_COMPILE_ERROR:
			return "E_COMPILE_ERROR";
			break;
		case E_USER_ERROR:
			return "E_USER_ERROR";
			break;
#if PHP_VERSION_ID >= 50200
		case E_RECOVERABLE_ERROR:
			return "E_RECOVERABLE_ERROR";
			break;
#endif
		case E_WARNING:
			return "E_WARNING";
			break;
		case E_CORE_WARNING:
			return "E_CORE_WARNING";
			break;
		case E_COMPILE_WARNING:
			return "E_COMPILE_WARNING";
			break;
		case E_USER_WARNING:
			return "E_USER_WARNING";
			break;
		case E_PARSE:
			return "E_PARSE";
			break;
		case E_NOTICE:
			return "E_NOTICE";
			break;
		case E_USER_NOTICE:
			return "E_USER_NOTICE";
			break;
		case E_STRICT:
			return "E_STRICT";
			break;
#if PHP_VERSION_ID >= 50300
		case E_DEPRECATED:
			return "E_DEPRECATED";
			break;
		case E_USER_DEPRECATED:
			return "E_USER_DEPRECATED";
			break;
#endif
		default:
			return "UNKNOWN";
			break;
	}
}

static void loggor_override_error_log()
{
  char * fname = estrdup("error_log");
  char * nfname = estrdup("loggor_error_log");
  long fname_len = strlen(fname);
  long nfname_len = strlen(nfname);
  zend_function *fe, *nfe;
  
  // Already overridden
  if( old_error_log_fe ) {
    return;
  }
  
  // Get new function entry
  if( zend_hash_find(CG(function_table), nfname, nfname_len + 1, (void **)&nfe) == FAILURE ) {
      zend_error(E_WARNING, "%s symbol not found", nfname);
      return;
  }
  
  // Get old function entry
  if( zend_hash_find(CG(function_table), fname, fname_len + 1, (void **)&fe) == FAILURE ) {
      zend_error(E_WARNING, "%s symbol not found", fname);
      return;
  }
  
  // Update function entry
  if( zend_hash_update(CG(function_table), fname, fname_len + 1, (void *) nfe, sizeof(zend_function), NULL) == FAILURE ) {
    zend_error(E_WARNING, "Error updating reference to function name %s()", fname);
    return;
  }
  
  // Increse ref count for loggor_error_log?
  function_add_ref(nfe);
  
  // Store reference to old function
  old_error_log_fe = fe;
}
