/*
 License: GPLv3
 */

/*
 * Software synthesis of time based effects for sinks that only have a constant force channel.
 *
 * The 4 Logitech force slots are tracked. While at least one time based effect is playing, the constant forces
 * and the waveforms of all the slots are summed into a single constant force. The time is provided by the caller.
 */

#ifndef HAPTIC_SYNTH_H_
#define HAPTIC_SYNTH_H_

#include <stdint.h>
#include "haptic_core.h"

#define HAPTIC_SYNTH_SLOTS 4

typedef struct {
    s_haptic_waveform wf[HAPTIC_SYNTH_SLOTS];
    uint8_t active[HAPTIC_SYNTH_SLOTS];
    uint64_t start_ns[HAPTIC_SYNTH_SLOTS];
    int16_t constant[HAPTIC_SYNTH_SLOTS];
    uint8_t constant_playing[HAPTIC_SYNTH_SLOTS];
    uint8_t on;
    uint8_t dirty;
    uint8_t ended_mask; // slots whose effect ended by itself since the last call to haptic_synth_take_ended()
    int32_t last_level;
} s_haptic_synth;

void haptic_synth_init(s_haptic_synth * synth);

/*
 * Offer a decoded data to the synthesizer.
 * Returns 1 if the data was consumed (the caller must not send it to the sink), 0 otherwise.
 */
int haptic_synth_feed(s_haptic_synth * synth, const s_haptic_core_data * data, uint64_t now_ns);

/*
 * Compute the constant force to send to the sink.
 * Returns 1 if out has to be sent to the sink, 0 if nothing changed.
 */
int haptic_synth_step(s_haptic_synth * synth, uint64_t now_ns, const s_haptic_core_tweaks * tweaks, s_haptic_core_data * out);

/*
 * Get (and clear) the mask of the slots whose effect ended by itself (ramp target reached, square periods played).
 * The source has to be told, otherwise it would still consider these slots as playing and ignore an identical replay.
 */
unsigned int haptic_synth_take_ended(s_haptic_synth * synth);

#endif /* HAPTIC_SYNTH_H_ */
