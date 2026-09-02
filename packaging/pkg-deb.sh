#!/usr/bin/env bash
# Turns $STAGE into a .deb in $OUTDIR.
set -euo pipefail
HERE="$(dirname "$(readlink -f "$0")")"
. "$HERE/target.sh"

STAGE="${STAGE:?}"; OUTDIR="${OUTDIR:?}"; VERSION="${MINOS_VERSION:?}"
REL="${PKG_RELEASE:-1}"
mkdir -p "$OUTDIR"

# dpkg-shlibdeps insists on being run from a source tree, so give it a
# throw-away one. -l points it at our bundled Hamlib, which has no shlibs
# file of its own; --ignore-missing-info keeps that from being fatal.
WORK="$(mktemp -d)"
mkdir -p "$WORK/debian"
printf 'Source: %s\n\nPackage: %s\nArchitecture: any\n' "$MINOS_PKGNAME" "$MINOS_PKGNAME" \
    > "$WORK/debian/control"

# The bundled Hamlib has to be scanned as well, not just linked against:
# its own dependencies (libusb, ...) are what the installed system needs.
SCAN=("$STAGE$MINOS_PREFIX/Bin"/Mqt*)
SHLIB_ARGS=()
if [ "${MINOS_HAMLIB_BUNDLED:-0}" = "1" ]; then
    SHLIB_ARGS+=(-l"$STAGE$MINOS_PREFIX/lib")
    for so in "$STAGE$MINOS_PREFIX/lib"/*.so.*.*; do
        [ -f "$so" ] && [ ! -L "$so" ] && SCAN+=("$so")
    done
fi

DEPS="$( cd "$WORK" && \
    dpkg-shlibdeps -O --ignore-missing-info "${SHLIB_ARGS[@]}" "${SCAN[@]}" \
        2> "$WORK/shlibdeps.log" \
    | sed -n 's/^shlibs:Depends=//p' )"
[ -z "$DEPS" ] && sed 's/^/   shlibdeps: /' "$WORK/shlibdeps.log" >&2
rm -rf "$WORK"

if [ -z "$DEPS" ]; then
    echo "!! dpkg-shlibdeps produced nothing, falling back to a static list" >&2
    DEPS="libc6, libqt5core5a | libqt5core5t64, libqt5gui5 | libqt5gui5t64, libqt5widgets5 | libqt5widgets5t64, libqt5network5 | libqt5network5t64, libqt5sql5 | libqt5sql5t64, libqt5serialport5, libqt5charts5, libqt5multimedia5"
fi
# Qt needs an SQLite driver and its own catalogues at runtime; neither is a
# linker dependency, so shlibdeps cannot see them.
DEPS="$DEPS, libqt5sql5-sqlite | libqt6sql6-sqlite, qttranslations5-l10n | qt6-translations-l10n"

INSTALLED_KB="$(du -sk "$STAGE" | cut -f1)"
mkdir -p "$STAGE/DEBIAN"
cat > "$STAGE/DEBIAN/control" <<EOF
Package: $MINOS_PKGNAME
Version: $VERSION-$REL
Section: hamradio
Priority: optional
Architecture: $MINOS_ARCH
Maintainer: Minos2-Mirror CI <noreply@github.com>
Installed-Size: $INSTALLED_KB
Depends: $DEPS
Homepage: https://sourceforge.net/projects/minos/
Description: Minos amateur radio contest logger
 Minos is a contest logging suite for amateur radio, covering VHF, UHF and
 microwave contesting, with rig and rotator control, DX cluster and KST chat
 integration, CW and voice keying, and multi-operator networking.
 .
 Run "minos2" to start the logger. On first start a runtime tree is created
 in ~/minos2-runtime; your configuration lives there and is never overwritten
 by later upgrades.
 .
 Built from $([ -n "${MINOS_REF:-}" ] && echo "$MINOS_REF" || echo "the Minos2 sources").
EOF

cat > "$STAGE/DEBIAN/postinst" <<'EOF'
#!/bin/sh
set -e
if [ "$1" = "configure" ]; then
    [ -x /usr/bin/update-desktop-database ] && update-desktop-database -q /usr/share/applications || true
    [ -x /usr/bin/gtk-update-icon-cache ] && gtk-update-icon-cache -qtf /usr/share/icons/hicolor || true
fi
exit 0
EOF
chmod 0755 "$STAGE/DEBIAN/postinst"

# Everything under /opt is configuration-free; the per-user copy in
# ~/minos2-runtime is what the operator edits, so no conffiles are declared.
OUT="$OUTDIR/${MINOS_PKGNAME}_${VERSION}-${REL}_${MINOS_ARCH}.deb"
dpkg-deb --build --root-owner-group -Zxz "$STAGE" "$OUT"
rm -rf "$STAGE/DEBIAN"

echo "==> $OUT"
dpkg-deb -I "$OUT"
