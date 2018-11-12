#ifndef NumTypeF10
#define NumTypeF10 NumTypeF10
#include <float.h>

typedef long double NumTypeF10_t;


typedef struct{
  NumTypeF10_t x, y;
}NumTypeF10Point_t;

extern const NumTypeF10_t F10One;

#define F10Max LDBL_MAX
#define F10Min LDBL_MIN

#define opF10Add(f1,f2) (f1+f2)
#define opF10Sub(f1,f2) (f1-f2)
#define opF10Mul(f1,f2) (f1*f2)
#define opF10Div(f1,f2) ((f2!=0)?(f1/f2): ((f1<0)? F10Min : F10Max))
#define opF10Neg(f1)    (-f1)
#define opF10Inv(f1)    ((f1!=0)?(F10One/f1): F10Max)
#define opF10G(f1,f2)   (f1>f2)
#define opF10GE(f1,f2)  (f1>=f2)
#define opF10L(f1,f2)   (f1<f2)
#define opF10LE(f1,f2)  (f1<=f2)
#define opF10E(f1,f2)   (f1==f2)
#define opF10NE(f1,f2)  (f1!=f2)
#define opF10Cast(c)    ((NumTypeF10_t)c)

#endif


