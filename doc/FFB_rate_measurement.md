# Force feedback: measuring the computation and restitution rates

Goal: tell apart (1) how often `haptic_core_update()` recomputes the synthesized force and (2) how often the SDL sink really receives and applies a new force, before changing anything in the synthesizer.

## Build

```
make HAPTIC_STATS=1        # in core/, adds -DHAPTIC_STATS
```

Without `HAPTIC_STATS` the `HSTAT_*` macros are empty, `core/haptic/haptic_stats.c` is left out of the core build (`core/Makefile`), and the core keeps no diagnostic field: there is no code, no data and no behaviour change. The unit tests build `haptic_stats.c` on their own. The statistics are not thread safe: they are a diagnostic, not a feature.

## What is measured

Nothing is logged per event. Intervals between events go into 100 us histograms; a summary is printed to stderr every 5 s and when the core is cleaned. Resolution of the median/p95/p99 values is 100 us (upper bound of the bucket) up to 51.2 ms; above, intervals go into power of two buckets (51.2-102.4 ms, 102.4-204.8 ms, ...) and a percentile is the upper bound of its bucket, never above the real maximum. Only a percentile falling in the last bucket (above 6.5 s) is reported as the maximum.

| Line | Point |
|---|---|
| `haptic_core_update (all)` | every entry of `haptic_core_update()` (periodic refresh and after a report) |
| `haptic_core_update (after report)` | entries that followed the reception of an FFB report (a report processed by `haptic_core_process_report()` since the previous update) |
| `haptic_synth_step` | computations of the synthesized force |
| `synth result requested` | computations whose result had to be sent to the sink |
| `sink->process` | commands handed to the sink (any type: constant, spring, damper) |
| `sink->update` | calls to `sink->update()` (a no-op for the OS sink) |
| `SDL call attempted` | every call to `ginput_joystick_set_haptic()`, which is synchronous down to SDL |
| `SDL call accepted` | the same calls, only those that returned a non negative value |
| `SDL call duration` | duration of each call: the software submission time (processing and hand-over to the driver). It is not the delay between the request and the mechanical reaction of the wheel, which this instrumentation cannot see |

Also printed: SDL errors (the return value was ignored before), SDL calls that were not a constant force, and the constant levels (changes and unchanged repeats) at three stages: requested by the synthesizer, attempted (passed to SDL, whatever the result) and accepted (SDL returned success). A refused call is attempted but never accepted. The most recent transitions of the three series are printed as `[time in ms modulo 1000 s : level]`, with the same time base, so that requested, attempted and accepted transitions can be compared.

The SDL operation itself (`SDL_HapticNewEffect`, `SDL_HapticUpdateEffect`, `SDL_HapticRunEffect`) happens inside `sdlinput_joystick_set_haptic()` in the `shared/gimxinput` submodule: this instrumentation measures the whole call and cannot tell which SDL function dominates. Instrument the submodule only if the duration turns out to be the limiting factor.

## Protocol

Same duration (60 s) and same effect parameters for each trial. Run on Linux and Windows if possible, with `--refresh 5`, `--refresh 2` and `--refresh 1` (ms). Stop GIMX normally so that the final summary is printed.

1. Idle: no effect (background rate of the periodic calls).
2. Constant force: does SDL receive repeated updates while the level does not change? (`unchanged` counters)
3. Slow square wave (half period 20 ms): does every transition give one SDL call?
4. Fast square wave (half periods 2, 4, 6, 8 ms): from which period do transitions disappear?
5. Ramp: regularity of the transmitted levels.
6. Combined load: several simultaneous effects (two waves and a constant), to see the cost of the mix.

## Reading the results

| Observation | Likely limiting factor |
|---|---|
| `haptic_core_update` slow | upstream of the sink: scheduler or controller refresh |
| core fast, `sink->process` slow | the synthesizer does not produce enough updates |
| `sink->process` fast, SDL calls slow | the sink or the SDL update mechanism |
| SDL regular but transitions missing | the sampling rate is too low for the waveform |
| very variable intervals | scheduling jitter: look at p99 and the `>Nms` counters, not at the mean |

## Decision grid

| Observation | Decision to consider |
|---|---|
| SDL calls regular and fast, transitions missing | sampling limit of the synthesizer |
| SDL calls spaced out or irregular | look at the sink and the scheduler |
| SDL calls regular but long | instrument `SDL_HapticNewEffect`, `SDL_HapticUpdateEffect` and `SDL_HapticRunEffect` separately in `shared/gimxinput` |
| SDL errors not zero | diagnose the failures first |
| requested and accepted levels diverge | examine the sending conditions and the errors before any rate change |

## Report to produce

OS, device, refresh period; the summaries of the six trials; an excerpt of requested vs transmitted transitions; the limiting factor. Do not change the synthesizer rate before this report exists.

## Computation level prediction (no hardware)

`make -C core/test/haptic haptic_rate_sim && core/test/haptic/haptic_rate_sim [jitter_percent]` runs the real synthesizer with simulated ticks and a mock sink, for a square wave. It predicts what the polling period alone does, and says nothing about SDL or the scheduler. With no jitter, 100 % of the transitions of a square wave of half period >= 6 ms are delivered at a 5 ms tick, but only about 60 % at 4 ms (125 Hz) and about 20 % at 2 ms (250 Hz); at a 2 ms or 1 ms tick every transition is delivered. Compare the measured numbers with its output to separate the sampling limit from the SDL/OS limit.
