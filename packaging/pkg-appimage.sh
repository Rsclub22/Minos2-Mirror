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

COMMON=(
    --appdir "$APPDIR"
    "${EXEC_ARGS[@]}"
    --desktop-file "$HERE/files/minos2.desktop"
    --icon-file "$STAGE/usr/share/icons/hicolor/32x32/apps/minos2.png"
    --custom-apprun "$HERE/files/AppRun"
    --plugin qt
)

# Deploy the applications, Qt and everything linuxdeploy pulls in with them.
linuxdeploy "${COMMON[@]}"

# linuxdeploy drops everything on the AppImage excludelist, and refuses to
# deploy those libraries even when they are named with --library. Most of that
# list genuinely has to come from the host: glibc, the GL and X stack, ALSA,
# and the font libraries, which must match the host's rendering. But it also
# drops libraries no ordinary desktop is guaranteed to have - libusb, which our
# bundled Hamlib needs, among them. Those are copied in by hand below; the
# AppRun puts usr/lib first on LD_LIBRARY_PATH, so they are found.
HOST_LIBS='^(ld-linux.*|libc|libm|libdl|libpthread|librt|libresolv|libnsl|libutil|libgcc_s|libstdc\+\+|libGL|libGLX|libGLdispatch|libGLESv2|libEGL|libOpenGL|libdrm|libglapi|libX11|libX11-xcb|libxcb.*|libICE|libSM|libXext|libXrender|libXi|libXfixes|libXcursor|libXrandr|libXinerama|libXss|libXtst|libxshmfence|libwayland.*|libasound|libfontconfig|libfreetype|libharfbuzz|libz|libexpat)\.so.*$'

resolve_missing() {
    for f in "$APPDIR/usr/bin"/Mqt* "$APPDIR/usr/lib"/*.so* "$APPDIR"/usr/plugins/*/*.so; do
        [ -f "$f" ] || continue
        objdump -p "$f" 2>/dev/null | awk '/NEEDED/{print $2}'
    done | sort -u | while read -r n; do
        [ -e "$APPDIR/usr/lib/$n" ] && continue
        echo "$n" | grep -qE "$HOST_LIBS" && continue
        p="$(ldconfig -p | awk -v n="$n" '$1 == n {print $NF; exit}')"
        [ -n "$p" ] && [ -e "$p" ] && echo "$p"
    done | sort -u
}

# Copying can pull in further dependencies, so repeat until nothing is left.
for _ in 1 2 3 4 5; do
    mapfile -t EXTRA_LIBS < <(resolve_missing)
    [ "${#EXTRA_LIBS[@]}" -eq 0 ] && break
    echo "==> bundling ${#EXTRA_LIBS[@]} librar(y/ies) the excludelist dropped:"
    printf '    %s\n' "${EXTRA_LIBS[@]}"
    for l in "${EXTRA_LIBS[@]}"; do
        cp -L "$l" "$APPDIR/usr/lib/$(basename "$l")"
        chmod 0755 "$APPDIR/usr/lib/$(basename "$l")"
    done
done

still_missing="$(resolve_missing)"
if [ -n "$still_missing" ]; then
    echo "!! could not resolve: $still_missing" >&2
    exit 1
fi

# Build the image from the finished AppDir. Going through the plugin directly
# rather than `linuxdeploy --output appimage` keeps linuxdeploy from running
# another deployment pass, which would strip the libraries just added.
linuxdeploy-plugin-appimage --appdir "$APPDIR"

rm -rf "$BINSRC"
echo "==> $OUTPUT"
ls -la "$OUTPUT"
