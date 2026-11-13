# Roadmap: GIMX Reborn and PC G29 force feedback

Last updated: 2026-11-13

## Scope and current branches

- Application: [SounMayon/GIMX, `GIMX-Reborn`](https://github.com/SounMayon/GIMX/tree/GIMX-Reborn)
- Firmware: [SounMayon/GIMX-firmwares, `G29-Finished`](https://github.com/SounMayon/GIMX-firmwares/tree/G29-Finished/EMUG29PS4)
- Documentation: [GIMX wiki](https://gimx.fr/wiki/index.php?title=Main_Page)

## Final acceptance goal: G29 FFB on PC

The emulated G29 must enumerate on a PC as a force-feedback device. FFB test software must discover each supported effect type, create/configure it, start and stop it, and produce the expected force through the chosen output path. Merely advertising an effect in the descriptor does not count as support.

The target effect set follows USB HID PID 1.0: constant, ramp, custom force, square, sine, triangle, sawtooth up/down, spring, damper, inertia and friction. Validate discovery and commands with Windows DirectInput/GameInput tools, then verify force output on a connected actuator or a documented simulator. Keep the existing PS4 target as a separate regression path.

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
- Define the PC HID PID report path and effect execution model; the current firmware is a PS4-focused Logitech report emulator and does not yet implement the standard PC PID effect contract.
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

### PC USB PID gap

- [Descriptors.c](https://github.com/SounMayon/GIMX-firmwares/blob/G29-Finished/EMUG29PS4/Descriptors.c) advertises Generic Desktop, Button and Logitech vendor pages. The report descriptor has no PID Usage Page `0x0F` or standard Set Effect/Periodic/Condition report collections. From that descriptor, standard PC PID software has no advertised effect capabilities; verify this on Windows as the PC path is implemented.
- [emu.c](https://github.com/SounMayon/GIMX-firmwares/blob/G29-Finished/EMUG29PS4/emu.c) handles the existing vendor feature IDs `F0-F3`; it does not allocate PID effect blocks or process standard PID output reports.
- The application haptic core currently models rumble, constant, spring and damper (plus LEDs/range). The Logitech conversion supports only constant, a limited variable-to-constant case, spring, damper and high-resolution spring/damper. Ramp, periodic waveforms, custom force, inertia and friction are not represented in the generic core.
- PC support therefore requires coordinated firmware descriptor/report handling, an expanded effect model, parameter-block lifecycle and effect execution, plus a defined route to the physical wheel or simulator. Keep those changes separate from the PS4 vendor protocol and compile both paths after each step.
- USB-IF references: [PID Class Definition 1.0](https://www.usb.org/sites/default/files/documents/pid1_01.pdf) and [HID Usage Tables](https://www.usb.org/sites/default/files/hut1_3_0.pdf).

### Existing G29 PS4 emulation

- The firmware descriptor advertises Logitech `046d:C260` and interrupt input/output endpoints in [Descriptors.c](https://github.com/SounMayon/GIMX-firmwares/blob/G29-Finished/EMUG29PS4/Descriptors.c#L95-L105).
- The firmware handles HID feature requests `F1/F2` in [emu.c](https://github.com/SounMayon/GIMX-firmwares/blob/G29-Finished/EMUG29PS4/emu.c#L32-L45), sends a serial header and spins until `spoofReply` arrives. There is no timeout or recovery path, so a lost reply can stall that request.
- The application already registers a G29 PS4 controller, builds wheel/pedal/button reports, handles authentication and sends reports at a 5 ms default interval.
- FFB is not universally flattened to constant: the Logitech parser maps `CONSTANT`, `SPRING` and `DAMPER` to distinct core types; `VARIABLE` is approximated as constant only when it has no step timing/size. Other Logitech force types fall through as unsupported. On output, a core constant is encoded as Logitech `VARIABLE` with fixed level, while spring and damper remain their own types. See [the conversions](https://github.com/SounMayon/GIMX/blob/GIMX-Reborn/core/haptic/common/ff_lg.c#L385-L497) and [Logitech report encoding](https://github.com/SounMayon/GIMX/blob/GIMX-Reborn/core/haptic/common/ff_lg.c#L550-L655).
- GIMX's guide documents working wheel, pedal, wheel-button and shifter controls, combined input devices, and force feedback; the GIMX path is an existing G29 emulator that needs validation and robustness work. See the [G29 to PS4 guide](https://gimx.fr/wiki/index.php?connectiontype=USB&device=SteeringWheel&ostype=windows&platform=ps4&title=Guide) and [force feedback notes](https://gimx.fr/wiki/index.php?title=Force_Feedback).
- Highest-value additions: bound control/serial lengths, add a timeout and recoverable error path for `F1/F2`, test report boundaries and reconnect behavior, compare descriptors/reports with a genuine G29 capture, then validate authentication and force feedback on a real G29, adapter and PS4. Do not change framing or timing without captures and hardware validation.
- Firmware baseline used bundled LUFA `130901`. After both baseline builds passed, only the G29 target was switched to the pinned upstream `LUFA-170418` submodule. Its build passes with the descriptor callback updated to LUFA's `uint16_t wIndex` signature; other firmware targets still use the existing shared LUFA tree.

## Next steps

1. Preserve the green Linux and AVR build baselines and check the Windows/MSYS2 application build.
2. Specify the PC PID descriptor, report IDs, effect pool strategy, capabilities and physical-output/simulator path against the USB-IF PID 1.0 contract.
3. Extend the shared haptic model and firmware/application transport for every required effect type; implement effect allocation, parameter updates, gain/envelopes, operation/status, and condition calculations from wheel motion.
4. Build a Windows test matrix that enumerates each effect, creates/configures it, starts/stops it and checks the resulting force; retain the PS4 build as a regression check.
5. Validate descriptor and report behavior with USB captures, then test force output with a real G29/adapter or the documented simulator.
6. Update this roadmap after every implementation step and CI result.

## References

- [GIMX Development on Linux](https://gimx.fr/wiki/index.php?title=Development_on_Linux)
- [GIMX Development on Windows](https://gimx.fr/wiki/index.php?title=Development_on_Windows)
- [GIMX guide for steering wheel to PS4](https://gimx.fr/wiki/index.php?connectiontype=USB&device=SteeringWheel&ostype=windows&platform=ps4&title=Guide)
- [GIMX force feedback documentation](https://gimx.fr/wiki/index.php?title=Force_Feedback%2Fen)
- [LUFA releases](https://github.com/abcminiuser/lufa/releases)
