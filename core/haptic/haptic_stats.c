/*
 License: GPLv3
 */

#include <string.h>
#include <haptic/haptic_stats.h>

s_haptic_stats haptic_stats;

static const char * const point_names[HSTAT_POINTS] = {
    "haptic_core_update (all)",
    "haptic_core_update (after report)",
    "haptic_synth_step",
    "synth result requested",
    "sink->process",
    "sink->update",
    "SDL call attempted",
    "SDL call accepted",
};

void haptic_stats_reset(void) {

    memset(&haptic_stats, 0x00, sizeof(haptic_stats));
}

static void series_add_interval(s_haptic_stat_series * s, uint64_t interval) {

    if (s->count == 0 || interval < s->min_ns) {
        s->min_ns = interval;
    }
    if (interval > s->max_ns) {
        s->max_ns = interval;
    }
    s->sum_ns += interval;
    if (interval > 1000000ULL) ++s->gt1ms;
    if (interval > 2000000ULL) ++s->gt2ms;
    if (interval > 5000000ULL) ++s->gt5ms;
    if (interval > 10000000ULL) ++s->gt10ms;
    uint64_t bucket = interval / HSTAT_BUCKET_NS;
    if (bucket >= HSTAT_BUCKETS) {
        ++s->overflow;
        unsigned int k = 0;
        uint64_t limit = 2 * HSTAT_BUCKETS * HSTAT_BUCKET_NS; // upper bound of the first overflow bucket
        while (k < HSTAT_OVERFLOW_BUCKETS - 1 && interval >= limit) {
            ++k;
            limit *= 2;
        }
        ++s->overflow_log[k];
    } else {
        ++s->buckets[bucket];
    }
    ++s->count; // number of intervals
}

static void series_event(s_haptic_stat_series * s, uint64_t now) {

    if (s->last_ns != 0 && now >= s->last_ns) {
        series_add_interval(s, now - s->last_ns);
    }
    s->last_ns = now;
}

void haptic_stats_event(e_haptic_stat_point point, uint64_t now_ns) {

    if ((unsigned int)point < HSTAT_POINTS) {
        series_event(haptic_stats.points + point, now_ns);
    }
}

static void level_record(uint64_t now, int level, uint8_t * valid, int32_t * last, uint64_t * changes, uint64_t * unchanged,
        s_haptic_stat_transition * ring, uint64_t * n) {

    if (*valid && *last == level) {
        ++*unchanged;
        return;
    }
    if (*valid) {
        ++*changes;
    }
    *valid = 1;
    *last = level;
    ring[*n % HSTAT_RING].ns = now;
    ring[*n % HSTAT_RING].level = level;
    ++*n;
}

void haptic_stats_level(e_haptic_stat_point point, uint64_t now_ns, int level) {

    s_haptic_stats * s = &haptic_stats;
    if (point == HSTAT_SYNTH_RESULT) {
        level_record(now_ns, level, &s->requested_valid, &s->requested_last, &s->requested_changes, &s->requested_unchanged,
                s->requested, &s->requested_n);
    }
}

void haptic_stats_sdl_call(uint64_t start_ns, uint64_t end_ns, int ret, int constant, int level) {

    s_haptic_stats * s = &haptic_stats;
    series_event(s->points + HSTAT_SDL_CALL, start_ns);
    if (end_ns >= start_ns) {
        series_add_interval(&s->sdl_duration, end_ns - start_ns);
    }
    if (constant) {
        level_record(start_ns, level, &s->attempted_valid, &s->attempted_last, &s->attempted_changes, &s->attempted_unchanged,
                s->attempted, &s->attempted_n);
    } else {
        ++s->sdl_non_constant;
    }
    if (ret < 0) {
        ++s->sdl_errors;
        return;
    }
    series_event(s->points + HSTAT_SDL_ACCEPTED, start_ns);
    if (constant) {
        level_record(start_ns, level, &s->accepted_valid, &s->accepted_last, &s->accepted_changes, &s->accepted_unchanged,
                s->accepted, &s->accepted_n);
    }
}

uint64_t haptic_stats_percentile_ns(const s_haptic_stat_series * series, unsigned int percentile) {

    if (series->count == 0) {
        return 0;
    }
    uint64_t target = (series->count * percentile + 99) / 100; // rank, rounded up
    if (target == 0) {
        target = 1;
    }
    uint64_t seen = 0;
    unsigned int i;
    for (i = 0; i < HSTAT_BUCKETS; ++i) {
        seen += series->buckets[i];
        if (seen >= target) {
            return (uint64_t)(i + 1) * HSTAT_BUCKET_NS;
        }
    }
    // above the histogram: power of two buckets, the value is the upper bound of the bucket, never above the real maximum
    uint64_t upper = 2 * HSTAT_BUCKETS * HSTAT_BUCKET_NS;
    for (i = 0; i < HSTAT_OVERFLOW_BUCKETS; ++i) {
        seen += series->overflow_log[i];
        if (seen >= target) {
            return (i == HSTAT_OVERFLOW_BUCKETS - 1 || upper > series->max_ns) ? series->max_ns : upper;
        }
        upper *= 2;
    }
    return series->max_ns;
}

uint64_t haptic_stats_mean_ns(const s_haptic_stat_series * series) {

    return series->count ? series->sum_ns / series->count : 0;
}

static void dump_series(FILE * out, const char * name, const s_haptic_stat_series * s) {

    if (s->count == 0) {
        fprintf(out, "  %-34s no interval recorded\n", name);
        return;
    }
    uint64_t mean = haptic_stats_mean_ns(s);
    fprintf(out, "  %-34s n=%-8llu mean=%7.3f ms (%7.1f Hz) median<=%6.2f p95<=%6.2f p99<=%6.2f min=%6.3f max=%7.3f ms  >1ms:%llu >2ms:%llu >5ms:%llu >10ms:%llu\n",
            name, (unsigned long long)s->count, mean / 1e6, mean ? 1e9 / mean : 0.0,
            haptic_stats_percentile_ns(s, 50) / 1e6, haptic_stats_percentile_ns(s, 95) / 1e6, haptic_stats_percentile_ns(s, 99) / 1e6,
            s->min_ns / 1e6, s->max_ns / 1e6,
            (unsigned long long)s->gt1ms, (unsigned long long)s->gt2ms, (unsigned long long)s->gt5ms, (unsigned long long)s->gt10ms);
}

static void dump_ring(FILE * out, const char * name, const s_haptic_stat_transition * ring, uint64_t n) {

    uint64_t count = n < HSTAT_RING ? n : HSTAT_RING;
    uint64_t i;
    fprintf(out, "  %s (%llu most recent shown of %llu, [time in ms modulo 1000 s: level]):", name, (unsigned long long)(count < 24 ? count : 24), (unsigned long long)n);
    for (i = 0; i < count && i < 24; ++i) { // the 24 most recent in chronological order
        uint64_t idx = n - (count < 24 ? count : 24) + i;
        fprintf(out, " [%.3f ms:%d]", (ring[idx % HSTAT_RING].ns % 1000000000000ULL) / 1e6, (int)ring[idx % HSTAT_RING].level);
    }
    fprintf(out, "\n");
}

void haptic_stats_dump(FILE * out, int with_transitions) {

    const s_haptic_stats * s = &haptic_stats;
    int i;
    fprintf(out, "haptic stats: intervals between events\n");
    for (i = 0; i < HSTAT_POINTS; ++i) {
        dump_series(out, point_names[i], s->points + i);
    }
    dump_series(out, "SDL call duration", &s->sdl_duration);
    fprintf(out, "  SDL errors=%llu, SDL calls that were not a constant force=%llu\n",
            (unsigned long long)s->sdl_errors, (unsigned long long)s->sdl_non_constant);
    fprintf(out, "  constant level, requested by the synthesizer: %llu changes, %llu unchanged\n",
            (unsigned long long)s->requested_changes, (unsigned long long)s->requested_unchanged);
    fprintf(out, "  constant level, attempted (passed to SDL): %llu changes, %llu unchanged; accepted by SDL: %llu changes, %llu unchanged\n",
            (unsigned long long)s->attempted_changes, (unsigned long long)s->attempted_unchanged,
            (unsigned long long)s->accepted_changes, (unsigned long long)s->accepted_unchanged);
    if (with_transitions) {
        dump_ring(out, "requested transitions", s->requested, s->requested_n);
        dump_ring(out, "attempted transitions", s->attempted, s->attempted_n);
        dump_ring(out, "accepted transitions", s->accepted, s->accepted_n);
    }
}

void haptic_stats_report_if_due(FILE * out, uint64_t now_ns, uint64_t period_ns) {

    if (haptic_stats.last_report_ns == 0) {
        haptic_stats.last_report_ns = now_ns;
    } else if (now_ns - haptic_stats.last_report_ns >= period_ns) {
        haptic_stats.last_report_ns = now_ns;
        haptic_stats_dump(out, 0);
    }
}
