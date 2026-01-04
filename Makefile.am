dist_sysconf_DATA = wmnutrc

EXTRA_DIST = BUGS ChangeLog README.asciidoc autogen.sh

# Files referenced by main README.asciidoc
EXTRA_DIST += \
	images/WMnut_desc.png \
	images/wmnut-0_09w.jpg \
	images/wmnut-0_09win.jpg \
	images/wmnut-0_60kde.jpg \
	images/wmnut-pr.pdf

SUBDIRS = src icons contrib doc

# ----------------------------------------------------------------------
# Maintainers targets: distribution signature and hashes
# Assume tools are available (and maintainer GPG keys)
dist-files: dist dist-sig dist-hash

wmnut-@PACKAGE_VERSION@.tar.gz: dist
wmnut-@PACKAGE_VERSION@.tar.gz.sig: dist-sig
wmnut-@PACKAGE_VERSION@.tar.gz.md5 wmnut-@PACKAGE_VERSION@.tar.gz.sha256: dist-hash

dist-sig: wmnut-@PACKAGE_VERSION@.tar.gz
	rm -f wmnut-@PACKAGE_VERSION@.tar.gz.sig
	gpg --detach-sign wmnut-@PACKAGE_VERSION@.tar.gz

dist-hash: wmnut-@PACKAGE_VERSION@.tar.gz
	md5sum wmnut-@PACKAGE_VERSION@.tar.gz > wmnut-@PACKAGE_VERSION@.tar.gz.md5
	sha256sum wmnut-@PACKAGE_VERSION@.tar.gz > wmnut-@PACKAGE_VERSION@.tar.gz.sha256

# Partially snatched from automake generated code
distcheck-completeness: dist
	@chmod -R +w $(distdir) $(distdir)-orig-* $(distdir)-derived-* || true
	@rm -rf $(distdir) $(distdir)-orig-* $(distdir)-derived-* || true
	@case '$(DIST_ARCHIVES)' in \
	*.tar.gz*) \
	  eval GZIP= gzip $(GZIP_ENV) -dc $(distdir).tar.gz | $(am__untar) ;;\
	*.tar.bz2*) \
	  bzip2 -dc $(distdir).tar.bz2 | $(am__untar) ;;\
	*.tar.lz*) \
	  lzip -dc $(distdir).tar.lz | $(am__untar) ;;\
	*.tar.xz*) \
	  xz -dc $(distdir).tar.xz | $(am__untar) ;;\
	*.tar.Z*) \
	  uncompress -c $(distdir).tar.Z | $(am__untar) ;;\
	*.shar.gz*) \
	  eval GZIP= gzip $(GZIP_ENV) -dc $(distdir).shar.gz | unshar ;;\
	*.zip*) \
	  unzip $(distdir).zip ;;\
	*.tar.zst*) \
	  zstd -dc $(distdir).tar.zst | $(am__untar) ;;\
	esac
	+@RES=0 ; \
	{ cp -rf $(distdir) $(distdir)-orig-$$$$ \
	  && am__cwd="`pwd`" \
	  && $(am__cd) $(distdir) \
	  && ./configure \
	      $(AM_DISTCHECK_CONFIGURE_FLAGS) \
	      $(DISTCHECK_CONFIGURE_FLAGS) \
	      --prefix="`pwd`/.inst" \
	  && $(MAKE) $(AM_MAKEFLAGS) dist \
	  && case '$(DIST_ARCHIVES)' in \
	      *.tar.gz*) \
	        eval GZIP= gzip $(GZIP_ENV) -dc $(distdir).tar.gz | $(am__untar) ;;\
	      *.tar.bz2*) \
	        bzip2 -dc $(distdir).tar.bz2 | $(am__untar) ;;\
	      *.tar.lz*) \
	        lzip -dc $(distdir).tar.lz | $(am__untar) ;;\
	      *.tar.xz*) \
	        xz -dc $(distdir).tar.xz | $(am__untar) ;;\
	      *.tar.Z*) \
	        uncompress -c $(distdir).tar.Z | $(am__untar) ;;\
	      *.shar.gz*) \
	        eval GZIP= gzip $(GZIP_ENV) -dc $(distdir).shar.gz | unshar ;;\
	      *.zip*) \
	        unzip $(distdir).zip ;;\
	      *.tar.zst*) \
	        zstd -dc $(distdir).tar.zst | $(am__untar) ;;\
	      esac \
	  && mv $(distdir) ../$(distdir)-derived-$$$$ \
	  && $(MAKE) $(AM_MAKEFLAGS) maintainer-clean \
	  && cd "$$am__cwd" \
	  && echo "  $@	diff $(distdir)-orig-$$$$ $(distdir)-derived-$$$$" \
	  && diff $(distdir)-orig-$$$$ $(distdir)-derived-$$$$ ; \
	  } || RES=$$? ; \
	  rm -rf $(distdir)-orig-$$$$ $(distdir)-derived-$$$$ $(distdir) ; \
	  exit $$RES
	@echo "SUCCESS: $(distdir) archives are self-reproducing"

MAINTAINERCLEANFILES = wmnut-*.tar*
