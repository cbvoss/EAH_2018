#include "NumType_I4.h"

#define INLINE_MACRO 

NumTypeI4_t I4One=0;
NumTypeI4_t I4Max;
NumTypeI4_t I4Min;
NumTypeI4_t I4Factor;

static NumTypeI4_t IntPart;
static NumTypeI4_t FracPart;
static NumTypeI4_t IntMask;
static NumTypeI4_t FracMask;


void SetNumTypeI4Format(const unsigned char aIntPart, const unsigned char aFracPart)
{
  if ( (IntPart==aIntPart)&&(FracPart==aFracPart)) return;
  IntPart=aIntPart;
  FracPart=aFracPart;
  IntMask=((1L << IntPart) -1) << FracPart;
  FracMask=~IntMask;
  I4One=I4Factor=1L << FracPart;
  I4Max=1L << (FracPart+IntPart-1) - 1 ;
  I4Min=-(1L << (FracPart+IntPart-1));
}


INLINE_MACRO NumTypeI4_t Rgchk_I4(long int f1)
{
  if (f1>I4Max) {
    return I4Max;
  }else if (f1<I4Min) {
    return I4Min;
  }else return (NumTypeI4_t) f1;
}
NumTypeI4_t opI4Add(const NumTypeI4_t f1, const NumTypeI4_t f2){ return f1+f2; }
NumTypeI4_t opI4Sub(const NumTypeI4_t f1, const NumTypeI4_t f2){ return f1-f2; }

NumTypeI4_t opI4Mul(const NumTypeI4_t f1, const NumTypeI4_t f2)
{ long int zwll;
  NumTypeI4_t  f1g, f1f, f2g, f2f;
  if(f1<0){
    f1g=(-f1) >> FracPart;
    f1f=(-f1) &  FracMask;
  }else{
    f1g= f1 >> FracPart;
    f1f= f1 &  FracMask;
  }
  if(f2<0){
    f2g=(-f2) >> FracPart;
    f2f=(-f2) &  FracMask;
  }else{
    f2g= f2 >> FracPart;
    f2f= f2 &  FracMask;
  }
  zwll=Rgchk_I4(f1g*f2f);
  zwll=Rgchk_I4(zwll+f2g*f1f);
  zwll=Rgchk_I4(zwll+((NumTypeI4_t)(f1f*f2f)>>FracPart));
  zwll=Rgchk_I4(zwll+((NumTypeI4_t)(f1g*f2g)<<FracPart));
  return ((f1^f2)<0)? -zwll : zwll;
}


NumTypeI4_t opI4Div(const NumTypeI4_t f1, const NumTypeI4_t f2)
{ int i,v;
  long int  lkf1=f1;
  NumTypeI4_t  lkf2=f2;
  if ((f1^f2)<0) v=-1; else v=1;
  if (f2==0) return (f1<0)? I4Min : I4Max;
  if (f1<0) lkf1=-f1;
  if (f2<0) lkf2=-f2;

  if (lkf2&0xffff)
    for(i=0;!(lkf2&0x01);i++) lkf2>>=1;
  else {
    for(i=16,lkf2>>=16;!(lkf2&0x01);i++) lkf2>>=1;
  }

  if(lkf1!=0){
    for(;(!(lkf1&0x40000000L))&&(i<=32);lkf1<<=1,i++);
  }
  if (i<FracPart){
    i=FracPart-i;
    return (v==1)?Rgchk_I4(lkf1/lkf2<<i) : -Rgchk_I4(lkf1/lkf2<<i);
  }else{
    i-=FracPart;
    return (v==1)?Rgchk_I4(lkf1/lkf2>>i) : -Rgchk_I4(lkf1/lkf2>>i);
  }
}

NumTypeI4_t opI4Neg(const NumTypeI4_t f1)                    { return -f1; }
NumTypeI4_t opI4Inv(const NumTypeI4_t f1)                    { return opI4Div(I4One,f1); }
int         opI4G  (const NumTypeI4_t f1, const NumTypeI4_t f2){ return f1>f2; }
int         opI4GE (const NumTypeI4_t f1, const NumTypeI4_t f2){ return f1>=f2; }
int         opI4L  (const NumTypeI4_t f1, const NumTypeI4_t f2){ return f1<f2; }
int         opI4LE (const NumTypeI4_t f1, const NumTypeI4_t f2){ return f1<=f2; }
int         opI4E  (const NumTypeI4_t f1, const NumTypeI4_t f2){ return f1==f2; }
int         opI4NE (const NumTypeI4_t f1, const NumTypeI4_t f2){ return f1!=f2; }

NumTypeI4_t opI4Cast(const long int c)
{ 
  return Rgchk_I4(c*I4One);
}

NumTypeI4_t opI4FromFraction(const NumeratorI4_t numerator, const DenominatorI4_t denominator)
{
  long int m;
  if (numerator==0) { return 0;}
  m=numerator*I4One;
  m/=denominator;
  return Rgchk_I4(m);
}

void opI4ToFraction(const NumTypeI4_t f1, NumeratorI4_t *numerator, DenominatorI4_t *denominator)
{
  *numerator=f1;
  *denominator=I4One;
  return ;
}





