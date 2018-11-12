#ifndef NumTypeI4
#define NumTypeI4 NumTypeI4

typedef long int NumTypeI4_t;
typedef long int NumeratorI4_t;
typedef unsigned long int DenominatorI4_t;

typedef struct{
  NumTypeI4_t x, y;
}NumTypeI4Point_t;

extern NumTypeI4_t I4One;
extern NumTypeI4_t I4Max;
extern NumTypeI4_t I4Min;
extern NumTypeI4_t I4Factor;

#ifdef __cplusplus
extern "C"{
#endif
  NumTypeI4_t opI4Add (const NumTypeI4_t f1, const NumTypeI4_t f2);
  NumTypeI4_t opI4Sub (const NumTypeI4_t f1, const NumTypeI4_t f2);
  NumTypeI4_t opI4Mul (const NumTypeI4_t f1, const NumTypeI4_t f2);
  NumTypeI4_t opI4Div (const NumTypeI4_t f1, const NumTypeI4_t f2);
  NumTypeI4_t opI4Neg (const NumTypeI4_t f1);
  NumTypeI4_t opI4Inv (const NumTypeI4_t f1);
  int         opI4G   (const NumTypeI4_t f1, const NumTypeI4_t f2);
  int         opI4GE  (const NumTypeI4_t f1, const NumTypeI4_t f2);
  int         opI4L   (const NumTypeI4_t f1, const NumTypeI4_t f2);
  int         opI4LE  (const NumTypeI4_t f1, const NumTypeI4_t f2);
  int         opI4E   (const NumTypeI4_t f1, const NumTypeI4_t f2);
  int         opI4NEy (const NumTypeI4_t f1, const NumTypeI4_t f2);
  NumTypeI4_t opI4Cast(const long int c);
  NumTypeI4_t opI4FromFraction(const NumeratorI4_t numerator, const DenominatorI4_t denominator);
  void        opI4ToFraction(const NumTypeI4_t f1, NumeratorI4_t *numerator, DenominatorI4_t *denominator);
  void        SetNumTypeI4Format(const unsigned char aIntPart, const unsigned char aFracPart);
#ifdef __cplusplus
}
#endif

#endif
