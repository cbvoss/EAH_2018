/*
 * event_map.c
 *
 *  Created on: 09.02.2017
 *      Author: Eric Elsner
 *
 *  Reviewed on 03.03.2017
 *  	Florian Schädlich
 *  	Franz Lübke
 *  	Adrian Zentgraf
 */

#define ResyncVectorLen 8

#include "event_map.h"
#include <math.h>
#include <stdio.h>

struct EventMapEntry
{
	float velocity;
	float distance;
	enum track_event eventType;
};

struct ResyncEntry
{
	float distance;
	enum track_event eventType;
};

float g_MapDistanceOffset = 0;			//offset to global tachometer distance

struct EventMapEntry g_eventMap[40];	//The event map. Start at index '0'.
int g_MapEventCount = 0;				//the event count in the event map
int g_MapIndexActive = 0;				//the map index where we are supposed to be

//Resync vector contains the last occurred events. Needed for map resync.
// Newest entry is always at index '0'.
struct ResyncEntry g_resyncVector[ResyncVectorLen];
enum SyncStates g_syncState = SYNC; //states whether map is still sync

float g_velocityFallback = 0.0f;		//a velocity which is always 'safe'
										//-> will be used if we loose synchronization

/**
 * Adds an entry to the map.
 * @param distance
 * 	The distance to the last section [m].
 * @param velocity
 *  The recommended velocity in the given section [m/s].
 * @param event
 *  The track event kind.
 * @param totalDistance
 *  The total distance so far. This is only used to store the distances in a accumulated manner.
 */
void addMapEntry(float distance, float velocity, enum track_event event, float* totalDistance)
{
	*totalDistance += distance;
	g_eventMap[g_MapEventCount].distance = *totalDistance;
	g_eventMap[g_MapEventCount].velocity = velocity;
	g_eventMap[g_MapEventCount].eventType = event;
	++g_MapEventCount;
}

/**
 * Gets the distance from one map element to another. Lap border will be taken into account.
 * @param mapIndexFrom
 * 	The start index.
 * @param mapIndexTo
 * 	The destination index.
 * @return
 * 	The distance [m].
 */
float getLapCorrectDistanceForward(int mapIndexFrom, int mapIndexTo)
{
	float result = g_eventMap[mapIndexTo].distance - g_eventMap[mapIndexFrom].distance;
	if (mapIndexTo < mapIndexFrom)
		result += g_eventMap[g_MapEventCount-1].distance;

	return result;
}

/**
 * Removes the map entry with the given index.
 */
void removeMapEntry(int index)
{
	int mapIndex, nextIndex;
	if (g_MapEventCount <= 0)
		return;

	--g_MapEventCount;
	for (mapIndex=index; mapIndex<g_MapEventCount; ++mapIndex)
	{
		nextIndex = mapIndex + 1;
		g_eventMap[mapIndex].distance = g_eventMap[nextIndex].distance;
		g_eventMap[mapIndex].eventType = g_eventMap[nextIndex].eventType;
		g_eventMap[mapIndex].velocity = g_eventMap[nextIndex].velocity;
	}
}

/**
 * Returns the maximum of two values.
 * @param f0
 * 	First value.
 * @param f1
 * 	Second value.
 * @return
 * 	The maximum.
 */
float fMax(float f0, float f1)
{
	if (f0 > f1)
		return f0;

	return f1;
}

/**
 * Gets a table like, human readable string of the event map data.
 * @param buffer
 *  The string buffer.
 * @param bufferCount
 * 	The size of the buffer.
 */
void event_map_get_map_string(char buffer[], int bufferCount)
{
    int mapIndex;
    int chars, charsRemaining;
    char event;
    float dist, vel;
    charsRemaining = bufferCount;
    for (mapIndex=0; (mapIndex<g_MapEventCount) && (charsRemaining > 0); ++mapIndex)
    {
        switch (g_eventMap[mapIndex].eventType)
        {
            case JUMP_RIGHT: event = 'R'; break;
            case JUMP_LEFT: event = 'L'; break;
            case SQUARE: event = 'Q'; break;
            case NONE: event = 'N'; break;
        }
        dist = g_eventMap[mapIndex].distance;
        vel = g_eventMap[mapIndex].velocity;
        chars = snprintf(buffer, charsRemaining, "%c\t%f\t%f\n", event, dist, vel);

        if (chars < 0)
            return;

        charsRemaining -= chars;
        buffer += chars;
    }

    if (charsRemaining >= 2)
        chars = snprintf(buffer, charsRemaining, "\n");
}

/**
 * Changes the map distances to break earlier and
 * accelerate later in map areas which can't be synchronized.
 * @param safetyFactor
 * 	A factor in the range 0(no additional safety)...1(only slower used)
 */
void changeMapDistancesToImproveSafety(float safetyFactor)
{
	int mapIndex, nextMapIndex0, nextMapIndex1, lastSyncMapIndex;
	float safetyDist;
	float dist;

	lastSyncMapIndex = 0;
	for (mapIndex=0; mapIndex<g_MapEventCount; ++mapIndex)
	{
		if (g_eventMap[mapIndex].eventType != NONE)
		{
			lastSyncMapIndex = mapIndex;
			continue;
		}

		nextMapIndex0 = mapIndex+1;
		if (nextMapIndex0>=g_MapEventCount)
			nextMapIndex0 = 0;

		if ((g_eventMap[nextMapIndex0].eventType != NONE) ||
			(g_eventMap[nextMapIndex0].velocity <= g_eventMap[mapIndex].velocity))
			continue;

		dist = getLapCorrectDistanceForward(lastSyncMapIndex, nextMapIndex0);
		safetyDist = dist * (1+safetyFactor);
		g_eventMap[nextMapIndex0].distance = g_eventMap[lastSyncMapIndex].distance + safetyDist;

		nextMapIndex1 = nextMapIndex0+1;
		if ((nextMapIndex1 < g_MapEventCount) &&
			(g_eventMap[nextMapIndex0].distance >= g_eventMap[nextMapIndex1].distance))
			removeMapEntry(nextMapIndex0);
	}
}

/**
 * Initializes the event map to the test course map.
 */
void InitializeTestMap()
{
	float totalDistance;
	totalDistance = 0;

	addMapEntry(0.000, 1.0, NONE, &totalDistance);	//larger curve
	addMapEntry(1.357, 1.0, NONE, &totalDistance);	//printed curve starts
	addMapEntry(2.714, 2.0, NONE, &totalDistance);	//printed curve end
	addMapEntry(0.700, 1.0, SQUARE, &totalDistance);	//square event
	addMapEntry(0.700, 2.0, NONE, &totalDistance);
	addMapEntry(0.844, 1.0, JUMP_RIGHT, &totalDistance);	//jump event
	addMapEntry(1.866, 1.0, SQUARE, &totalDistance);	//square event
	addMapEntry(0.700, 2.0, NONE, &totalDistance);
	addMapEntry(1.170, 1.0, JUMP_LEFT, &totalDistance);	//square event
	addMapEntry(1.132, 2.0, NONE, &totalDistance);	//lap end
	//TODO: more straight parts?
	changeMapDistancesToImproveSafety(0.2);	//20% additional safety
}

/**
 * Initializes the event map to the competition map.
 */
void InitializeCompetitionMap()
{
	float totalDistance;
	totalDistance = 0;

	addMapEntry(0.000, 2.0, NONE, &totalDistance);
	addMapEntry(0.304, 1.0, JUMP_RIGHT, &totalDistance);
	addMapEntry(2.105, 1.0, JUMP_LEFT, &totalDistance);
	addMapEntry(2.763, 1.0, NONE, &totalDistance);		//now a pile of curves
	addMapEntry(8.926, 4.0, NONE, &totalDistance);		//now long straight
	addMapEntry(3.451, 0.8, SQUARE, &totalDistance);	//right
	addMapEntry(2.763, 4.0, NONE, &totalDistance);		//now long straight
	addMapEntry(6.664, 1.0, NONE, &totalDistance);		//now curves
	addMapEntry(8.360, 0.8, SQUARE, &totalDistance);	//right and curves
	addMapEntry(6.285, 3.0, NONE, &totalDistance);		//now straight
	addMapEntry(1.210, 1.0, NONE, &totalDistance);		//now very small curve
	addMapEntry(0.540, 3.0, NONE, &totalDistance);		//now straight
	addMapEntry(2.366, 1.0, NONE, &totalDistance);		//now curve
	addMapEntry(2.970, 0.8, SQUARE, &totalDistance);	//left
	addMapEntry(1.838, 0.8, SQUARE, &totalDistance);	//left again
	addMapEntry(2.526, 0.8, SQUARE, &totalDistance);	//left again
	addMapEntry(3.130, 0.8, SQUARE, &totalDistance);	//right
	addMapEntry(2.579, 4.0, NONE, &totalDistance);		//now straight
	addMapEntry(3.504, 1.0, NONE, &totalDistance);		//now short curve
	addMapEntry(1.032, 3.0, NONE, &totalDistance);		//now straight
	addMapEntry(1.198, 1.0, NONE, &totalDistance);		//now short curve
	addMapEntry(0.984, 3.0, NONE, &totalDistance);		//now straight
	addMapEntry(1.263, 1.0, NONE, &totalDistance);		//now small curve
	addMapEntry(0.415, 2.0, NONE, &totalDistance);		//curve over, lap end

	changeMapDistancesToImproveSafety(0.2);	//20% additional safety
}

/**
 * Initializes the event map.
 */
void event_map_Initialize()
{
	//InitializeCompetitionMap();
	InitializeTestMap();
	g_syncState = SYNC;
	g_velocityFallback = 0.5f;
}

/**
 * Searches in event map for the next index which contains an element which is different to 'NONE'.
 * Searches backwards.
 * @param startIndex
 * 	The index where to start searching (excluding).
 * @return
 *  The first index of an element different to 'NONE'.
 */
int getPreviousNotNoneMapIndex(int startIndex)
{
	int nextNotNoneIndex, i, mapIndex;

	nextNotNoneIndex = -1;
	for (i=0; (i<g_MapEventCount)&&(nextNotNoneIndex==-1); ++i)
	{
		mapIndex = startIndex - 1 - i;
		if (mapIndex < 0)
			mapIndex += g_MapEventCount;

		if (g_eventMap[mapIndex].eventType != NONE)
			nextNotNoneIndex = mapIndex;
	}

	return nextNotNoneIndex;
}

/**
 * Searches in event map for the next index which contains an element which is different to 'NONE'.
 * Searches forward.
 * @param startIndex
 * 	The index where to start searching (excluding).
 * @return
 *  The first index of an element different to 'NONE'.
 */
int getNextNotNoneMapIndex(int startIndex)
{
	int nextNotNoneIndex, i, mapIndex;

	nextNotNoneIndex = -1;
	for (i=0; (i<g_MapEventCount)&&(nextNotNoneIndex==-1); ++i)
	{
		mapIndex = startIndex + 1 + i;
		if (mapIndex >= g_MapEventCount)
			mapIndex -= g_MapEventCount;

		if (g_eventMap[mapIndex].eventType != NONE)
			nextNotNoneIndex = mapIndex;
	}

	return nextNotNoneIndex;
}

/**
 * Removes the oldest resync map entry.
 */
void removeOldestResyncMapEntry()
{
    int i;
    for (i=ResyncVectorLen-1; i>=0; --i)
	{
	    if (g_resyncVector[i].eventType == NONE)
            continue;

        g_resyncVector[i].eventType = NONE;
        g_resyncVector[i].distance = 0;
        break;
	}
}

/**
 * Appends an Element to the resync map at index '0'.
 * When resync map gets larger, the map can possibly be resynchronized.
 * @param distanceMeter
 * 	The current start distance [m].
 * @param event
 * 	The current event.
 */
void appendToResyncMap(float distanceMeter, enum track_event event)
{
	int i;
	if (event == NONE)
		return;

	for (i=ResyncVectorLen-1; i>0; --i)
	{
		g_resyncVector[i].eventType = g_resyncVector[i-1].eventType;
		g_resyncVector[i].distance = g_resyncVector[i-1].distance;
	}

	g_resyncVector[0].eventType = event;
	g_resyncVector[0].distance = distanceMeter;
}

/**
* Returns the position of a map section which lies behind in the map. Map bound will be taken into account.
 * @param mapIndex
 * 	The map index of interest.
 * @param referenceIndex
 * 	The map index from where we start.
 * @return
 * 	The position of the map section [m].
 */
float getLapCorrectedPosOfMapIndexBackward(int mapIndex, int referenceIndex)
{
    float result;
    result = g_eventMap[mapIndex].distance;
    if (mapIndex > referenceIndex)
        result -= g_eventMap[g_MapEventCount-1].distance;

    result += g_MapDistanceOffset;
    return result;
}

/**
 * Returns the position of a map section which lies ahead in the map. Map bound will be taken into account.
 * @param mapIndex
 * 	The map index of interest.
 * @param referenceIndex
 * 	The map index from where we start.
 * @return
 * 	The position of the map section [m].
 */
float getLapCorrectedPosOfMapIndexForward(int mapIndex, int referenceIndex)
{
    float result;
    result = g_eventMap[mapIndex].distance;
    if (mapIndex < referenceIndex)
        result += g_eventMap[g_MapEventCount-1].distance;

    result += g_MapDistanceOffset;
    return result;
}

/**
 * Checks whether the resync vector at the given index matches the map at the given index.
 * @param resyncIdx
 * 	The resync vector index.
 * @param mapIdx
 * 	The map index.
 * @param lastMapIdx
 *  The next not 'NONE' map index in forward direction.
 * @return
 */
char matchResyncVectorIndexToMapIndex(int resyncIdx, int mapIdx, int nextForwardMapIdx)
{
	float mapDist, resyncDist;

	if (g_eventMap[mapIdx].eventType != g_resyncVector[resyncIdx].eventType)
		return 0;

	if (resyncIdx == 0)	//if resync vector contains only one element, distance can't be checked.
		return 1;

	mapDist =
		getLapCorrectedPosOfMapIndexBackward(nextForwardMapIdx, nextForwardMapIdx) -
		getLapCorrectedPosOfMapIndexBackward(mapIdx, nextForwardMapIdx);	//lap correct distance
	resyncDist = g_resyncVector[resyncIdx-1].distance - g_resyncVector[resyncIdx].distance;

	//If the measured distance is shorter than calculated distance in the map,
	// then this is probably not the right map position.
	if (mapDist*0.95 > resyncDist) //map distances are rough -> the safety factor might avoid false negatives
		return 0;

	return 1;
}

/**
 * Checks whether the resync vector matches to the given start index of the event map.
 * @param mapStartIndex
 * 	The start index where to match resync vector against map.
 * @return
 * 	'0' if no match. '1' if match.
 */
char matchResyncVectorToMapIndex(int mapStartIndex)
{
	int resyncIdx, mapIdx, lastMapIdx;
	if (g_eventMap[mapStartIndex].eventType == NONE)
		return 0;

	mapIdx = mapStartIndex;
	lastMapIdx = mapIdx;
	for (resyncIdx=0;
		(resyncIdx<ResyncVectorLen)&&(g_resyncVector[resyncIdx].eventType!=NONE);
		++resyncIdx)
	{
		//Resync vector contains newest element at index '0' while latest map element is at last index.
		// This way it's easier to get the current map position of the match.
		if (matchResyncVectorIndexToMapIndex(resyncIdx, mapIdx, lastMapIdx) == 0)
			return 0;

		lastMapIdx = mapIdx;
		mapIdx = getPreviousNotNoneMapIndex(mapIdx);
	}

	return 1;
}

/**
 * Searches all possible map positions that fit to the resync vector.
 * @param matchCount
 * 	The number of matched positions.
 * @param mapIndex
 * 	One matching map index.
 */
void findResyncVectorMatches(int* matchCount, int* mapIndex)
{
	int i;
	*matchCount = 0;
	*mapIndex = -1;
	for (i=0; (i<g_MapEventCount) && (*matchCount<2); ++i)
	{
		if (matchResyncVectorToMapIndex(i) == 0)
			continue;

		*mapIndex = i;
		++(*matchCount);
	}
}

/**
 * Tries to synchronize the map to the resync vector.
 */
void tryResycMap()
{
	int i, matchCount, mapIndex;
	matchCount=0;

	for (i=0; i<ResyncVectorLen; ++i)	//retry with oldest entry removed if no match
    {
        findResyncVectorMatches(&matchCount, &mapIndex);

        if (matchCount != 0)
            break;

        removeOldestResyncMapEntry();
    }

	if (matchCount != 1)
		return;

	g_MapIndexActive = mapIndex;
	g_MapDistanceOffset = g_resyncVector[0].distance - g_eventMap[g_MapIndexActive].distance;
	g_syncState = SYNC;
}

/**
 * Indicates that map position isn't synchronized to the real position anymore.
 * Starts the continuous resync attempt.
 * @param justPartially
 *  Set to '1' if rough map position is still correct. Next track event will resync the map then.
 *  Set to '0' if we have no clue, where we are. Resync vector will be generated to find the map position. But that might take time.
 */
void event_map_Indicate_SyncLost(char justPartially)
{
	int i;
	if (justPartially)
    {
		if (g_syncState != SYNC)
			return;

        g_syncState = PARTIALLY_LOST;
        return;
    }

	g_syncState = FULLY_LOST;

	for (i=0; i<ResyncVectorLen; ++i)
	{
		g_resyncVector[i].distance = 0;
		g_resyncVector[i].eventType = NONE;
	}
}

/**
 * Updates the position on the map with a detected event.
 * @param distanceMeter
 * 	The distance to the start position [m].
 * @param event
 *  The detected event.
 */
void event_map_update(float distanceMeter, enum track_event event)
{
	int nextNotNoneIndex;

	if (g_syncState == FULLY_LOST)
	{
		appendToResyncMap(distanceMeter, event);
		tryResycMap();
		return;
	}

	nextNotNoneIndex = getNextNotNoneMapIndex(g_MapIndexActive);
	if (nextNotNoneIndex < 0)
		return;	//if we have a proper map, this will never happen

	if (g_eventMap[nextNotNoneIndex].eventType != event)
	{
		event_map_Indicate_SyncLost(0);
		appendToResyncMap(distanceMeter, event);
		tryResycMap();
		return;
	}

	g_MapIndexActive = nextNotNoneIndex;
	g_MapDistanceOffset = distanceMeter - g_eventMap[g_MapIndexActive].distance;
	g_syncState = SYNC;
}

/**
 * Gets a velocity recommendation at the current position on the map.
 * @param distanceMeter
 *  The distance to the start position [m].
 * @return
 *  The recommended velocity [m/s].
 */
float event_map_get_velocity_recomendation(float distanceMeter)
{
	int nextNotNoneMapIndex0, nextNotNoneMapIndex1, i, currIndex;
	float distCurr;
	if (g_syncState != SYNC)
		return g_velocityFallback;

	nextNotNoneMapIndex0 = getNextNotNoneMapIndex(g_MapIndexActive);
	nextNotNoneMapIndex1 = getNextNotNoneMapIndex(nextNotNoneMapIndex0);

	//"did we miss an event?"
	distCurr = getLapCorrectedPosOfMapIndexForward(nextNotNoneMapIndex1, g_MapIndexActive);
	if (distanceMeter > distCurr)
	{
		event_map_Indicate_SyncLost(1); //Let's hope, we just made a detour?
		return g_velocityFallback;
	}

	for (i=0; i<g_MapEventCount; ++i)
	{
		currIndex = nextNotNoneMapIndex1-i;
		if (currIndex < 0)
			currIndex += g_MapEventCount;

		distCurr = getLapCorrectedPosOfMapIndexForward(currIndex, g_MapIndexActive);
		if (distCurr <= distanceMeter)
			return g_eventMap[currIndex].velocity;
	}

	//if first map element starts with distance '0' -> this case shouldn't happen
	return g_velocityFallback;
}

/**
* Returns the current sync state.
**/
enum SyncStates event_map_get_sync_State()
{
    return g_syncState;
}
