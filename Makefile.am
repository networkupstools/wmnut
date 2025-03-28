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

MAINTAINERCLEANFILES = wmnut-*.tar*
