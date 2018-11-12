
#ifndef Fuzzy_I4
#define Fuzzy_I4 Fuzzy_I4

#include "fuzzy_enums.h"
#include "NumType_I4.h"
#include <assert.h>

typedef unsigned char NumOfVal_t;
typedef unsigned char Bool_t;

typedef struct{
  NumOfVal_t                n;
  const NumTypeI4Point_t* p;
}FuzzySetI4_t;


typedef struct{
  NumOfVal_t            n;
  const FuzzySetI4_t*  fs;
  NumTypeI4_t          defaultvalue;
  char                  defaultactive;
}LinguisticOutputVariableI4_t;

typedef struct{
  NumOfVal_t n;
  const FuzzySetI4_t* fs;
}LinguisticInputVariableI4_t;

typedef struct{
  NumOfVal_t nI;
  NumOfVal_t nO;
  NumOfVal_t nR;
  const LinguisticInputVariableI4_t* iL;
  const LinguisticOutputVariableI4_t* oL;
  const char* pre;
  const char* con;
  const NumTypeI4_t* w;
  enum Inference_t inf;
  enum Defuzzy_t method;
  unsigned char steps;
  enum FuzzyAnd_t AndOp;
  enum FuzzyOr_t OrOp;
}FuzzyControllerI4_t;

 typedef struct{
    Bool_t **ihit; /* input hit [LingVar][Set] */
    Bool_t **ohit;
    NumTypeI4_t **imv;
    NumTypeI4_t **omv;
    NumTypeI4_t *res;
 }FCMemI4_t;


void FCI4_free(const FuzzyControllerI4_t *fc, FCMemI4_t *v);

void FCI4_calc(const FuzzyControllerI4_t *fc, FCMemI4_t *v, NumTypeI4_t *e, NumTypeI4_t *a);

void FCI4_init(const FuzzyControllerI4_t *fc, FCMemI4_t *v);

#endif

