#ifndef NumTypeI2
#define NumTypeI2 NumTypeI2

typedef short           NumTypeI2_t;
typedef short           NumeratorI2_t;
typedef unsigned short  DenominatorI2_t;

typedef struct{
  NumTypeI2_t x, y;
}NumTypeI2Point_t;

extern NumTypeI2_t I2One;
extern NumTypeI2_t I2Max;
extern NumTypeI2_t I2Min;
extern NumTypeI2_t I2Factor;

#ifdef __cplusplus
extern "C"{
#endif
  NumTypeI2_t opI2Add (const  NumTypeI2_t f1, const NumTypeI2_t f2);
  NumTypeI2_t opI2Sub (const  NumTypeI2_t f1, const NumTypeI2_t f2);
  NumTypeI2_t opI2Mul (const  NumTypeI2_t f1, const NumTypeI2_t f2);
  NumTypeI2_t opI2Div (const  NumTypeI2_t f1, const NumTypeI2_t f2);
  NumTypeI2_t opI2Neg (const  NumTypeI2_t f1);
  NumTypeI2_t opI2Inv (const  NumTypeI2_t f1);
  int         opI2G   (const  NumTypeI2_t f1, const NumTypeI2_t f2);
  int         opI2GE  (const  NumTypeI2_t f1, const NumTypeI2_t f2);
  int         opI2L   (const  NumTypeI2_t f1, const NumTypeI2_t f2);
  int         opI2LE  (const  NumTypeI2_t f1, const NumTypeI2_t f2);
  int         opI2E   (const  NumTypeI2_t f1, const NumTypeI2_t f2);
  int         opI2NE  (const  NumTypeI2_t f1, const NumTypeI2_t f2);
  NumTypeI2_t opI2Cast (const short c);
  NumTypeI2_t opI2FromFraction(const NumeratorI2_t numerator, const DenominatorI2_t denominator);
  void        opI2ToFraction(const NumTypeI2_t f1, NumeratorI2_t *numerator, DenominatorI2_t *denominator);
  
  void        SetNumTypeI2Format(const unsigned char aIntPart, const unsigned char aFracPart);
#ifdef __cplusplus
}
#endif

#endif
