## intltool_se_plugin.m4 - Adapt intltool for plugin description files. -*-Shell-script-*-

dnl IT_SE_PLUGIN_INTLTOOL
AC_DEFUN([IT_SE_PLUGIN_INTLTOOL],
[AC_REQUIRE([IT_PROG_INTLTOOL])dnl

INTLTOOL_SE_PLUGIN_RULE='%.se-plugin: %.se-plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; LC_ALL=C $(INTLTOOL_MERGE) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< [$]@' 
AC_SUBST(INTLTOOL_SE_PLUGIN_RULE)

])dnl
