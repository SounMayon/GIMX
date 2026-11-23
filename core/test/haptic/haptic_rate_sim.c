/*
 License: GPLv3
 */

/*
 * Computation level prediction of how many square wave transitions survive a given polling period.
 * It runs the real haptic_synth with simulated ticks and a mock sink. It does NOT measure SDL nor the OS scheduler:
 * the real restitution rate has to be measured with make HAPTIC_STATS=1 on the target system.
 *
 * usage: haptic_rate_sim [jitter_percent]   (the tick period is varied randomly by +/- jitter_percent)
 */

#include <stdio.h>
#include <stdlib.h>

#include <haptic/common/ff_lg.h>
#include <haptic/haptic_synth.h>
#include <gimx.h>

s_gimx_params gimx_params = { 0 };

#define LOOP HAPTIC_WAVEFORM_LOOP_NS
#define MAX_DURATION_NS 4000000000ULL

static void run(uint64_t tick_ns, unsigned int half_loops, int jitter, unsigned long * sent, unsigned long * expected, unsigned long * wrong) {

    s_haptic_synth synth;
    s_haptic_core_tweaks tweaks = { .invert = 0, .gain = { 100, 100, 100, 100 } };
    s_haptic_core_data in, out;
    unsigned char sq[7] = { 0x11, FF_LG_FTYPE_SQUARE_WAVE, 255, (unsigned char)half_loops, 0, 0, 0 };
    uint64_t t0 = 1000000000ULL, t = t0;
    int last = 0, valid = 0;
    long expected_state;

    // N = 0 means 256 periods: the effect ends by itself, stay clear of its end (one loop margin)
    uint64_t duration = 256ULL * 2 * half_loops * LOOP - LOOP;
    if (duration > MAX_DURATION_NS) {
        duration = MAX_DURATION_NS;
    }

    haptic_synth_init(&synth);
    ff_lg_convert_force(0, 0, (const s_ff_lg_command *)(sq + 1), 1, &in);
    haptic_synth_feed(&synth, &in, t0);

    *sent = *wrong = 0;
    while (t - t0 < duration) {
        if (haptic_synth_step(&synth, t, &tweaks, &out)) {
            ++*sent;
            // the level must follow the wave: check it against the state at the time of the tick
            expected_state = (((t - t0) / LOOP) % (2 * half_loops)) < half_loops ? 1 : -1;
            if ((out.constant.level > 0 ? 1 : -1) != expected_state) {
                ++*wrong;
            }
            valid = 1;
            last = out.constant.level;
        }
        uint64_t step = tick_ns;
        if (jitter) {
            long delta = (long)(((double)rand() / RAND_MAX * 2.0 - 1.0) * jitter / 100.0 * (double)tick_ns);
            step = (uint64_t)((long)tick_ns + delta);
        }
        t += step;
    }
    (void)valid; (void)last;
    // expected transitions: one per half period of the wave over the simulated duration
    *expected = (unsigned long)(duration / (half_loops * LOOP));
}

int main(int argc, char ** argv) {

    int jitter = argc > 1 ? atoi(argv[1]) : 0;
    const uint64_t ticks_ms[] = { 5, 2, 1 };
    const unsigned int halves[] = { 1, 2, 3, 4, 5, 6, 8, 10, 16 };
    unsigned int i, j;

    printf("square wave transitions delivered to the sink (simulation, jitter +/-%d%%, 4 s per case)\n", jitter);
    printf("%-8s %-12s %-10s %-9s %-9s %-10s %-11s\n", "tick", "half period", "wave freq", "expected", "sent", "delivered", "wrong level");
    for (i = 0; i < sizeof(ticks_ms) / sizeof(*ticks_ms); ++i) {
        for (j = 0; j < sizeof(halves) / sizeof(*halves); ++j) {
            unsigned long sent, expected, wrong;
            srand(1);
            run(ticks_ms[i] * 1000000ULL, halves[j], jitter, &sent, &expected, &wrong);
            printf("%2llu ms    %2u ms        %6.1f Hz  %-9lu %-9lu %5.1f %%     %-11lu\n", (unsigned long long)ticks_ms[i],
                    halves[j] * 2, 1000.0 / (halves[j] * 4), expected, sent, expected ? 100.0 * sent / expected : 0.0, wrong);
        }
    }
    return 0;
}
