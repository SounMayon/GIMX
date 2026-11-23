/*
 License: GPLv3
 */

/*
 * Time based Logitech "classic" effects (sawtooth, trapezoid, rectangle, square, ramp, variable).
 *
 * The evaluator is a pure function of the elapsed time since the effect was started.
 * It has no dependency on the OS, on the sink or on the clock: the caller provides the elapsed time.
 * This makes the result independent from the polling jitter and easy to unit test.
 *
 * Note: the waveform is sampled when the haptic core runs, i.e. once per controller refresh period (5 ms by default
 * for the PS4 G29) and after each FFB report. Waveforms with a period shorter than two refresh periods are aliased.
 *
 * Reference: Logitech Force Feedback Protocol V1.6, chapter 3.2 (doc/Logitech_Force_Feedback_Protocol_V1.6.pdf).
 */

#ifndef HAPTIC_WAVEFORM_H_
#define HAPTIC_WAVEFORM_H_

#include <stdint.h>

/*
 * Duration of a Logitech "main loop", in nanoseconds.
 * The specification gives 2 ms when the fixed time loop mode is on (CMD 0x0d).
 * The duration is unspecified otherwise: 2 ms is assumed in both cases.
 */
#define HAPTIC_WAVEFORM_LOOP_NS 2000000ULL

typedef enum {
    E_WAVEFORM_NONE,
    E_WAVEFORM_SAWTOOTH_UP,
    E_WAVEFORM_SAWTOOTH_DOWN,
    E_WAVEFORM_TRAPEZOID,
    E_WAVEFORM_RECTANGLE,
    E_WAVEFORM_SQUARE,
    E_WAVEFORM_RAMP,
    E_WAVEFORM_VARIABLE,
} e_haptic_waveform_kind;

/*
 * Raw Logitech parameters. Levels are unsigned 8-bit values, 128 is the zero force.
 */
typedef struct {
    e_haptic_waveform_kind kind;
    uint8_t l0;        // initial level (sawtooth)
    uint8_t l1;        // max level, or start level (variable)
    uint8_t l2;        // min level
    uint8_t t1;        // trapezoid/rectangle: duration of L1 (loops)
    uint8_t t2;        // trapezoid/rectangle: duration of L2 (loops)
    uint8_t t3;        // loops per step (sawtooth, trapezoid, ramp, variable)
    uint8_t step;      // step size (sawtooth INC, trapezoid S, ramp S, variable S)
    uint8_t phase;     // rectangle phase P
    uint8_t direction; // ramp D, variable D (1 = decreasing)
    uint8_t amplitude; // square A
    uint16_t half_period; // square T (loops per state)
    uint8_t periods;   // square N (0 means 256)
} s_haptic_waveform;

/*
 * Evaluate the waveform.
 *
 * elapsed_ns: time since the effect started playing.
 * finished: set to 1 if the effect ended by itself (ramp target reached, square periods played), 0 otherwise.
 *
 * Returns the force level in [-32767, 32767]. 0 is returned when the effect is finished.
 */
int16_t haptic_waveform_eval(const s_haptic_waveform * wf, uint64_t elapsed_ns, int * finished);

#endif /* HAPTIC_WAVEFORM_H_ */
