# Roadmap: GIMX Reborn and PC G29 force feedback

Last updated: 2026-11-13

## Scope and current branches

- Application: [SounMayon/GIMX, `GIMX-Reborn`](https://github.com/SounMayon/GIMX/tree/GIMX-Reborn)
- Firmware: [SounMayon/GIMX-firmwares, `G29-Finished`](https://github.com/SounMayon/GIMX-firmwares/tree/G29-Finished/EMUG29PS4)
- Documentation: [GIMX wiki](https://gimx.fr/wiki/index.php?title=Main_Page)

## Final acceptance goal: G29 FFB on PC

The emulated wheel must enumerate on a PC as a force-feedback device. FFB test software must discover every target effect, create/configure it, start and stop it, and produce the expected force on a physical wheel connected through GIMX. Merely advertising an effect in the descriptor does not count as support.

Use the USB HID PID 1.0 effect set as the coverage target: constant, ramp, custom force, square, sine, triangle, sawtooth up/down, spring, damper, inertia and friction. Microsoft's GameInput HID force-feedback path aligns with PID 1.0. Logitech documents the real G29's PC-mode identity as VID/PID 046D:C24F. Before implementation, verify whether Windows test software accepts PID reports with that identity or requires Logitech's native report protocol. Test discovery and commands with Windows DirectInput/GameInput tools, then confirm the expected force reaches the connected physical wheel. Keep the existing PS4 target as a separate regression path.

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
- PC target output is confirmed: effects must reach a physical wheel connected through GIMX. Standard USB HID PID 1.0 is the first candidate for broad PC effect discovery. The real G29 is identified as 046D:C24F in PC mode, but compatibility between that identity, PID reports and Windows test software must be verified before changing firmware behavior.
- Upstream Linux recognizes a G29 mode, but its hid-lg4ff driver exposes only FF_CONSTANT and autocenter to Linux's input force-feedback API. This describes that driver's supported interface; it does not establish the wheel hardware's full capability or Windows behavior. Use it as a reason to test the Windows protocol directly, not as a hardware capability limit. See [upstream hid-lg4ff.c](https://github.com/torvalds/linux/blob/master/drivers/hid/hid-lg4ff.c).
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

- [Descriptors.c](https://github.com/SounMayon/GIMX-firmwares/blob/G29-Finished/EMUG29PS4/Descriptors.c) advertises Generic Desktop, Button and Logitech vendor pages. The report descriptor has no PID Usage Page `0x0F` or standard Set Effect/Periodic/Condition report collections, so it does not currently advertise standard HID PID effects to a PC.
- Microsoft documents GameInput force feedback over HID as aligned with USB PID 1.0. Logitech's G29 product specification identifies `046D:C24F` as PC mode. These facts support a standard PID implementation as the first design candidate, but do not prove that Windows will route every test application through PID when using the Logitech identity. Verify enumeration and effect creation on Windows before settling descriptor identity and protocol. References: [Microsoft GameInput FFB](https://learn.microsoft.com/en-us/gaming/gdk/docs/features/common/input/hardware/input-hardware-force-feedback?view=gdk-2604), [Logitech G29 specifications](https://support.logi.com/hc/en-us/articles/360023463753-G29-Driving-Force-Racing-Wheel-Technical-Specifications).
- [emu.c](https://github.com/SounMayon/GIMX-firmwares/blob/G29-Finished/EMUG29PS4/emu.c) handles the existing vendor feature IDs `F0-F3`; it does not allocate PID effect blocks or process standard PID output reports.
- The application haptic core currently models rumble, constant, spring and damper (plus LEDs/range). The Logitech conversion supports only constant, a limited variable-to-constant case, spring, damper and high-resolution spring/damper. Ramp, periodic waveforms, custom force, inertia and friction are not represented in the generic core.
- Both current application sinks (Logitech protocol and OS/SDL) expose only rumble, constant, spring and damper; the core forwards discrete reports and has no periodic effect scheduler or motion-aware force model. Ramp/periodic effects will need time-varying output updates; inertia/friction need current wheel motion or a native condition-effect path, then output on the attached wheel.
- PC support therefore requires coordinated firmware descriptor/report handling, an expanded effect model, parameter-block lifecycle and effect execution, plus a defined route to the physical wheel. Keep the PS4 vendor protocol as a separate regression path and compile both targets after each step.
- Protocol decision checkpoint: test standard HID PID enumeration and all target effect creation on Windows using the intended PC identity. If the Logitech identity selects a native-only path, compare a generic PID identity and the real G29 descriptor/report capture before choosing a compatible architecture.
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
2. Resolve PC protocol compatibility on Windows: test the target PC identity and the real G29's descriptor/report behavior, then choose standard HID PID, native Logitech reports or a measured combination. Record the test application and observable capabilities.
3. Specify the selected descriptor/report IDs, effect pool strategy, capabilities and route to physical output against USB-IF PID 1.0 and the GIMX haptic sink.
4. Extend the shared haptic model and firmware/application transport for every required effect type; implement effect allocation, parameter updates, gain/envelopes, operation/status, and condition calculations from wheel motion.
5. Build a Windows test matrix that enumerates each effect, creates/configures it, starts/stops it and checks the resulting force; retain the PS4 build as a regression check.
6. Validate descriptor and report behavior with USB captures, then confirm every effect produces force on the physical wheel connected through GIMX.
7. Update this roadmap after every implementation step and CI result.

## References

- [GIMX Development on Linux](https://gimx.fr/wiki/index.php?title=Development_on_Linux)
- [GIMX Development on Windows](https://gimx.fr/wiki/index.php?title=Development_on_Windows)
- [GIMX guide for steering wheel to PS4](https://gimx.fr/wiki/index.php?connectiontype=USB&device=SteeringWheel&ostype=windows&platform=ps4&title=Guide)
- [GIMX force feedback documentation](https://gimx.fr/wiki/index.php?title=Force_Feedback%2Fen)
- [LUFA releases](https://github.com/abcminiuser/lufa/releases)
