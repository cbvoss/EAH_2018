#ifndef NumTypeF2
#define NumTypeF2 NumTypeF2

typedef signed short    NumTypeF2_t;
typedef short           NumeratorF2_t;
typedef unsigned short  DenominatorF2_t;

typedef struct{
  NumTypeF2_t x, y;
}NumTypeF2Point_t;

extern NumTypeF2_t F2One;
extern NumTypeF2_t F2Max;
extern NumTypeF2_t F2Min;

#ifdef __cplusplus
extern "C"{
#endif
  NumTypeF2_t opF2Add (const  NumTypeF2_t f1, const NumTypeF2_t f2);
  NumTypeF2_t opF2Sub (const  NumTypeF2_t f1, const NumTypeF2_t f2);
  NumTypeF2_t opF2Mul (const  NumTypeF2_t f1, const NumTypeF2_t f2);
  NumTypeF2_t opF2Div (const  NumTypeF2_t f1, const NumTypeF2_t f2);
  NumTypeF2_t opF2Neg (const  NumTypeF2_t f1);
  NumTypeF2_t opF2Inv (const  NumTypeF2_t f1);
  int         opF2G   (const  NumTypeF2_t f1, const NumTypeF2_t f2);
  int         opF2GE  (const  NumTypeF2_t f1, const NumTypeF2_t f2);
  int         opF2L   (const  NumTypeF2_t f1, const NumTypeF2_t f2);
  int         opF2LE  (const  NumTypeF2_t f1, const NumTypeF2_t f2);
  int         opF2E   (const  NumTypeF2_t f1, const NumTypeF2_t f2);
  int         opF2NE  (const  NumTypeF2_t f1, const NumTypeF2_t f2);
  NumTypeF2_t opF2Cast(const  short c);

  NumTypeF2_t opF2FromFraction  (const NumeratorF2_t numerator, const DenominatorF2_t denominator);
  void        opF2ToFraction    (const NumTypeF2_t f1, NumeratorF2_t *numerator, DenominatorF2_t *denominator);
  void        SetNumTypeF2Format(const unsigned char anF2ManBit, const unsigned char anF2ExpBit);
#ifdef __cplusplus
}
#endif


#endif
