
#ifndef Fuzzy_F8
#define Fuzzy_F8 Fuzzy_F8

#include "fuzzy_enums.h"
#include "NumType_F8.h"
#include <assert.h>

typedef unsigned char NumOfVal_t;
typedef unsigned char Bool_t;

typedef struct{
  NumOfVal_t                n;
  const NumTypeF8Point_t* p;
}FuzzySetF8_t;


typedef struct{
  NumOfVal_t            n;
  const FuzzySetF8_t*  fs;
  NumTypeF8_t          defaultvalue;
  char                  defaultactive;
}LinguisticOutputVariableF8_t;

typedef struct{
  NumOfVal_t n;
  const FuzzySetF8_t* fs;
}LinguisticInputVariableF8_t;

typedef struct{
  NumOfVal_t nI;
  NumOfVal_t nO;
  NumOfVal_t nR;
  const LinguisticInputVariableF8_t* iL;
  const LinguisticOutputVariableF8_t* oL;
  const char* pre;
  const char* con;
  const NumTypeF8_t* w;
  enum Inference_t inf;
  enum Defuzzy_t method;
  unsigned char steps;
  enum FuzzyAnd_t AndOp;
  enum FuzzyOr_t OrOp;
}FuzzyControllerF8_t;

 typedef struct{
    Bool_t **ihit; /* input hit [LingVar][Set] */
    Bool_t **ohit;
    NumTypeF8_t **imv;
    NumTypeF8_t **omv;
    NumTypeF8_t *res;
 }FCMemF8_t;


void FCF8_free(const FuzzyControllerF8_t *fc, FCMemF8_t *v);

void FCF8_calc(const FuzzyControllerF8_t *fc, FCMemF8_t *v, NumTypeF8_t *e, NumTypeF8_t *a);

void FCF8_init(const FuzzyControllerF8_t *fc, FCMemF8_t *v);

#endif

