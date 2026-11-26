/*
 License: GPLv3
 */

#include <string.h>
#include <limits.h>
#include <haptic/haptic_synth.h>
#include <haptic/haptic_tweaks.h>

void haptic_synth_init(s_haptic_synth * synth) {

    memset(synth, 0x00, sizeof(*synth));
}

static int any_active(const s_haptic_synth * synth) {
    int i;
    for (i = 0; i < HAPTIC_SYNTH_SLOTS; ++i) {
        if (synth->active[i]) {
            return 1;
        }
    }
    return 0;
}

int haptic_synth_feed(s_haptic_synth * synth, const s_haptic_core_data * data, uint64_t now_ns) {

    if (!data->source_slot_valid || data->source_slot >= HAPTIC_SYNTH_SLOTS) {
        return 0;
    }

    unsigned int slot = data->source_slot;

    if (data->waveform_valid) {
        if (data->playing) {
            // a new start, or new parameters: restart the effect
            if (!synth->active[slot] || memcmp(&synth->wf[slot], &data->waveform, sizeof(data->waveform))) {
                synth->wf[slot] = data->waveform;
                synth->start_ns[slot] = now_ns;
            }
            synth->active[slot] = 1;
        } else {
            synth->active[slot] = 0;
        }
        synth->dirty = 1;
        return 1;
    }

    if (data->type == E_DATA_TYPE_CONSTANT) {
        synth->constant[slot] = data->playing ? data->constant.level : 0;
        synth->constant_playing[slot] = data->playing;
        if (synth->on || any_active(synth)) {
            synth->dirty = 1;
            return 1;
        }
        return 0; // no time based effect: let it go through unchanged
    }

    return 0;
}

int haptic_synth_step(s_haptic_synth * synth, uint64_t now_ns, const s_haptic_core_tweaks * tweaks, s_haptic_core_data * out) {

    unsigned int i;
    int32_t sum = 0;

    for (i = 0; i < HAPTIC_SYNTH_SLOTS; ++i) {
        if (synth->active[i]) {
            int finished = 0;
            uint64_t elapsed = now_ns > synth->start_ns[i] ? now_ns - synth->start_ns[i] : 0;
            int16_t level = haptic_waveform_eval(synth->wf + i, elapsed, &finished);
            if (finished) {
                synth->active[i] = 0;
                synth->ended_mask |= (1U << i);
                synth->dirty = 1;
                continue;
            }
            // apply the constant force tweaks (gain, invert) to the waveform
            s_haptic_core_data tmp;
            memset(&tmp, 0x00, sizeof(tmp));
            tmp.type = E_DATA_TYPE_CONSTANT;
            tmp.playing = 1;
            tmp.constant.level = level;
            haptic_tweak_apply(tweaks, &tmp);
            sum += tmp.constant.level;
        }
    }

    int now_on = any_active(synth);

    if (!now_on && !synth->on) {
        synth->dirty = 0;
        return 0;
    }

    for (i = 0; i < HAPTIC_SYNTH_SLOTS; ++i) {
        if (synth->constant_playing[i]) {
            sum += synth->constant[i];
        }
    }
    if (sum > SHRT_MAX) {
        sum = SHRT_MAX;
    } else if (sum < -SHRT_MAX) {
        sum = -SHRT_MAX;
    }

    int changed = (sum != synth->last_level) || (now_on != synth->on) || synth->dirty;
    synth->on = now_on;
    synth->dirty = 0;

    if (!changed) {
        return 0;
    }
    synth->last_level = sum;

    memset(out, 0x00, sizeof(*out));
    out->type = E_DATA_TYPE_CONSTANT;
    out->playing = 1;
    out->constant.level = (int16_t)sum;
    return 1;
}

unsigned int haptic_synth_take_ended(s_haptic_synth * synth) {

    unsigned int mask = synth->ended_mask;
    synth->ended_mask = 0;
    return mask;
}
