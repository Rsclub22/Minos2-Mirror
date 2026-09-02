# Minos2 — GitHub mirror

A read-only mirror of the [Minos](https://sourceforge.net/projects/minos/)
SourceForge repository, plus a pipeline that builds Linux packages for every
release tag.

**Upstream:** `https://git.code.sf.net/p/minos/minos` — that is where
development happens and where pull requests and issues belong. Nothing is ever
pushed from here back to SourceForge.

## What lives on which branch

| Branch | Contents |
|---|---|
| `ci` (default) | This pipeline. Not part of Minos, never mirrored upstream. |
| `master`, `GJV_*`, `Mqt_Release_*`, … | Verbatim copies of the upstream branches. |
| tags | Verbatim copies of the upstream tags. |

The `ci` branch is the repository default only because GitHub runs scheduled
workflows from the default branch. The source code you want is on `master` or
on one of the release branches, exactly as upstream has it.

## Downloads

Every upstream tag matching `Mqt_Release_*` gets a
[release](../../releases) with:

| File | For |
|---|---|
| `minos2_<version>_amd64.deb` | Ubuntu 24.04 and newer, x86-64 |
| `minos2_<version>_arm64.deb` | Debian 12 / Raspberry Pi OS, 64-bit |
| `minos2-<version>-*.rpm` | Fedora |
| `minos2-<version>-*.pkg.tar.zst` | Arch Linux |
| `minos2-<version>-x86_64.AppImage` | Anything else, x86-64 |
| `SHA256SUMS` | Checksums for the above |

Tags whose name contains `Beta`, `RC` or `Pre` are published as pre-releases.

```sh
sudo apt install ./minos2_*.deb          # Debian, Ubuntu, Raspberry Pi OS
sudo dnf install ./minos2-*.rpm          # Fedora
sudo pacman -U ./minos2-*.pkg.tar.zst    # Arch
chmod +x minos2-*.AppImage               # everything else
```

Start the logger with `minos2`. The first start creates `~/minos2-runtime`
holding your configuration, logs and lists. Upgrades add new configuration
files but never overwrite one you have edited. Point `MINOS2_RUNTIME` somewhere
else to keep several setups apart, and set `MINOS2_APP` to start a different
Minos application than `MqtLogger`.

## How the mirror stays in sync

`.github/workflows/mirror-sync.yml` runs every three hours and on demand:

1. Clones upstream and force-pushes every branch and tag into this repository.
   Branches deleted upstream are deleted here; the `ci` branch is left alone
   and tags are never deleted, so published releases keep resolving.
2. Works out which release tags are new since the previous sync, skipping any
   that already have a release — so a re-run repairs a failed build instead of
   duplicating it.
3. Calls the build workflow for each of them.

If more than `MAX_NEW_TAG_BUILDS` (3) release tags appear at once — a
re-seeded mirror, say — the sync refuses to build any of them and asks you to
start the ones you want by hand. That guard is what keeps an accident from
spending a month of Actions minutes in one run.

## How the packages are built

`.github/workflows/build-linux.yml` builds one tag across five targets, each in
a container, all driven by `packaging/run.sh`:

| Target | Base | Qt |
|---|---|---|
| `deb` amd64 | `ubuntu:24.04` | Qt 5.15 |
| `deb` arm64 | `debian:bookworm` | Qt 5.15 |
| `rpm` amd64 | `fedora:41` | Qt 6.8 |
| `arch` amd64 | `archlinux:base` | Qt 6.11 |
| `appimage` amd64 | `ubuntu:22.04` | Qt 5.15 |

Two things about that table are deliberate, and both come from what actually
compiles:

**Qt5 on Debian and Ubuntu.** `mqt/maia/maiaXmlRpcServerConnection.cpp` guards
its `QVariant` construction on Qt 6.5, so Qt 6.0–6.4 fails to compile. Ubuntu
24.04 and Debian 12 both ship Qt 6.4, so those targets use their Qt 5.15
instead. Fedora (6.8) and Arch (6.11) are past the guard and use Qt6.

**Hamlib is built from source, not taken from the distribution.**
`mqt/mqthamlib.pri` pins the version, and Minos uses API that only arrived in
4.6/4.7 (`rig_stop_voice_mem`, `RIG_MTYPE_VOICE`, `RIG_CAPS_HAS_SET_LEVEL`, …).
Ubuntu 24.04 and Fedora 41 both ship 4.5.5, which does not compile. So the
pinned release is built, installed into `/opt/minos2/lib`, and reached through
an `$ORIGIN/../lib` run path stamped in with `patchelf` — a literal `$ORIGIN`
cannot survive the qmake → make → shell quoting chain. The build is cached per
target and keyed on `mqthamlib.pri`, so bumping the pin upstream rebuilds it by
itself. If a distribution ever ships a Hamlib at or past the pinned
major.minor, `build.sh` links against that one instead and drops the bundle.

**The Arch package pins a Qt floor.** Qt moves symbols between ELF version
nodes even in patch releases — 6.11.2 promoted
`QUntypedPropertyBinding(QPropertyBindingPrivate*)` from `Qt_6_PRIVATE_API` to
`Qt_6`. A `qt6-serialport` from one patch release loaded against a `qt6-base`
from another then dies at startup with

```
symbol lookup error: /usr/lib/libQt6SerialPort.so.6: undefined symbol:
_ZN23QUntypedPropertyBindingC1EP23QPropertyBindingPrivate, version Qt_6
```

That is a partial upgrade on the user's machine rather than a Minos fault, and
a full `pacman -Syu` fixes it — but the package used to invite it by depending
on `qt6-base` with no bound at all. `pkg-arch.sh` now emits
`qt6-base>=<the Qt it was built against>` and the same for the other Qt
modules, so pacman reports a dependency conflict instead of letting the crash
happen at startup. The deb needs no equivalent: Qt5 is long stable and
`dpkg-shlibdeps` already emits versioned dependencies. Fedora ships its Qt
stack as one consistent set per release, so the rpm does not need it either.

**The AppImage bundles three libraries linuxdeploy refuses to.** Its
excludelist is mostly right — glibc, the GL and X stack, ALSA and the font
libraries have to come from the host, and linuxdeploy will not deploy them even
when named with `--library`. But the list also drops `libusb`, which the
bundled Hamlib needs, along with `libgpg-error` and `libcom_err`, which the
bundled Kerberos and gcrypt need. `pkg-appimage.sh` therefore works out what is
still unresolved, copies those in by hand, and then builds the image through
`linuxdeploy-plugin-appimage` directly — going through `linuxdeploy --output
appimage` would run another deployment pass and strip them out again. What is
left for the host is the ordinary desktop baseline; the AppImage will not start
on a bare container without a GUI stack.

arm64 is emulated through QEMU because hosted arm64 runners are not available
for private repositories on this plan. It is by far the most expensive job in
the matrix; if the minutes get tight, dropping that one matrix entry is the
first thing to do.

### Building a tag by hand

Actions → **Build Linux packages** → *Run workflow*, and give it a tag. Turn
*publish* off to get artifacts without touching the releases page. Re-running
for a tag that already has a release tops that release up rather than replacing
it.

### Building locally

The packaging scripts have no dependency on Actions:

```sh
git clone https://github.com/Rsclub22/Minos2-Mirror.git -b ci ci
git clone https://github.com/Rsclub22/Minos2-Mirror.git -b master src
mkdir -p wd out && cd wd
docker run --rm -v "$PWD/../ci:/ci:ro" -v "$PWD/../src:/src" \
    -v "$PWD:/wd" -v "$PWD/../out:/out" \
    -e SRC=/src -e OUTDIR=/out -w /wd ubuntu:24.04 \
    bash /ci/packaging/run.sh deb
```

## Layout

```
packaging/
  run.sh            deps -> build -> package, one entry point
  target.sh         per-distribution settings (Qt major, package family, arch)
  deps.sh           build dependencies
  build.sh          Hamlib, qmake, make, translations, staging tree
  fetch-data.sh     cty.dat and the RSGB contest files
  pkg-deb.sh  pkg-rpm.sh  pkg-arch.sh  pkg-appimage.sh
  files/            launcher, .desktop, AppStream metadata, AppImage AppRun
scripts/
  sync-mirror.sh    the mirroring itself
  publish-release.sh
```

## Configuration

Repository variables (Settings → Secrets and variables → Actions → Variables):

| Variable | Default | Meaning |
|---|---|---|
| `TAG_REGEX` | `^(tags/)?[Mm]qt_Rel` | Which tags count as releases |
| `MAX_NEW_TAG_BUILDS` | `3` | Refuse to auto-build more than this at once |

No secrets are needed. The sync pushes with the built-in `GITHUB_TOKEN`.

## Licence

Minos is GPL-3.0-or-later; see `mqt/License.txt` on the mirrored branches. The
pipeline in this branch is offered under the same terms.
