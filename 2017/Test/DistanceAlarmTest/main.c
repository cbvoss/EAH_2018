#include <stdio.h>
#include <stdlib.h>
#include "./hall.h"
#include "../../src/global_clock.h"
#include "../../src/tachometer.h"
#include "../../src/distance_alarm.h"

void waitClocks(int clockCycles)
{
    int i, clock;
    for (i=0; i<clockCycles; ++i)
    {
        global_clock_fixed_update();
        clock = global_clock_get_100us_count();

        if (clock % 4 > 0)  // one of ... tachometer updates will be missed
            tachometer_update();
    }
}

void dumpTimeDistSpeedLeft()
{
    char buffer[128];
    int time;
    float dist, distExtrapolated, speed;

    time = global_clock_get_100us_count();
    dist = tachometer_get_distance_meter(BACK_LEFT);
    distExtrapolated = tachometer_get_extrapolated_distance_meter(BACK_LEFT);
    speed = tachometer_get_velocity_mps(BACK_LEFT);

    snprintf(buffer, 128, "t=%i\ts=%f\ts'=%f\tv=%f\n", time, dist, distExtrapolated, speed);
    printf(buffer);
}

void checkAlarmAndDump(int cycles, struct DistanceAlarm* alarmDist)
{
    int i;
    for (i=0; i<cycles; ++i)
    {
        waitClocks(1);
        if (distance_alarm_has_distance_reached(alarmDist))
        {
            dumpTimeDistSpeedLeft();
            distance_alarm_reset(alarmDist);
        }
    }
}

void testDistanceAlarm()
{
    struct DistanceAlarm alarmDist;
    int deltaCycle;

    waitClocks(1);
    hall_setHallTickLeft();
    hall_setHallTickRight();
    dumpTimeDistSpeedLeft();

    waitClocks(80);
    hall_setHallTickLeft();
    dumpTimeDistSpeedLeft();
    waitClocks(80);
    hall_setHallTickLeft();
    dumpTimeDistSpeedLeft();

    printf("start distance alarm\n");
    distance_alarm_new_specific(&alarmDist, 0.001, 1, BACK_LEFT);

    deltaCycle = 83;
    checkAlarmAndDump(deltaCycle, &alarmDist);
    hall_setHallTickLeft();
    deltaCycle = 83;
    checkAlarmAndDump(deltaCycle, &alarmDist);
    hall_setHallTickLeft();
    deltaCycle = 79;
    checkAlarmAndDump(deltaCycle, &alarmDist);
    hall_setHallTickLeft();
    deltaCycle = 78;
    checkAlarmAndDump(deltaCycle, &alarmDist);
    hall_setHallTickLeft();
    deltaCycle = 84;
    checkAlarmAndDump(deltaCycle, &alarmDist);
    hall_setHallTickLeft();
    deltaCycle = 84;
    checkAlarmAndDump(deltaCycle, &alarmDist);
    hall_setHallTickLeft();
    deltaCycle = 84;
    checkAlarmAndDump(deltaCycle, &alarmDist);
    hall_setHallTickLeft();
    deltaCycle = 84;
    checkAlarmAndDump(deltaCycle, &alarmDist);
    hall_setHallTickLeft();
}

int main()
{
    tachometer_initialize();
    hall_initialize();

    testDistanceAlarm();

    return 0;
}
