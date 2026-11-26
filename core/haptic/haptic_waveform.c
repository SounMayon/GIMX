/*
 License: GPLv3
 */

#include <stddef.h>
#include <limits.h>
#include <haptic/haptic_waveform.h>

// same conversion as ff_lg_u8_to_s16 (127 and 128 are the center positions)
static int16_t level_to_s16(int level) {
    int c = level;
    if (c < 0) {
        c = 0;
    } else if (c > UCHAR_MAX) {
        c = UCHAR_MAX;
    }
    if (c < 128) {
        ++c;
    }
    return (int16_t)((c - 128) * SHRT_MAX / 127);
}

static uint64_t max1(uint64_t v) {
    return v ? v : 1;
}

static int16_t eval_sawtooth(const s_haptic_waveform * wf, uint64_t loops, int up) {

    int l1 = wf->l1, l2 = wf->l2, l0 = wf->l0;
    if (l1 < l2) {
        int tmp = l1; l1 = l2; l2 = tmp;
    }
    if (l0 > l1) {
        l0 = l1;
    } else if (l0 < l2) {
        l0 = l2;
    }
    uint64_t span = (uint64_t)(l1 - l2) + 1;
    uint64_t steps = loops / max1(wf->t3);
    uint64_t advance = (steps * wf->step) % span;
    int level;
    if (up) {
        level = l2 + (int)(((uint64_t)(l0 - l2) + advance) % span);
    } else {
        level = l1 - (int)(((uint64_t)(l1 - l0) + advance) % span);
    }
    return level_to_s16(level);
}

static int16_t eval_trapezoid(const s_haptic_waveform * wf, uint64_t loops) {

    int l1 = wf->l1, l2 = wf->l2;
    int amplitude = l1 > l2 ? l1 - l2 : l2 - l1;
    uint64_t ramp = 0;
    if (wf->step) {
        // number of steps to go from one level to the other
        ramp = ((uint64_t)amplitude + wf->step - 1) / wf->step;
        ramp *= max1(wf->t3);
    }
    uint64_t cycle = (uint64_t)wf->t1 + ramp + wf->t2 + ramp;
    if (cycle == 0) {
        return level_to_s16(l1);
    }
    uint64_t p = loops % cycle;
    if (p < wf->t1) {
        return level_to_s16(l1);
    }
    p -= wf->t1;
    if (p < ramp) {
        int moved = (int)((p / max1(wf->t3)) * wf->step);
        if (moved > amplitude) {
            moved = amplitude;
        }
        return level_to_s16(l1 > l2 ? l1 - moved : l1 + moved);
    }
    p -= ramp;
    if (p < wf->t2) {
        return level_to_s16(l2);
    }
    p -= wf->t2;
    int moved = (int)((p / max1(wf->t3)) * wf->step);
    if (moved > amplitude) {
        moved = amplitude;
    }
    return level_to_s16(l1 > l2 ? l2 + moved : l2 - moved);
}

static int16_t eval_rectangle(const s_haptic_waveform *wf, uint64_t loops)
{
    uint32_t period = (uint32_t)wf->t1 + (uint32_t)wf->t2;
    if (period == 0) {
        return level_to_s16(wf->l1);
    }

    uint64_t offset = 0;
    if (wf->phase <= period) {
        offset = (uint64_t)wf->phase * 2ULL;
        if (offset >= period) {
            offset = wf->t1;
        }
    }

    uint32_t p = (uint32_t)((loops + offset) % period);
    return level_to_s16(p < wf->t1 ? wf->l1 : wf->l2);
}

static int16_t eval_square(const s_haptic_waveform * wf, uint64_t loops, int * finished) {

    uint64_t half = max1(wf->half_period);
    uint64_t periods = wf->periods ? wf->periods : 256;
    if (loops / (2 * half) >= periods) {
        *finished = 1;
        return 0;
    }
    int amplitude = (int)wf->amplitude * SHRT_MAX / UCHAR_MAX;
    // hypothesis to confirm on hardware: the high state comes first
    return ((loops % (2 * half)) < half) ? amplitude : -amplitude;
}

static int16_t eval_ramp(const s_haptic_waveform * wf, uint64_t loops, int * finished) {

    int l1 = wf->l1, l2 = wf->l2;
    int amplitude = l1 > l2 ? l1 - l2 : l2 - l1;
    int hi = l1 > l2 ? l1 : l2;
    int lo = l1 > l2 ? l2 : l1;
    uint64_t moved = (loops / max1(wf->t3)) * wf->step;
    if (wf->step == 0 || moved >= (uint64_t)amplitude) {
        // the force stops once the target level is reached
        if (wf->step != 0) {
            *finished = 1;
            return 0;
        }
        return level_to_s16(wf->direction ? hi : lo);
    }
    // D = 0: ramp up from L2 to L1, D = 1: reverse
    return level_to_s16(wf->direction ? hi - (int)moved : lo + (int)moved);
}

static int16_t eval_variable(const s_haptic_waveform * wf, uint64_t loops) {

    // the level is clipped to [0, 255] and remains there until the force is updated
    int64_t moved = (int64_t)(loops / max1(wf->t3)) * wf->step;
    int64_t level = wf->l1 + (wf->direction ? -moved : moved);
    if (level < 0) {
        level = 0;
    } else if (level > UCHAR_MAX) {
        level = UCHAR_MAX;
    }
    return level_to_s16((int)level);
}

int16_t haptic_waveform_eval(const s_haptic_waveform * wf, uint64_t elapsed_ns, int * finished) {

    int dummy = 0;
    if (finished == NULL) {
        finished = &dummy;
    }
    *finished = 0;

    uint64_t loops = elapsed_ns / HAPTIC_WAVEFORM_LOOP_NS;

    switch (wf->kind) {
    case E_WAVEFORM_SAWTOOTH_UP:
        return eval_sawtooth(wf, loops, 1);
    case E_WAVEFORM_SAWTOOTH_DOWN:
        return eval_sawtooth(wf, loops, 0);
    case E_WAVEFORM_TRAPEZOID:
        return eval_trapezoid(wf, loops);
    case E_WAVEFORM_RECTANGLE:
        return eval_rectangle(wf, loops);
    case E_WAVEFORM_SQUARE:
        return eval_square(wf, loops, finished);
    case E_WAVEFORM_RAMP:
        return eval_ramp(wf, loops, finished);
    case E_WAVEFORM_VARIABLE:
        return eval_variable(wf, loops);
    case E_WAVEFORM_NONE:
    default:
        break;
    }
    return 0;
}
