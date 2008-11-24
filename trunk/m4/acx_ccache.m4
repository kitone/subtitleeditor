dnl --------------------------------------------------------------------------
dnl ACX_CCACHE
dnl
dnl Checks if ccache is requested and available, and makes use of it
dnl --------------------------------------------------------------------------
AC_DEFUN([ACX_CCACHE],
[
	AC_ARG_ENABLE([ccache],
		[AS_HELP_STRING([--enable-ccache], [enable ccache support for fast recompilation])])

	AC_ARG_WITH([ccache-prefix],
		[AS_HELP_STRING([--with-ccache-prefix=PREFIX], [prefix where ccache is installed])])

	AC_MSG_CHECKING([whether ccache support should be added])
	AC_MSG_RESULT([${enable_ccache:-no}])

	AS_IF([test ${enable_ccache:-no} = yes], [
		AC_MSG_CHECKING([for ccache presence])
		AS_IF([test -z "$with_ccache_prefix"], [
			ccache_full=`which ccache`
			with_ccache_prefix=`dirname ${ccache_full}`
		])
		AS_IF([$with_ccache_prefix/ccache -V >/dev/null 2>&1], [
			AC_MSG_RESULT([yes])
			CC="$with_ccache_prefix/ccache $CC"
			CXX="$with_ccache_prefix/ccache $CXX"
			BUILD_CC="$with_ccache_prefix/ccache $BUILD_CC"
		], [
			enable_ccache=no
			AC_MSG_RESULT([no])
		])
	])
])
