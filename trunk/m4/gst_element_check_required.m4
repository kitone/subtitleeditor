## check gstreamer element

AC_DEFUN([SE_GST_ELEMENT_CHECK_REQUIRED],
[
  if test "x$SED" = x; then
    AC_MSG_ERROR([sed not found. Please install it first!])
  fi

	if test "x$EGREP" = x; then
    AC_MSG_ERROR([egrep not found. Please install it first!])
  fi

	toolsdir=`$PKG_CONFIG --variable=toolsdir gstreamer-$1`

	gst_inspect="$toolsdir/gst-inspect-$1"

	if test "x$gst_inspect" != "x"; then
		AC_MSG_CHECKING(for GStreamer-$1 element $2 ($3))
		
		element_version=`$gst_inspect $2 | $EGREP Version | $SED 's/^.*\s$1/$1/'`

		if [ $gst_inspect $2 > /dev/null 2> /dev/null ]; then
			AC_MSG_RESULT([yes])
		else
			AC_MSG_RESULT([no])
			AC_MSG_ERROR([
	GStreamer-$1 plugin '$2' not found. This plugins is 
	absolutely required. Please install it.

	The name of the package you need to install varies among
	distros/systems. Debian and Ubuntu users will need to
	install the $3 package.])
		fi

	else										# test gst_inspect
		AC_MSG_RESULT([no])
		AC_MSG_ERROR([gst-inspect-$1 not found. Please install it.])
	fi
])

