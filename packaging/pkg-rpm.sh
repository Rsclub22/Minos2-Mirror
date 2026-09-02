#!/usr/bin/env bash
# Turns $STAGE into an .rpm in $OUTDIR.
set -euo pipefail
HERE="$(dirname "$(readlink -f "$0")")"
. "$HERE/target.sh"

STAGE="${STAGE:?}"; OUTDIR="${OUTDIR:?}"; VERSION="${MINOS_VERSION:?}"
REL="${PKG_RELEASE:-1}"
mkdir -p "$OUTDIR"

TOP="$(mktemp -d)"
mkdir -p "$TOP"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

cat > "$TOP/SPECS/minos2.spec" <<EOF
# The tree is already built and staged; this spec only wraps it.
%global debug_package %{nil}
%global __brp_strip %{nil}
%global __brp_check_rpaths %{nil}
%global _build_id_links none

Name:           $MINOS_PKGNAME
Version:        $VERSION
Release:        $REL
Summary:        Minos amateur radio contest logger
License:        GPL-3.0-or-later
URL:            https://sourceforge.net/projects/minos/
BuildArch:      $(uname -m)

# rpm derives the shared-library requirements from the ELF files itself, the
# bundled Hamlib included, and satisfies libhamlib.so.4 from this package's own
# auto-provides. Only what no ELF file points at is named here.
Requires:       qt6-qttranslations

%description
Minos is a contest logging suite for amateur radio, covering VHF, UHF and
microwave contesting, with rig and rotator control, DX cluster and KST chat
integration, CW and voice keying, and multi-operator networking.

Run "minos2" to start the logger. On first start a runtime tree is created in
~/minos2-runtime; your configuration lives there and is never overwritten by
later upgrades.

%prep
%build

%install
cp -a "$STAGE"/. %{buildroot}/

%files
/opt/minos2
/usr/bin/minos2
/usr/share/applications/minos2.desktop
/usr/share/icons/hicolor/32x32/apps/minos2.png
/usr/share/metainfo/minos2.metainfo.xml

%changelog
* $(LC_ALL=C date -u '+%a %b %d %Y') Minos2-Mirror CI <noreply@github.com> - $VERSION-$REL
- Automated build of ${MINOS_REF:-the Minos2 sources}
EOF

rpmbuild -bb --define "_topdir $TOP" "$TOP/SPECS/minos2.spec"

find "$TOP/RPMS" -name '*.rpm' -exec cp -v {} "$OUTDIR/" \;
rm -rf "$TOP"

for f in "$OUTDIR"/*.rpm; do
    echo "==> $f"
    rpm -qip "$f"
    echo "--- requires ---"
    rpm -qpR "$f"
done
