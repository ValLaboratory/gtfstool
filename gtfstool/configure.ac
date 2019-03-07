#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([gtfstool], [0.2], [yamamoto@val.co.jp])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([gtfstool.c])
AC_CONFIG_HEADERS([config.h])

# Initialize prefix.
if test "$prefix" = "NONE"; then
   prefix="/usr/local"
fi

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([iconv], [iconv])
AC_CHECK_LIB([z], [deflate])
AC_CHECK_LIB([ssl], [SSL_library_init])
AC_CHECK_LIB([crypto], [ERR_get_error])
#AC_CHECK_LIB([xml2], [xmlReadMemory])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h memory.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h unistd.h utime.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_CHECK_FUNCS([clock_gettime gethostbyaddr gethostbyname gettimeofday inet_ntoa memmove memset mkdir realpath select socket sqrt strchr strerror strrchr strstr utime])
AC_CHECK_FUNCS([iconv])
AC_CHECK_FUNC([epoll_create],
  [AC_DEFINE([HAVE_EPOLL], 1, [Define if your system supports the epoll system call])], [])
AC_CHECK_FUNC([kqueue],
  [AC_DEFINE([HAVE_KQUEUE], 1, [Define if your system supports the kqueue system call])], [])

# check optional
AC_CHECK_LIB([iconv], [iconv],
  [AC_DEFINE([HAVE_ICONV], 1, [Define if your system supports the libiconv])])
AC_CHECK_LIB([z], [deflate],
  [AC_DEFINE([HAVE_ZLIB], 1, [Define if your system supports the libz])]) 
AC_CHECK_LIB([ssl], [SSL_library_init],
  [AC_DEFINE([HAVE_OPENSSL], 1, [Define if your system supports the OPENSSL])])
#AC_CHECK_LIB([xml2], [xmlReadMemory],
#  [AC_DEFINE([HAVE_LIBXML2], 1, [Define if your system supports the LIBXML2])])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
