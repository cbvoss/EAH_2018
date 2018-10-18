#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../../src/event_map.h"

void dumpMap()
{
    char buffer[1024];
    event_map_get_map_string(buffer, 1024);
    printf(buffer);
}

void dumpState(float pos)
{
    float v;
    enum SyncStates state;
    char syncChar;
    char buffer[64];
    v = event_map_get_velocity_recomendation(pos);
    state = event_map_get_sync_State();
    switch (state)
    {
        case SYNC: syncChar = 'S'; break;
        case PARTIALLY_LOST: syncChar = 'P'; break;
        case FULLY_LOST: syncChar = 'L'; break;
    }

    snprintf(buffer, 64, "s=%0.3f v=%0.3f syn=%c\n", pos, v, syncChar);
    printf(buffer);
}

void sendTrackEventIfSuitable(float posCurr, float posEvent, enum track_event event, float resolution)
{
    char buffer[32];
    if ((posCurr<posEvent) || (posCurr-posEvent>resolution))
        return;

    snprintf(buffer, 32, "event %d at %2.2f\n", event, posEvent);
    printf(buffer);

    event_map_update(posEvent, event);
}

void TestSyncCase()
{
    float pos;
    float resolution = 0.1;
    event_map_Initialize();
    dumpMap();

    for (pos=0; pos<78; pos+=resolution)
    {
        sendTrackEventIfSuitable(pos,  0.404, JUMP_RIGHT, resolution);
        sendTrackEventIfSuitable(pos,  2.609, JUMP_LEFT, resolution);
        sendTrackEventIfSuitable(pos, 18.051, SQUARE, resolution);
        sendTrackEventIfSuitable(pos, 36.336, SQUARE, resolution);
        sendTrackEventIfSuitable(pos, 50.207, SQUARE, resolution);
        sendTrackEventIfSuitable(pos, 52.545, SQUARE, resolution);
        sendTrackEventIfSuitable(pos, 55.371, SQUARE, resolution);
        sendTrackEventIfSuitable(pos, 58.601, SQUARE, resolution);
        sendTrackEventIfSuitable(pos, 72.670, JUMP_RIGHT, resolution);
        sendTrackEventIfSuitable(pos, 74.875, JUMP_LEFT, resolution);

        dumpState(pos);
    }
}

void TestLooseSync()
{
    float pos;
    float resolution = 0.1;
    event_map_Initialize();
    dumpMap();

    for (pos=0; pos<78; pos+=resolution)
    {
        sendTrackEventIfSuitable(pos,  0.404, JUMP_RIGHT, resolution);
        //sendTrackEventIfSuitable(pos,  2.609, JUMP_LEFT, resolution);
        sendTrackEventIfSuitable(pos, 18.051, SQUARE, resolution);
        sendTrackEventIfSuitable(pos, 36.336, SQUARE, resolution);
        sendTrackEventIfSuitable(pos, 50.207, SQUARE, resolution);
        sendTrackEventIfSuitable(pos, 52.545, SQUARE, resolution);
        sendTrackEventIfSuitable(pos, 55.371, SQUARE, resolution);
        sendTrackEventIfSuitable(pos, 58.601, SQUARE, resolution);
        sendTrackEventIfSuitable(pos, 58.651, SQUARE, resolution);  // to loose sync again
        sendTrackEventIfSuitable(pos, 72.670, JUMP_RIGHT, resolution);
        sendTrackEventIfSuitable(pos, 74.875, JUMP_LEFT, resolution);

        dumpState(pos);
    }
}

int main()
{
    //TestSyncCase();
    TestLooseSync();
    return 0;
}
