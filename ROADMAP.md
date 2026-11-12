# Roadmap: GIMX Reborn and G29 PS4 emulation

Last updated: 2026-11-12

## Scope and current branches

- Application: [SounMayon/GIMX, `GIMX-Reborn`](https://github.com/SounMayon/GIMX/tree/GIMX-Reborn)
- Firmware: [SounMayon/GIMX-firmwares, `G29-Finished`](https://github.com/SounMayon/GIMX-firmwares/tree/G29-Finished/EMUG29PS4)
- Documentation: [GIMX wiki](https://gimx.fr/wiki/index.php?title=Main_Page)

## Progress

### Done

- Added Ubuntu 26.04 GitHub Actions builds for the application and the `EMUG29PS4` AVR firmware.
- Added the full recursive submodule checkout to the application build workflow.
- Added current Linux build instructions in [BUILDING.md](BUILDING.md).
- Baseline application build passed on Ubuntu 26.04 in [GitHub Actions run 43323280446](https://github.com/SounMayon/GIMX/actions/runs/43323280446) at commit `d5c9407`.
- Baseline `EMUG29PS4` firmware build passed on GitHub Actions in [run 43320703237](https://github.com/SounMayon/GIMX-firmwares/actions/runs/43320703237) using the existing source and vendored LUFA.
- Added Linux build instructions that include the maintained patches for the pinned HID and USB submodules.

### In progress

- Audit the exact dependency versions resolved by Ubuntu 26.04 and identify any separately vendored libraries that can be upgraded safely.
- Check whether the Windows/MSYS2 build can use a current supported toolchain.
- Keep protocol and firmware behavior changes separate from dependency updates so each step can be compiled and reviewed.

## Audit notes

### Application and dependencies

The project is a C/C++ Makefile build with shared Git submodules. System dependencies include wxWidgets, SDL2, libxml2, libcurl, BlueZ, ncurses, mhash, libusb, X11/Xi and gettext. The Ubuntu 26.04 workflow resolves package versions from the current Ubuntu repositories.

The successful baseline exposed and fixed these issues under the current compiler and libraries:
- The pinned `gimxhid` and `gimxusb` submodules freed libusb poll descriptors before rollback accessed them. The fixes are maintained in this repository under `patches/` and applied by CI and the documented Linux build.
- The root Makefile did not make `build-loader` depend on `build-shared`, allowing parallel linking before `gimxtime` existed.
- Linux depended on the removed `ncursesw5-config` executable. The core Makefile now supports ncursesw5, ncursesw6 and pkg-config.
- Current compiler diagnostics exposed missing standard-library declarations and mismatched/no-prototype Bluetooth APIs; the affected includes, declarations and call sites are corrected.

The Ubuntu 26.04 Linux application build is green. The Windows/MSYS2 build has not yet been run.

### G29 PS4 emulation

- The firmware descriptor advertises Logitech `046d:C260`, HID input and output reports, and relays control/feature requests through the adapter serial protocol.
- The application already registers a G29 PS4 controller, builds the wheel/pedal/button report, requires authentication and sends reports at a 5 ms default interval.
- The GIMX core handles output reports and has a haptics path; the wiki documents G29 PS4 emulation and force feedback. This is an existing emulation chain to validate and improve, not an empty firmware project.
- The firmware's `F1/F2` feature-request handling waits in a loop for a serial reply without a timeout. Treat this as a robustness risk; protocol changes require real-device captures and PS4 validation.
- Candidate work after build baselines: report-size/value checks, bounded UART/control lengths, timeout/recovery design, report unit tests, USB capture comparisons, reconnect behavior and real G29/adapter/console force-feedback checks.
- Vendored LUFA is currently version `130901`. Updating it is deferred until both baseline builds are green; the candidate stable release is `LUFA-170418`.

## Next steps

1. Record exact compiler and package versions from the green Ubuntu 26.04 build.
2. Inventory bundled and system dependencies; verify upstream release versions before changing any pinned library.
3. Upgrade one dependency at a time and run the application and AVR firmware workflows after every change.
4. Check a Windows/MSYS2 build against a current supported toolchain.
5. Add report-level tests and compare USB descriptors/reports with captures from a genuine G29.
6. Validate authentication, reconnects and force feedback with a real G29, GIMX adapter and PS4 before changing emulation behavior.
7. Update this roadmap with each build result and code change.

## References

- [GIMX Development on Linux](https://gimx.fr/wiki/index.php?title=Development_on_Linux)
- [GIMX Development on Windows](https://gimx.fr/wiki/index.php?title=Development_on_Windows)
- [GIMX guide for steering wheel to PS4](https://gimx.fr/wiki/index.php?connectiontype=USB&device=SteeringWheel&ostype=windows&platform=ps4&title=Guide)
- [GIMX force feedback documentation](https://gimx.fr/wiki/index.php?title=Force_Feedback%2Fen)
- [LUFA releases](https://github.com/abcminiuser/lufa/releases)
