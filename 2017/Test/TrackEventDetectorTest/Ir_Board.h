#ifndef IR_BOARD_H_INCLUDED
#define IR_BOARD_H_INCLUDED

#include <stdio.h>
#include "../../src/tachometer.h"

char ir_get_value(char elementIndex);
void Ir_Initialize(char file[]);
int ir_get_end_of_recorded_track();

#endif // IR_BOARD_H_INCLUDED
