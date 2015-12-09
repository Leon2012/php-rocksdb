PHP_ARG_WITH(rocksdb, for rocksdb support,
  [  --with-rocksdb             Include rocksdb support])


dnl PHP_ARG_ENABLE(rocksdb, whether to enable rocksdb support,
dnl Make sure that the comment is aligned:
dnl [  --enable-rocksdb           Enable rocksdb support])

if test "$PHP_ROCKSDB" != "no"; then

  SEARCH_PATH="/usr/local /usr"     # you might want to change this
  SEARCH_FOR="/include/rocksdb/c.h"  # you most likely want to change this
  SEARCH_LIB="/lib/librocksdb.a"
  if test -r $PHP_ROCKSDB/$SEARCH_FOR; then # path given as parameter
     ROCKSDB_DIR=$PHP_ROCKSDB
  else # search default path list
     AC_MSG_CHECKING([for rocksdb files in default path])
     for i in $SEARCH_PATH ; do
       if test -r $i/$SEARCH_FOR && test -r $i/$SEARCH_LIB; then
         ROCKSDB_DIR=$i
         AC_MSG_RESULT(found in $i)
       fi
     done
  fi
  
  if test -z "$ROCKSDB_DIR"; then
     AC_MSG_RESULT([not found])
     AC_MSG_ERROR([Please reinstall the rocksdb distribution])
  fi

  dnl # --with-rocksdb -> add include path
  PHP_ADD_INCLUDE($ROCKSDB_DIR/include)

  dnl # --with-rocksdb -> check for lib and symbol presence
  LIBNAME=rocksdb # you may want to change this
  LIBSYMBOL=rocksdb_open # you most likely want to change this 

  PHP_REQUIRE_CXX()
  PHP_ADD_LIBRARY(stdc++, 1, ROCKSDB_SHARED_LIBADD)

  dnl  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
     PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $ROCKSDB_DIR, ROCKSDB_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_ROCKSDBLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong rocksdb lib version or lib not found])
  dnl ],[
  dnl   -L$ROCKSDB_DIR/$PHP_LIBDIR -lm
  dnl ])
  
  PHP_SUBST(ROCKSDB_SHARED_LIBADD)

  PHP_NEW_EXTENSION(rocksdb, rocksdb.c, $ext_shared)
fi
