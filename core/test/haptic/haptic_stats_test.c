/*
 License: GPLv3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <haptic/haptic_stats.h>
#include <gimx.h>

s_gimx_params gimx_params = { 0 };

static int failures = 0;

#define CHECK(NAME, COND) do { \
    int ok_ = (COND); \
    printf("test case: %s: %s\n", NAME, ok_ ? "success" : "failure"); \
    if (!ok_) { ++failures; } \
} while (0)

#define MS 1000000ULL

int main(void) {

    haptic_stats_reset();

    // 1000 events every 5 ms, with one 12 ms gap and one 2.4 ms interval
    uint64_t t = 1000 * MS;
    int i;
    for (i = 0; i < 1000; ++i) {
        haptic_stats_event(HSTAT_CORE_UPDATE, t);
        t += (i == 400) ? 12 * MS : (i == 700) ? 2400000ULL : 5 * MS;
    }
    const s_haptic_stat_series * s = haptic_stats.points + HSTAT_CORE_UPDATE;
    CHECK("stats: n events gives n-1 intervals", s->count == 999);
    CHECK("stats: min and max", s->min_ns == 2400000ULL && s->max_ns == 12 * MS);
    CHECK("stats: threshold counters", s->gt1ms == 999 && s->gt2ms == 999 && s->gt5ms == 1 && s->gt10ms == 1);
    uint64_t mean = haptic_stats_mean_ns(s);
    CHECK("stats: mean close to 5 ms", mean > 4990000ULL && mean < 5010000ULL);
    CHECK("stats: median is the 5 ms bucket", haptic_stats_percentile_ns(s, 50) >= 5 * MS && haptic_stats_percentile_ns(s, 50) <= 5100000ULL);
    CHECK("stats: p99 not polluted by a single outlier", haptic_stats_percentile_ns(s, 99) <= 5100000ULL);
    CHECK("stats: p100-like rank reaches the outlier", haptic_stats_percentile_ns(s, 100) >= 12 * MS);

    // intervals above the histogram: many moderate delays (60-90 ms) and a single extreme one (3 s)
    haptic_stats_reset();
    uint64_t u = 1 * MS;
    haptic_stats_event(HSTAT_SINK_UPDATE, u);
    for (i = 0; i < 90; ++i) { // 90 intervals of 5 ms
        u += 5 * MS;
        haptic_stats_event(HSTAT_SINK_UPDATE, u);
    }
    for (i = 0; i < 9; ++i) { // 9 intervals of 60..92 ms
        u += (60 + 4 * (uint64_t)i) * MS;
        haptic_stats_event(HSTAT_SINK_UPDATE, u);
    }
    u += 3000 * MS; // 1 extreme interval
    haptic_stats_event(HSTAT_SINK_UPDATE, u);
    const s_haptic_stat_series * o = haptic_stats.points + HSTAT_SINK_UPDATE;
    CHECK("overflow: 100 intervals, 10 above the histogram", o->count == 100 && o->overflow == 10);
    CHECK("overflow: p95 is not the maximum", haptic_stats_percentile_ns(o, 95) < 200 * MS);
    CHECK("overflow: p95 reflects the moderate delays", haptic_stats_percentile_ns(o, 95) >= 60 * MS);
    CHECK("overflow: p99 and p100 reach the extreme delay", haptic_stats_percentile_ns(o, 100) == 3000 * MS);
    CHECK("overflow: p99 stays in the moderate delays", haptic_stats_percentile_ns(o, 99) < 3000 * MS);
    CHECK("overflow: bucket upper bound never exceeds the maximum", haptic_stats_percentile_ns(o, 91) <= o->max_ns);
    haptic_stats_reset();
    haptic_stats_event(HSTAT_SINK_UPDATE, 1 * MS);
    haptic_stats_event(HSTAT_SINK_UPDATE, 200 * MS);
    CHECK("overflow: a lone interval above the histogram", haptic_stats_percentile_ns(haptic_stats.points + HSTAT_SINK_UPDATE, 99) == 199 * MS);

    // levels: requested by the synthesizer is tracked on its own
    haptic_stats_reset();
    haptic_stats_level(HSTAT_SYNTH_RESULT, 1 * MS, 100);
    haptic_stats_level(HSTAT_SYNTH_RESULT, 2 * MS, 100);
    haptic_stats_level(HSTAT_SYNTH_RESULT, 3 * MS, -100);
    CHECK("levels: requested changes and repeats", haptic_stats.requested_changes == 1 && haptic_stats.requested_unchanged == 1);
    CHECK("levels: nothing attempted yet", haptic_stats.attempted_n == 0 && haptic_stats.accepted_n == 0);

    // ring keeps the most recent transitions
    for (i = 0; i < 1000; ++i) {
        haptic_stats_sdl_call((uint64_t)(10 + i) * MS, (uint64_t)(10 + i) * MS + 100000ULL, 0, 1, i);
    }
    CHECK("ring: total counter keeps counting", haptic_stats.attempted_n == 1000 && haptic_stats.accepted_n == 1000);
    CHECK("ring: oldest entries are overwritten, latest kept",
            haptic_stats.attempted[(haptic_stats.attempted_n - 1) % HSTAT_RING].level == 999);

    // SDL calls: attempted vs accepted
    haptic_stats_reset();
    haptic_stats_sdl_call(10 * MS, 10 * MS + 300000ULL, 0, 1, 100);   // accepted constant
    haptic_stats_sdl_call(15 * MS, 15 * MS + 900000ULL, -1, 1, -100); // refused constant
    haptic_stats_sdl_call(20 * MS, 20 * MS + 200000ULL, 0, 0, 0);     // accepted spring
    CHECK("sdl: durations recorded", haptic_stats.sdl_duration.count == 3 && haptic_stats.sdl_duration.max_ns == 900000ULL);
    CHECK("sdl: errors and non constant counted", haptic_stats.sdl_errors == 1 && haptic_stats.sdl_non_constant == 1);
    CHECK("sdl: a refused call is attempted but not accepted", haptic_stats.attempted_n == 2 && haptic_stats.accepted_n == 1
            && haptic_stats.attempted_last == -100 && haptic_stats.accepted_last == 100);
    CHECK("sdl: interval between attempted calls", haptic_stats.points[HSTAT_SDL_CALL].count == 2
            && haptic_stats.points[HSTAT_SDL_CALL].min_ns == 5 * MS);
    CHECK("sdl: accepted series skips the refused call", haptic_stats.points[HSTAT_SDL_ACCEPTED].count == 1
            && haptic_stats.points[HSTAT_SDL_ACCEPTED].min_ns == 10 * MS);

    // transition time stamps: ms modulo 1000 s, same time base for every series
    haptic_stats_reset();
    haptic_stats_level(HSTAT_SYNTH_RESULT, 1234567890123ULL, 42);
    haptic_stats_sdl_call(1234567890123ULL, 1234567890123ULL + 1000, 0, 1, 42);
    FILE * g = tmpfile();
    haptic_stats_dump(g, 1);
    rewind(g);
    char text[8192] = { 0 };
    size_t n = fread(text, 1, sizeof(text) - 1, g);
    text[n] = '\0';
    fclose(g);
    CHECK("dump: time stamp is in ms modulo 1000 s", strstr(text, "[234567.890 ms:42]") != NULL);
    CHECK("dump: requested, attempted and accepted series are printed", strstr(text, "requested transitions") && strstr(text, "attempted transitions")
            && strstr(text, "accepted transitions"));

    // periodic report
    haptic_stats_reset();
    FILE * f = tmpfile();
    haptic_stats_report_if_due(f, 1000 * MS, 5000 * MS);
    haptic_stats_report_if_due(f, 3000 * MS, 5000 * MS);
    long before = ftell(f);
    haptic_stats_report_if_due(f, 7000 * MS, 5000 * MS);
    CHECK("report: nothing before the period, a summary after", before == 0 && ftell(f) > 0);
    fclose(f);

    printf("%d failure(s)\n", failures);
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
