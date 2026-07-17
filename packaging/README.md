# Packaging

Whatly ships as a **snap** (the primary, published channel — see `snap/`) plus
these additional formats. All build the same CMake project; only the wrapper
differs.

| Format | Where | Build-tested here | Notes |
|--------|-------|-------------------|-------|
| Snap | `snap/snapcraft.yaml` | ✅ published (`snap install whatly`) | Primary channel |
| Debian `.deb` | `debian/` | build-deps satisfied on host | `dpkg-buildpackage -b -us -uc` |
| Fedora / COPR | `packaging/rpm/whatly.spec` | spec only | `rpmbuild` / COPR |
| Flatpak | `packaging/flatpak/net.shakaran.whatly.yml` | ❌ no `flatpak-builder` here | scaffold |
| AppImage + zsync | `packaging/appimage/build-appimage.sh` | ❌ no `linuxdeploy` here | scaffold |

## Two shared gotchas

- **Qt ≥ 6.10.** Whatly's CMake enforces it, so anything built against an older
  Qt (many stable distros, older Flatpak/KDE runtimes) will configure-fail.
- **The `libnotify-qt` submodule** is not in a plain `git archive` / GitHub
  source tarball. Build from a checkout with submodules initialised
  (`git submodule update --init`), or from a tarball that bundles them.

## Debian `.deb`

Proper `debhelper` + CMake packaging in `debian/` (root). On a host with the
build-deps (Qt 6.10 dev packages):

```bash
git submodule update --init
dpkg-buildpackage -b -us -uc      # -> ../whatly_6.0.0_amd64.deb
sudo apt install ../whatly_6.0.0_amd64.deb
```

`dpkg-checkbuilddeps` passes on the current dev host. This replaces the old
manual `debianpkg/` stub (which just dropped in a prebuilt binary).

## Fedora / COPR

```bash
rpmbuild -ba packaging/rpm/whatly.spec      # local
# or add the spec + a submodule-bundled tarball to a COPR project and let it build
```

## Flatpak (scaffold)

Manifest targets `org.kde.Platform` + `io.qt.qtwebengine.BaseApp`. Bump the
three runtime-version fields to a branch that ships Qt ≥ 6.10, then:

```bash
flatpak-builder --user --install --force-clean build-dir \
    packaging/flatpak/net.shakaran.whatly.yml
```

## AppImage (scaffold)

`build-appimage.sh` downloads `linuxdeploy` + the Qt plugin, bundles Qt WebEngine
and bakes in `.zsync` update information pointing at the GitHub releases, so
AppImageUpdate pulls only changed chunks. Run on the oldest glibc you want to
support. Upload both `Whatly-<ver>-x86_64.AppImage` and its `.zsync` to the
release.
