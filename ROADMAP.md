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
- Baseline firmware build succeeded on GitHub Actions with the existing source and vendored LUFA.

### In progress

- The first application build stopped before source compilation because the workflow omitted shared Git submodules. The workflow now fetches them recursively. A second run is underway; no application source changes have been made yet.
- Keep the firmware and application build logs as the baseline before changing dependencies or protocol code.

## Audit notes

### Application and dependencies

The project is a large C/C++ Makefile build. Shared components are Git submodules. System dependencies include wxWidgets, SDL2, libxml2, libcurl, BlueZ, ncurses, mhash, libusb, X11/Xi and gettext. The branch's Linux Makefile currently calls `ncursesw5-config`; this may be missing from current Ubuntu and will be addressed only after the fixed baseline build confirms the failure.

The upstream GIMX development instructions still list Ubuntu 20.04-era dependency packages. The new CI uses Ubuntu 26.04 repositories so dependency versions resolve to current packages on every run. Windows/MSYS2 is not yet built.

### G29 PS4 emulation

- The firmware descriptor advertises Logitech `046d:C260`, HID input and output reports, and relays control/feature requests through the adapter serial protocol.
- The application already registers a G29 PS4 controller, builds the wheel/pedal/button report, requires authentication and sends reports at a 5 ms default interval.
- The GIMX core handles output reports and has a haptics path; the wiki documents G29 PS4 emulation and force feedback. This is an existing emulation chain to validate and improve, not an empty firmware project.
- The firmware's `F1/F2` feature-request handling waits in a loop for a serial reply without a timeout. Treat this as a robustness risk; protocol changes require real-device captures and PS4 validation.
- Candidate work after build baselines: report-size/value checks, bounded UART/control lengths, timeout/recovery design, report unit tests, USB capture comparisons, reconnect behavior and real G29/adapter/console force-feedback checks.
- Vendored LUFA is currently version `130901`. Updating it is deferred until both baseline builds are green; the candidate stable release is `LUFA-170418`.

## Next steps

1. Finish the application baseline build and fix any missing checkout/build prerequisites.
2. Record compiler and package versions from the successful Ubuntu build.
3. Check a Windows/MSYS2 build.
4. Upgrade dependencies in small steps, building both targets after each step; start with ncurses compatibility, then evaluate LUFA.
5. Add report-level tests and validate descriptor/report behavior using USB captures.
6. Test authentication, reconnects and force feedback with a real G29, GIMX adapter and PS4.
7. Update this roadmap with each build result and code change.

## References

- [GIMX Development on Linux](https://gimx.fr/wiki/index.php?title=Development_on_Linux)
- [GIMX Development on Windows](https://gimx.fr/wiki/index.php?title=Development_on_Windows)
- [GIMX guide for steering wheel to PS4](https://gimx.fr/wiki/index.php?connectiontype=USB&device=SteeringWheel&ostype=windows&platform=ps4&title=Guide)
- [GIMX force feedback documentation](https://gimx.fr/wiki/index.php?title=Force_Feedback%2Fen)
- [LUFA releases](https://github.com/abcminiuser/lufa/releases)
