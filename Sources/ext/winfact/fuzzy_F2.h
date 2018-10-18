
#ifndef Fuzzy_F2
#define Fuzzy_F2 Fuzzy_F2

#include "fuzzy_enums.h"
#include "NumType_F2.h"
#include <assert.h>

typedef unsigned char NumOfVal_t;
typedef unsigned char Bool_t;

typedef struct{
  NumOfVal_t                n;
  const NumTypeF2Point_t* p;
}FuzzySetF2_t;


typedef struct{
  NumOfVal_t            n;
  const FuzzySetF2_t*  fs;
  NumTypeF2_t          defaultvalue;
  char                  defaultactive;
}LinguisticOutputVariableF2_t;

typedef struct{
  NumOfVal_t n;
  const FuzzySetF2_t* fs;
}LinguisticInputVariableF2_t;

typedef struct{
  NumOfVal_t nI;
  NumOfVal_t nO;
  NumOfVal_t nR;
  const LinguisticInputVariableF2_t* iL;
  const LinguisticOutputVariableF2_t* oL;
  const char* pre;
  const char* con;
  const NumTypeF2_t* w;
  enum Inference_t inf;
  enum Defuzzy_t method;
  unsigned char steps;
  enum FuzzyAnd_t AndOp;
  enum FuzzyOr_t OrOp;
}FuzzyControllerF2_t;

 typedef struct{
    Bool_t **ihit; /* input hit [LingVar][Set] */
    Bool_t **ohit;
    NumTypeF2_t **imv;
    NumTypeF2_t **omv;
    NumTypeF2_t *res;
 }FCMemF2_t;


void FCF2_free(const FuzzyControllerF2_t *fc, FCMemF2_t *v);

void FCF2_calc(const FuzzyControllerF2_t *fc, FCMemF2_t *v, NumTypeF2_t *e, NumTypeF2_t *a);

void FCF2_init(const FuzzyControllerF2_t *fc, FCMemF2_t *v);

#endif

