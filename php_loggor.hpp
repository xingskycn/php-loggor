
#ifndef PHP_LOGGOR_H
#define PHP_LOGGOR_H

extern zend_module_entry loggor_module_entry;
#define phpext_loggor_ptr &loggor_module_entry

PHP_MINIT_FUNCTION(loggor);
PHP_MSHUTDOWN_FUNCTION(loggor);
PHP_RINIT_FUNCTION(loggor);
PHP_RSHUTDOWN_FUNCTION(loggor);
PHP_MINFO_FUNCTION(loggor);

#endif