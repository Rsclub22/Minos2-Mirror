#!/usr/bin/env bash
# Turns $STAGE into an AppImage in $OUTDIR.
set -euo pipefail
HERE="$(dirname "$(readlink -f "$0")")"
. "$HERE/target.sh"

STAGE="${STAGE:?}"; OUTDIR="${OUTDIR:?}"; VERSION="${MINOS_VERSION:?}"
mkdir -p "$OUTDIR"

case "$(uname -m)" in
    x86_64)  LD_ARCH="x86_64" ;;
    aarch64) LD_ARCH="aarch64" ;;
    *) echo "no linuxdeploy build for $(uname -m)" >&2; exit 1 ;;
esac

TOOLS="${APPIMAGE_TOOLS:-$PWD/appimage-tools}"
mkdir -p "$TOOLS"
for tool in linuxdeploy linuxdeploy-plugin-qt linuxdeploy-plugin-appimage; do
    if [ ! -x "$TOOLS/$tool" ]; then
        curl -sSfL -o "$TOOLS/$tool" \
            "https://github.com/linuxdeploy/${tool%%-plugin-*}/releases/download/continuous/${tool}-${LD_ARCH}.AppImage" ||
        curl -sSfL -o "$TOOLS/$tool" \
            "https://github.com/linuxdeploy/${tool}/releases/download/continuous/${tool}-${LD_ARCH}.AppImage"
        chmod +x "$TOOLS/$tool"
    fi
done
export PATH="$TOOLS:$PATH"
# No FUSE inside containers or on GitHub runners.
export APPIMAGE_EXTRACT_AND_RUN=1

APPDIR="$PWD/AppDir"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/lib" "$APPDIR/usr/share/minos2" \
         "$APPDIR/usr/share/metainfo"
cp "$HERE/files/minos2.metainfo.xml" "$APPDIR/usr/share/metainfo/"

# Payload that is not an executable: catalogues, configuration templates, docs.
cp -a "$STAGE$MINOS_PREFIX/Bin/translations" "$APPDIR/usr/bin/"
cp -a "$STAGE$MINOS_PREFIX/Configuration"    "$APPDIR/usr/share/minos2/"
cp -a "$STAGE$MINOS_PREFIX/Docs"             "$APPDIR/usr/share/minos2/"
cp -a "$STAGE$MINOS_PREFIX/BUILDINFO"        "$APPDIR/usr/share/minos2/"
if [ -d "$STAGE$MINOS_PREFIX/lib" ]; then
    cp -a "$STAGE$MINOS_PREFIX/lib"/libhamlib.so* "$APPDIR/usr/lib/"
fi

# linuxdeploy wants to copy the executables in itself, so hand it a copy that
# is not already inside the AppDir.
BINSRC="$(mktemp -d)"
cp "$STAGE$MINOS_PREFIX/Bin"/Mqt* "$BINSRC/"
EXEC_ARGS=()
for b in "$BINSRC"/Mqt*; do
    # The bundled Hamlib now lives in usr/lib, not ../lib.
    patchelf --set-rpath '$ORIGIN/../lib' "$b" 2>/dev/null || true
    EXEC_ARGS+=(--executable "$b")
done

export QMAKE
if [ "$MINOS_QT_MAJOR" = "6" ]; then
    QMAKE="$(command -v qmake6)"
else
    QMAKE="$(command -v qmake-qt5 || command -v qmake)"
fi
# Let the Qt plugin resolve our bundled Hamlib while it walks dependencies.
export LD_LIBRARY_PATH="$APPDIR/usr/lib:${LD_LIBRARY_PATH:-}"
export OUTPUT="$OUTDIR/${MINOS_PKGNAME}-${VERSION}-${LD_ARCH}.AppImage"
export VERSION

linuxdeploy \
    --appdir "$APPDIR" \
    "${EXEC_ARGS[@]}" \
    --desktop-file "$HERE/files/minos2.desktop" \
    --icon-file "$STAGE/usr/share/icons/hicolor/32x32/apps/minos2.png" \
    --custom-apprun "$HERE/files/AppRun" \
    --plugin qt \
    --output appimage

rm -rf "$BINSRC"
echo "==> $OUTPUT"
ls -la "$OUTPUT"
