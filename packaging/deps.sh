#!/usr/bin/env bash
# Installs the build dependencies for the distribution we are running in.
set -euo pipefail
. "$(dirname "$(readlink -f "$0")")/target.sh"

# Distribution mirrors time out often enough that a single failure must not
# sink a release build.
retry() {
    local n=1 max=4
    until "$@"; do
        if [ "$n" -ge "$max" ]; then
            echo "!! '$1 ...' failed $max times, giving up" >&2
            return 1
        fi
        echo "-- attempt $n failed, retrying in $((n * 15))s" >&2
        sleep $((n * 15))
        n=$((n + 1))
    done
}

echo "==> installing build dependencies for $ID $VERSION_ID (Qt$MINOS_QT_MAJOR)"

case "$MINOS_FAMILY" in
debian)
    export DEBIAN_FRONTEND=noninteractive
    retry apt-get update -qq
    retry apt-get install -y -qq --no-install-recommends \
        build-essential pkg-config ca-certificates curl file xz-utils \
        dpkg-dev fakeroot patchelf \
        libgl1-mesa-dev libasound2-dev libusb-1.0-0-dev \
        libqt5serialport5-dev libqt5charts5-dev \
        qtbase5-dev qtbase5-dev-tools qtchooser qt5-qmake \
        qtmultimedia5-dev qttools5-dev qttools5-dev-tools \
        qtdeclarative5-dev libqt5sql5-sqlite qttranslations5-l10n
    ;;
rpm)
    retry dnf -y --setopt=install_weak_deps=False install \
        gcc-c++ make pkgconf which file curl tar xz rpm-build patchelf \
        mesa-libGL-devel alsa-lib-devel libusbx-devel \
        qt6-qtbase-devel qt6-qtserialport-devel qt6-qtcharts-devel \
        qt6-qtmultimedia-devel qt6-qtdeclarative-devel \
        qt6-qttools-devel qt6-qttranslations
    ;;
arch)
    retry pacman -Syu --noconfirm --needed \
        base-devel curl file patchelf sudo \
        alsa-lib libusb \
        qt6-base qt6-serialport qt6-charts qt6-multimedia \
        qt6-declarative qt6-tools qt6-translations
    ;;
esac

echo "==> build dependencies installed"
