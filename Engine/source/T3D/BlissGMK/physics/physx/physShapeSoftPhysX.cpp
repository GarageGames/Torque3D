//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/logickingMechanics/physics/physX/physShapeSoftPhysX.h"
#include "scene/sceneObject.h"
#include "T3D/physics/physx/pxStream.h"

PhysShapeSoftPhysX::PhysShapeSoftPhysX(Physics* phys, const PhysSoftInfo &physDescr) : PhysShapeSoft(physDescr),m_softBody(NULL)
{
	PhysicsPhysX* physics = static_cast<PhysicsPhysX*>(phys);
	m_world = physics->getWorld();
	m_scene = physics->getPhysScene();

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

	//transform into local space
	MatrixF tr;
	Point3F p;
	m_physSoftInfo.physPolyList->getTransform(&tr,&p);
	tr.inverse();
	VectorF* it = (VectorF*)vBuffer;
	for (int i=0;i<vNum;i++)
	{
		VectorF& vert = *it;
		tr.mulP(vert);	
		it++;
		m_initLocalPos.push_back(vert);
	}


	//cooking mesh
	NxClothMeshDesc meshDesc;    
	meshDesc.numVertices = vNum;    
	meshDesc.numTriangles               = iNum/3;   
	meshDesc.pointStrideBytes           = sizeof(NxVec3);    
	meshDesc.triangleStrideBytes        = 3*sizeof(NxU32);    
	meshDesc.points                     = vBuffer;    
	meshDesc.triangles                  = iBuffer;    
	meshDesc.flags                      = 0;

	NxInitCooking();
	
	PxMemStream cooked;	
	if ( NxCookClothMesh( meshDesc, cooked ) )
	{
		m_world->releaseWriteLocks();

		cooked.resetPosition();
		NxClothMesh *clothMesh = gPhysicsSDK->createClothMesh( cooked );

		
		NxMeshData receiveBuffers;
		receiveBuffers.verticesPosBegin = vBuffer;
		receiveBuffers.verticesPosByteStride = sizeof(NxVec3);
		receiveBuffers.maxVertices = vNum;
		m_vertNum = 0;
		receiveBuffers.numVerticesPtr = &m_vertNum;

		NxClothDesc desc;
		
		desc.clothMesh = clothMesh;    
		desc.meshData = receiveBuffers;    
		if (m_physSoftInfo.poseMatchKoef>0.f)
		{
			desc.flags |= NX_CLF_PRESSURE;
			desc.pressure = m_physSoftInfo.poseMatchKoef*5;
		}
	
		
		m_cloth = m_scene->createCloth(desc);
		//m_softBody = m_scene->createSoftBody(desc);

		MatrixF tr;
		VectorF scale;
		m_physSoftInfo.physPolyList->getTransform(&tr,&scale);
		for (int i=0;i<m_physSoftInfo.attachPoints.size();i++)
		{
			VectorF pos = m_physSoftInfo.attachPoints[i];
			//tr.mulP(pos);			
			int vertexIdx = getNearestNode(pos);
			m_attachedVertexIdx.push_back(vertexIdx);
			m_cloth->attachVertexToGlobalPosition(vertexIdx,vectorToPx(m_physSoftInfo.physPolyList->mVertexList[vertexIdx]));
		}

		m_cloth->userData = &mUserData ;

		m_localCOMTransform.setPosition(getCOM());
		m_localCOMTransform.inverse();		
	}

	NxCloseCooking();
}

PhysShapeSoftPhysX::~PhysShapeSoftPhysX()
{
	m_world->releaseWriteLock();
	m_cloth->userData = NULL;
	m_scene->releaseCloth(*m_cloth);
	m_cloth = NULL;
	if (m_physSoftInfo.physPolyList)
		delete m_physSoftInfo.physPolyList;
}

void PhysShapeSoftPhysX::setPhysicTransform(const MatrixF& tr)
{
	//find relative transform
/*	MatrixF relTr;
	getPhysicTransform(relTr);
	relTr.inverse();
	relTr = tr*relTr;
*/

	NxMat34 trPx = matrixToPx(tr);
	
	int attachedNum = m_attachedVertexIdx.size();
	if (attachedNum>0)
	{
		for (int i=0; i<attachedNum; i++)
			m_cloth->freeVertex(m_attachedVertexIdx[i]);
	}

	int vertNum = m_physSoftInfo.physPolyList->mVertexList.size();
	for (int i=0; i<vertNum; i++)
	{
		/*NxVec3 gPos = m_cloth->getPosition(i);
		gPos = relTrPx*gPos;*/
		NxVec3 gPos = trPx*vectorToPx(m_initLocalPos[i]);
		m_cloth->setPosition(gPos,i);
		m_physSoftInfo.physPolyList->mVertexList[i] = vectorFromPx(gPos);
	}

	if (attachedNum>0)
	{
		for (int i=0; i<attachedNum; i++)
		{
			m_cloth->attachVertexToGlobalPosition(m_attachedVertexIdx[i],vectorToPx(m_physSoftInfo.physPolyList->mVertexList[m_attachedVertexIdx[i]]));/*m_cloth->getPosition(m_attachedVertexIdx[i])*/
		}
	}

	//rotate local com
	MatrixF rotMat = tr;
	rotMat.inverse();
	rotMat.setPosition(m_localCOMTransform.getPosition());
	m_localCOMTransform = rotMat;

	addForce(VectorF(0.f,-0.002f,0.f),VectorF::Zero);
}

void PhysShapeSoftPhysX::getPhysicTransform(MatrixF & tr)
{
	tr.identity();
	tr.setPosition(getCOM());	
	tr = tr*m_localCOMTransform;
}

Point3F PhysShapeSoftPhysX::getCOM()
{
	Point3F com(0.f,0.f,0.f);
	if (m_cloth)
	{
		int vertNum =0;
		if ((vertNum=m_attachedVertexIdx.size())>0)
		{
			for(int i=0;i<vertNum;i++)
			{
				com+= vectorFromPx(m_cloth->getPosition(m_attachedVertexIdx[i]));//m_physSoftInfo.physPolyList->mVertexList[m_attachedVertexIdx[i]];
			}
		}
		else
		{
			vertNum = m_physSoftInfo.physPolyList->mVertexList.size();
			for(int i=0;i<vertNum;i++)
			{
				com+=vectorFromPx(m_cloth->getPosition(i));//m_physSoftInfo.physPolyList->mVertexList[i];
			}
		}
		com /= vertNum;
	}

	return com;
}


VectorF PhysShapeSoftPhysX::getLinVelocity()
{
	return Point3F::Zero;
}

void PhysShapeSoftPhysX::setLinVelocity(const VectorF& vel)
{

}

VectorF PhysShapeSoftPhysX::getAngVelocity()
{
	return VectorF::Zero;
}

void PhysShapeSoftPhysX::setAngVelocity(const VectorF& vel)
{

}

void PhysShapeSoftPhysX::addForce(const VectorF& force)
{

}

void PhysShapeSoftPhysX::addForce(const VectorF& forceAdd, const Point3F& pos)
{
	static F32 scaler = 2.f;
	VectorF force = scaler * forceAdd;
	NxVec3 f = vectorToPx(force);
	
	m_cloth->addForceAtVertex(f,getNearestNode(pos));
	
}

void PhysShapeSoftPhysX::reset()
{

}


void PhysShapeSoftPhysX::setForce(const VectorF& force)
{

}

VectorF  PhysShapeSoftPhysX::getForce()
{
	return VectorF::Zero;
}
void  PhysShapeSoftPhysX::setTorque(const VectorF& torque)
{

}
VectorF  PhysShapeSoftPhysX::getTorque()
{
	return VectorF::Zero;
}

void PhysShapeSoftPhysX::setEnable(bool isEnabled)
{
	/*
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
	*/
}

bool PhysShapeSoftPhysX::isEnabled()
{	
	return true;
}

bool PhysShapeSoftPhysX::isActive()
{	
	return true;
}
void PhysShapeSoftPhysX::setActive(bool flg)
{	
	
}

size_t PhysShapeSoftPhysX::getNearestNode(VectorF pos)
{
	int res = 0;
	NxVec3 nxpos = vectorToPx(pos);
	NxF32 minDist = FLT_MAX;
	NxU32 numNodes = m_physSoftInfo.physPolyList->mVertexList.size();
	for(int i=0,ni=numNodes;i<ni;++i)
	{
		NxF32 dist = vectorToPx(m_physSoftInfo.physPolyList->mVertexList[i]).distanceSquared(nxpos);
		if (dist<minDist)
		{
			res = i;
			minDist = dist;
		}
	}
	return res;
}

int PhysShapeSoftPhysX::getNodesNum()
{
	return m_physSoftInfo.physPolyList->mVertexList.size();
}
VectorF PhysShapeSoftPhysX::getNodePos(int idx)
{
	AssertFatal(idx>=0 && idx<m_physSoftInfo.physPolyList->mVertexList.size(),"PhysShapeSoftPhysX: wrong node index");
	return m_physSoftInfo.physPolyList->mVertexList[idx];
}

bool PhysShapeSoftPhysX::castRay(const Point3F &start, const Point3F &end, RayInfo* info)
{
	NxVec3 s = vectorToPx(start);
	NxVec3 e = vectorToPx(end);
	NxRay ray;
	ray.orig = s;
	ray.dir = e-s;
	ray.dir.normalize();

	NxVec3 hit;
	NxU32 vertID;
	if (m_cloth->raycast(ray,hit,vertID))
	{
		F32 fraction = (F32)(hit-s).magnitude()/(e-s).magnitude();
		if (fraction>1.f)
			return false;
		
		if (info)
		{
			info->point = vectorFromPx(hit);
			info->t     = fraction;
			
			info->material = NULL;
			info->object = (SceneObject*)m_physSoftInfo.owner;
		}

		return true;
	}
	return false;
}
