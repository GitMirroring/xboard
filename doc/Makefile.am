info_TEXINFOS = xboard.texi
xboard_TEXINFOS = copyright.texi
man6_MANS = xboard.man

xboard.man: xboard.texi copyright.texi gpl-3.0.texi version.texi
	$(srcdir)/texi2man $(srcdir)/xboard.texi > xboard.man || (rm -f xboard.man ; false)

CLEANFILES = xboard.man
