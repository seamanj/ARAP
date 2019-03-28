#ifndef MATH_H
#define MATH_H

#include <math.h>
#include <stdlib.h>
#include <glm/vec3.hpp>
#include <boost/algorithm/clamp.hpp>

#ifdef EPSILON
#undef EPSILON
#endif

#ifdef _EPSILON_
#undef _EPSILON_
#endif

#define _EPSILON_ 1e-6
#define EPSILON _EPSILON_

#define EQ(a, b) ( fabs((a) - (b)) < _EPSILON_ )
#define NEQ(a, b) ( fabs((a) - (b)) > _EPSILON_ )

#define EQF(a, b) ( fabsf((a) - (b)) < _EPSILON_ )
#define NEQF(a, b) ( fabsf((a) - (b)) > _EPSILON_ )

#ifdef MIN
#undef MIN
#endif
//    __typeof__ (X) _X_ = (X);           
//    __typeof__ (Y) _Y_ = (Y);           
#define MIN( _X_, _Y_ )                     \
({                                      \
    ( (_X_<_Y_) ? _X_ : _Y_ );          \
})

#ifdef MAX
#undef MAX
#endif

#define MAX( _X_, _Y_ )                     \                                      \
    ( (_X_>_Y_) ? _X_ : _Y_ );          \
//    __typeof__ (X) _X_ = (X);           
//    __typeof__ (Y) _Y_ = (Y); 

//__typeof__ (VALUE) _V_ = (VALUE);                   \
//__typeof__ (A) _A_ = (A);                           \
//__typeof__ (B) _B_ = (B);  

#define CLAMP( _V_, _A_, _B_ )                          \
({                                                      \
    ((_V_ < _A_) ? _A_ : ((_V_ > _B_) ? _B_ : _V_));    \
})

static inline float urand( float min = 0.f, float max = 1.f )
{
    return min + (float(rand())/float(RAND_MAX))*(max-min);
}

static inline float smoothstep( float value, float edge0, float edge1 )
{
    //float x = CLAMP( (value-edge0)/(edge1-edge0), 0.f, 1.f );
    float x = boost::algorithm::clamp((value-edge0)/(edge1-edge0), 0.f, 1.f);
    return x*x*(3-2*x);
}

static inline float smootherstep( float value, float edge0, float edge1 )
{
    //float x = CLAMP( (value-edge0)/(edge1-edge0), 0.f, 1.f );
    float x = boost::algorithm::clamp((value-edge0)/(edge1-edge0), 0.f, 1.f);
    return x*x*x*(x*(x*6-15)+10);
}

static int majorAxis(glm::vec3 v) { return ( (fabsf(v.x)>fabsf(v.y)) ? ((fabsf(v.x)>fabsf(v.z)) ? 0 : 2) : ((fabsf(v.y)>fabsf(v.z)) ? 1 : 2) ); }



#endif // MATH_H
