/*
 Copyright (c) 2026 GIMX Reborn-ffb
 License: GPLv3
 */

#include <g29_pc.h>
#include <report.h>
#include <string.h>
#include "../include/controller.h"

#define G29PC_CROSS_MASK      0x10
#define G29PC_SQUARE_MASK     0x20
#define G29PC_CIRCLE_MASK     0x40
#define G29PC_TRIANGLE_MASK   0x80

#define G29PC_R1_MASK         0x0001
#define G29PC_L1_MASK         0x0002
#define G29PC_R2_MASK         0x0004
#define G29PC_L2_MASK         0x0008
#define G29PC_SHARE_MASK      0x0010
#define G29PC_OPTIONS_MASK    0x0020
#define G29PC_R3_MASK         0x0040
#define G29PC_L3_MASK         0x0080
#define G29PC_SHIFTER_1_MASK  0x0100
#define G29PC_SHIFTER_2_MASK  0x0200
#define G29PC_SHIFTER_3_MASK  0x0400
#define G29PC_SHIFTER_4_MASK  0x0800
#define G29PC_SHIFTER_5_MASK  0x1000
#define G29PC_SHIFTER_6_MASK  0x2000
#define G29PC_SHIFTER_R_MASK  0x4000
#define G29PC_PLUS_MASK       0x8000

#define G29PC_MINUS_MASK      0x01
#define G29PC_DIAL_DOWN_MASK  0x02
#define G29PC_DIAL_UP_MASK    0x04
#define G29PC_ENTER_MASK      0x08
#define G29PC_PS_MASK         0x10

static s_axis axes[AXIS_MAX] =
{
  [g29Pca_wheel]         = { .name = "wheel",          .max_unsigned_value = MAX_AXIS_VALUE_16BITS },

  [g29Pca_gasPedal]      = { .name = "gas",            .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_brakePedal]    = { .name = "brake",          .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_clutchPedal]   = { .name = "clutch",         .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_share]         = { .name = "share",          .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_options]       = { .name = "options",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_up]            = { .name = "up",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_right]         = { .name = "right",          .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_down]          = { .name = "down",           .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_left]          = { .name = "left",           .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_triangle]      = { .name = "triangle",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_circle]        = { .name = "circle",         .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_cross]         = { .name = "cross",          .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_square]        = { .name = "square",         .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_l1]            = { .name = "l1",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_r1]            = { .name = "r1",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_l2]            = { .name = "l2",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_r2]            = { .name = "r2",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_l3]            = { .name = "l3",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_r3]            = { .name = "r3",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_ps]            = { .name = "PS",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_gearShifter1]  = { .name = "gear shifter 1", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_gearShifter2]  = { .name = "gear shifter 2", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_gearShifter3]  = { .name = "gear shifter 3", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_gearShifter4]  = { .name = "gear shifter 4", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_gearShifter5]  = { .name = "gear shifter 5", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_gearShifter6]  = { .name = "gear shifter 6", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_gearShifterR]  = { .name = "gear shifter R", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_plus]          = { .name = "plus",           .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_minus]         = { .name = "minus",          .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_dialUp]        = { .name = "dial up",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_dialDown]      = { .name = "dial down",      .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Pca_enter]         = { .name = "enter",          .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
};

static s_axis_name_dir axis_name_dirs[] =
{
  {.name = "wheel",        {.axis = g29Pca_wheel,      .props = AXIS_PROP_CENTERED}},
  {.name = "wheel left",   {.axis = g29Pca_wheel,      .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "wheel right",  {.axis = g29Pca_wheel,      .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

  {.name = "gas",          {.axis = g29Pca_gasPedal,   .props = AXIS_PROP_POSITIVE}},
  {.name = "brake",        {.axis = g29Pca_brakePedal, .props = AXIS_PROP_POSITIVE}},
  {.name = "clutch",       {.axis = g29Pca_clutchPedal,.props = AXIS_PROP_POSITIVE}},

  {.name = "r2",           {.axis = g29Pca_r2,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l2",           {.axis = g29Pca_l2,         .props = AXIS_PROP_TOGGLE}},

  {.name = "share",        {.axis = g29Pca_share,      .props = AXIS_PROP_TOGGLE}},
  {.name = "options",      {.axis = g29Pca_options,    .props = AXIS_PROP_TOGGLE}},
  {.name = "PS",           {.axis = g29Pca_ps,         .props = AXIS_PROP_TOGGLE}},
  {.name = "up",           {.axis = g29Pca_up,         .props = AXIS_PROP_TOGGLE}},
  {.name = "down",         {.axis = g29Pca_down,       .props = AXIS_PROP_TOGGLE}},
  {.name = "right",        {.axis = g29Pca_right,      .props = AXIS_PROP_TOGGLE}},
  {.name = "left",         {.axis = g29Pca_left,       .props = AXIS_PROP_TOGGLE}},
  {.name = "r1",           {.axis = g29Pca_r1,         .props = AXIS_PROP_TOGGLE}},
  {.name = "r3",           {.axis = g29Pca_r3,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l1",           {.axis = g29Pca_l1,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l3",           {.axis = g29Pca_l3,         .props = AXIS_PROP_TOGGLE}},
  {.name = "circle",       {.axis = g29Pca_circle,     .props = AXIS_PROP_TOGGLE}},
  {.name = "square",       {.axis = g29Pca_square,     .props = AXIS_PROP_TOGGLE}},
  {.name = "cross",        {.axis = g29Pca_cross,      .props = AXIS_PROP_TOGGLE}},
  {.name = "triangle",     {.axis = g29Pca_triangle,   .props = AXIS_PROP_TOGGLE}},

  {.name = "gear shifter 1", {.axis = g29Pca_gearShifter1, .props = AXIS_PROP_TOGGLE}},
  {.name = "gear shifter 2", {.axis = g29Pca_gearShifter2, .props = AXIS_PROP_TOGGLE}},
  {.name = "gear shifter 3", {.axis = g29Pca_gearShifter3, .props = AXIS_PROP_TOGGLE}},
  {.name = "gear shifter 4", {.axis = g29Pca_gearShifter4, .props = AXIS_PROP_TOGGLE}},
  {.name = "gear shifter 5", {.axis = g29Pca_gearShifter5, .props = AXIS_PROP_TOGGLE}},
  {.name = "gear shifter 6", {.axis = g29Pca_gearShifter6, .props = AXIS_PROP_TOGGLE}},
  {.name = "gear shifter R", {.axis = g29Pca_gearShifterR, .props = AXIS_PROP_TOGGLE}},

  {.name = "plus",         {.axis = g29Pca_plus,       .props = AXIS_PROP_TOGGLE}},
  {.name = "minus",        {.axis = g29Pca_minus,      .props = AXIS_PROP_TOGGLE}},
  {.name = "dial up",      {.axis = g29Pca_dialUp,     .props = AXIS_PROP_TOGGLE}},
  {.name = "dial down",    {.axis = g29Pca_dialDown,   .props = AXIS_PROP_TOGGLE}},
  {.name = "enter",        {.axis = g29Pca_enter,      .props = AXIS_PROP_TOGGLE}},
};

static s_report_g29Pc default_report =
{
  .wheel = CENTER_AXIS_VALUE_16BITS,
  .gasPedal = MAX_AXIS_VALUE_8BITS,
  .brakePedal = MAX_AXIS_VALUE_8BITS,
  .clutchPedal = MAX_AXIS_VALUE_8BITS,
  .hatAndButtons = 0x08,
  .buttons = 0x0000,
  .buttons2 = 0x00,
  .vendor = { 0x80, 0x80, 0x9c },
};

static void init_report(s_report * report)
{
  memcpy(report, &default_report, sizeof(default_report));
}

static unsigned int build_report(int axis[AXIS_MAX], s_report_packet report[MAX_REPORTS])
{
  unsigned int index = 0;
  report[index].length = sizeof(s_report_g29Pc);
  s_report_g29Pc* g29Pc = &report[index].value.g29Pc;

  g29Pc->wheel = clamp(0, axis[g29Pca_wheel] + CENTER_AXIS_VALUE_16BITS, MAX_AXIS_VALUE_16BITS);

  g29Pc->gasPedal = clamp(0, MAX_AXIS_VALUE_8BITS - axis[g29Pca_gasPedal], MAX_AXIS_VALUE_8BITS);
  g29Pc->brakePedal = clamp(0, MAX_AXIS_VALUE_8BITS - axis[g29Pca_brakePedal], MAX_AXIS_VALUE_8BITS);
  g29Pc->clutchPedal = clamp(0, MAX_AXIS_VALUE_8BITS - axis[g29Pca_clutchPedal], MAX_AXIS_VALUE_8BITS);

  if (axis[g29Pca_right])
  {
    if (axis[g29Pca_down])
      g29Pc->hatAndButtons = 0x03;
    else if (axis[g29Pca_up])
      g29Pc->hatAndButtons = 0x01;
    else
      g29Pc->hatAndButtons = 0x02;
  }
  else if (axis[g29Pca_left])
  {
    if (axis[g29Pca_down])
      g29Pc->hatAndButtons = 0x05;
    else if (axis[g29Pca_up])
      g29Pc->hatAndButtons = 0x07;
    else
      g29Pc->hatAndButtons = 0x06;
  }
  else if (axis[g29Pca_down])
    g29Pc->hatAndButtons = 0x04;
  else if (axis[g29Pca_up])
    g29Pc->hatAndButtons = 0x00;
  else
    g29Pc->hatAndButtons = 0x08;

  if (axis[g29Pca_square])   g29Pc->hatAndButtons |= G29PC_SQUARE_MASK;
  if (axis[g29Pca_cross])    g29Pc->hatAndButtons |= G29PC_CROSS_MASK;
  if (axis[g29Pca_circle])   g29Pc->hatAndButtons |= G29PC_CIRCLE_MASK;
  if (axis[g29Pca_triangle]) g29Pc->hatAndButtons |= G29PC_TRIANGLE_MASK;

  g29Pc->buttons = 0x0000;
  if (axis[g29Pca_r1])           g29Pc->buttons |= G29PC_R1_MASK;
  if (axis[g29Pca_l1])           g29Pc->buttons |= G29PC_L1_MASK;
  if (axis[g29Pca_r2])           g29Pc->buttons |= G29PC_R2_MASK;
  if (axis[g29Pca_l2])           g29Pc->buttons |= G29PC_L2_MASK;
  if (axis[g29Pca_share])        g29Pc->buttons |= G29PC_SHARE_MASK;
  if (axis[g29Pca_options])      g29Pc->buttons |= G29PC_OPTIONS_MASK;
  if (axis[g29Pca_r3])           g29Pc->buttons |= G29PC_R3_MASK;
  if (axis[g29Pca_l3])           g29Pc->buttons |= G29PC_L3_MASK;
  if (axis[g29Pca_gearShifter1]) g29Pc->buttons |= G29PC_SHIFTER_1_MASK;
  if (axis[g29Pca_gearShifter2]) g29Pc->buttons |= G29PC_SHIFTER_2_MASK;
  if (axis[g29Pca_gearShifter3]) g29Pc->buttons |= G29PC_SHIFTER_3_MASK;
  if (axis[g29Pca_gearShifter4]) g29Pc->buttons |= G29PC_SHIFTER_4_MASK;
  if (axis[g29Pca_gearShifter5]) g29Pc->buttons |= G29PC_SHIFTER_5_MASK;
  if (axis[g29Pca_gearShifter6]) g29Pc->buttons |= G29PC_SHIFTER_6_MASK;
  if (axis[g29Pca_gearShifterR]) g29Pc->buttons |= G29PC_SHIFTER_R_MASK;
  if (axis[g29Pca_plus])         g29Pc->buttons |= G29PC_PLUS_MASK;

  g29Pc->buttons2 = 0x00;
  if (axis[g29Pca_minus])    g29Pc->buttons2 |= G29PC_MINUS_MASK;
  if (axis[g29Pca_dialDown]) g29Pc->buttons2 |= G29PC_DIAL_DOWN_MASK;
  if (axis[g29Pca_dialUp])   g29Pc->buttons2 |= G29PC_DIAL_UP_MASK;
  if (axis[g29Pca_enter])    g29Pc->buttons2 |= G29PC_ENTER_MASK;
  if (axis[g29Pca_ps])       g29Pc->buttons2 |= G29PC_PS_MASK;

  g29Pc->vendor[0] = 0x80;
  g29Pc->vendor[1] = 0x80;
  g29Pc->vendor[2] = 0x9c;

  return index;
}

static s_controller controller =
{
  .name = "G29 PC",
  .vid = 0x046d,
  .pid = 0xc24f,
  .refresh_period = { .min_value = 1000, .default_value = 5000 },
  .auth_required = 0,
  .axes = axes,
  .axis_name_dirs = { .nb = sizeof(axis_name_dirs)/sizeof(*axis_name_dirs), .values = axis_name_dirs },
  .fp_build_report = build_report,
  .fp_init_report = init_report,
};

void g29Pc_init(void) __attribute__((constructor));
void g29Pc_init(void)
{
  controller_register(C_TYPE_G29_PC, &controller);
}
