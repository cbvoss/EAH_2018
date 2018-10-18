#include "NumType_F2.h"

NumTypeF2_t F2One=0;
NumTypeF2_t F2Max;
NumTypeF2_t F2Min;
NumTypeF2_t F2Factor;

static NumTypeF2_t   lkF2One=0;
static unsigned char MantBit;
static unsigned char ExpoBit;
static NumTypeF2_t   ExpoMask;
static NumTypeF2_t   MantMask;
static NumTypeF2_t   vMask;
static NumTypeF2_t   Bias;



void F2split(const NumTypeF2_t f1, unsigned short *v, short *e, unsigned short *m)
{
  *m =(f1 & MantMask)+lkF2One;
  *e =((f1 & ExpoMask) >> MantBit) - Bias;  /* e can be negative ! */
  *v = f1 & vMask;
}


NumTypeF2_t F2join (unsigned short v, short e, unsigned short m)
{
  if (m==0) { e=0; return 0;}
  while (m >= (lkF2One<<1)) {
    e+=1;
    m>>=1;
  }
  while (m < lkF2One) {
    e-=1;
    m<<=1;
  }
  if (e > Bias-1 ){
    /* the number is too big; - result is the pos. or neg. maximum */
    return (v | F2Max); 
  }
  if (e < -Bias) {
    return 0;
  }
  m -= lkF2One;
  e = (e + Bias) << MantBit;
  return (v | e | m);
}

NumTypeF2_t opF2Add(const NumTypeF2_t f1, const NumTypeF2_t f2)
{
  short  am, m;
  short  ae, e;
  unsigned short  av, v;

  if (f1==0) return f2;
  if (f2==0) return f1;

  F2split(f1, &v, &e, &m);
  F2split(f2, &av, &ae, &am);
  if (ae>e){
    m=m >> (ae-e);
    e=ae;
  }else if (e>ae) {
    am=am >> (e-ae);
    ae=e;
  }
  if (v!=0) m=-m;
  if (av!=0) am=-am;
  m+=am;
  v=0;
  if (m<0) {
    m=-m;
    v=vMask;
  }
  return F2join(v,e,m);
}

NumTypeF2_t opF2Sub(const NumTypeF2_t f1, const NumTypeF2_t f2)
{
  return opF2Add(f1,opF2Neg(f2));
}


NumTypeF2_t opF2Mul(const NumTypeF2_t f1, const NumTypeF2_t f2)
{
  short am, m;
  short ae, e;
  unsigned short av, v;

  if (f1==0) return 0;
  if (f2==0) return 0;
  F2split(f1,&v,&e,&m);
  F2split(f2,&av,&ae,&am);
  e+=ae;
  if (v!=0) m=-m;
  if (av!=0) am=-am;
  m=(long int)m*am/lkF2One;
  v=0;
  if (m<0){
    m=-m;
    v=vMask;
  }
  return F2join(v,e,m);
}

NumTypeF2_t opF2Div(const NumTypeF2_t f1, const NumTypeF2_t f2)
{
  short  am, m;
  short  ae, e;
  unsigned short av, v;

  if (f2==0) return (f1<0)? F2Min : F2Max;
  if (f1==0) return 0;
  F2split(f1,&v,&e,&m);
  F2split(f2,&av,&ae,&am);
  e-=ae;
  if (v!=0) m=-m;
  if (av!=0) am=-am;
  m=(long int)m*lkF2One/am;
  v=0;
  if (m<0) {
    m=-m;
    v=vMask;
  }
  return F2join(v,e,m);
}

NumTypeF2_t opF2Neg(const NumTypeF2_t f1)
{
  if (f1==0) return 0;
  return (f1 & vMask)? f1 & ~vMask : f1 | vMask;
}

NumTypeF2_t opF2Inv(const NumTypeF2_t f1)
{
  return opF2Div(lkF2One,f1);
}

int opF2G(const NumTypeF2_t f1, const NumTypeF2_t f2)
{
  return opF2Sub(f1,f2)>0 ? 1 : 0 ;
}

int opF2GE(const NumTypeF2_t f1, const NumTypeF2_t f2)
{
  return opF2Sub(f1,f2)>=0 ? 1 : 0 ;
}

int opF2L(const NumTypeF2_t f1, const NumTypeF2_t f2)
{
  return opF2Sub(f1,f2)<0 ? 1 : 0 ;
}

int opF2LE(const NumTypeF2_t f1, const NumTypeF2_t f2)
{
  return opF2Sub(f1,f2)<=0 ? 1 : 0 ;
}

int opF2E(const NumTypeF2_t f1, const NumTypeF2_t f2)
{
  return opF2Sub(f1,f2)==0 ? 1 : 0 ;
}

int opF2NE(const NumTypeF2_t f1, const NumTypeF2_t f2)
{
  return opF2Sub(f1,f2)!=0 ? 1 : 0 ;
}

NumTypeF2_t opF2Cast(const short c)
{
  short m;
  short e=0;
  unsigned short v;

  if (c==0) { return 0;}
  if (c<0){
    m=-c*lkF2One;
    v=vMask;
  }else{
    m=c*lkF2One;
    v=0;
  }
  return F2join(v,e,m);
}

NumTypeF2_t opF2FromFraction(const NumeratorF2_t numerator, const DenominatorF2_t denominator)
{
  long int lm;
  short m;
  unsigned short v;
  short e=0;

  if (numerator==0) { return 0;}
  lm=(numerator<0) ? -numerator*lkF2One : numerator*lkF2One;
  lm/=denominator;
  v=(numerator<0)? vMask : 0;
  while (lm > (lkF2One<<1)) {
    e+=1;
    lm>>=1;
  }
  m=lm;
  return F2join(v,e,m);
}


void opF2ToFraction(const NumTypeF2_t f1, NumeratorF2_t *numerator, DenominatorF2_t *denominator)
{
  short  m;
  short  e;
  unsigned short  v;

  if (f1==0) {
    *numerator=0;
    *denominator=1;
    return ;
  }
  F2split(f1,&v,&e,&m);
  if (e>0){
    *numerator=(v)? -m :m;
    *denominator=lkF2One / (1 << e);
  }else{
    m>>=ExpoBit-1;
    e+=ExpoBit-1;
    *numerator=(v)? -m :m;
    *denominator=(e>0)?lkF2One / (1 << e) : lkF2One * (1 << -e);
  }
  return ;
}

void SetNumTypeF2Format(const unsigned char aMantBit, const unsigned char aExpoBit)
{
  short  m;
  short  e=0;
  unsigned short  v=0;

  ExpoBit=aExpoBit;
  MantBit=aMantBit;
  Bias=1 << (ExpoBit-1);
  ExpoMask=( ((1 << ExpoBit) -1) << MantBit);
  MantMask=((1 << MantBit) -1);
  vMask=(1 << (MantBit + ExpoBit));
  lkF2One= m = 1 << MantBit;
  F2One=F2join(v,e,m);
  F2Max=MantMask | ExpoMask;
  F2Min=vMask | F2Max;
}



