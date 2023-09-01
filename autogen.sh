#!/bin/sh

# Pre-configure step for WMNut

if (command -v pkg-config) >/dev/null 2>/dev/null ; then
    NUTVER="`pkg-config --modversion libupsclient`"
    case "$NUTVER" in
        [01].*|2.[01234567].*)
            echo "WARNING: pkg-config finds older NUT ($NUTVER) libraries than required by this WMNut release. You may get build warnings, or have to pass custom ones with configure options!" >&2
            echo "===========" >&2
            ;;
        "") echo "WARNING: pkg-config did not find NUT libraries. You may have to pass custom ones with configure options!" >&2
            echo "===========" >&2
            ;;
    esac
fi

autoreconf -i
