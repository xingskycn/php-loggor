
PHP_ARG_ENABLE(loggor, whether to enable loggor support,
dnl Make sure that the comment is aligned:
[ --enable-loggor Enable loggor support])

if test "$PHP_LOGGOR" != "no"; then
  
  AC_DEFINE(HAVE_LOGGOR, 1, [Whether you have loggor support]) 
  PHP_REQUIRE_CXX()
  
  PHP_ADD_LIBRARY(stdc++, 1, LOGGOR_SHARED_LIBADD)
  PHP_ADD_LIBRARY(jansson, 1, LOGGOR_SHARED_LIBADD)
  PHP_SUBST(LOGGOR_SHARED_LIBADD)
  PHP_NEW_EXTENSION(loggor, loggor.cpp, $ext_shared,,,yes)
fi
