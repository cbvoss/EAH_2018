#include <stdio.h>
#include <stdlib.h>
#include "../../src/regulator.h"

/**
 * Reads the input signal from file.
 * @param input
 *  The input buffer. Will be filled by the function.
 * @param inputBufferLen
 *  The length of the input buffer.
 * @param elementCount
 *  The number of read elements. Will be set by the function.
 */
void loadInput(float input[], int inputBufferLen, int* elementCount)
{
    *elementCount = 0;
    float f;

    FILE* fp = fopen("./input/input.txt", "r");

    for (*elementCount=0; *elementCount < inputBufferLen && !feof(fp); ++(*elementCount))
    {
        fscanf(fp, "%f", &f);
        input[*elementCount] = f;
    }

    fclose(fp);
}

/**
 * Saves input and output data in excel compatible format.
 * @param input
 *  The input data.
 * @param output
 *  the output data.
 * @param elementCount
 *  The number of elements in input and output.
 */
void saveOutput(float input[], float output[], int elementCount)
{
    int i;
    FILE* fp = fopen("./result/regulator.txt", "w");
    fprintf(fp, "input\toutput\n");

    for (i=0; i<elementCount; ++i)
    {
        fprintf(fp, "%f\t%f\n", input[i], output[i]);
    }

    fclose(fp);
}

/**
 * Tests the regulator with data read from a file.
 * Writes the result into another file.
 */
void DoRegulatorTest()
{
    float Ta = 0.1;
    float Ti = 10;
    float Td = 0.1;
    float V = 1.5;

    int maxElementCount = 127;
    int elementCount = 0;
    float input[maxElementCount];
    float output[maxElementCount];
    float deviation = 0;

    loadInput(input, maxElementCount, &elementCount);

    struct Regulator regulator;
    regulator_new(&regulator, V, 1.0f/Ti, Td, Ta);

    int i;
    for (i=0; i<elementCount; ++i)
    {
        deviation = input[i];
        output[i] = regulator_calculate_value(&regulator, deviation);
    }

    saveOutput(input, output, elementCount);
}

int main()
{
    DoRegulatorTest();
    return 0;
}
