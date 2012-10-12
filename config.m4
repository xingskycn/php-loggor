
PHP_ARG_ENABLE(loggor, whether to enable loggor support,
dnl Make sure that the comment is aligned:
[ --enable-loggor Enable loggor support])
PHP_ARG_WITH(jansson, for JSON support,
[  --with-jansson[=DIR]   jansson support])

if test -r $PHP_JANSSON/include/jansson.h; then
  JANSSON_DIR=$PHP_JANSSON
else
  AC_MSG_CHECKING(for jansson in default path)
  for i in /usr/local /usr; do
    if test -r $i/include/jansson.h; then
      JANSSON_DIR=$i
      AC_MSG_RESULT(found in $i)
      break
    fi
  done
fi

if test "$PHP_LOGGOR" != "no"; then
  
  if test -z "$JANSSON_DIR"; then
    AC_MSG_RESULT(not found)
    AC_MSG_ERROR(Please reinstall the jansson distribution)
  fi
  
  PHP_ADD_INCLUDE($JANSSON_DIR/include)
  PHP_ADD_LIBRARY_WITH_PATH(jansson, $JANSSON_DIR/lib, LOGGOR_SHARED_LIBADD)
  AC_DEFINE(HAVE_BZ2_FILTER,1,[ ])

  AC_DEFINE(HAVE_LOGGOR, 1, [Whether you have loggor support]) 
  PHP_REQUIRE_CXX()
  
  PHP_ADD_LIBRARY(stdc++, 1, LOGGOR_SHARED_LIBADD)
  PHP_ADD_LIBRARY(jansson, 1, LOGGOR_SHARED_LIBADD)
  PHP_SUBST(LOGGOR_SHARED_LIBADD)
  PHP_NEW_EXTENSION(loggor, loggor.cpp, $ext_shared,,,yes)
fi
