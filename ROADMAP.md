# Roadmap: GIMX Reborn and G29 PS4 emulation

Last updated: 2026-11-13

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
- Application build also passes with compiler/library version reporting enabled in [run 43324543826](https://github.com/SounMayon/GIMX/actions/runs/43324543826).
- Baseline `EMUG29PS4` firmware build passed on GitHub Actions in [run 43320703237](https://github.com/SounMayon/GIMX-firmwares/actions/runs/43320703237) using the existing source and vendored LUFA.
- After that baseline, `EMUG29PS4` was moved to upstream LUFA `LUFA-170418`; the updated firmware and AVR toolchain version report pass in [run 43324588408](https://github.com/SounMayon/GIMX-firmwares/actions/runs/43324588408).
- Added Linux build instructions that include the maintained patches for the pinned HID and USB submodules.

### In progress

- Finish inventorying other bundled/shared dependencies and decide whether they need independent upgrades; the current G29-only LUFA update does not change other firmware targets.
- Validate G29 behavior on hardware after the compiled LUFA update; the GitHub build cannot validate enumeration, authentication or force feedback.
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

The Ubuntu 26.04 Linux application build is green. The version-reporting step recorded GCC/G++ 15.2.0, wxWidgets 3.2.9, SDL2 2.32.10, libxml2 2.15.2, libusb 1.0.29, ncursesw 6.6, libcurl 8.18.0 and BlueZ 5.85. The Windows/MSYS2 build has not yet been run.

The AVR workflow recorded avr-gcc 14.3.0, avr-libc 2.2.1, binutils-avr 2.43.50 and make 4.4.1.

### G29 PS4 emulation

- The firmware descriptor advertises Logitech `046d:C260`, HID input and output reports, and relays control/feature requests through the adapter serial protocol.
- The application already registers a G29 PS4 controller, builds the wheel/pedal/button report, requires authentication and sends reports at a 5 ms default interval.
- The GIMX core handles output reports and has a haptics path; the wiki documents G29 PS4 emulation and force feedback. This is an existing emulation chain to validate and improve, not an empty firmware project.
- The firmware's `F1/F2` feature-request handling waits in a loop for a serial reply without a timeout. Treat this as a robustness risk; protocol changes require real-device captures and PS4 validation.
- Candidate work after build baselines: report-size/value checks, bounded UART/control lengths, timeout/recovery design, report unit tests, USB capture comparisons, reconnect behavior and real G29/adapter/console force-feedback checks.
- Firmware baseline used bundled LUFA `130901`. After both baseline builds passed, only the G29 target was switched to the pinned upstream `LUFA-170418` submodule. Its build passes with the descriptor callback updated to LUFA's `uint16_t wIndex` signature; other firmware targets still use the existing shared LUFA tree.

## Next steps

1. Review remaining bundled/shared dependencies and keep the current Linux/AVR version reports as the baseline.
2. Check a Windows/MSYS2 build against a current supported toolchain.
3. Add report-level tests and compare USB descriptors/reports with captures from a genuine G29.
4. Validate authentication, reconnects and force feedback with a real G29, GIMX adapter and PS4 before changing emulation behavior.
5. Expand the firmware build matrix before changing the shared LUFA tree used by other targets.
6. Update this roadmap with each build result and code change.

## References

- [GIMX Development on Linux](https://gimx.fr/wiki/index.php?title=Development_on_Linux)
- [GIMX Development on Windows](https://gimx.fr/wiki/index.php?title=Development_on_Windows)
- [GIMX guide for steering wheel to PS4](https://gimx.fr/wiki/index.php?connectiontype=USB&device=SteeringWheel&ostype=windows&platform=ps4&title=Guide)
- [GIMX force feedback documentation](https://gimx.fr/wiki/index.php?title=Force_Feedback%2Fen)
- [LUFA releases](https://github.com/abcminiuser/lufa/releases)
