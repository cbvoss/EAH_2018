/*
 * drive.h
 *
 *  Created on: 02.12.2016
 *      Author: Tobias Hupel
 *      		Eric Elsner
 *
 *      Reviewed on 05.01.2017 by:
 *      		Tobias Hupel
 *      		Eric Elsner
 *      		Adrian Zentgraf
 *
 *      Extension Reviewed on 26.01.2017 by
 *      		Eric Elsner
 *      		Tobias Hupel
 */

#ifndef DRIVE_H_
#define DRIVE_H_

#include "enums.h"
#include "regulator.h"
#include "engine.h"

struct RegulationTargets
{
    struct Engines
    {
        float abs_target_velocity_mps, break_threshold_mps;
        char use_regulator_break, target_direction_mode_changed, active_break;
        unsigned int active_break_pw;
        enum OperationMode target_direction_mode;
        struct Regulator regulator;
    } engines[2];
} typedef regulation_targets_t;
extern regulation_targets_t g_regulation_targets;

void drive_initialize();

char drive_is_differential_active();

float drive_get_differential_target_velocity_mps();

float drive_get_wheel_target_velocity_mps(enum Side side);

void drive_accelerate_to_differential_advanced(float velocity_mps, char use_break, float break_threshold);

void drive_accelerate_to_differential(float velocity_mps, char use_break);

void drive_accelerate_to(enum Side side, float velocity_mps, char use_break);

void drive_accelerate_to_advanced(enum Side side, float velocity_mps, char use_break, float break_threshold);

void drive_active_break(enum Side side, unsigned int pulse_width);

void drive_fixed_update();

#endif /* DRIVE_H_ */
