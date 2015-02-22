//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/BlissGMK/physics/physShape.h"
#include "math/mathIO.h"
#include "T3D/BlissGMK/physics/physBody.h"

PhysShape::PhysShape()
{

}

PhysShape::PhysShape(const PhysInfo& physDescr ) : m_physInfo(physDescr) 
{
	mTransformInv = m_physInfo.transform;
	mTransformInv.inverse();
	mPrevActive = true;
	mUserData.setObject( m_physInfo.owner );
}

PhysShape::~PhysShape() 
{

}


void	PhysShape::setTransform(const MatrixF& mat)
{
	MatrixF physTransform = mat*m_physInfo.transform;
	setPhysicTransform(physTransform);
}
MatrixF	PhysShape::getTransform()
{
	MatrixF physTransform;
	getPhysicTransform(physTransform);
	MatrixF res = physTransform*mTransformInv;
	return res;
}


Point3F	PhysShape::getPosition()
{
	MatrixF res = getTransform();

	return res.getPosition();
}

void	PhysShape::setPosition(const Point3F& pos)
{
	MatrixF tr = getTransform();
	tr.setPosition(pos);
	setTransform(tr);
}

QuatF	PhysShape::getRotation()
{
	MatrixF tr = getTransform();
	QuatF q(tr);
	return q;
}
void	PhysShape::setRotation(const QuatF& rot)
{
	MatrixF tr;
	rot.setMatrix(&tr);
	tr.setPosition(getPosition());
	setTransform(tr);
}

void	PhysShape::pack(BitStream* stream)
{
	if (stream->writeFlag(isEnabled()))
	{
		bool active = stream->writeFlag(isActive());
		if (!active)
		{
			bool toInactive = active!=mPrevActive;
			mPrevActive = active;
			if (!stream->writeFlag(toInactive))
				return;
		}
		else
			mPrevActive = true;
				
		mathWrite(*stream, getPosition());
		mathWrite(*stream, getRotation());
		mathWrite(*stream, getForce());
		mathWrite(*stream, getTorque());
		mathWrite(*stream, getLinVelocity());
		mathWrite(*stream, getAngVelocity());
	}
	
}

void	PhysShape::unpack(BitStream* stream)
{
	bool enabled = stream->readFlag();
	if (isEnabled()!=enabled)	
		setEnable(enabled);
	if (enabled)
	{
		bool active = stream->readFlag();
		bool toInactive = false;
		if (!active)
		{
			toInactive = stream->readFlag();
			if (!toInactive)
				return;
		}

		if (!isActive())
			setActive(true);
		QuatF q;
		VectorF vec;
		mathRead(*stream, &vec);
		setPosition(vec);
		mathRead(*stream, &q);
		setRotation(q);
		mathRead(*stream, &vec);
		setForce(vec);
		mathRead(*stream, &vec);
		setTorque(vec);
		mathRead(*stream, &vec);
		setLinVelocity(vec);
		mathRead(*stream, &vec);
		setAngVelocity(vec);
		
		if (toInactive)
			setActive(false);
	}
}

bool PhysShape::castRayLocal(const Point3F &startLocal, const Point3F &endLocal, RayInfo* info)
{
	if (m_physInfo.owner)
	{
		const VectorF& scale = m_physInfo.owner->getScale();
		const MatrixF& objToWorld = m_physInfo.owner->getTransform();
		Point3F start(startLocal);
		Point3F end (endLocal);

		start.convolve(scale);
		end.convolve(scale);
		objToWorld.mulP(start);
		objToWorld.mulP(end);

		bool res = castRay(start,end,info);
		if (res && info)
		{
			info->normal = startLocal - endLocal;
			info->normal.normalize();

		}
		return res;
	}
	return false;
}

//-----------------------------------------------------------------
PhysShapeSoft::PhysShapeSoft()
{
	m_localCOMTransform.identity();	
}

PhysShapeSoft::PhysShapeSoft(const PhysSoftInfo& physSoftInfo ):PhysShape((PhysInfo)physSoftInfo),m_physSoftInfo(physSoftInfo)
{
	m_localCOMTransform.identity();
}

PhysShapeSoft::~PhysShapeSoft() 
{

}
