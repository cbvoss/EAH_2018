#include "NumType_I2.h"

#define INLINE_MACRO 

NumTypeI2_t I2One=0;
NumTypeI2_t I2Max;
NumTypeI2_t I2Min;
NumTypeI2_t I2Factor;

static NumTypeI2_t IntPart;
static NumTypeI2_t FracPart;
static NumTypeI2_t IntMask;
static NumTypeI2_t FracMask;


void SetNumTypeI2Format(const unsigned char aIntPart, const unsigned char aFracPart)
{
  if ( (IntPart==aIntPart)&&(FracPart==aFracPart)) return;
  IntPart=aIntPart;
  FracPart=aFracPart;
  IntMask=((1 << IntPart) -1) << FracPart;
  FracMask=~IntMask;
  I2One=I2Factor=1 << FracPart;
  I2Max=1 << (FracPart+IntPart-1) - 1 ;
  I2Min=-(1 << (FracPart+IntPart-1));
}


INLINE_MACRO NumTypeI2_t Rgchk_I2(short f1)
{
  if (f1>I2Max) {
    return I2Max;
  }else if (f1<I2Min) {
    return I2Min;
  }else return (NumTypeI2_t) f1;
}
NumTypeI2_t opI2Add(const NumTypeI2_t f1, const NumTypeI2_t f2){ return f1+f2; }
NumTypeI2_t opI2Sub(const NumTypeI2_t f1, const NumTypeI2_t f2){ return f1-f2; }
NumTypeI2_t opI2Mul(const NumTypeI2_t f1, const NumTypeI2_t f2){ return Rgchk_I2( ( (long int)f1*f2 ) >> FracPart ); }
NumTypeI2_t opI2Div(const NumTypeI2_t f1, const NumTypeI2_t f2)
{
  if (f2==0) return (f1>0)? I2Max : I2Min;
  return Rgchk_I2( ( (long int)f1<<FracPart)/f2);
}
NumTypeI2_t opI2Neg(const NumTypeI2_t f1)                    { return -f1; }
NumTypeI2_t opI2Inv(const NumTypeI2_t f1)                    { return opI2Div(I2One,f1); }
int       opI2G  (const NumTypeI2_t f1, const NumTypeI2_t f2){ return f1>f2; }
int       opI2GE (const NumTypeI2_t f1, const NumTypeI2_t f2){ return f1>=f2; }
int       opI2L  (const NumTypeI2_t f1, const NumTypeI2_t f2){ return f1<f2; }
int       opI2LE (const NumTypeI2_t f1, const NumTypeI2_t f2){ return f1<=f2; }
int       opI2E  (const NumTypeI2_t f1, const NumTypeI2_t f2){ return f1==f2; }
int       opI2NE (const NumTypeI2_t f1, const NumTypeI2_t f2){ return f1!=f2; }

NumTypeI2_t opI2Cast(const short c)                   
{
  return Rgchk_I2(c*I2One); 
}

NumTypeI2_t opI2FromFraction(const NumeratorI2_t numerator, const DenominatorI2_t denominator)
{
  long int m;
  if (numerator==0) { return 0;}
  m=numerator*I2One;
  m/=denominator;
  return Rgchk_I2(m);
}

void opI2ToFraction(const NumTypeI2_t f1, NumeratorI2_t *numerator, DenominatorI2_t *denominator)
{
  *numerator=f1;
  *denominator=I2One;
  return ;
}




