# Process this file with autoconf to produce a configure script.
AC_INIT(Golgotha, V1.0.8.3 , golgotha@qbic.vipnet.ro)



AC_CONFIG_SRCDIR([golg__main.cpp])
#AC_PREREQ(2.57)
AC_CONFIG_HEADER([config.h])


# Checks for programs.
AC_PROG_CXX([icc gcc34 gcc33 gcc32 gcc31 gcc30 gcc])
AC_PROG_CC([gcc34 gcc33 gcc32 gcc31 gcc30 gcc])
AC_PROG_MAKE_SET
AC_PROG_CPP
AC_LANG([C++])
AC_REQUIRE_CPP
AC_CHECK_LIB([stdc++], [__cxa_pure_virtual])
AC_MSG_CHECKING([wheter the compiler accepts -pthread])
libsave=$LIBS
LIBS="-pthread $LIBS"
AC_LINK_IFELSE([[int main() {int i; i++;}]],[LINKER_NEEDS_PTHREAD=Yes],
[LIBS=$libsave
LINKER_NEEDS_PTHREAD=No])
AC_MSG_RESULT([$LINKER_NEEDS_PTHREAD])
AC_PATH_X
AC_PATH_XTRA
 
# Checks for libraries.
# FIXME: Replace `main' with a function in `-lGL':
#AC_CHECK_LIB([GL], [glEnable])
# FIXME: Replace `main' with a function in `-lGLU':
# AC_CHECK_LIB([GLU], [main])
# FIXME: Replace `main' with a function in `-lX11':
AC_CHECK_LIB([X11], [XOpenDisplay])
# FIXME: Replace `main' with a function in `-lc_r':
#AC_CHECK_LIB([c_r], [main])
# FIXME: Replace `main' with a function in `-lgcc':
#AC_CHECK_LIB([gcc], [main])
# FIXME: Replace `main' with a function in `-lm':
#AC_CHECK_LIB([m], [sqrt])
AC_CHECK_LIB([cipher],[encrypt])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h memory.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])
#AC_CHECK_HEADERS([netinet/ip.h])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET
# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_C_BIGENDIAN
AC_C_VOLATILE
AC_TYPE_SIZE_T
AC_C_INLINE
AC_HEADER_TIME
AC_STRUCT_TM
AC_CHECK_TYPES([ptrdiff_t])
AC_SYS_LONG_FILE_NAMES


# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([floor getcwd gethostbyname gethostname gettimeofday memmove memset mkdir munmap pow realpath select socket sqrt strcasecmp strchr strcspn strdup strndup strrchr strstr])
#AC_CHECK_FUNCS([pthread_mutexattr_settype pthread_mutexattr_setkind_np])
AC_MSG_CHECKING([how to initialize recursive mutexes])
#AC_DEFINE([HAVE_PTHREAD_MUTEX_RECURSIVE],0)
ac_cv_mutex_recursive1=unknown
AC_COMPILE_IFELSE(
[
#include <pthread.h>
int main(void)
{
  int k=PTHREAD_MUTEX_RECURSIVE;
  return 0;
}

],
[
AC_DEFINE([HAVE_PTHREAD_MUTEX_RECURSIVE],1,"How to set recursive mutexes?")
ac_cv_mutex_recursive1=PTHREAD_MUTEX_RECURSIVE
])
if test $ac_cv_mutex_recursive1 = default; then
AC_COMPILE_IFELSE(
[
#include <pthread.h>
int main(void)
{
  int k=PTHREAD_MUTEX_RECURSIVE_NP;
  return 0;
}
],
[
AC_DEFINE([HAVE_PTHREAD_MUTEX_RECURSIVE_NP],1,"Recursive_NP present")
ac_cv_mutex_recursive1=PTHREAD_MUTEX_RECURSIVE_NP
])
fi
AC_MSG_RESULT([$ac_cv_mutex_recursive1])
AC_MSG_CHECKING([how to set mutexes recursive])
ac_cv_mutex_recursive2="unknown"
AC_COMPILE_IFELSE(
[#include <pthread.h>
int main(void)
{
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setkind_np(&attr,$ac_cv_mutex_recursive1);
}
],
[AC_DEFINE([HAVE_PTHREAD_MUTEXATTR_SETKIND_NP],1,"setkind") 
ac_cv_mutex_recursive2="setkind_np"
])
AC_COMPILE_IFELSE(
[#include <pthread.h>
int main(void)
{
pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_settype(&attr,$ac_cv_mutex_recursive1);
}],
[
AC_DEFINE([HAVE_PTHREAD_MUTEXATTR_SETTYPE],1,"settype")
ac_cv_mutex_recursive2="settype"
])
AC_MSG_RESULT([$ac_cv_mutex_recursive2])
AC_SUBST(debug_or_release)
debug_or_release="release"
AC_MSG_CHECKING([for --with-debug flag])
AC_ARG_WITH([debug],
AC_HELP_STRING([--with-debug],
[Enable debugging support. Defaults to no])
,
[if test withval=yes; then
#Passing something like -O2 -O0 results in the last one being effective
  CXXFLAGS="$CXXFLAGS -O0 -D _DEBUG"
  debug_or_release="debug"
fi
AC_MSG_RESULT([$debug_or_release])
],
[AC_MSG_RESULT([$debug_or_release])])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
