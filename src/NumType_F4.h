#ifndef NumTypeF4
#define NumTypeF4 NumTypeF4
#include <float.h>

typedef float NumTypeF4_t;

typedef struct{
  NumTypeF4_t x, y;
}NumTypeF4Point_t;

extern const NumTypeF4_t F4One;

#define F4Max FLT_MAX
#define F4Min FLT_MIN

#define opF4Add(f1,f2) (f1+f2)
#define opF4Sub(f1,f2) (f1-f2)
#define opF4Mul(f1,f2) (f1*f2)
#define opF4Div(f1,f2) ((f2!=0)?(f1/f2): ((f1<0)? F4Min : F4Max))
#define opF4Neg(f1)    (-f1)
#define opF4Inv(f1)    ((f1!=0)?(F4One/f1): F4Max)
#define opF4G(f1,f2)   (f1>f2)
#define opF4GE(f1,f2)  (f1>=f2)
#define opF4L(f1,f2)   (f1<f2)
#define opF4LE(f1,f2)  (f1<=f2)
#define opF4E(f1,f2)   (f1==f2)
#define opF4NE(f1,f2)  (f1!=f2)
#define opF4Cast(c)    ((NumTypeF4_t)c)

#endif


