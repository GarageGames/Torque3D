// NOTE: methods on the EaseF convenience class

#include "math/mMath.h"
#include "math/mEase.h"
#include "core/strings/stringFunctions.h"

EaseF::EaseF()
{
	mDir = 0;
	mType = 0;
	mParam[0] = mParam[1] = -1.0f;
}

EaseF::EaseF(const EaseF &ease)
{
	this->mDir = ease.mDir;
	this->mType = ease.mType;
	this->mParam[0] = ease.mParam[0];
	this->mParam[1] = ease.mParam[1];
}

EaseF::EaseF(const S32 dir, const S32 type)
{
	this->mDir = dir;
	this->mType = type;
	this->mParam[0] = this->mParam[1] = -1.0f;
}

EaseF::EaseF(const S32 dir, const S32 type, F32 param[2])
{
	this->mDir = dir;
	this->mType = type;
	this->mParam[0] = param[0];
	this->mParam[1] = param[1];
}

void EaseF::set(const S32 dir, const S32 type)
{
	this->mDir = dir;
	this->mType = type;
	this->mParam[0] = this->mParam[1] = -1.0f;
}

void EaseF::set(const S32 dir, const S32 type, F32 param[2])
{
	this->mDir = dir;
	this->mType = type;
	this->mParam[0] = param[0];
	this->mParam[1] = param[1];
}

void EaseF::set(const S32 dir, const S32 type, F32 param0, F32 param1)
{
	this->mDir = dir;
	this->mType = type;
	this->mParam[0] = param0;
	this->mParam[1] = param1;
}

void EaseF::set(const char *s)
{
   dSscanf(s,"%d %d %f %f",&mDir,&mType,&mParam[0],&mParam[1]);
}

F32 EaseF::getValue(F32 t, F32 b, F32 c, F32 d) const
{
	F32 value = 0;

	if (mType == Ease::Linear)
	{	   
		value = mLinearTween(t,b, c, d);
	}
	else if (mType == Ease::Quadratic)
	{
		if (mDir == Ease::In)
		   value = mEaseInQuad(t,b, c, d);
		else if (mDir == Ease::Out)
		   value = mEaseOutQuad(t,b, c, d);
		else if (mDir == Ease::InOut)
		   value = mEaseInOutQuad(t,b, c, d);
	}
	else if (mType == Ease::Cubic)
	{
		if (mDir == Ease::In)
		   value = mEaseInCubic(t,b, c, d);
		else if (mDir == Ease::Out)
		   value = mEaseOutCubic(t,b, c, d);
		else if (mDir == Ease::InOut)
		   value = mEaseInOutCubic(t,b, c, d);
	}
	else if (mType == Ease::Quartic)
	{
		if (mDir == Ease::In)
		   value = mEaseInQuart(t,b, c, d);
		else if (mDir == Ease::Out)
		   value = mEaseOutQuart(t,b, c, d);
		else if (mDir == Ease::InOut)
		   value = mEaseInOutQuart(t,b, c, d);
	}
	else if (mType == Ease::Quintic)
	{
		if (mDir == Ease::In)
		   value = mEaseInQuint(t,b, c, d);
		else if (mDir == Ease::Out)
		   value = mEaseOutQuint(t,b, c, d);
		else if (mDir == Ease::InOut)
		   value = mEaseInOutQuint(t,b, c, d);
	}
	else if (mType == Ease::Sinusoidal)
	{
		if (mDir == Ease::In)
		   value = mEaseInSine(t,b, c, d);
		else if (mDir == Ease::Out)
		   value = mEaseOutSine(t,b, c, d);
		else if (mDir == Ease::InOut)
		   value = mEaseInOutSine(t,b, c, d);
	}
	else if (mType == Ease::Exponential)
	{
		if (mDir == Ease::In)
		   value = mEaseInExpo(t,b, c, d);
		else if (mDir == Ease::Out)
		   value = mEaseOutExpo(t,b, c, d);
		else if (mDir == Ease::InOut)
		   value = mEaseInOutExpo(t,b, c, d);
	}
	else if (mType == Ease::Circular)
	{
		if (mDir == Ease::In)
		   value = mEaseInCirc(t,b, c, d);
		else if (mDir == Ease::Out)
		   value = mEaseOutCirc(t,b, c, d);
		else if (mDir == Ease::InOut)
		   value = mEaseInOutCirc(t,b, c, d);
	}
	else if (mType == Ease::Elastic)
	{
		if (mDir == Ease::In)
		   value = mEaseInElastic(t,b, c, d, mParam[0], mParam[1]);
		else if (mDir == Ease::Out)
		   value = mEaseOutElastic(t,b, c, d, mParam[0], mParam[1]);
		else if (mDir == Ease::InOut)
		   value = mEaseInOutElastic(t,b, c, d, mParam[0], mParam[1]);
	}
	else if (mType == Ease::Back)
	{
		if (mDir == Ease::In)
		   value = mEaseInBack(t,b, c, d, mParam[0]);
		else if (mDir == Ease::Out)
		   value = mEaseOutBack(t,b, c, d, mParam[0]);
		else if (mDir == Ease::InOut)
		   value = mEaseInOutBack(t,b, c, d, mParam[0]);
	}
	else if (mType == Ease::Bounce)
	{
		if (mDir == Ease::In)
		   value = mEaseInBounce(t,b, c, d);
		else if (mDir == Ease::Out)
		   value = mEaseOutBounce(t,b, c, d);
		else if (mDir == Ease::InOut)
		   value = mEaseInOutBounce(t,b, c, d);
	}
	else
	{
		// what ?
	}

	return value;
}

// < pg
