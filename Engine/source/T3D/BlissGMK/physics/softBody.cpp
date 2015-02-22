//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/BlissGMK/physics/physics.h"
#include "T3D/BlissGMK/physics/softBody.h"
#include "core/stream/bitStream.h"
#include "math/mathIO.h"
#include "console/consoleTypes.h"
#include "ts/tsShapeInstance.h"
#include "scene/sceneRenderState.h"


IMPLEMENT_CO_DATABLOCK_V1(SoftBodyData);
IMPLEMENT_CO_NETOBJECT_V1(SoftBody);

extern bool gFreezeSim;

SoftBodyData::SoftBodyData()
{
	mShapeType=PhysInfo::ST_SOFTMESH;
	poseMatchKoef = 0.f;
	attachedPointsNum = 0;
	for (int i = 0; i < MaxAttachedPoints; i++) 
		attachedPoints[i].set(0.f, 0.f, 0.f);
}

SoftBodyData::~SoftBodyData()
{

}

//----------------------------------------------------------------------------
void SoftBodyData::packData(BitStream* stream)
{
	Parent::packData(stream);
	stream->write(poseMatchKoef);
	stream->write(attachedPointsNum);
	for (int i = 0;i<attachedPointsNum; i++)
		mathWrite(*stream,attachedPoints[i]);
}   

void SoftBodyData::unpackData(BitStream* stream)
{
	Parent::unpackData(stream);
	stream->read(&poseMatchKoef);
	stream->read(&attachedPointsNum);
	for (int i = 0;i<attachedPointsNum; i++)
		mathRead(*stream,&(attachedPoints[i]));
}   

void SoftBodyData::initPersistFields()
{
	Parent::initPersistFields();
	addField("poseMatchKoef",    TypeF32,		Offset(poseMatchKoef,      SoftBodyData));
	addField("attachedPointsNum",    TypeS8,	Offset(attachedPointsNum,      SoftBodyData));
	addField("attachedPoints",    TypePoint3F,	Offset(attachedPoints,      SoftBodyData),MaxAttachedPoints);
}


//----------------------------------------------------------------------------

SoftBody::SoftBody()
{
	mDataBlock = 0;
	mPhysPolyList = NULL;
	m_physShapeSoft = NULL;
	m_stopSimulation = false;
	m_initTransform.identity();
}   

SoftBody::~SoftBody()
{

}

void SoftBody::createUniqVertexList()
{
	mPhysPolyList = new ConcretePolyList();
	mPhysPolyList->setTransform(&mObjToWorld, mObjScale);
	mPhysPolyList->setObject(this);

	TSShapeInstance* shapeInst = getShapeInstance();
	S32 dl = 0;
	const TSShape* shape = getShape();
	const TSDetail * detail = &shape->details[dl];
	S32 ss = detail->subShapeNum;
	S32 od = detail->objectDetailNum;

	S32 start = shape->subShapeFirstObject[ss];
	S32 end   = shape->subShapeNumObjects[ss] + start;
	for (S32 i=start; i<end; i++)
	{
		TSShapeInstance::MeshObjectInstance* meshOI = &shapeInst->mMeshObjects[i];

		if (od >= meshOI->object->numMeshes)
			continue;
		U32 matKey(0);
		meshOI->buildPolyList(od,mPhysPolyList,matKey,NULL);
	}

	//create uniq vertex list
	ConcretePolyList::VertexList uniqVertexList;
	U32 vertexSize = mPhysPolyList->mVertexList.size();
	m_vertexBindingVec.setSize(vertexSize);
	for (U32 i=0; i<vertexSize; i++)
	{
		Point3F& vertex = mPhysPolyList->mVertexList[i];
		U32 uniqIdx = -1;

		for(U32 j=0;j<uniqVertexList.size();j++)
		{
			if (vertex==uniqVertexList[j])
			{
				uniqIdx=j;
				break;
			}
		}
		if (uniqIdx==-1)
		{
			uniqVertexList.push_back(vertex);
			uniqIdx = uniqVertexList.size()-1;
		}

		m_vertexBindingVec[i] = uniqIdx;
	}

	mPhysPolyList->mVertexList = uniqVertexList;

	//update indexes
	for(U32 i = 0; i < mPhysPolyList->mIndexList.size() ; i++)
	{
		mPhysPolyList->mIndexList[i] = m_vertexBindingVec[mPhysPolyList->mIndexList[i]];
	}
}

void SoftBody::createPhysShape()
{
	Physics* physics = Physics::getPhysics(isServerObject());

	if (physics)
	{
		createUniqVertexList();

		PhysSoftInfo physDescr;
		
		physDescr.shapeType = PhysInfo::ST_SOFTMESH;
		physDescr.physPolyList = mPhysPolyList;
		physDescr.owner = this;
		physDescr.mass = mDataBlock->mass;
		physDescr.poseMatchKoef = mDataBlock->poseMatchKoef;
		for(U8 i=0;i<mDataBlock->attachedPointsNum;i++)
		{
			physDescr.attachPoints.push_back(mDataBlock->attachedPoints[i]);
		}

		mPhysShape = physics->createPhysShapeSoft(physDescr);

		mPhysShape->setTransform(mObjToWorld);
	}
}

void SoftBody::prepBatchRender(SceneRenderState* state, S32 mountedImageIndex )
{
	if (!m_physShapeSoft || m_physShapeSoft->getNodesNum()==0 || gFreezeSim || m_stopSimulation)
	{
		Parent::prepBatchRender(state,mountedImageIndex);
		return;
	}

	TSShapeInstance* shapeInst = getShapeInstance();

	S32 dl = 0;
	const TSShape* shape = getShape();
	const TSDetail * detail = &shape->details[dl];
	S32 ss = detail->subShapeNum;
	S32 od = detail->objectDetailNum;

	U32 vertexStride = shape->getVertexSize();

	S32 start = shape->subShapeFirstObject[ss];
	S32 end   = shape->subShapeNumObjects[ss] + start;
	int physVertexIdx = 0;
	
	for (S32 i=start; i<end; i++)
	{
		TSShapeInstance::MeshObjectInstance* meshOI = &shapeInst->mMeshObjects[i];
		
		if (od >= meshOI->object->numMeshes)
			continue;
		TSMesh* mesh = meshOI->getMesh(od);
		if (mesh)
		{
			mesh->setManualDynamic(true);
			TSVertexBufferHandle& vb = meshOI->mVertexBuffer;
			if (vb == NULL)
				mesh->createVBIB(vb);

			
			U8 *vertData = (U8*)vb.lock();
			
			for(size_t i = 0;i<mesh->mNumVerts;i++)
			{
				Point3F physVertex = m_physShapeSoft->getNodePos(m_vertexBindingVec[physVertexIdx++]);
				//transform into local coordinates
				mRenderWorldToObj.mulP(physVertex);
				physVertex.convolveInverse(mObjScale);
	
				*((Point3F*)vertData) = physVertex;
				vertData += vertexStride;
			}
			
			vb.unlock();
			
		}
	}
	
	Parent::prepBatchRender(state,mountedImageIndex);

	for (S32 i=start; i<end; i++)
	{
		TSShapeInstance::MeshObjectInstance* meshOI = &shapeInst->mMeshObjects[i];
		if (od >= meshOI->object->numMeshes)
			continue;
		TSMesh* mesh = meshOI->getMesh(od);
		if (mesh)
			mesh->setManualDynamic(false);
	}
	
}

bool SoftBody::onAdd()
{
	if (!Parent::onAdd())
		return false;

	if (isServerObject())
		m_initTransform = mObjToWorld;

	//.hack - for client sharing physics with server
	if (mHasServerPhysic)
		createUniqVertexList();
		
	m_physShapeSoft = static_cast<PhysShapeSoft*>(mPhysShape.getPointer());	
	return true;
}

void SoftBody::onRemove()
{
	Parent::onRemove();
	if (mPhysPolyList && mHasServerPhysic)
	{
		delete mPhysPolyList;
	}
	mPhysPolyList = NULL;
	m_physShapeSoft = NULL;
}


//----------------------------------------------------------------------------

void SoftBody::processTick(const Move* move)
{     
	if (m_stopSimulation)
		return;
	Parent::processTick(move);
	int nodesNum = m_physShapeSoft ? m_physShapeSoft->getNodesNum() : 0;
	if (nodesNum>0)
	{
		//update obj box
		mObjBox.minExtents.set( 10E30f, 10E30f, 10E30f);
		mObjBox.maxExtents.set(-10E30f,-10E30f,-10E30f);
		for (U32 i=0; i < nodesNum; i++)
		{
			Point3F physVertex = m_physShapeSoft->getNodePos(i);
			mRenderWorldToObj.mulP(physVertex);
			physVertex.convolveInverse(mObjScale);
			mObjBox.minExtents.setMin(physVertex);
			mObjBox.maxExtents.setMax(physVertex);
		}
	}
}

bool SoftBody::onNewDataBlock(GameBaseData* dptr, bool reload)
{
	mDataBlock = dynamic_cast<SoftBodyData*>(dptr);
	if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
		return false;

	// Update Rigid Info
	scriptOnNewDataBlock();

	return true;
}

bool SoftBody::castRay(const Point3F &start, const Point3F &end, RayInfo* info)
{
	if (!m_physShapeSoft)
		return false;
	return m_physShapeSoft->castRayLocal(start,end,info);
}

void SoftBody::onEditorEnable()
{
	m_stopSimulation = true;
	//recalc obj box by init mesh;
	mObjBox = getShape()->bounds;
	setTransform(m_initTransform);
	mDelta.pos = m_initTransform.getPosition();
	mDelta.rot[0] = QuatF(m_initTransform);
	mDelta.rot[1] = mDelta.rot[0];
}
void SoftBody::onEditorDisable()
{
	m_stopSimulation = false;
}

U32 SoftBody::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
	U32 retMask = Parent::packUpdate(con, mask, stream);
	//set hasServerPhysic flag
	if ((stream->writeFlag((mask&InitialUpdateMask))))
		mathWrite(*stream,m_initTransform);
	return retMask;
}

void SoftBody::unpackUpdate(NetConnection *con, BitStream *stream)
{
	Parent::unpackUpdate(con,stream);

	// Initial update
	if (stream->readFlag())
	{
		mathRead(*stream,&m_initTransform);
		setTransform(m_initTransform);
		if (mPhysShape && !mHasServerPhysic)
			mPhysShape->setTransform(m_initTransform);
	}
}

void SoftBody::setTransform(const MatrixF& mat)
{

	PhysBody::setTransform(mat);
	if(gFreezeSim || m_stopSimulation)
	{
		m_initTransform = mat;
		if(mPhysShape) 
			mPhysShape->setTransform(mat);

		mDelta.pos = mat.getPosition();
		mDelta.rot[0] = QuatF(mat);
		mDelta.rot[1] = mDelta.rot[0];
		mPhysPosition = mDelta.pos;
		mPhysRotation = mDelta.rot[0];
		mDelta.posVec = VectorF::Zero;
	}
}