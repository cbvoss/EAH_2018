/*
 * event_map.h
 *
 *  Created on: 09.02.2017
 *      Author: Eric Elsner
 *
 *  Reviewed on 03.03.2017
 *  	Florian Schädlich
 *  	Franz Lübke
 *  	Adrian Zentgraf
 */

#ifndef EVENT_MAP_H_
#define EVENT_MAP_H_

#include "enums.h"

enum SyncStates
{
    SYNC = 0,
    PARTIALLY_LOST = 1,
    FULLY_LOST = 2,
};

void event_map_Initialize();
void event_map_Indicate_SyncLost(char justPartially);
void event_map_update(float distanceMeter, enum track_event event);
float event_map_get_velocity_recomendation(float distanceMeter);

enum SyncStates event_map_get_sync_State();
void event_map_get_map_string(char buffer[], int bufferCount);

#endif /* EVENT_MAP_H_ */
