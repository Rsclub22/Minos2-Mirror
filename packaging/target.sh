#!/usr/bin/env bash
# Sourced by the other packaging scripts. Derives the build target settings
# from the distribution we are currently running in.
#
# Qt major version, per target:
#   Minos' bundled maia XML-RPC code only compiles against Qt5, or against
#   Qt >= 6.5 (mqt/maia/maiaXmlRpcServerConnection.cpp guards on 6.5.0).
#   Distributions shipping Qt 6.0 - 6.4 must therefore be built with Qt5.

# shellcheck disable=SC1091
. /etc/os-release

MINOS_PKGNAME="minos2"
MINOS_PREFIX="/opt/minos2"

case "$ID" in
    ubuntu|debian|raspbian)
        MINOS_FAMILY="debian"
        # Ubuntu 24.04 = Qt 6.4, Debian 12 = Qt 6.4 -> both too old, use Qt5.
        MINOS_QT_MAJOR="5"
        ;;
    fedora|rhel|centos|rocky|almalinux)
        MINOS_FAMILY="rpm"
        MINOS_QT_MAJOR="6"
        ;;
    arch)
        MINOS_FAMILY="arch"
        MINOS_QT_MAJOR="6"
        ;;
    *)
        echo "packaging: unsupported distribution '$ID'" >&2
        exit 1
        ;;
esac

# Debian architecture name, used for package filenames on every family.
case "$(uname -m)" in
    x86_64)  MINOS_ARCH="amd64" ;;
    aarch64) MINOS_ARCH="arm64" ;;
    armv7l)  MINOS_ARCH="armhf" ;;
    *)       MINOS_ARCH="$(uname -m)" ;;
esac

export MINOS_PKGNAME MINOS_PREFIX MINOS_FAMILY MINOS_QT_MAJOR MINOS_ARCH
