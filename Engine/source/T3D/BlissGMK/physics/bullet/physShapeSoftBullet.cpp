//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/BlissGMK/physics/bullet/PhysShapeSoftBullet.h"
#include "scene/sceneObject.h"

PhysShapeSoftBullet::PhysShapeSoftBullet(Physics* phys, const PhysSoftInfo &physDescr) : PhysShapeSoft(physDescr),m_softBody(NULL)
{
	PhysicsBullet* physics = reinterpret_cast<PhysicsBullet*>(phys);
	m_world = static_cast<btSoftRigidDynamicsWorld*>(physics->getWorld());

	AssertFatal(m_physSoftInfo.shapeType == PhysInfo::ST_SOFTMESH, "Failed to create physics shape!");
	
	for(U32 i = 0; i < m_physSoftInfo.physPolyList->mPolyList.size() ; i++)
	{
		ConcretePolyList::Poly& poly = m_physSoftInfo.physPolyList->mPolyList[i];
		for (int i = 0;i<poly.vertexCount-2;i++)
		{
			mIndexBuffer.push_back(m_physSoftInfo.physPolyList->mIndexList[poly.vertexStart]);
			mIndexBuffer.push_back(m_physSoftInfo.physPolyList->mIndexList[poly.vertexStart+i+2]);
			mIndexBuffer.push_back(m_physSoftInfo.physPolyList->mIndexList[poly.vertexStart+i+1]);
		}
	}

	Point3F* vBuffer = m_physSoftInfo.physPolyList->mVertexList.address();
	int  	vNum = m_physSoftInfo.physPolyList->mVertexList.size();
	U32*		iBuffer = mIndexBuffer.address();
	int  	iNum = mIndexBuffer.size();

	MatrixF tr;
	Point3F p;
	m_physSoftInfo.physPolyList->getTransform(&tr,&p);
	tr.inverse();
	VectorF* it = (VectorF*)vBuffer;
	for (int i=0;i<vNum;i++)
	{
		VectorF& vert = *it;
		tr.mulP(vert);	//transform into local space
		it++;
	}

	m_softBody = btSoftBodyHelpers::CreateFromTriMesh(*physics->getSoftBodyWorldInfo()/*m_world->getWorldInfo()*/,	(btScalar*)vBuffer,
		(int*)iBuffer,iNum/3);

	//m_softBody->m_cfg.kDF				=	1.f;
	//m_softBody->m_cfg.kDP				=	1.f;
	//m_softBody->generateBendingConstraints(2);
	//m_softBody->m_cfg.piterations=2;
	//m_softBody->randomizeConstraints();
	//m_softBody->generateClusters(1);

	m_softBody->setTotalMass(m_physInfo.mass,true);
	
	if (m_physSoftInfo.poseMatchKoef>0.f)
	{
		m_softBody->m_cfg.kMT = m_physSoftInfo.poseMatchKoef;//0.05;
		m_softBody->randomizeConstraints();
		m_softBody->setPose(false,true);		
	}

	for(int i = 0;i<m_physSoftInfo.attachPoints.size();i++)
	{
		m_softBody->setMass(getNearestNode(m_physSoftInfo.attachPoints[i]),0);
	}
	
	m_softBody->setUserPointer(&mUserData);

	m_world->addSoftBody(m_softBody);

	m_localCOMTransform.setPosition(getCOM());
	m_localCOMTransform.inverse();
}

PhysShapeSoftBullet::~PhysShapeSoftBullet()
{
	m_world->removeSoftBody(m_softBody);
	delete m_softBody;
	if (m_physSoftInfo.physPolyList)
		delete m_physSoftInfo.physPolyList;
}

void PhysShapeSoftBullet::setPhysicTransform(const MatrixF& tr)
{
	btTransform physTr = matrix4toBt(tr);

	MatrixF currentTr;
	getPhysicTransform(currentTr);
	btTransform currentTrInv = matrix4toBt(currentTr);
	currentTrInv = currentTrInv.inverse();
	btTransform localTr = physTr*currentTrInv;

	for(int i=0,ni=m_softBody->m_nodes.size();i<ni;++i)
	{	
		btSoftBody::Node&	n = m_softBody->m_nodes[i];
		btVector3 newPos = localTr* n.m_x;
		n.m_x =	newPos;
		n.m_q =	n.m_x;
	}

	m_softBody->updateNormals();
	m_softBody->updateBounds();


	if (m_softBody->m_pose.m_bframe)
		m_softBody->updatePose();

	//update com
	MatrixF comInv(true);
	comInv.setPosition(getCOM());
	comInv.inverse();
	m_localCOMTransform = comInv*tr;
}

void PhysShapeSoftBullet::getPhysicTransform(MatrixF & tr)
{
	tr.identity();
	tr.setPosition(getCOM());	

	//add inverse com translation
	tr = tr*m_localCOMTransform;
}

VectorF PhysShapeSoftBullet::getCOM()
{
	btVector3 com(0,0,0);
	if(!(m_softBody->m_pose.m_bframe))
	{
		bool hasStationaryNodes = false;
		btScalar totalMass(0.f);
		for(int i=0,ni=m_softBody->m_nodes.size();i<ni;++i)
		{
			if (m_softBody->m_nodes[i].m_im == 0.f)
			{
				if (!hasStationaryNodes)
				{
					hasStationaryNodes = true;
					com.setZero();
					totalMass = 0.f;
				}
			}
			else if (hasStationaryNodes) 
			{
				//ignore dynamic nodes if there are stationary ones
				continue;
			}
			btScalar m = m_softBody->m_nodes[i].m_im > 0.f ? 1.f/m_softBody->m_nodes[i].m_im : 1.f;
			com+= m_softBody->m_nodes[i].m_x*m;
			totalMass+=m;
		}
		com /= totalMass;
	}
	else
		com = m_softBody->m_pose.m_com;
	return vectorFromBt(com);
}

VectorF PhysShapeSoftBullet::getLinVelocity()
{
	return Point3F::Zero;
}

void PhysShapeSoftBullet::setLinVelocity(const VectorF& vel)
{

}

VectorF PhysShapeSoftBullet::getAngVelocity()
{
	return VectorF::Zero;
}

void PhysShapeSoftBullet::setAngVelocity(const VectorF& vel)
{

}

void PhysShapeSoftBullet::addForce(const VectorF& force)
{
}

void PhysShapeSoftBullet::addForce(const VectorF& forceAdd, const Point3F& pos)
{
	static F32 scaler = .5f;
	static F32 radius = .2f;
	VectorF force = scaler * forceAdd;
	btVector3 f = vectorToBt(force);
	btVector3 p = vectorToBt(pos);
	int idx = 0;
	btScalar minDist = FLT_MAX;
	for(int i=0,ni=m_softBody->m_nodes.size();i<ni;++i)
	{
		btScalar dist = btVector3(p - m_softBody->m_nodes[i].m_x).length2();
		if (dist<minDist)
		{
			idx = i;
			minDist = dist;
		}
/*
		if (dist<radius*radius)
		{
			m_softBody->m_nodes[idx].m_stiffness = 1.f - dist/(radius*radius);		
			m_softBody->addForce(f*(1.f - dist/(radius*radius)),i);
		}
*/		
	}
	m_softBody->addForce(f,idx);
}

void PhysShapeSoftBullet::reset()
{

}


void PhysShapeSoftBullet::setForce(const VectorF& force)
{

}

VectorF  PhysShapeSoftBullet::getForce()
{
	return VectorF::Zero;
}
void  PhysShapeSoftBullet::setTorque(const VectorF& torque)
{

}
VectorF  PhysShapeSoftBullet::getTorque()
{
	return VectorF::Zero;
}

void PhysShapeSoftBullet::setEnable(bool isEnabled)
{
	if(isEnabled)
	{
		m_softBody->forceActivationState(ACTIVE_TAG);
		m_softBody->setCollisionFlags(m_softBody->getCollisionFlags() & ~btCollisionObject::CF_NO_CONTACT_RESPONSE);

	}
	else
	{
		m_softBody->setActivationState(DISABLE_SIMULATION);
		m_softBody->setCollisionFlags(m_softBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	}
}

bool PhysShapeSoftBullet::isEnabled()
{	
	return m_softBody->getActivationState()!=DISABLE_SIMULATION;
}

bool PhysShapeSoftBullet::isActive()
{	
	return m_softBody->getActivationState()!=ISLAND_SLEEPING;
}
void PhysShapeSoftBullet::setActive(bool flg)
{	
	if (flg)
		m_softBody->forceActivationState(ACTIVE_TAG);
	else
		m_softBody->forceActivationState(ISLAND_SLEEPING);
}

size_t PhysShapeSoftBullet::getNearestNode(VectorF pos)
{
	int res = 0;
	btVector3 btPos = vectorToBt(pos);
	btScalar minDist = FLT_MAX;
	for(int i=0,ni=m_softBody->m_nodes.size();i<ni;++i)
	{
		btScalar dist = btVector3(btPos - m_softBody->m_nodes[i].m_x).length2();
		if (dist<minDist)
		{
			res = i;
			minDist = dist;
		}
	}
	return res;
}

int PhysShapeSoftBullet::getNodesNum()
{
	return m_softBody->m_nodes.size();
}
VectorF PhysShapeSoftBullet::getNodePos(int idx)
{
	AssertFatal(idx>=0 && idx<m_softBody->m_nodes.size(),"PhysShapeSoftBullet: wrong node index");
	return vectorFromBt(m_softBody->m_nodes[idx].m_x);
}

bool PhysShapeSoftBullet::castRay(const Point3F &start, const Point3F &end, RayInfo* info)
{
	btVector3 from = vectorToBt(start);
	btVector3 to = vectorToBt(end);
	btSoftBody::sRayCast result;
	if (m_softBody->rayTest(from,to,result))
	{
		if (info)
		{
			info->t        = result.fraction; // finally divide...
			info->setContactPoint( start, end );

			info->normal   = start-end; //force will be applying in direction opposite to normal
			vectorFromBt(m_softBody->m_faces[result.index].m_normal);
			info->normal.normalize();

			info->material = NULL;
			info->object = (SceneObject*)m_physSoftInfo.owner;
		}

		return true;
	}
	return false;
}