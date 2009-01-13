## intltool_se_plugin.m4 - Adapt intltool for pattern files. -*-Shell-script-*-

dnl IT_SE_PATTERN_INTLTOOL
AC_DEFUN([IT_SE_PATTERN_INTLTOOL],
[AC_REQUIRE([IT_PROG_INTLTOOL])dnl

INTLTOOL_SE_PATTERN_RULE='%.se-pattern: %.se-pattern.in $(INTLTOOL_MERGE) ; LC_ALL=C $(INTLTOOL_MERGE) -x -u /tmp $< [$]@' 
AC_SUBST(INTLTOOL_SE_PATTERN_RULE)

])dnl
