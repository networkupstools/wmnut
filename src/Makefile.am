bin_PROGRAMS = wmnut

wmnut_SOURCES = \
	wmnut.c \
	wmnut-common.h \
	wmnut.h \
	wmgeneral.c \
	wmgeneral.h \
	compat.c \
	rcfiles.c \
	wmnut_mask.xbm \
	wmnut_master_LowColor.xpm \
	wmnut_master.xpm 

AM_CFLAGS =  \
	@HEADER_SEARCH_PATH@ \
	@NUT_HEADER@ \
	@XCFLAGS@ \
	@DBGFLAGS@ \
	@CFLAGS@

wmnut_LDADD = \
	@LIBRARY_SEARCH_PATH@ \
	@XLIBS@ \
	@XLFLAGS@ \
	@NUT_LIB@

INSTALL_PROGRAM = \
	@INSTALL_PROGRAM@ \
	$(AM_INSTALL_PROGRAM_FLAGS)

clean-local:
	rm -rf .deps
