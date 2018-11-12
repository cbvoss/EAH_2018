#ifndef NumTypeF8
#define NumTypeF8 NumTypeF8
#include <float.h>

typedef double NumTypeF8_t;

typedef struct{
  NumTypeF8_t x, y;
}NumTypeF8Point_t;

extern const NumTypeF8_t F8One;

#define F8Max DBL_MAX
#define F8Min DBL_MIN

#define opF8Add(f1,f2) (f1+f2)
#define opF8Sub(f1,f2) (f1-f2)
#define opF8Mul(f1,f2) (f1*f2)
#define opF8Div(f1,f2) ((f2!=0)? (f1/f2): ((f1<0)? F8Min : F8Max))
#define opF8Neg(f1)    (-f1)
#define opF8Inv(f1)    ((f1!=0)?(F8One/f1): F8Max)
#define opF8G(f1,f2)   (f1>f2)
#define opF8GE(f1,f2)  (f1>=f2)
#define opF8L(f1,f2)   (f1<f2)
#define opF8LE(f1,f2)  (f1<=f2)
#define opF8E(f1,f2)   (f1==f2)
#define opF8NE(f1,f2)  (f1!=f2)
#define opF8Cast(c)    ((NumTypeF8_t)c)

#endif


