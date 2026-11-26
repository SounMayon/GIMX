/*
 License: GPLv3
 */

/*
 * Diagnostic statistics for the force feedback path, to separate:
 *  - the computation rate: how often haptic_core_update() runs and recomputes the synthesized force,
 *  - the restitution rate: how often the sink really receives a new force and calls into SDL.
 *
 * Nothing is logged per event: intervals go into histograms, transitions into fixed-size rings, and the summary
 * is printed every few seconds and when the core is cleaned. The clock is provided by the caller (nanoseconds).
 *
 * The instrumentation is compiled in only with -DHAPTIC_STATS (make HAPTIC_STATS=1). Without it the macros below are
 * empty, haptic_stats.c is not part of the core build and the core keeps no diagnostic field. The unit tests build
 * haptic_stats.c on their own. The counters are not thread safe: they are meant for diagnostics.
 */

#ifndef HAPTIC_STATS_H_
#define HAPTIC_STATS_H_

#include <stdint.h>
#include <stdio.h>

typedef enum {
    HSTAT_CORE_UPDATE,        // entry of haptic_core_update(), any caller
    HSTAT_CORE_UPDATE_REPORT, // entry of haptic_core_update() right after an FFB report was received
    HSTAT_SYNTH_STEP,         // haptic_synth_step() computations
    HSTAT_SYNTH_RESULT,       // haptic_synth_step() results to send to the sink (requested level)
    HSTAT_SINK_PROCESS,       // haptic_sink->process() calls, any type
    HSTAT_SINK_UPDATE,        // haptic_sink->update() calls
    HSTAT_SDL_CALL,           // ginput_joystick_set_haptic() calls attempted (synchronous down to SDL)
    HSTAT_SDL_ACCEPTED,       // the same calls, only those that returned a non negative value
    HSTAT_POINTS
} e_haptic_stat_point;

#define HSTAT_BUCKET_NS 100000ULL // 100 us per histogram bucket
#define HSTAT_BUCKETS 512         // up to 51.2 ms, then overflow
#define HSTAT_OVERFLOW_BUCKETS 8 // above the histogram: [51.2 ms * 2^k, 51.2 ms * 2^(k+1)), the last one is open ended
#define HSTAT_RING 256

typedef struct {
    uint64_t count;
    uint64_t last_ns;
    uint64_t min_ns, max_ns, sum_ns; // intervals between consecutive events (durations for the SDL duration series)
    uint64_t gt1ms, gt2ms, gt5ms, gt10ms;
    uint32_t buckets[HSTAT_BUCKETS];
    uint32_t overflow;                            // total number of intervals above the histogram
    uint32_t overflow_log[HSTAT_OVERFLOW_BUCKETS]; // the same, in power of two buckets
} s_haptic_stat_series;

typedef struct {
    uint64_t ns;
    int32_t level;
} s_haptic_stat_transition;

typedef struct {
    s_haptic_stat_series points[HSTAT_POINTS];
    s_haptic_stat_series sdl_duration; // duration of each ginput_joystick_set_haptic() call (software submission only)
    uint64_t sdl_errors;               // calls that returned a negative value
    uint64_t sdl_non_constant;         // calls that were not a constant force (spring, damper)
    uint64_t requested_changes, requested_unchanged; // constant level requested by the synthesizer
    uint64_t attempted_changes, attempted_unchanged; // constant level passed to SDL, whatever the result
    uint64_t accepted_changes, accepted_unchanged;   // constant level passed to SDL and accepted (return value >= 0)
    int32_t requested_last, attempted_last, accepted_last;
    uint8_t requested_valid, attempted_valid, accepted_valid;
    s_haptic_stat_transition requested[HSTAT_RING], attempted[HSTAT_RING], accepted[HSTAT_RING];
    uint64_t requested_n, attempted_n, accepted_n; // total number of transitions recorded (ring index = n % HSTAT_RING)
    uint64_t last_report_ns;
} s_haptic_stats;

extern s_haptic_stats haptic_stats;

void haptic_stats_reset(void);

// record one occurrence of a point
void haptic_stats_event(e_haptic_stat_point point, uint64_t now_ns);

// record the constant force level requested by the synthesizer (point must be HSTAT_SYNTH_RESULT)
void haptic_stats_level(e_haptic_stat_point point, uint64_t now_ns, int level);

// record one SDL call: attempted always, accepted only if ret >= 0. The level is only used if constant is not 0.
// The duration is the software submission time (up to the driver), not the physical reaction of the wheel.
void haptic_stats_sdl_call(uint64_t start_ns, uint64_t end_ns, int ret, int constant, int level);

// percentile (0-100) of the intervals of a series, resolution = HSTAT_BUCKET_NS, upper bound of the bucket
uint64_t haptic_stats_percentile_ns(const s_haptic_stat_series * series, unsigned int percentile);

// mean interval in ns (0 if fewer than 2 events)
uint64_t haptic_stats_mean_ns(const s_haptic_stat_series * series);

// print the summary
void haptic_stats_dump(FILE * out, int with_transitions);

// print the summary if at least period_ns elapsed since the last report
void haptic_stats_report_if_due(FILE * out, uint64_t now_ns, uint64_t period_ns);

#ifdef HAPTIC_STATS
#define HSTAT_EVENT(POINT, NOW) haptic_stats_event(POINT, NOW)
#define HSTAT_LEVEL(POINT, NOW, LEVEL) haptic_stats_level(POINT, NOW, LEVEL)
#define HSTAT_SDL(START, END, RET, CONSTANT, LEVEL) haptic_stats_sdl_call(START, END, RET, CONSTANT, LEVEL)
#define HSTAT_REPORT(NOW) haptic_stats_report_if_due(stderr, NOW, 5000000000ULL)
#define HSTAT_DUMP() haptic_stats_dump(stderr, 1)
#else
#define HSTAT_EVENT(POINT, NOW) do { } while (0)
#define HSTAT_LEVEL(POINT, NOW, LEVEL) do { } while (0)
#define HSTAT_SDL(START, END, RET, CONSTANT, LEVEL) do { } while (0)
#define HSTAT_REPORT(NOW) do { } while (0)
#define HSTAT_DUMP() do { } while (0)
#endif

#endif /* HAPTIC_STATS_H_ */
