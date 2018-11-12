/*
 * line_buffer.c
 *
 *  Created on: Mar 10, 2017
 *      Author: Tobias Hupel
 *
 *  Module for buffering of and calculations on ir sensor board patterns.
 */

#include "line_buffer.h"

#define NO_MEAN_VALUE -1.0f

/**
 * Initializes the given Line Buffer.
 *
 * @param lineBuffer
 *      the Line Buffer to initialize
 */
void line_buffer_new(struct LineBuffer * lineBuffer)
{
    for (int i = 0; i < LINE_BUFFER_SIZE; i++)
        lineBuffer->pattern_mean_values[i] = NO_MEAN_VALUE;

    lineBuffer->index = -1;
}

/**
 * Adds and saves the average position of ones of the given pattern to the given Line Buffer.
 *
 * @param lineBuffer
 *      the Line Buffer
 * @param pattern
 *      the pattern to add
 */
void line_buffer_add_pattern(struct LineBuffer * lineBuffer, unsigned char pattern)
{
    float meanValue = 0;
    int onesCount = 0;

    for (int i = 0; i <= 7; i++)
    {
        if (pattern & (1 << i))
        {
            meanValue += i;
            onesCount++;
        }
    }

    lineBuffer->index++;

    if (lineBuffer->index >= LINE_BUFFER_SIZE)
        lineBuffer->index = 0;

    lineBuffer->pattern_mean_values[lineBuffer->index] = onesCount > 0 ? meanValue / onesCount : NO_MEAN_VALUE;
}

/**
 * Returns the saved average position of ones defined by the steps to go back of the given Line Buffer.
 *
 * @param lineBuffer
 *      the Line Buffer
 * @param stepsBack
 *      how many steps to go back
 * @return
 *      the mean value of ones
 */
float get_pattern_mean_value(struct LineBuffer * lineBuffer, unsigned int stepsBack)
{
    int index;

    if (stepsBack >= LINE_BUFFER_SIZE)
        stepsBack = LINE_BUFFER_SIZE - 1;

    index = lineBuffer->index - stepsBack;

    if (index < 0)
        index = LINE_BUFFER_SIZE - (stepsBack - lineBuffer->index);

    return lineBuffer->pattern_mean_values[index];
}

/**
 * Calculates the gradient of average positions of ones over the given length of the given Line Buffer.
 *
 * @param lineBuffer
 *      the Line Buffer
 * @param length
 *      the length
 * @return
 *      the gradient of average positions
 */
float line_buffer_calculate_gradient(struct LineBuffer * lineBuffer, unsigned int length)
{
    float gradientsSum = 0, currentMean = NO_MEAN_VALUE, previousMean;
    int i, gradientsCount = 0, differencesCount = 0;

    if (length > LINE_BUFFER_SIZE)
        length = LINE_BUFFER_SIZE;
    else if (length < 2)
        length = 2;

    previousMean = get_pattern_mean_value (lineBuffer, 0);

    for (i = 1; i < length; i++)
    {
        if (previousMean != NO_MEAN_VALUE)
            currentMean = previousMean;

        previousMean = get_pattern_mean_value (lineBuffer, i);

        if (currentMean != NO_MEAN_VALUE)
            differencesCount++;

        if (previousMean != NO_MEAN_VALUE && currentMean != NO_MEAN_VALUE)
        {
            gradientsSum += (currentMean - previousMean) / differencesCount;
            gradientsCount++;
            differencesCount = 0;
        }
    }

    return gradientsCount > 0 ? gradientsSum / gradientsCount : 0;
}

/**
 * Calculates the average of average positions of ones over the given length of the given Line Buffer.
 *
 * @param lineBuffer
 *      the Line Buffer
 * @param length
 *      the length
 * @return
 *      the average of average positions
 */
float line_buffer_calculate_average(struct LineBuffer * lineBuffer, unsigned int length)
{
    float valuesSum = 0, currentMean;
    int i, valuesCount = 0;

    if (length > LINE_BUFFER_SIZE)
        length = LINE_BUFFER_SIZE;

    for (i = 0; i < length; i++)
    {
        currentMean = get_pattern_mean_value (lineBuffer, i);

        if (currentMean != NO_MEAN_VALUE)
        {
            valuesSum += currentMean;
            valuesCount++;
        }
    }

    return valuesCount > 0 ? valuesSum / valuesCount : NO_MEAN_VALUE;
}
