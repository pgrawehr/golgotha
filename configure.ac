# Process this file with autoconf to produce a configure script.
AC_INIT(Golgotha, V1.0.8.3 , golgotha@qbic.vipnet.ro)
AC_CONFIG_SRCDIR([golg__main.cpp])
#AC_PREREQ(2.57)
#AC_CONFIG_HEADER([config.h])

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
#LIBS="-I$x_includes -L$x_libraries $LIBS" 
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
#AC_CHECK_LIB([stdc++], [__cxa_pure_virtual])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h memory.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

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

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
