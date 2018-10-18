#include "Ir_Board.h"

#define PatternCapacity 1024

struct PatternElement
{
    float Distance;
    unsigned char Pattern;
};

struct PatternElement recordedElements[PatternCapacity];
unsigned int recordedElementCount = 0;

unsigned char ir_get_pattern(){
	 unsigned int upper, lower, current;
	 unsigned char sensor;
	 float dist;

	 dist = tachometer_get_extrapolated_distance_meter(BACK_LEFT);

	 upper = recordedElementCount -1;
	 lower = 0;

	 while (upper-lower > 1)
     {
        current = (upper + lower) / 2;
        if (recordedElements[current].Distance < dist)
        {
            lower = current;
        }
        else
        {
            upper = current;
        }
     }

	 sensor = recordedElements[lower].Pattern;
	 return sensor;
}

char ir_get_value(char elementIndex){
	unsigned char mask;
	char result;

	mask = 1 << elementIndex;
	result = ir_get_pattern();
	result &= mask;
	result = (result == 0 ? 0:1);

	return result;
}

/**
* Just for debug issues. Give information about how long we got recorded Ir information.
**/
int ir_get_end_of_recorded_track()
{
    if (recordedElementCount == 0)
        return 0;

    return recordedElements[recordedElementCount - 1].Distance;
}

void Ir_Initialize(char file[])
{
    float dist;
    unsigned char pattern;
    int tmp;
    char buffer[128];
    char* success;
    int matchCount = 2;
    FILE* fp = fopen(file, "r");
    if (fp == 0)
        return;

    recordedElementCount = 0;

    while (!feof(fp) && (recordedElementCount < PatternCapacity))
    {
        success = fgets(buffer, 128, fp);

        if (success == NULL)
            continue;

        matchCount = sscanf(buffer, "%f\t%d", &dist, &tmp);
        if (matchCount < 2)
            continue;

        pattern = tmp;
        recordedElements[recordedElementCount].Distance = dist;
        recordedElements[recordedElementCount].Pattern = pattern;
        ++recordedElementCount;
    }

    fclose(fp);
}
