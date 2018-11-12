
#ifndef Fuzzy_I2
#define Fuzzy_I2 Fuzzy_I2

#include "fuzzy_enums.h"
#include "NumType_I2.h"
#include <assert.h>

typedef unsigned char NumOfVal_t;
typedef unsigned char Bool_t;

typedef struct{
  NumOfVal_t                n;
  const NumTypeI2Point_t* p;
}FuzzySetI2_t;


typedef struct{
  NumOfVal_t            n;
  const FuzzySetI2_t*  fs;
  NumTypeI2_t          defaultvalue;
  char                  defaultactive;
}LinguisticOutputVariableI2_t;

typedef struct{
  NumOfVal_t n;
  const FuzzySetI2_t* fs;
}LinguisticInputVariableI2_t;

typedef struct{
  NumOfVal_t nI;
  NumOfVal_t nO;
  NumOfVal_t nR;
  const LinguisticInputVariableI2_t* iL;
  const LinguisticOutputVariableI2_t* oL;
  const char* pre;
  const char* con;
  const NumTypeI2_t* w;
  enum Inference_t inf;
  enum Defuzzy_t method;
  unsigned char steps;
  enum FuzzyAnd_t AndOp;
  enum FuzzyOr_t OrOp;
}FuzzyControllerI2_t;

 typedef struct{
    Bool_t **ihit; /* input hit [LingVar][Set] */
    Bool_t **ohit;
    NumTypeI2_t **imv;
    NumTypeI2_t **omv;
    NumTypeI2_t *res;
 }FCMemI2_t;


void FCI2_free(const FuzzyControllerI2_t *fc, FCMemI2_t *v);

void FCI2_calc(const FuzzyControllerI2_t *fc, FCMemI2_t *v, NumTypeI2_t *e, NumTypeI2_t *a);

void FCI2_init(const FuzzyControllerI2_t *fc, FCMemI2_t *v);

#endif

