/***************************************************************************
  This file was created by the program FALCO which is a module 
  of WinFACT (Windows Fuzzy And Control Tools).

  Instead of editing this file, modify FALCOs Source-File
  with FLOP and generate it once again.

  Generated by : Tobias Hupel
            on : TOBYS-NOTEBOOK
  FALCO Version: 8.0.0.9
  Source-File  : C:\Users\Tobias Hupel\Documents\Projects\MCU_Rally_2018\res\Car2018\Increased_Length\target_speed.fuz from:02/05/2018 16:13:08
  Timestamp    : 02/05/2018 16:13:14

  Description of the fuzzy controller implemented in this code:
  =============================================================

    Inputs: 1
     LineDevAbs

    Outputs: 1
     Velocity

    Rules: 2

    Inferencemechanism: Max-Min
    Method of defuzzyfication: center of gravity (Intergration via 60 steps)
    Number format: 4 Byte IEEE float
***************************************************************************/

#ifndef target_speed_F4
#define target_speed_F4 target_speed_F4



#include "fuzzy_F4.h"

#ifdef __cplusplus
  extern "C" {
#endif

void target_speed_F4_SetNumType(void);

void target_speed_F4_init(void);

void target_speed_F4_calc(
       const NumTypeF4_t i0,
       NumTypeF4_t *o0);

void target_speed_F4_free(void);
#ifdef __cplusplus
  }
#endif


#endif
