/*
 Copyright (c) 2026 GIMX Reborn-ffb
 License: GPLv3
 */

#ifndef G29_PC_H_
#define G29_PC_H_

#include "defs.h"

typedef enum
{
  g29Pca_wheel = rel_axis_lstick_x,
  g29Pca_share = abs_axis_0,
  g29Pca_options = abs_axis_1,
  g29Pca_ps = abs_axis_2,
  g29Pca_up = abs_axis_3,
  g29Pca_right = abs_axis_4,
  g29Pca_down = abs_axis_5,
  g29Pca_left = abs_axis_6,
  g29Pca_triangle = abs_axis_7,
  g29Pca_circle = abs_axis_8,
  g29Pca_cross = abs_axis_9,
  g29Pca_square = abs_axis_10,
  g29Pca_l1 = abs_axis_11,
  g29Pca_r1 = abs_axis_12,
  g29Pca_l2 = abs_axis_13,
  g29Pca_r2 = abs_axis_14,
  g29Pca_l3 = abs_axis_15,
  g29Pca_r3 = abs_axis_16,
  g29Pca_gasPedal = abs_axis_17,
  g29Pca_brakePedal = abs_axis_18,
  g29Pca_clutchPedal = abs_axis_19,
  g29Pca_gearShifter1 = abs_axis_20,
  g29Pca_gearShifter2 = abs_axis_21,
  g29Pca_gearShifter3 = abs_axis_22,
  g29Pca_gearShifter4 = abs_axis_23,
  g29Pca_gearShifter5 = abs_axis_24,
  g29Pca_gearShifter6 = abs_axis_25,
  g29Pca_gearShifterR = abs_axis_26,
  g29Pca_plus = abs_axis_27,
  g29Pca_minus = abs_axis_28,
  g29Pca_dialUp = abs_axis_29,
  g29Pca_dialDown = abs_axis_30,
  g29Pca_enter = abs_axis_31,
} e_g29Pc_axis_index;

typedef struct GIMX_PACKED
{
  unsigned short wheel;       // 16-bit unsigned, center = 0x8000
  unsigned char gasPedal;     // 8-bit unsigned, released = 0xFF
  unsigned char brakePedal;   // 8-bit unsigned, released = 0xFF
  unsigned char clutchPedal;  // 8-bit unsigned, released = 0xFF
  unsigned char hatAndButtons;// 4 LSB = hat (0..7, 8=null), 4 MSB = buttons 1..4
  unsigned short buttons;     // buttons 5..20
  unsigned char buttons2;     // 5 LSB = buttons 21..25, 3 MSB = vendor padding
  unsigned char vendor[3];    // 3 bytes vendor defined (0xFF00)
} s_report_g29Pc;

#endif /* G29_PC_H_ */
