/***************************************************************************
  This file was created by the program FALCO which is a module 
  of WinFACT (Windows Fuzzy And Control Tools).

  Instead of editing this file, modify FALCOs Source-File
  with FLOP and generate it once again.

  Generated by : Tobias Hupel
            on : TOBYS-NOTEBOOK
  FALCO Version: 8.0.0.9
  Source-File  : C:\Users\Tobias Hupel\Documents\Projects\MCU_Rally_2018\res\Car2018\Increased_Length\line_abs_9SI.fuz from:02/23/2018 13:44:44
  Timestamp    : 02/23/2018 13:45:06

  Description of the fuzzy controller implemented in this code:
  =============================================================

    Inputs: 1
     LineDeviation

    Outputs: 1
     ServoAngleDeg

    Rules: 9

    Inferencemechanism: Max-Min
    Method of defuzzyfication: center of gravity (Intergration via 60 steps)
    Number format: 4 Byte IEEE float
***************************************************************************/

#include "line_abs_9SI_F4.h"

/*############################################################################*/
/*                        Sets of all input variables                         */
/*############################################################################*/

static const NumTypeF4Point_t line_abs_9SI_F4_LineDeviation_0__4_R[3]={
   {-4, 0},
   {-4, 1},
   {-3, 0}};
static const NumTypeF4Point_t line_abs_9SI_F4_LineDeviation_1__3_R[3]={
   {-4, 0},
   {-3, 1},
   {-2, 0}};
static const NumTypeF4Point_t line_abs_9SI_F4_LineDeviation_2__2_R[3]={
   {-3, 0},
   {-2, 1},
   {-1, 0}};
static const NumTypeF4Point_t line_abs_9SI_F4_LineDeviation_3__1_R[3]={
   {-2, 0},
   {-1, 1},
   {0, 0}};
static const NumTypeF4Point_t line_abs_9SI_F4_LineDeviation_4__0[3]={
   {-1, 0},
   {0, 1},
   {1, 0}};
static const NumTypeF4Point_t line_abs_9SI_F4_LineDeviation_5__1_L[3]={
   {0, 0},
   {1, 1},
   {2, 0}};
static const NumTypeF4Point_t line_abs_9SI_F4_LineDeviation_6__2_L[3]={
   {1, 0},
   {2, 1},
   {3, 0}};
static const NumTypeF4Point_t line_abs_9SI_F4_LineDeviation_7__3_L[3]={
   {2, 0},
   {3, 1},
   {4, 0}};
static const NumTypeF4Point_t line_abs_9SI_F4_LineDeviation_8__4_L[3]={
   {3, 0},
   {4, 1},
   {4, 0}};

/*############################################################################*/
/*                        Sets of all output variables                        */
/*############################################################################*/

static const NumTypeF4Point_t line_abs_9SI_F4_ServoAngleDeg_0_Right_B_VS[1]={
   {-26, 1}};
static const NumTypeF4Point_t line_abs_9SI_F4_ServoAngleDeg_1_Right_M_VS[1]={
   {-20, 1}};
static const NumTypeF4Point_t line_abs_9SI_F4_ServoAngleDeg_2_Right_S_VS[1]={
   {-14, 1}};
static const NumTypeF4Point_t line_abs_9SI_F4_ServoAngleDeg_3_RightCorrect[1]={
   {-5, 1}};
static const NumTypeF4Point_t line_abs_9SI_F4_ServoAngleDeg_4_Middle[1]={
   {0, 1}};
static const NumTypeF4Point_t line_abs_9SI_F4_ServoAngleDeg_5_LeftCorrect[1]={
   {5, 1}};
static const NumTypeF4Point_t line_abs_9SI_F4_ServoAngleDeg_6_Left_S_VS[1]={
   {14, 1}};
static const NumTypeF4Point_t line_abs_9SI_F4_ServoAngleDeg_7_Left_M_VS[1]={
   {20, 1}};
static const NumTypeF4Point_t line_abs_9SI_F4_ServoAngleDeg_8_Left_B_VS[1]={
   {26, 1}};


static const FuzzySetF4_t line_abs_9SI_F4_LineDeviation_0[9]={
   {3, line_abs_9SI_F4_LineDeviation_0__4_R},
   {3, line_abs_9SI_F4_LineDeviation_1__3_R},
   {3, line_abs_9SI_F4_LineDeviation_2__2_R},
   {3, line_abs_9SI_F4_LineDeviation_3__1_R},
   {3, line_abs_9SI_F4_LineDeviation_4__0},
   {3, line_abs_9SI_F4_LineDeviation_5__1_L},
   {3, line_abs_9SI_F4_LineDeviation_6__2_L},
   {3, line_abs_9SI_F4_LineDeviation_7__3_L},
   {3, line_abs_9SI_F4_LineDeviation_8__4_L}};
/*############################################################################*/
/*                              Input variables                               */
/*############################################################################*/

static const LinguisticInputVariableF4_t line_abs_9SI_F4_Input[1]={
   {9, line_abs_9SI_F4_LineDeviation_0}};


static const FuzzySetF4_t line_abs_9SI_F4_ServoAngleDeg_0[9]={
   {1, line_abs_9SI_F4_ServoAngleDeg_0_Right_B_VS},
   {1, line_abs_9SI_F4_ServoAngleDeg_1_Right_M_VS},
   {1, line_abs_9SI_F4_ServoAngleDeg_2_Right_S_VS},
   {1, line_abs_9SI_F4_ServoAngleDeg_3_RightCorrect},
   {1, line_abs_9SI_F4_ServoAngleDeg_4_Middle},
   {1, line_abs_9SI_F4_ServoAngleDeg_5_LeftCorrect},
   {1, line_abs_9SI_F4_ServoAngleDeg_6_Left_S_VS},
   {1, line_abs_9SI_F4_ServoAngleDeg_7_Left_M_VS},
   {1, line_abs_9SI_F4_ServoAngleDeg_8_Left_B_VS}};
/*############################################################################*/
/*                              Output variables                              */
/*############################################################################*/

static const LinguisticOutputVariableF4_t line_abs_9SI_F4_Output[1]={
   {9, line_abs_9SI_F4_ServoAngleDeg_0, 0, 0}};
/*############################################################################*/
/*                         Premises of the rule base                          */
/*############################################################################*/

static const char line_abs_9SI_F4_RuleBase_pre[9*1]={
   5,
   4,
   6,
   3,
   7,
   2,
   8,
   1,
   9};

/*############################################################################*/
/*                        Conclusion of the rule base                         */
/*############################################################################*/

static const char line_abs_9SI_F4_RuleBase_con[9*1]={
   5,
   4,
   6,
   3,
   7,
   2,
   8,
   1,
   9};

/*############################################################################*/
/*                           Weighting of the rules                           */
/*############################################################################*/

static const NumTypeF4_t line_abs_9SI_F4_RuleBase_weight[9]={
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1};

/*############################################################################*/
/*                    All together in FuzzyControllerF4_t                     */
/*############################################################################*/
static const FuzzyControllerF4_t line_abs_9SI_F4_FC={
   1,
   1,
   9,
   line_abs_9SI_F4_Input,
   line_abs_9SI_F4_Output,
   line_abs_9SI_F4_RuleBase_pre,
   line_abs_9SI_F4_RuleBase_con,
   line_abs_9SI_F4_RuleBase_weight,
   MAX_MIN,
   DEFUZZY_COG,
   60,
   AND_MIN,
   OR_MAX};


/*############################################################################*/
/*                       Call this function to be sure                        */
/*                  to work with the correct number format.                   */
/*############################################################################*/


void line_abs_9SI_F4_SetNumType(void)
{
}


/*############################################################################*/
/*                           Structure of pointers                            */
/*           This structure is allocated in the xx_init() function            */
/*        and is needed for the calculation in the xx_calc() function.        */
/*############################################################################*/

static FCMemF4_t line_abs_9SI_F4_FCMem;


/*############################################################################*/
/*                          Initializiation function                          */
/*                  Call this function before the others !!                   */
/*############################################################################*/


void line_abs_9SI_F4_init(void)
{
  FCF4_init(&line_abs_9SI_F4_FC, &line_abs_9SI_F4_FCMem);
}
/*############################################################################*/
/*                    Function representing the controller                    */
/*############################################################################*/


void line_abs_9SI_F4_calc(
       const NumTypeF4_t i0,
       NumTypeF4_t *o0)
{
  NumTypeF4_t ai[1];
  NumTypeF4_t ao[1];
  ai[0]=i0;
  FCF4_calc(&line_abs_9SI_F4_FC, &line_abs_9SI_F4_FCMem, ai, ao);
  *o0=ao[0];
}
/*############################################################################*/
/*                       Function to release the memory                       */
/*                     which was allocated by xx-init().                      */
/*############################################################################*/


void line_abs_9SI_F4_free(void)
{
  FCF4_free(&line_abs_9SI_F4_FC, &line_abs_9SI_F4_FCMem);
}
