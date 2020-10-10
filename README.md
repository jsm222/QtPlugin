# Panda Qt5 Plugins [![Build Status](https://api.cirrus-ci.com/github/probonopd/panda-qt5-plugin.svg)](https://cirrus-ci.com/github/probonopd/panda-qt5-plugin)

Qt platform plugin and style. Exports windows to KDE-style global menus via D-Bus.

## Dependencies

On Arch Linux:

```
sudo pacman -S gcc extra-cmake-modules qt5-base qt5-tools qt5-x11extras libqtxdg libdbusmenu-qt5 libxcb
```

On FreeBSD:

```
sudo pkg install -y curl zip cmake pkgconf libfm qt5-core qt5-x11extras qt5-widgets qt5-qmake qt5-buildtools qt5-linguisttools kf5-extra-cmake-modules libqtxdg libxcb libdbusmenu-qt5 kf5-kwindowsystem
```

## Build

```shell
mkdir build
cd build
cmake ..
make
sudo make install
```

## License

panda-qt5-plugins is licensed under GPLv3.
