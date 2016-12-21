/*
  ROBERT PENNER'S MOST EXCELLENT EASING METHODS - ported to Torque C++ by Paul Dana

  Easing Equations v1.5
  May 1, 2003
  (c) 2003 Robert Penner, all rights reserved. 
  This work is subject to the terms in http://www.robertpenner.com/easing_terms_of_use.html.  
  
  These tweening functions provide different flavors of 
  math-based motion under a consistent API. 
  
  Types of easing:
  
	  Linear
	  Quadratic
	  Cubic
	  Quartic
	  Quintic
	  Sinusoidal
	  Exponential
	  Circular
	  Elastic
	  Back
	  Bounce

  Changes:
  1.5 - added bounce easing
  1.4 - added elastic and back easing
  1.3 - tweaked the exponential easing functions to make endpoints exact
  1.2 - inline optimizations (changing t and multiplying in one step)--thanks to Tatsuo Kato for the idea
  
  Discussed in Chapter 7 of 
  Robert Penner's Programming Macromedia Flash MX
  (including graphs of the easing equations)
  
  http://www.robertpenner.com/profmx
  http://www.amazon.com/exec/obidos/ASIN/0072223561/robertpennerc-20
*/

#ifndef _MEASE_H_
#define _MEASE_H_

// the ease methods below all are static and take atomic types as params
// so they are the most generally useful. for convenience, define here
// a type that can contain all the params needed for below to make 
// data structures that use these methods cleaner...
//------------------------------------------------------------------------------
class Ease
{
   //-------------------------------------- Public data
  public:
	enum enumDirection
	{
	   InOut=0,
	   In,
	   Out
	};

	enum enumType
	{
	  Linear=0,
	  Quadratic,
	  Cubic,
	  Quartic,
	  Quintic,
	  Sinusoidal,
	  Exponential,
	  Circular,
	  Elastic,
	  Back,
	  Bounce,
	};
};

class EaseF : public Ease
{
   //-------------------------------------- Public data
  public:
   S32 dir;       // inout, in, out
   S32 type;      // linear, etc...
   F32 param[2];  // optional params

   //-------------------------------------- Public interface
  public:
   EaseF();
   EaseF(const EaseF &ease);
   EaseF(const S32 dir, const S32 type);
   EaseF(const S32 dir, const S32 type, F32 param[2]);

   //-------------------------------------- Non-math mutators and misc functions
   void set(const S32 dir, const S32 type);
   void set(const S32 dir, const S32 type, F32 param[2]);
   void set(const S32 dir, const S32 type, F32 param0, F32 param1);
   void set(const char *s);

   F32 getValue(F32 t, F32 b, F32 c, F32 d) const;
   F32 getUnitValue(F32 t, bool noExtrapolation) const
   {
       F32 v = getValue(t,0.0f,1.0f,1.0f);
       if (noExtrapolation)
           v = mClampF(v,0.0f,1.0f);
       return v;
   }
   F32 getUnitValue(F32 t) const
   {
       return getValue(t,0.0f,1.0f,1.0f);
   }
};


// simple linear tweening - no easing
// t: current time, b: beginning value, c: change in value, d: duration
inline F32 mLinearTween(F32 t, F32 b, F32 c, F32 d) 
{
	return c*t/d + b;
}


 ///////////// QUADRATIC EASING: t^2 ///////////////////

// quadratic easing in - accelerating from zero velocity
// t: current time, b: beginning value, c: change in value, d: duration
// t and d can be in frames or seconds/milliseconds
inline F32 mEaseInQuad(F32 t, F32 b, F32 c, F32 d) 
{
	t /= d;
	return c*t*t + b;
};

// quadratic easing out - decelerating to zero velocity
inline F32 mEaseOutQuad(F32 t, F32 b, F32 c, F32 d) 
{
	t /= d;
	return -c * t*(t-2) + b;
};

// quadratic easing in/out - acceleration until halfway, then deceleration
inline F32 mEaseInOutQuad(F32 t, F32 b, F32 c, F32 d) 
{
	t /= d/2;
	if (t < 1) 
      return c/2*t*t + b;
	t--;
	return -c/2 * (t*(t-2) - 1) + b;
};

 ///////////// CUBIC EASING: t^3 ///////////////////////

// cubic easing in - accelerating from zero velocity
// t: current time, b: beginning value, c: change in value, d: duration
// t and d can be frames or seconds/milliseconds
inline F32 mEaseInCubic(F32 t, F32 b, F32 c, F32 d) 
{
	t /= d;
	return c*t*t*t + b;
};

// cubic easing out - decelerating to zero velocity
inline F32 mEaseOutCubic(F32 t, F32 b, F32 c, F32 d) 
{
	t /= d;
	t--;
	return c*(t*t*t + 1) + b;
};

// cubic easing in/out - acceleration until halfway, then deceleration
inline F32 mEaseInOutCubic(F32 t, F32 b, F32 c, F32 d) 
{
	t /= d/2;
	if (t < 1) 
      return c/2*t*t*t + b;
	t -= 2;
	return c/2*(t*t*t + 2) + b;
};


 ///////////// QUARTIC EASING: t^4 /////////////////////

// quartic easing in - accelerating from zero velocity
// t: current time, b: beginning value, c: change in value, d: duration
// t and d can be frames or seconds/milliseconds
inline F32 mEaseInQuart(F32 t, F32 b, F32 c, F32 d) 
{
	t /= d;
	return c*t*t*t*t + b;
};

// quartic easing out - decelerating to zero velocity
inline F32 mEaseOutQuart(F32 t, F32 b, F32 c, F32 d) 
{
	t /= d;
	t--;
	return -c * (t*t*t*t - 1) + b;
};

// quartic easing in/out - acceleration until halfway, then deceleration
inline F32 mEaseInOutQuart(F32 t, F32 b, F32 c, F32 d) 
{
	t /= d/2;
	if (t < 1) 
      return c/2*t*t*t*t + b;
	t -= 2;
	return -c/2 * (t*t*t*t - 2) + b;
};


 ///////////// QUINTIC EASING: t^5  ////////////////////

// quintic easing in - accelerating from zero velocity
// t: current time, b: beginning value, c: change in value, d: duration
// t and d can be frames or seconds/milliseconds
inline F32 mEaseInQuint(F32 t, F32 b, F32 c, F32 d) 
{
	t /= d;
	return c*t*t*t*t*t + b;
};

// quintic easing out - decelerating to zero velocity
inline F32 mEaseOutQuint(F32 t, F32 b, F32 c, F32 d) 
{
	t /= d;
	t--;
	return c*(t*t*t*t*t + 1) + b;
};

// quintic easing in/out - acceleration until halfway, then deceleration
inline F32 mEaseInOutQuint(F32 t, F32 b, F32 c, F32 d) 
{
	t /= d/2;
	if (t < 1) 
      return c/2*t*t*t*t*t + b;
	t -= 2;
	return c/2*(t*t*t*t*t + 2) + b;
};



 ///////////// SINUSOIDAL EASING: sin(t) ///////////////

// sinusoidal easing in - accelerating from zero velocity
// t: current time, b: beginning value, c: change in position, d: duration
inline F32 mEaseInSine(F32 t, F32 b, F32 c, F32 d) 
{
	return -c * mCos(t/d * (M_PI_F/2)) + c + b;
};

// sinusoidal easing out - decelerating to zero velocity
inline F32 mEaseOutSine(F32 t, F32 b, F32 c, F32 d) 
{
	return c * mSin(t/d * (M_PI_F/2)) + b;
};

// sinusoidal easing in/out - accelerating until halfway, then decelerating
inline F32 mEaseInOutSine(F32 t, F32 b, F32 c, F32 d) 
{
	return -c/2 * (mCos(M_PI_F*t/d) - 1) + b;
};


 ///////////// EXPONENTIAL EASING: 2^t /////////////////

// exponential easing in - accelerating from zero velocity
// t: current time, b: beginning value, c: change in position, d: duration
inline F32 mEaseInExpo(F32 t, F32 b, F32 c, F32 d) 
{
	return c * mPow( 2, 10 * (t/d - 1) ) + b;
};

// exponential easing out - decelerating to zero velocity
inline F32 mEaseOutExpo(F32 t, F32 b, F32 c, F32 d) 
{
	return c * ( -mPow( 2, -10 * t/d ) + 1 ) + b;
};

// exponential easing in/out - accelerating until halfway, then decelerating
inline F32 mEaseInOutExpo(F32 t, F32 b, F32 c, F32 d) 
{
	t /= d/2;
	if (t < 1) 
      return c/2 * mPow( 2, 10 * (t - 1) ) + b;
	t--;
	return c/2 * ( -mPow( 2, -10 * t) + 2 ) + b;
};


 /////////// CIRCULAR EASING: sqrt(1-t^2) //////////////

// circular easing in - accelerating from zero velocity
// t: current time, b: beginning value, c: change in position, d: duration
inline F32 mEaseInCirc (F32 t, F32 b, F32 c, F32 d) 
{
	t/=d;
	return -c * (mSqrt(1 - (t)*t) - 1) + b;
};

// circular easing out - decelerating to zero velocity
inline F32 mEaseOutCirc (F32 t, F32 b, F32 c, F32 d) 
{
	t/=d;
	t--;
	return c * mSqrt(1 - (t)*t) + b;
};

// circular easing in/out - acceleration until halfway, then deceleration
inline F32 mEaseInOutCirc(F32 t, F32 b, F32 c, F32 d) 
{
	if ((t/=d/2) < 1) 
      return -c/2 * (mSqrt(1 - t*t) - 1) + b;
	t-=2;
	return c/2 * (mSqrt(1 - (t)*t) + 1) + b;
};


 /////////// ELASTIC EASING: exponentially decaying sine wave  //////////////

// t: current time, b: beginning value, c: change in value, d: duration, a: amplitude (optional), p: period (optional)
// t and d can be in frames or seconds/milliseconds

inline F32 mEaseInElastic(F32 t, F32 b, F32 c, F32 d, F32 a, F32 p) 
{
	if (t==0) 
      return b;  
   
   F32 dt = t /= d;
   if (dt == 1)
      return b+c;  
   
   if (p<=0) 
      p=d*.3f;

	F32 s;
	if (a < mFabs(c))
	{
	   a=c; 
      s=p/4;
	}
	else 
      s = p/(2*M_PI_F) * mAsin (c/a);

   t -= 1;
	return -(a*mPow(2,10*t) * mSin( (t*d-s)*(2*M_PI_F)/p )) + b;
};

inline F32 mEaseOutElastic(F32 t, F32 b, F32 c, F32 d, F32 a, F32 p) 
{
	if (t==0) 
      return b; 

   F32 dt = t /= d;
   if (dt == 1)
      return b+c; 

   if (p<=0) 
      p=d*.3f;

	F32 s;
	if (a < mFabs(c))
	{
	   a=c; 
      s=p/4;
	}
	else 
      s = p/(2*M_PI_F) * mAsin (c/a);

	return a*mPow(2,-10*t) * mSin( (t*d-s)*(2*M_PI_F)/p ) + c + b;
};

inline F32 mEaseInOutElastic(F32 t, F32 b, F32 c, F32 d, F32 a, F32 p) 
{
	if (t==0) 
      return b;  
   
   F32 dt = t /= d / 2;
   if (dt == 2)
      return b+c;  
   
   if (p<=0) 
      p=d*(.3f*1.5f);

	F32 s;
	if (a < mFabs(c))
	{
	   a=c; 
      s=p/4;
	}
	else 
      s = p/(2*M_PI_F) * mAsin (c/a);

   if (t < 1)
   {
      t -= 1;
      return -.5f*(a*mPow(2, 10 * t) * mSin((t*d - s)*(2 * M_PI_F) / p)) + b;
   }

   t -= 1;
	return a*mPow(2,-10*t) * mSin( (t*d-s)*(2*M_PI_F)/p )*.5f + c + b;
};


 /////////// BACK EASING: overshooting cubic easing: (s+1)*t^3 - s*t^2  //////////////

// back easing in - backtracking slightly, then reversing direction and moving to target
// t: current time, b: beginning value, c: change in value, d: duration, s: overshoot amount (optional)
// t and d can be in frames or seconds/milliseconds
// s controls the amount of overshoot: higher s means greater overshoot
// s has a default value of 1.70158, which produces an overshoot of 10 percent
// s==0 produces cubic easing with no overshoot
inline F32 mEaseInBack(F32 t, F32 b, F32 c, F32 d, F32 s) 
{
	if (s < 0) 
      s = 1.70158f;

   F32 td = t /= d;
   return c*td*t*((s + 1)*t - s) + b;
};

// back easing out - moving towards target, overshooting it slightly, then reversing and coming back to target
inline F32 mEaseOutBack(F32 t, F32 b, F32 c, F32 d, F32 s) 
{
	if (s < 0) 
      s = 1.70158f;

   F32 td = t / d - 1;
   t = td;
   return c*(td*t*((s + 1)*t + s) + 1) + b;
};

// back easing in/out - backtracking slightly, then reversing direction and moving to target,
// then overshooting target, reversing, and finally coming back to target
inline F32 mEaseInOutBack(F32 t, F32 b, F32 c, F32 d, F32 s) 
{
	if (s < 0) 
      s = 1.70158f; 

   F32 td = t /= d / 2;
   if (td < 1)
   {
      s *= 1.525f;
      return c / 2 * (t*t*((s + 1)*t - s)) + b;
   }

   s *= 1.525f;
   t -= 2;
	return c/2*(t*t*((s+1)*t + s) + 2) + b;
};


 /////////// BOUNCE EASING: exponentially decaying parabolic bounce  //////////////

// bounce easing out
inline F32 mEaseOutBounce(F32 t, F32 b, F32 c, F32 d) 
{
	if ((t/=d) < (1/2.75f)) 
   {
		return c*(7.5625f*t*t) + b;
	} 
   else if (t < (2/2.75)) 
   {
      t -= 1.5f / 2.75f;
		return c*(7.5625f*t*t + .75f) + b;
	} 
   else if (t < (2.5/2.75)) 
   {
      t -= 2.25f / 2.75f;
		return c*(7.5625f*t*t + .9375f) + b;
	} 
   else 
   {
      t -= 2.625f / 2.75f;
		return c*(7.5625f*t*t + .984375f) + b;
	}
};

// bounce easing in
// t: current time, b: beginning value, c: change in position, d: duration
inline F32 mEaseInBounce(F32 t, F32 b, F32 c, F32 d) 
{
	return c - mEaseOutBounce (d-t, 0, c, d) + b;
};

// bounce easing in/out
inline F32 mEaseInOutBounce(F32 t, F32 b, F32 c, F32 d) 
{
	if (t < d/2) 
      return mEaseInBounce (t*2, 0, c, d) * .5f + b;

	return mEaseOutBounce (t*2-d, 0, c, d) * .5f + c*.5f + b;
};

#endif // _MEASE_H_
