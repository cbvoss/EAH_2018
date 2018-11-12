
#ifndef Fuzzy_F4
#define Fuzzy_F4 Fuzzy_F4

#include "fuzzy_enums.h"
#include "NumType_F4.h"
#include <assert.h>

typedef unsigned char NumOfVal_t;
typedef unsigned char Bool_t;

typedef struct{
  NumOfVal_t                n;
  const NumTypeF4Point_t* p;
}FuzzySetF4_t;


typedef struct{
  NumOfVal_t            n;
  const FuzzySetF4_t*  fs;
  NumTypeF4_t          defaultvalue;
  char                  defaultactive;
}LinguisticOutputVariableF4_t;

typedef struct{
  NumOfVal_t n;
  const FuzzySetF4_t* fs;
}LinguisticInputVariableF4_t;

typedef struct{
  NumOfVal_t nI;
  NumOfVal_t nO;
  NumOfVal_t nR;
  const LinguisticInputVariableF4_t* iL;
  const LinguisticOutputVariableF4_t* oL;
  const char* pre;
  const char* con;
  const NumTypeF4_t* w;
  enum Inference_t inf;
  enum Defuzzy_t method;
  unsigned char steps;
  enum FuzzyAnd_t AndOp;
  enum FuzzyOr_t OrOp;
}FuzzyControllerF4_t;

 typedef struct{
    Bool_t **ihit; /* input hit [LingVar][Set] */
    Bool_t **ohit;
    NumTypeF4_t **imv;
    NumTypeF4_t **omv;
    NumTypeF4_t *res;
 }FCMemF4_t;


void FCF4_free(const FuzzyControllerF4_t *fc, FCMemF4_t *v);

void FCF4_calc(const FuzzyControllerF4_t *fc, FCMemF4_t *v, NumTypeF4_t *e, NumTypeF4_t *a);

void FCF4_init(const FuzzyControllerF4_t *fc, FCMemF4_t *v);

#endif

