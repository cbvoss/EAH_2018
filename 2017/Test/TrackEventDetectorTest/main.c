#include <stdio.h>
#include <stdlib.h>

#include "./hall.h"
#include "./Ir_Board.h"
#include "../../src/global_clock.h"
#include "../../src/tachometer.h"
#include "../../src/distance_alarm.h"
#include "../../src/ted.h"

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

void checkAndDumpTedEvent()
{
    enum track_event event;
    char ev[2];
    float dist;
    event =  ted_get_track_event();

    if (event == NONE)
        return;

    ev[1] = 0;
    switch (event)
    {
        case SQUARE: ev[0] = 'Q'; break;
        case JUMP_LEFT: ev[0] = 'L'; break;
        case JUMP_RIGHT: ev[0] = 'R'; break;
        default: return;
    }

    dist = tachometer_get_distance_meter(BACK_LEFT);
    printf("%f\t%s\n", dist, ev);

    ted_reset_track_event();
}

void TedTest()
{
    int i, j;
    float dist, maxDist;
    waitClocks(1);
    hall_setHallTickLeft();
    hall_setHallTickRight();

    dist = 0;
    maxDist = ir_get_end_of_recorded_track();
    for (i=0; dist<=maxDist ; ++i)
    {
        for (j=0; j<80; ++j)
        {
            waitClocks(1);
            ted_update();
            checkAndDumpTedEvent();
        }

        hall_setHallTickLeft();
        hall_setHallTickRight();
        dist = tachometer_get_distance_meter(BACK_LEFT);
    }

}

int main()
{
    hall_initialize();
    ted_initialize();
    Ir_Initialize("savedPatterns.txt");
    tachometer_initialize();

    TedTest();

    return 0;
}
