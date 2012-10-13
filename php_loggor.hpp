
#ifndef PHP_LOGGOR_H
#define PHP_LOGGOR_H

extern "C" {
  
#define PHP_LOGGOR_NAME "loggor"  
#define PHP_LOGGOR_VERSION "0.1.0"
#define PHP_LOGGOR_RELEASE "2012-10-12"
#define PHP_LOGGOR_BUILD "master"
#define PHP_LOGGOR_AUTHORS "John Boehr <jbboehr@gmail.com> (lead)"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#define LOGGOR_TYPE_INT 0
#define LOGGOR_TYPE_CONST 1
#define LOGGOR_TYPE_SIMPLE 2
#define LOGGOR_TYPE_STRING 3
  
extern "C" zend_module_entry loggor_module_entry;
#define phpext_loggor_ptr &loggor_module_entry

ZEND_BEGIN_MODULE_GLOBALS(loggor)
  zend_bool  enabled;
  int        error_reporting;
  zend_bool  php_enabled;
  zend_bool  udp_enabled;
  char      *udp_host;
  char      *udp_port;
  zend_bool  silence;
  int        type_format;
ZEND_END_MODULE_GLOBALS(loggor)
        
ZEND_EXTERN_MODULE_GLOBALS(loggor);

#ifdef ZTS
#define LOGGOR_G(v) TSRMG(loggor_globals_id, zend_loggor_globals *, v)
#else
#define LOGGOR_G(v) (loggor_globals.v)
#endif

#define SEC_TO_USEC(sec) ((sec) * 1000000.00)
#define USEC_TO_SEC(usec) ((usec) / 1000000.00)

} // extern "C" 

#endif