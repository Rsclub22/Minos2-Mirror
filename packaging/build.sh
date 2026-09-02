#!/usr/bin/env bash
# Builds Minos2 and lays out an installable tree under $STAGE.
#
#   SRC    checkout of the Minos2 sources (contains mqt/)   [required]
#   STAGE  staging root, created if missing                 [default ./stage]
#   JOBS   parallel make jobs                               [default nproc]
#
# The result is $STAGE/opt/minos2 plus the /usr entries every package needs.
set -euo pipefail
HERE="$(dirname "$(readlink -f "$0")")"
. "$HERE/target.sh"

SRC="${SRC:?SRC must point at the Minos2 checkout}"
STAGE="${STAGE:-$PWD/stage}"
BUILD="${BUILD:-$PWD/build}"
JOBS="${JOBS:-$(nproc)}"

# Libraries and applications, in the order buildInstall.sh uses them. The
# "libs" have translations that get merged into every application catalogue.
#
# Filtered against the checkout, because older tags predate some of these
# subprojects and we build whatever tag upstream publishes.
present() {
    local found=""
    for p in $1; do
        [ -f "$SRC/mqt/$p/$p.pro" ] && found="$found $p"
    done
    echo "${found# }"
}
LIBS="$(present "MqtUtils TinyXML XMPPLib MqtBase KeyerBase")"
APPS="$(present "MqtAppStarter MqtChat MqtCluster MqtControl MqtDataModes
                 MqtKeyer MqtKeyerProxy MqtKSTClient MqtLogger MqtMonitor
                 MqtPcCwKeyer MqtQrzServer MqtRigControl MqtRigSync
                 MqtRigRecorder MqtRotator MqtServer MqtWinkeyer")"
: "${APPS:?no Minos applications found under $SRC/mqt}"

# ---------------------------------------------------------------- versions --
MINOS_VERSION="$(sed -n 's/^VERSION=\([0-9][0-9.]*\).*/\1/p' "$SRC/mqt/mqt.pri" | head -1)"
HAMLIB_VERSION="$(sed -n 's/^HAMLIBVERSION=\([0-9][0-9.]*\).*/\1/p' "$SRC/mqt/mqthamlib.pri" | head -1)"
: "${MINOS_VERSION:?could not read VERSION from mqt/mqt.pri}"
: "${HAMLIB_VERSION:?could not read HAMLIBVERSION from mqt/mqthamlib.pri}"
echo "==> Minos $MINOS_VERSION, Hamlib $HAMLIB_VERSION, Qt$MINOS_QT_MAJOR, $MINOS_ARCH"

# ------------------------------------------------------------------ hamlib --
# Minos needs the API level of the Hamlib release it pins (rig_stop_voice_mem,
# RIG_MTYPE_VOICE, ... appeared in 4.6/4.7). Use the distribution's Hamlib when
# it is new enough, otherwise build the pinned release and ship it in /opt.
HAMLIB_REQ_MM="${HAMLIB_VERSION%.*}"                     # e.g. 4.7.2 -> 4.7
HAMLIB_SYS="$(pkg-config --modversion hamlib 2>/dev/null || true)"
HAMLIB_BUNDLED=0

if [ -n "$HAMLIB_SYS" ] && \
   [ "$(printf '%s\n%s\n' "$HAMLIB_REQ_MM" "$HAMLIB_SYS" | sort -V | head -1)" = "$HAMLIB_REQ_MM" ]; then
    echo "==> using system Hamlib $HAMLIB_SYS (>= $HAMLIB_REQ_MM)"
    HAMLIB_CFLAGS="$(pkg-config --cflags hamlib)"
    HAMLIB_LIBS="$(pkg-config --libs hamlib)"
else
    echo "==> system Hamlib is '${HAMLIB_SYS:-absent}', building $HAMLIB_VERSION from source"
    HAMLIB_BUNDLED=1
    HAMLIB_ROOT="${HAMLIB_ROOT:-$PWD/hamlib-root}"
    if [ ! -f "$HAMLIB_ROOT/lib/libhamlib.so" ]; then
        rm -rf "$PWD/hamlib-src" && mkdir -p "$PWD/hamlib-src"
        curl -sSfL "https://github.com/Hamlib/Hamlib/releases/download/$HAMLIB_VERSION/hamlib-$HAMLIB_VERSION.tar.gz" \
            | tar xz -C "$PWD/hamlib-src" --strip-components=1
        ( cd "$PWD/hamlib-src"
          ./configure --prefix="$MINOS_PREFIX" --libdir="$MINOS_PREFIX/lib" \
                      --disable-static --without-cxx-binding --disable-winradio
          make -j"$JOBS"
          make install DESTDIR="$HAMLIB_ROOT.destdir" )
        rm -rf "$HAMLIB_ROOT"
        mv "$HAMLIB_ROOT.destdir$MINOS_PREFIX" "$HAMLIB_ROOT"
    else
        echo "    (reusing cached Hamlib build in $HAMLIB_ROOT)"
    fi
    HAMLIB_CFLAGS="-I$HAMLIB_ROOT/include"
    HAMLIB_LIBS="-L$HAMLIB_ROOT/lib -lhamlib"
fi

# --------------------------------------------------------------------- qmake --
if [ "$MINOS_QT_MAJOR" = "6" ]; then
    QMAKE="$(command -v qmake6 || command -v qmake-qt6)"
    LRELEASE="$(command -v lrelease6 || command -v lrelease-qt6 || command -v lrelease)"
    LCONVERT="$(command -v lconvert6 || command -v lconvert-qt6 || command -v lconvert)"
else
    QMAKE="$(command -v qmake-qt5 || echo "qmake -qt=5")"
    LRELEASE="$(command -v lrelease-qt5 || echo "lrelease -qt=5")"
    LCONVERT="$(command -v lconvert-qt5 || echo "lconvert -qt=5")"
fi
echo "==> qmake: $($QMAKE -query QT_VERSION) ($QMAKE)"

# ------------------------------------------------------------------- compile --
rm -rf "$BUILD" && mkdir -p "$BUILD"
cd "$BUILD"
# shellcheck disable=SC2086
$QMAKE "$SRC/mqt/mqt.pro" \
    QMAKE_CXXFLAGS+="$HAMLIB_CFLAGS" \
    LIBS+="$HAMLIB_LIBS"
make -j"$JOBS"

# -------------------------------------------------------------- translations --
# Mirrors mqt/Installer/buildInstall.sh: compile each subproject's catalogue,
# then merge the library catalogues into every application catalogue.
for j in $LIBS $APPS; do
    $LRELEASE "$SRC/mqt/$j/$j.pro" >/dev/null
    mkdir -p "$BUILD/$j/translations"
    mv "$SRC/mqt/$j/translations"/*.qm "$BUILD/$j/translations/"
done

mkdir -p "$BUILD/translations"
for loc in en_GB fr_FR; do
    for j in $APPS; do
        libqm=""
        for l in $LIBS; do libqm="$libqm $BUILD/$l/translations/minos_$loc.qm"; done
        # shellcheck disable=SC2086
        $LCONVERT -o "$BUILD/translations/${j}_${loc}.qm" \
            $libqm "$BUILD/$j/translations/minos_$loc.qm"
    done
done

# ------------------------------------------------------------------- staging --
rm -rf "$STAGE"
P="$STAGE$MINOS_PREFIX"
mkdir -p "$P/Bin/translations" "$P/Configuration" "$P/Docs" "$P/lib" \
         "$STAGE/usr/bin" "$STAGE/usr/share/applications" \
         "$STAGE/usr/share/icons/hicolor/32x32/apps" \
         "$STAGE/usr/share/metainfo"

for j in $APPS; do install -m 0755 "$BUILD/$j/$j" "$P/Bin/$j"; done

# A literal $ORIGIN cannot survive the qmake -> make -> shell quoting chain,
# so the run path is stamped in afterwards. Only needed when we ship our own
# Hamlib next to the binaries.
if [ "$HAMLIB_BUNDLED" = "1" ]; then
    for j in $APPS; do patchelf --set-rpath '$ORIGIN/../lib' "$P/Bin/$j"; done
fi
install -m 0644 "$BUILD/translations"/*.qm "$P/Bin/translations/"

# Qt's own catalogues, so its dialogs are translated too. Which locales a
# distribution ships varies, so an unmatched pattern here is not an error.
QT_TR="$($QMAKE -query QT_INSTALL_TRANSLATIONS)"
for f in "$QT_TR"/qt_??.qm "$QT_TR"/qt_??_??.qm "$QT_TR"/qtbase_??.qm "$QT_TR"/qtbase_??_??.qm; do
    [ -f "$f" ] || continue
    install -m 0644 "$f" "$P/Bin/translations/"
done

if [ "$HAMLIB_BUNDLED" = "1" ]; then
    cp -P "$HAMLIB_ROOT/lib"/libhamlib.so* "$P/lib/"
    chmod 0755 "$P"/lib/libhamlib.so.*.*.* 2>/dev/null || true
else
    rmdir "$P/lib"
fi

cp -r "$SRC/mqt/ControlFiles/Configuration/." "$P/Configuration/"
cp -r "$SRC/mqt/ControlFiles/Configuration/LinuxFiles/." "$P/Configuration/"
rm -rf "$P/Configuration/WindowsFiles" "$P/Configuration/LinuxFiles" \
       "$P/Configuration/OSXFiles" "$P/Configuration/MacFiles"
cp "$SRC/mqt/Docs"/*.pdf "$P/Docs/" 2>/dev/null || true
cp "$SRC/mqt"/*.ico "$P/" 2>/dev/null || true
install -m 0644 "$SRC/mqt/minos.png" "$STAGE/usr/share/icons/hicolor/32x32/apps/minos2.png"

install -m 0755 "$HERE/files/minos2" "$STAGE/usr/bin/minos2"
install -m 0644 "$HERE/files/minos2.desktop" "$STAGE/usr/share/applications/minos2.desktop"
install -m 0644 "$HERE/files/minos2.metainfo.xml" "$STAGE/usr/share/metainfo/minos2.metainfo.xml"

# Contest and callsign data that buildInstall.sh downloads at install time.
# Never fatal: the application refreshes these itself and the upstream sites
# are outside our control.
"$HERE/fetch-data.sh" "$P/Configuration" || echo "!! contest data download incomplete"

# Record what went into this build, for the release notes and for support.
cat > "$P/BUILDINFO" <<EOF
Minos version:  $MINOS_VERSION
Source ref:     ${MINOS_REF:-unknown}
Source commit:  $(git -C "$SRC" rev-parse HEAD 2>/dev/null || echo unknown)
Built on:       $PRETTY_NAME ($MINOS_ARCH)
Qt:             $($QMAKE -query QT_VERSION)
Hamlib:         $([ "$HAMLIB_BUNDLED" = 1 ] && echo "$HAMLIB_VERSION (bundled)" || echo "$HAMLIB_SYS (system)")
Built at:       $(date -u +%Y-%m-%dT%H:%M:%SZ)
EOF

# Hand the results to whoever called us (run.sh, and the workflow through it).
cat > "${MINOS_ENV_FILE:-$PWD/minos-build.env}" <<EOF
MINOS_VERSION=$MINOS_VERSION
MINOS_HAMLIB_VERSION=$HAMLIB_VERSION
MINOS_HAMLIB_BUNDLED=$HAMLIB_BUNDLED
MINOS_QT_VERSION=$($QMAKE -query QT_VERSION)
MINOS_DISTRO=$ID-$VERSION_ID
MINOS_ARCH=$MINOS_ARCH
EOF
echo "==> staged $(find "$STAGE" -type f | wc -l) files in $STAGE"
cat "$P/BUILDINFO"
