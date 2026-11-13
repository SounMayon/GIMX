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

# Apply the branch-maintained fixes to the pinned shared submodules.
git -C shared/gimxhid apply ../../patches/gimxhid-pollfds-use-after-free.patch
git -C shared/gimxusb apply ../../patches/gimxusb-pollfds-use-after-free.patch

make -j2
```

After changing headers, run `make clean` before rebuilding; the legacy Makefiles do not reliably track header dependencies.


## Windows build with MSYS2 UCRT64

Use the current UCRT64 environment. Install the build tools and libraries, then clone the branch with its recursive submodules:

    pacman -Syu
    pacman -S --needed base-devel git mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-wxwidgets3.2-msw mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-libxml2 mingw-w64-ucrt-x86_64-libusb mingw-w64-ucrt-x86_64-curl mingw-w64-ucrt-x86_64-ncurses mingw-w64-ucrt-x86_64-gettext mingw-w64-ucrt-x86_64-pkgconf
    git submodule update --init --recursive

Apply the maintained shared-submodule fixes, then compile:

    git -C shared/gimxhid apply ../../patches/gimxhid-pollfds-use-after-free.patch
    git -C shared/gimxusb apply ../../patches/gimxusb-pollfds-use-after-free.patch
    git -C shared/gimxprio apply ../../patches/gimxprio-pointer-format.patch
    git -C shared/gimxtime apply ../../patches/gimxtime-standard-printf.patch
    git -C shared/gimxinput apply ../../patches/gimxinput-sdl2-steering-axis.patch
    make clean
    make -j2

The GitHub Actions workflow uses the same UCRT64 packages and builds the complete application. It does not package an installer.
