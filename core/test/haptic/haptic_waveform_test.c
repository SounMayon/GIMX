/*
 License: GPLv3
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <haptic/common/ff_lg.h>
#include <haptic/haptic_waveform.h>
#include <haptic/haptic_synth.h>
#include <haptic/haptic_source.h>
#include <gimx.h>

s_gimx_params gimx_params = { 0 };

#define LOOP HAPTIC_WAVEFORM_LOOP_NS

static int failures = 0;

#define CHECK(NAME, COND) do { \
    int ok_ = (COND); \
    printf("test case: %s: %s\n", NAME, ok_ ? "success" : "failure"); \
    if (!ok_) { ++failures; } \
} while (0)

// decode a Logitech report (7 bytes: cmd, type, 5 parameters) for slot 0 and return the converted data
static int decode(const unsigned char report[7], uint8_t slot, int playing, s_haptic_core_data * data) {
    return ff_lg_convert_force(0, slot, (const s_ff_lg_command *)(report + 1), playing, data);
}

// check that every level of a waveform stays within [min, max] over many loops
static int stays_within(const s_haptic_waveform * wf, int16_t min, int16_t max, uint64_t loops) {
    uint64_t t;
    for (t = 0; t < loops; ++t) {
        int f;
        int16_t v = haptic_waveform_eval(wf, t * LOOP, &f);
        if (v < min || v > max) {
            return 0;
        }
    }
    return 1;
}

static void test_sawtooth(void) {
    // L1=200 L2=100 L0=150 T3=2 INC=5
    unsigned char r[7] = { 0x11, FF_LG_FTYPE_SAWTOOTH_UP, 200, 100, 150, 0x00, 0x25 };
    s_haptic_core_data d;
    CHECK("sawtooth up: decoded", decode(r, 0, 1, &d) && d.waveform_valid && d.waveform.kind == E_WAVEFORM_SAWTOOTH_UP
            && d.type == E_DATA_TYPE_LOGITECH_RAW);
    int f;
    CHECK("sawtooth up: initial level is L0", haptic_waveform_eval(&d.waveform, 0, &f) == ff_lg_u8_to_s16(150));
    CHECK("sawtooth up: level after one step", haptic_waveform_eval(&d.waveform, 2 * LOOP, &f) == ff_lg_u8_to_s16(155));
    CHECK("sawtooth up: no step before T3 loops", haptic_waveform_eval(&d.waveform, 1 * LOOP, &f) == ff_lg_u8_to_s16(150));
    // 10 steps of 5 = +50 -> 200 (L1). span = 101: the 11th step gives (50 + 55) % 101 = 4 -> 104 (wrap to L2 side)
    CHECK("sawtooth up: reaches L1", haptic_waveform_eval(&d.waveform, 20 * LOOP, &f) == ff_lg_u8_to_s16(200));
    CHECK("sawtooth up: first wrap-around value", haptic_waveform_eval(&d.waveform, 22 * LOOP, &f) == ff_lg_u8_to_s16(104));
    CHECK("sawtooth up: stays within [L2, L1]", stays_within(&d.waveform, ff_lg_u8_to_s16(100), ff_lg_u8_to_s16(200), 5000));
    unsigned char rd[7] = { 0x11, FF_LG_FTYPE_SAWTOOTH_DOWN, 200, 100, 150, 0x00, 0x25 };
    decode(rd, 0, 1, &d);
    CHECK("sawtooth down: level after one step", haptic_waveform_eval(&d.waveform, 2 * LOOP, &f) == ff_lg_u8_to_s16(145));
    CHECK("sawtooth down: reaches L2", haptic_waveform_eval(&d.waveform, 20 * LOOP, &f) == ff_lg_u8_to_s16(100));
    CHECK("sawtooth down: first wrap-around value", haptic_waveform_eval(&d.waveform, 22 * LOOP, &f) == ff_lg_u8_to_s16(196));
    CHECK("sawtooth down: stays within [L2, L1]", stays_within(&d.waveform, ff_lg_u8_to_s16(100), ff_lg_u8_to_s16(200), 5000));
    unsigned char rz[7] = { 0x11, FF_LG_FTYPE_SAWTOOTH_UP, 200, 100, 150, 0x00, 0x20 };
    decode(rz, 0, 1, &d);
    CHECK("sawtooth: INC = 0 keeps L0", haptic_waveform_eval(&d.waveform, 1000 * LOOP, &f) == ff_lg_u8_to_s16(150));
}

static void test_trapezoid(void) {
    // L1=228 L2=28 T1=4 T2=6 T3=1 S=10 -> ramp = 20 loops
    unsigned char r[7] = { 0x11, FF_LG_FTYPE_TRAPEZOID, 228, 28, 4, 6, 0x1A };
    s_haptic_core_data d;
    int f;
    decode(r, 0, 1, &d);
    CHECK("trapezoid: starts at L1", haptic_waveform_eval(&d.waveform, 0, &f) == ff_lg_u8_to_s16(228));
    CHECK("trapezoid: holds L1 for T1 loops", haptic_waveform_eval(&d.waveform, 3 * LOOP, &f) == ff_lg_u8_to_s16(228));
    CHECK("trapezoid: falls by S per step", haptic_waveform_eval(&d.waveform, 5 * LOOP, &f) == ff_lg_u8_to_s16(218));
    CHECK("trapezoid: holds L2", haptic_waveform_eval(&d.waveform, (4 + 20 + 1) * LOOP, &f) == ff_lg_u8_to_s16(28));
    CHECK("trapezoid: rises back", haptic_waveform_eval(&d.waveform, (4 + 20 + 6 + 2) * LOOP, &f) == ff_lg_u8_to_s16(48));
    CHECK("trapezoid: periodic", haptic_waveform_eval(&d.waveform, (4 + 20 + 6 + 20) * LOOP, &f) == ff_lg_u8_to_s16(228));
}

static void test_rectangle_square(void) {
    unsigned char r[7] = { 0x11, FF_LG_FTYPE_RECTANGLE, 200, 50, 3, 5, 0 };
    s_haptic_core_data d;
    int f;
    decode(r, 0, 1, &d);
    CHECK("rectangle: L1 during T1", haptic_waveform_eval(&d.waveform, 2 * LOOP, &f) == ff_lg_u8_to_s16(200));
    CHECK("rectangle: L2 during T2", haptic_waveform_eval(&d.waveform, 3 * LOOP, &f) == ff_lg_u8_to_s16(50));
    CHECK("rectangle: periodic", haptic_waveform_eval(&d.waveform, 8 * LOOP, &f) == ff_lg_u8_to_s16(200));
    // phase P = 2: offset of 4 loops (hypothesis), the period is 8 loops: starts in the L2 part
    unsigned char rp[7] = { 0x11, FF_LG_FTYPE_RECTANGLE, 200, 50, 3, 5, 2 };
    decode(rp, 0, 1, &d);
    CHECK("rectangle: phase offset (2*P loops)", haptic_waveform_eval(&d.waveform, 0, &f) == ff_lg_u8_to_s16(50)
            && haptic_waveform_eval(&d.waveform, 4 * LOOP, &f) == ff_lg_u8_to_s16(200));
    unsigned char rq[7] = { 0x11, FF_LG_FTYPE_RECTANGLE, 200, 50, 3, 5, 9 };
    decode(rq, 0, 1, &d);
    CHECK("rectangle: phase > T1 + T2 is ignored", haptic_waveform_eval(&d.waveform, 0, &f) == ff_lg_u8_to_s16(200));

    // A=255, T=4 loops, N=2 periods
    unsigned char sq[7] = { 0x11, FF_LG_FTYPE_SQUARE_WAVE, 255, 4, 0, 2, 0 };
    decode(sq, 0, 1, &d);
    CHECK("square: high state first", haptic_waveform_eval(&d.waveform, 0, &f) == SHRT_MAX);
    CHECK("square: low state", haptic_waveform_eval(&d.waveform, 4 * LOOP, &f) == -SHRT_MAX);
    CHECK("square: still playing during period 2", haptic_waveform_eval(&d.waveform, 8 * LOOP, &f) == SHRT_MAX && !f);
    haptic_waveform_eval(&d.waveform, 16 * LOOP, &f);
    CHECK("square: finished after N periods", f == 1);
    unsigned char sq16[7] = { 0x11, FF_LG_FTYPE_SQUARE_WAVE, 255, 0x00, 0x01, 1, 0 };
    decode(sq16, 0, 1, &d);
    CHECK("square: 16 bits half period, low byte first", d.waveform.half_period == 256);
}

static void test_ramp_variable(void) {
    // L1=200 L2=100 D=0 T=2 S=5: up from L2 to L1
    unsigned char r[7] = { 0x11, FF_LG_FTYPE_RAMP, 200, 100, 0, 0x25, 0 };
    s_haptic_core_data d;
    int f;
    decode(r, 0, 1, &d);
    CHECK("ramp: starts at L2 (D=0)", haptic_waveform_eval(&d.waveform, 0, &f) == ff_lg_u8_to_s16(100));
    CHECK("ramp: rises by S every T loops", haptic_waveform_eval(&d.waveform, 4 * LOOP, &f) == ff_lg_u8_to_s16(110));
    haptic_waveform_eval(&d.waveform, 100 * LOOP, &f);
    CHECK("ramp: stops at the target", f == 1);
    unsigned char rr[7] = { 0x11, FF_LG_FTYPE_RAMP, 200, 100, 1, 0x25, 0 };
    decode(rr, 0, 1, &d);
    CHECK("ramp: starts at L1 (D=1)", haptic_waveform_eval(&d.waveform, 0, &f) == ff_lg_u8_to_s16(200));

    // variable force 0: L1=130 T1=1 S1=2 D1=1 (decreasing)
    unsigned char v[7] = { 0x11, FF_LG_FTYPE_VARIABLE, 130, 128, 0x12, 0x00, 0x01 };
    decode(v, 0, 1, &d);
    CHECK("variable: decoded as waveform", d.type == E_DATA_TYPE_LOGITECH_RAW && d.waveform.kind == E_WAVEFORM_VARIABLE);
    CHECK("variable: decreases", haptic_waveform_eval(&d.waveform, 3 * LOOP, &f) == ff_lg_u8_to_s16(124));
    // variable force 2: L2=120 T2=2 S2=3 D2=0 (increasing); the byte layout differs from force 0
    unsigned char v2[7] = { 0x14, FF_LG_FTYPE_VARIABLE, 128, 120, 0x00, 0x23, 0x00 };
    decode(v2, 2, 1, &d);
    CHECK("variable: slot 2 uses L2/T2/S2/D2", d.waveform.kind == E_WAVEFORM_VARIABLE && d.waveform.l1 == 120
            && haptic_waveform_eval(&d.waveform, 4 * LOOP, &f) == ff_lg_u8_to_s16(126));
    unsigned char v1[7] = { 0x12, FF_LG_FTYPE_VARIABLE, 130, 128, 0x12, 0x00, 0x01 };
    CHECK("variable: slots 1 and 3 are not supported as waveforms", decode(v1, 1, 1, &d) && !d.waveform_valid);
    decode(v, 0, 1, &d);
    CHECK("variable: clipped at 0", haptic_waveform_eval(&d.waveform, 1000 * LOOP, &f) == ff_lg_u8_to_s16(0));
}

static void test_conditions(void) {
    s_haptic_core_data d;
    unsigned char ac[7] = { 0x11, FF_LG_FTYPE_AUTO_CENTER_SPRING, 4, 4, 0xFF, 0, 0 };
    CHECK("auto-center: converted to a spring", decode(ac, 0, 1, &d) && d.type == E_DATA_TYPE_SPRING && d.playing
            && d.spring.center == 0 && d.spring.coefficient.left > 0 && d.spring.coefficient.right > 0);
    unsigned char fr[7] = { 0x11, FF_LG_FTYPE_FRICTION, 128, 64, 255, 0x10, 0 };
    CHECK("friction: converted to a damper", decode(fr, 0, 1, &d) && d.type == E_DATA_TYPE_DAMPER && d.damper.coefficient.left > 0
            && d.damper.coefficient.right < 0);
    CHECK("friction: stop", decode(fr, 0, 0, &d) && d.type == E_DATA_TYPE_DAMPER && !d.playing);
}

static void test_synth(void) {
    s_haptic_synth synth;
    s_haptic_core_tweaks tweaks = { .invert = 0, .gain = { 100, 100, 100, 100 } };
    s_haptic_core_data in, out;
    uint64_t t = 1000000000ULL;

    haptic_synth_init(&synth);

    // a constant alone goes through unchanged
    unsigned char c[7] = { 0x11, FF_LG_FTYPE_CONSTANT, 200, 128, 128, 128, 0 };
    decode(c, 0, 1, &in);
    CHECK("synth: lone constant is not consumed", haptic_synth_feed(&synth, &in, t) == 0);
    CHECK("synth: nothing to send without waveform", haptic_synth_step(&synth, t, &tweaks, &out) == 0);

    // a square wave on slot 1 is consumed and summed with the constant
    unsigned char sq[7] = { 0x12, FF_LG_FTYPE_SQUARE_WAVE, 100, 5, 0, 0, 0 };
    decode(sq, 1, 1, &in);
    CHECK("synth: waveform consumed", haptic_synth_feed(&synth, &in, t) == 1);
    CHECK("synth: sends a constant", haptic_synth_step(&synth, t, &tweaks, &out) == 1 && out.type == E_DATA_TYPE_CONSTANT);
    int expected = ff_lg_u8_to_s16(200) + 100 * SHRT_MAX / UCHAR_MAX;
    CHECK("synth: constant + waveform", out.constant.level == expected);
    CHECK("synth: no resend if nothing changed", haptic_synth_step(&synth, t + LOOP, &tweaks, &out) == 0);
    CHECK("synth: level follows the waveform", haptic_synth_step(&synth, t + 5 * LOOP, &tweaks, &out) == 1
            && out.constant.level == ff_lg_u8_to_s16(200) - 100 * SHRT_MAX / UCHAR_MAX);

    // a constant update while a waveform plays is merged
    unsigned char c2[7] = { 0x11, FF_LG_FTYPE_CONSTANT, 128, 128, 128, 128, 0 };
    decode(c2, 0, 1, &in);
    CHECK("synth: constant consumed while waveform plays", haptic_synth_feed(&synth, &in, t + 6 * LOOP) == 1);

    // stop
    decode(sq, 1, 0, &in);
    CHECK("synth: stop consumed", haptic_synth_feed(&synth, &in, t + 7 * LOOP) == 1);
    CHECK("synth: final constant sent", haptic_synth_step(&synth, t + 7 * LOOP, &tweaks, &out) == 1 && out.constant.level == 0);
    CHECK("synth: back to idle", haptic_synth_step(&synth, t + 8 * LOOP, &tweaks, &out) == 0);

    // gain and invert tweaks apply to the waveform
    haptic_synth_init(&synth);
    tweaks.invert = 1;
    tweaks.gain.constant = 50;
    decode(sq, 1, 1, &in);
    haptic_synth_feed(&synth, &in, t);
    haptic_synth_step(&synth, t, &tweaks, &out);
    CHECK("synth: gain and invert applied", out.constant.level == -((100 * SHRT_MAX / UCHAR_MAX) * 50 / 100));
}


static int send_step(s_haptic_synth * synth, uint64_t now, const s_haptic_core_tweaks * tweaks, int * level) {
    s_haptic_core_data out;
    int ret = haptic_synth_step(synth, now, tweaks, &out);
    if (ret) {
        *level = out.constant.level;
    }
    return ret;
}

static void test_synth_mix(void) {
    s_haptic_synth synth;
    s_haptic_core_tweaks tweaks = { .invert = 0, .gain = { 100, 100, 100, 100 } };
    s_haptic_core_data in;
    uint64_t t = 5000000000ULL;
    int level = 0;

    // two square waves with different amplitudes and start times, and a constant force on slot 2
    unsigned char a[7] = { 0x11, FF_LG_FTYPE_SQUARE_WAVE, 255, 4, 0, 0, 0 };
    unsigned char b[7] = { 0x12, FF_LG_FTYPE_SQUARE_WAVE, 128, 4, 0, 0, 0 };
    unsigned char c[7] = { 0x14, FF_LG_FTYPE_CONSTANT, 128, 128, 255, 128, 0 };
    int b_amp = 128 * SHRT_MAX / UCHAR_MAX;

    haptic_synth_init(&synth);
    decode(a, 0, 1, &in);
    haptic_synth_feed(&synth, &in, t);
    CHECK("mix: first wave alone", send_step(&synth, t, &tweaks, &level) && level == SHRT_MAX);

    decode(b, 1, 1, &in);
    haptic_synth_feed(&synth, &in, t + 4 * LOOP);
    CHECK("mix: second wave starts later, own phase", send_step(&synth, t + 4 * LOOP, &tweaks, &level)
            && level == -SHRT_MAX + b_amp);
    CHECK("mix: both waves, next half period", send_step(&synth, t + 8 * LOOP, &tweaks, &level)
            && level == SHRT_MAX - b_amp);

    decode(c, 2, 1, &in);
    CHECK("mix: constant on slot 2 merged", haptic_synth_feed(&synth, &in, t + 9 * LOOP) == 1);
    CHECK("mix: positive saturation", send_step(&synth, t + 9 * LOOP, &tweaks, &level) && level == SHRT_MAX);
    // at t + 12 loops: wave A is low, wave B is high again, the constant is +SHRT_MAX
    CHECK("mix: two waves and a constant", send_step(&synth, t + 12 * LOOP, &tweaks, &level)
            && level == -SHRT_MAX + b_amp + SHRT_MAX);

    // negative saturation: a low wave plus a negative constant on slot 3
    haptic_synth_init(&synth);
    unsigned char n[7] = { 0x18, FF_LG_FTYPE_CONSTANT, 128, 128, 128, 0, 0 };
    decode(a, 0, 1, &in);
    haptic_synth_feed(&synth, &in, t);
    decode(n, 3, 1, &in);
    haptic_synth_feed(&synth, &in, t);
    CHECK("mix: negative saturation", send_step(&synth, t + 4 * LOOP, &tweaks, &level) && level == -SHRT_MAX);
}

static s_haptic_core_ids g27_ids = { USB_VENDOR_ID_LOGITECH, USB_PRODUCT_ID_LOGITECH_G27_WHEEL };

// feed the synthesizer like haptic_core_update() does, return the number of waveform data received
static int drain(const s_haptic_source * src, struct haptic_source_state * state, s_haptic_synth * synth, uint64_t now) {
    s_haptic_core_data data;
    int waveforms = 0;
    while (src->get(state, &data)) {
        if (data.waveform_valid) {
            ++waveforms;
        }
        haptic_synth_feed(synth, &data, now);
    }
    return waveforms;
}

static void test_lifecycle(void) {
    const s_haptic_source * src = haptic_source_get(g27_ids);
    CHECK("lifecycle: Logitech source registered", src != NULL && src->ended != NULL);
    if (src == NULL) {
        return;
    }
    struct haptic_source_state * state = src->init(g27_ids);
    s_haptic_synth synth;
    s_haptic_core_tweaks tweaks = { .invert = 0, .gain = { 100, 100, 100, 100 } };
    uint64_t t = 9000000000ULL;
    int level = 0;
    haptic_synth_init(&synth);

    // ramp on slot 1: L1=200, L2=100, D=0, T=1, S=15 -> ends after 7 loops
    unsigned char ramp[7] = { 0x11, FF_LG_FTYPE_RAMP, 200, 100, 0, 0x1F, 0x00 };
    drain(src, state, &synth, t); // initial extended commands
    src->process(state, sizeof(ramp), ramp);
    CHECK("lifecycle: ramp starts", drain(src, state, &synth, t) == 1 && send_step(&synth, t, &tweaks, &level));

    CHECK("lifecycle: ramp plays", send_step(&synth, t + 4 * LOOP, &tweaks, &level));
    send_step(&synth, t + 100 * LOOP, &tweaks, &level);
    unsigned int ended = haptic_synth_take_ended(&synth);
    CHECK("lifecycle: natural end reported for slot 0", ended == 1);
    CHECK("lifecycle: nothing reported twice", haptic_synth_take_ended(&synth) == 0);

    // without telling the source, the identical replay is ignored: this is why the ended hook exists
    src->process(state, sizeof(ramp), ramp);
    CHECK("lifecycle: replay ignored while the source thinks the slot plays", drain(src, state, &synth, t + 101 * LOOP) == 0);

    src->ended(state, 0);
    src->process(state, sizeof(ramp), ramp);
    CHECK("lifecycle: identical replay restarts after the end", drain(src, state, &synth, t + 102 * LOOP) == 1);
    CHECK("lifecycle: replayed ramp starts from its first level", send_step(&synth, t + 102 * LOOP, &tweaks, &level)
            && level == ff_lg_u8_to_s16(100));

    // PLAY alone also restarts a finished effect
    send_step(&synth, t + 300 * LOOP, &tweaks, &level);
    src->ended(state, haptic_synth_take_ended(&synth) == 1 ? 0 : 3);
    unsigned char play[7] = { 0x12, 0, 0, 0, 0, 0, 0 };
    src->process(state, sizeof(play), play);
    CHECK("lifecycle: PLAY restarts a finished effect", drain(src, state, &synth, t + 301 * LOOP) == 1);

    // a finite square wave (N = 1, T = 2 loops) ends then restarts with the same parameters
    haptic_synth_init(&synth);
    unsigned char sq[7] = { 0x21, FF_LG_FTYPE_SQUARE_WAVE, 255, 2, 0, 1, 0 };
    src->process(state, sizeof(sq), sq);
    CHECK("lifecycle: square starts", drain(src, state, &synth, t) >= 1);
    send_step(&synth, t, &tweaks, &level);
    send_step(&synth, t + 10 * LOOP, &tweaks, &level);
    CHECK("lifecycle: finite square ended", haptic_synth_take_ended(&synth) == 2);
    src->ended(state, 1);
    src->process(state, sizeof(sq), sq);
    CHECK("lifecycle: finite square can be replayed", drain(src, state, &synth, t + 20 * LOOP) == 1);

    src->clean(state);
}

int main(void) {

    test_sawtooth();
    test_trapezoid();
    test_rectangle_square();
    test_ramp_variable();
    test_conditions();
    test_synth();
    test_synth_mix();
    test_lifecycle();

    printf("%d failure(s)\n", failures);
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
