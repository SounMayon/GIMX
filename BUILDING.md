# Building GIMX on Ubuntu 26.04

This branch uses the current package versions from the Ubuntu 26.04 repositories. The GitHub Actions workflow installs the build dependencies and compiles the complete project, including its shared Git submodules.

The same steps can be run manually:

```sh
sudo apt-get update
sudo apt-get install --yes \
  build-essential pkg-config git gettext \
  libwxgtk3.2-dev libsdl2-dev libxml2-dev libusb-1.0-0-dev \
  libbluetooth-dev libncurses-dev libmhash-dev \
  libcurl4-openssl-dev libxi-dev libx11-dev xterm xdg-utils
git submodule update --init --recursive
make -j2
```

After changing headers, run `make clean` before rebuilding; the legacy Makefiles do not reliably track header dependencies.

The Windows/MSYS2 build remains to be checked against current package versions.
