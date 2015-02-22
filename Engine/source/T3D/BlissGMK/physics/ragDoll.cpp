//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------
#include "T3D/BlissGMK/physics/ragDoll.h"
#include "T3D/BlissGMK/physics/physJoint.h"
#include "math/mathIO.h"
#include "core/stream/bitStream.h"
#include "ts/tsShapeInstance.h"
#include "console/consoleTypes.h"
#include "gfx/sim/debugDraw.h"
//----------------------------------------------------------------------------
IMPLEMENT_CO_DATABLOCK_V1(RagDollData);
IMPLEMENT_CO_NETOBJECT_V1(RagDoll);

RagDollData::RagDollData()
{
	for (int i = 0; i < MaxBones; i++) 
	{
		boneParentNodeName[i] = NULL;
		boneNodeName[i] = NULL;
		boneSize[i].set(0.2f, 0.2f, 0.2f);
		boneOffset[i].set(0.f, 0.f, 0.f);
		boneRotAngles[i].set(0.f, 0.f, 0.f);
		boneMass[i] = 1.f;
		boneShape[i] = 0;
		boneJointType[i] = 0;
		boneJointParam[i].set(0.f, 0.f, 0.f);
		boneJointParam2[i].set(0.f, 0.f, 0.f);
	}

	m_manualBoneRotations = false;
}

void RagDollData::initPersistFields()
{
	Parent::initPersistFields();

	addField("boneParentNodeName", TypeCaseString, Offset(boneParentNodeName, RagDollData), MaxBones);
	addField("boneNodeName", TypeCaseString, Offset(boneNodeName, RagDollData), MaxBones);
	addField("boneSize", TypePoint3F, Offset(boneSize, RagDollData), MaxBones);
	addField("boneMass", TypeF32, Offset(boneMass, RagDollData), MaxBones);
	addField("boneShape", TypeS8, Offset(boneShape, RagDollData), MaxBones);
	addField("boneOffset", TypePoint3F, Offset(boneOffset, RagDollData), MaxBones);
	addField("boneRotAngles", TypePoint3F, Offset(boneRotAngles, RagDollData), MaxBones);	
	addField("boneJointType", TypeS8, Offset(boneJointType, RagDollData), MaxBones);
	addField("boneJointParam", TypePoint3F, Offset(boneJointParam, RagDollData), MaxBones);
	addField("boneJointParam2", TypePoint3F, Offset(boneJointParam2, RagDollData), MaxBones);
	addField("manualBoneRotations", TypeBool, Offset(m_manualBoneRotations, RagDollData));
}

void RagDollData::packData(BitStream* stream)
{
	Parent::packData(stream);
	for (int i = 0;i<MaxBones; i++)
	{
		if (stream->writeFlag(boneNodeName[i]!=NULL))
		{
			stream->writeString(boneNodeName[i]);
			if (stream->writeFlag(boneParentNodeName[i]!=NULL))
				stream->writeString(boneParentNodeName[i]);
			mathWrite(*stream,boneSize[i]);
			stream->write(boneMass[i]);
			stream->write(boneShape[i]);
			mathWrite(*stream,boneOffset[i]);
			mathWrite(*stream,boneRotAngles[i]);
			stream->write(boneJointType[i]);
			mathWrite(*stream,boneJointParam[i]);
			mathWrite(*stream,boneJointParam2[i]);
		}
		
	}
	stream->writeFlag(m_manualBoneRotations);
}

void RagDollData::unpackData(BitStream* stream)
{
	Parent::unpackData(stream);
	for (int i = 0;i<MaxBones; i++)
	{
		if (stream->readFlag())
		{
			boneNodeName[i] = stream->readSTString();
			if (stream->readFlag())
				boneParentNodeName[i] = stream->readSTString();
			mathRead(*stream,&boneSize[i]);
			stream->read(&(boneMass[i]));
			
			stream->read(&(boneShape[i]));
			mathRead(*stream,&boneOffset[i]);
			mathRead(*stream,&boneRotAngles[i]);
			stream->read(&(boneJointType[i]));
					
			mathRead(*stream,&boneJointParam[i]);
			mathRead(*stream,&boneJointParam2[i]);
		}
	}
	m_manualBoneRotations = stream->readFlag();
}

//----------------------------------------------------------------------------

RagDoll::RagDoll()
{
	mNetFlags.set(Ghostable);
	//mNetFlags |= ScopeAlways;
	mDataBlock = 0;
	mTypeMask |= VehicleObjectType;

	mpBones = NULL;
	mRootBoneNodeIdx = -1;

	m_interpolatingBones = false;
}

RagDoll::~RagDoll()
{
}

//----------------------------------------------------------------------------

bool RagDoll::onAdd()
{
	if (!Parent::onAdd()) return false;

	if (!mpBones) 
		setRagDoll();

	addToScene();

	if (isServerObject())
		scriptOnAdd();

	return true;
}


bool RagDoll::onNewDataBlock(GameBaseData* dptr, bool reload)
{
	mDataBlock = dynamic_cast<RagDollData*>(dptr);
	if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
		return false;

	// Update Rigid Info
	scriptOnNewDataBlock();

	return true;
}

void RagDoll::onRemove()
{
	scriptOnRemove();
	removeFromScene();

	destroyRagDoll();
	Parent::onRemove();
}

//----------------------------------------------------------------------------
U32 RagDoll::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
	U32 retMask = Parent::packUpdate(con, mask, stream);
	bool hasServerPhysics = !Physics::getPhysics(false) && con->isLocalConnection();
	
	if(stream->writeFlag(mask & InitialUpdateMask))
	{
		mathWrite(*stream, mObjToWorld);
		mathWrite(*stream, mObjScale);

		if (hasServerPhysics)
		{
			stream->_write(sizeof(mpBones), &mpBones);
			stream->_write(sizeof(mRootBoneNodeIdx), &mRootBoneNodeIdx);
		}
	}

	if (!hasServerPhysics)
	{
		for(boneInfoMap::iterator it=mpBones->begin(),end = mpBones->end();it!=end;it++)
		{
			it->second.physShape->pack(stream);
		}
	}

	return(retMask);
}

//------------------------------------------------------------------------------

void RagDoll::unpackUpdate(NetConnection * con, BitStream * stream)
{
	Parent::unpackUpdate(con, stream);

	if(stream->readFlag()) //initial
	{
		MatrixF mat;

		mathRead(*stream,&mat);
		Parent::setTransform(mat);
		Parent::setRenderTransform(mat);

		VectorF scale;
		mathRead(*stream, &scale);
		setScale(scale);

	
		if (!Physics::getPhysics(false))
		{
			mHasServerPhysic = true;
			
			boneInfoMap* serverBones = NULL;
			stream->_read(sizeof(mpBones), &serverBones);
			//create client bone info map
			mpBones = new boneInfoMap;
			for(boneInfoMap::iterator it = serverBones->begin(); it != serverBones->end(); it++)
			{
				mpBones->insert(*it);
			}
			stream->_read(sizeof(mRootBoneNodeIdx), &mRootBoneNodeIdx);
		}
		else
			setRagDoll();

	}

	if (!mHasServerPhysic)
	{
		for(boneInfoMap::iterator it=mpBones->begin(),end = mpBones->end();it!=end;it++)
		{
			it->second.physShape->unpack(stream);
		}
	}

}

//----------------------------------------------------------------------------
void RagDoll::initPersistFields()
{
	Parent::initPersistFields();
}


void RagDoll::createRagDoll()
{
	mpBones = new boneInfoMap;
	
	for (int i = 0; i < RagDollData::MaxBones; i++) 
	{
		if(mDataBlock->boneNodeName[i] == NULL ) continue;

		addBone(i, mDataBlock->boneParentNodeName[i], 
				mDataBlock->boneNodeName[i], 
				mDataBlock->boneSize[i], 
				mDataBlock->boneOffset[i],
				mDataBlock->boneRotAngles[i],
				mDataBlock->boneMass[i] * 1,
				mDataBlock->boneShape[i],
				mDataBlock->boneJointType[i],
				mDataBlock->boneJointParam[i],mDataBlock->boneJointParam2[i]);
	}

	//////////////////////////////////////////////////////////////////////////
	AssertFatal(!getBones().empty(), "No bones in ragdoll!");
	AssertFatal(getBones().find(mRootBoneNodeIdx) != getBones().end(), "mRootBoneNodeIdx not set");
	//mPhysShape = getBones()[mRootBoneNodeIdx].physShape;
}

std::size_t RagDoll::addBone(S32 bode_idx, const char* parent_node_name, 
							 const char* node_name,
							 const VectorF& size,const VectorF& offset,const VectorF& rotAngels,
							 float mass,int shapeType,int jointType,VectorF jointParam,VectorF jointParam2)
{
	TSShape * shape = mShapeInstance->getShape();
	S32 node_idx = shape->findNode(node_name);
	AssertFatal(node_idx != -1 , avar(" Node not found %s", node_name));

	boneInfo bone_info;

	MatrixF model_transform;
	model_transform.identity();

	S32 cur_node_idx = node_idx;

	MatrixF parent_transform;
	parent_transform = mShapeInstance->mNodeTransforms[cur_node_idx];

	model_transform.mul(parent_transform, MatrixF(model_transform));

	bone_info.bode_idx = bode_idx;
	bone_info.model_transform = model_transform;
	bone_info.model_inverse_transform = bone_info.model_transform;
	bone_info.model_inverse_transform.inverse();

	bone_info.pivot.identity();

	MatrixF rotMatrix(true);
	if (!mDataBlock->m_manualBoneRotations)
		bone_info.pivot.set(EulerF(0.f,Float_HalfPi,0.f)); //turn on 90 degrees into base pos
	else
	{
		VectorF rotAnglesRad = rotAngels/180.f*float(M_PI);
		rotMatrix.set((EulerF)rotAnglesRad);
	}
	

	Point3F pivotPos = Point3F::Zero;
	pivotPos = offset;
	bone_info.pivot.setPosition(pivotPos);
	bone_info.pivot.mul(rotMatrix);

	//calculate joint pos
	MatrixF real_bone_transform = model_transform;
	real_bone_transform.mul(bone_info.pivot);
	VectorF joint_global_pos = real_bone_transform.getPosition();

	//add offset to center of bone and calc phys bone transform	
	if(parent_node_name != NULL)
	{
		VectorF offsetDir(1.f,0.f,0.f);
		if (mDataBlock->m_manualBoneRotations)
		{
			bone_info.pivot.identity();
			offsetDir = VectorF(0.f,0.f,1.f);
		}

		if (shapeType == 2)
			pivotPos += (size.y*0.5f+size.x)*offsetDir;
		else
			pivotPos += (size.x/2.f)*offsetDir;

/*		if (shapeType == 2)
			//pivotPos += (Point3F((size.y)*0.5+size.x, 0, 0));
			pivotPos += (Point3F(0, 0, (size.y)*0.5+size.x));
		else
			pivotPos += (Point3F(size.x/2, 0, 0));
*/		
		bone_info.pivot.setPosition(pivotPos);
		bone_info.pivot.mul(rotMatrix,MatrixF(bone_info.pivot));
		
	}
	MatrixF bone_transform = model_transform;
	bone_transform.mul(bone_info.pivot);

	bone_info.invPivot = bone_info.pivot; 
	bone_info.invPivot.inverse();

	PhysInfo physInfo;
	physInfo.owner = this;
	physInfo.shapeType = PhysInfo::ShapeType(shapeType);
	
	physInfo.mass = mass;
	VectorF paramSize = size;
	if (shapeType == 2)
	{
		paramSize.x = 2.f*size.x;
		paramSize.z = paramSize.y = size.y + paramSize.x;
	}
	physInfo.params = paramSize;
	PhysShape* physShape = mPhysics->createPhysShape(physInfo);
	
	physShape->setTransform(bone_transform);

	if(parent_node_name != NULL)
	{
		S32 parent_node_idx = shape->findNode(parent_node_name);
		AssertFatal(parent_node_idx != -1 , avar(" Node not found %s", parent_node_name));

		AssertFatal(getBones().find(parent_node_idx) != getBones().end(), avar("parent_bone not set! %s", parent_node_name));
		boneInfo& parent_body_info = getBones()[parent_node_idx];
		PhysShape* parentShape = parent_body_info.physShape;
		PhysJointInfo jointInfo;
		jointInfo.shape1 = physShape;
		jointInfo.shape2 = parentShape;
		jointInfo.jointType = (PhysJointInfo::JointType)jointType;
		jointInfo.params1 = jointParam;
		jointInfo.params2 = jointParam2;
		jointInfo.pos = joint_global_pos;//bone_info.model_transform.getPosition();
		
		bone_info.constraint = mPhysics->createPhysJoint(jointInfo);
	}
	else
	{
		mRootBoneNodeIdx = node_idx;
	}

	bone_info.physShape = physShape;
	bone_info.node_idx = node_idx;
	bone_info.node_name = node_name;

	getBones().insert(std::make_pair(bone_info.node_idx, bone_info));
	return getBones().size() - 1;
}


void RagDoll::destroyRagDoll()
{
	for(boneInfoMap::iterator it = getBones().begin(); it != getBones().end(); it++)
	{
		boneInfo& bone_info = it->second;
		if(bone_info.physShape)
		{
			bone_info.physShape->setEnable(false);
			bone_info.physShape->getInfo().owner = NULL;
		}
	}
	getBones().clear();

	delete mpBones;
	mpBones = NULL;
}


void RagDoll::setRagDoll()
{   
	mPhysics = (Physics::getPhysics(isServerObject()));

	TSShape* shape = mShapeInstance->getShape();
	S32 ragDollSeq = getShape()->findSequence("ragdoll");
	TSThread* thread = NULL;
	//AssertFatal(ragDollSeq != -1, "There must a 'ragdoll' animation.");
	if (ragDollSeq != -1) 
	{
		thread = mShapeInstance->addThread();
		mShapeInstance->setSequence(thread, ragDollSeq, 0.01f/*0.f*/);
		mShapeInstance->animate();
	}

	createRagDoll();
	initRagDoll(this);

	if (thread)
		mShapeInstance->destroyThread(thread);

	//turn off node animations
	for (U32 i=0; i < shape->nodes.size(); i++)
	{      
		mShapeInstance->setNodeAnimationState(i, mShapeInstance->MaskNodeHandsOff);
	}
	mShapeInstance->animate();
	updateRagDoll();
}

//Setup ragDoll from the bones pos of current model pose
void RagDoll::initRagDoll(ShapeBase* object)
{
	MatrixF worldTransform = object->getTransform();
	setTransform(worldTransform);

	for(boneInfoMap::iterator it = getBones().begin(); it != getBones().end(); it++)
	{
		boneInfo& bone_info = it->second;
		
		S32 idx = object->getShape()->findNode(bone_info.node_name.c_str());
		
		AssertWarn(idx != -1, "Ragdoll node structure doesnt match the external shape structure.");
		if(idx == -1) continue;

		MatrixF bone_mat = object->getShapeInstance()->mNodeTransforms[idx];
		bone_info.model_inverse_transform = bone_mat;
		bone_info.model_inverse_transform.inverse();

		bone_info.curRot.set(bone_mat);
		bone_info.curPos = bone_mat.getPosition();
		bone_info.prevRot = bone_info.curRot;
		bone_info.prevPos = bone_info.curPos;

		bone_mat.mul(bone_info.pivot);
		bone_mat.mul(worldTransform,MatrixF(bone_mat));			
		

		bone_info.physShape->setTransform(bone_mat);
	}
}

void RagDoll::updateRagDoll()
{   
	TSShape* shape = mShapeInstance->getShape();

	if(!m_interpolatingBones)
	{
		mPrevToCur = getTransform();
		boneInfo& bone = (*mpBones)[mRootBoneNodeIdx];
		PhysShape* body = bone.physShape;
		MatrixF tr = body->getTransform();
		tr.mul(bone.invPivot);	
		tr.mul(bone.model_inverse_transform);		
		setTransform(tr);
		mPrevToCur.mul(mWorldToObj,MatrixF(mPrevToCur));
	}

	mCalculatedNones.clear();
	
	
	mObjBox.minExtents.set( 10E30f, 10E30f, 10E30f);
	mObjBox.maxExtents.set(-10E30f,-10E30f,-10E30f);

	for (U32 i=0; i < shape->nodes.size(); i++)
	{
		calcNodeTransform(i);
		VectorF pos = mShapeInstance->mNodeTransforms[i].getPosition();
		mObjBox.minExtents.setMin(pos);
		mObjBox.maxExtents.setMax(pos);
	}

}

MatrixF RagDoll::calcNodeTransform(S32 node_idx)
{
	if(mCalculatedNones.find(node_idx) != mCalculatedNones.end()) 
	{
		return mShapeInstance->mNodeTransforms[node_idx];
	}

	mCalculatedNones.insert(node_idx);

	//there's a bone in ragDoll - get physics position for it.
	boneInfoMap::iterator it = getBones().find(node_idx);
	if(it != getBones().end())
	{
		boneInfo& bone_info = it->second;
		
		//Interpolating
		if(m_interpolatingBones)
		{
			MatrixF m;
			QuatF rot;
			Point3F pos;
			TSTransform::interpolate(bone_info.prevRot, bone_info.curRot, m_interpolateTime, &rot);
			TSTransform::interpolate(bone_info.prevPos, bone_info.curPos, m_interpolateTime, &pos);
			rot.setMatrix(&m);
			m.setPosition(pos);
			mShapeInstance->mNodeTransforms[bone_info.node_idx] = m;
		}
		//Updating actual physics position.
		else
		{
			MatrixF bone_mat = bone_info.physShape->getTransform();
			bone_mat.mul(bone_info.invPivot);
			bone_mat.mul(mWorldToObj, MatrixF(bone_mat));
			

			//recalculate prev pos relative to new world transform
			QuatF quatTr(mPrevToCur);
			bone_info.prevRot.mul(bone_info.curRot,quatTr);
			Point3F pos = bone_info.curPos;
			mPrevToCur.mulP(pos);
			bone_info.prevPos = pos;

			bone_info.curRot.set(bone_mat);
			bone_info.curPos = bone_mat.getPosition();

			MatrixF m;
			bone_info.prevRot.setMatrix(&m);
			m.setPosition(bone_info.prevPos);
			mShapeInstance->mNodeTransforms[bone_info.node_idx] = m;
			
		}
		return mShapeInstance->mNodeTransforms[bone_info.node_idx];
	}

	//Otherwise, take position from a default pose.
	TSShape* shape = mShapeInstance->getShape();
    MatrixF default_transform, parent_transform;
	QuatF quat;
	shape->defaultRotations[node_idx].getQuatF(&quat);
	TSTransform::setMatrix(quat, shape->defaultTranslations[node_idx], &default_transform);

	if(shape->nodes[node_idx].parentIndex != -1)
		parent_transform = calcNodeTransform(shape->nodes[node_idx].parentIndex);
	else
		parent_transform.identity();

	mShapeInstance->mNodeTransforms[node_idx].mul(parent_transform, default_transform);
	return mShapeInstance->mNodeTransforms[node_idx];
}

void RagDoll::applyImpulse(const Point3F& pos,const VectorF& vec)
{
	
	/*
	//get random bone
	int idx = rand()%getBones().size();
	int i=0;
	boneInfoMap::iterator it = getBones().begin();
	while (idx!=i)
	{
		i++;
		it++;
	}
	*/

	//boneInfo& bone = (*mpBones)[mRootBoneNodeIdx];	//force to pelvis
	//apply force to nearest bone
	
	float minDist = 10E30f;
	boneInfoMap::iterator it = getBones().begin(),end = getBones().end();
	boneInfoMap::iterator closeIt = it;
	while (it!=end)
	{
		PhysShape* body = it->second.physShape;
		float dist = (body->getPosition() - pos).lenSquared();
		if (dist<minDist)
		{
			closeIt = it;
			minDist = dist;
		}		
		it++;
	}
	
	boneInfo& bone = closeIt->second;
	
	PhysShape* body = bone.physShape;
	/*VectorF ragDollPos = body->getPosition();
	static float scaler = 1.f;
	VectorF force = -(pos - ragDollPos);
	force.z = 0.f;
	force.normalize();
	force*=vec.x;*/
	//body->addForce(force);
	body->addForce(vec,pos);
}

bool RagDoll::castRay(const Point3F &start, const Point3F &end, RayInfo* info)
{
	bool res = false;
	F32 closestT = 2.0;
	RayInfo closestRi;
	for(boneInfoMap::iterator it = getBones().begin(); it != getBones().end(); it++)
	{
		boneInfo& bone_info = it->second;
		if(bone_info.physShape)
		{
			
			if (bone_info.physShape->castRayLocal(start,end,info) && info->t<closestT)
			{
				res = true;
				closestT = info->t;
				closestRi = *info;
			}
				
		}
	}

	if (res)
		*info = closestRi;
	return res;
}
//----------------------------------------------------------------------------
void RagDoll::processTick(const Move* move) 
{
	Parent::processTick(move);
	updateRagDoll();
	updateContainer();
	setMaskBits(PositionMask);
}

void RagDoll::interpolateTick(F32 dt)
{
	Parent::interpolateTick(dt);
	
	m_interpolateTime = 1.f - dt;
	
	m_interpolatingBones = true;
	updateRagDoll();
	m_interpolatingBones = false;
}

//----------------------------------------------------------------------------
ConsoleMethod( RagDoll, initRagDoll, void, 3, 3, "( ShapeBase obj )" "Initialize position of ragdoll bones from given shape object.")
{
	ShapeBase* sample_object;
	if( Sim::findObject( argv[2], sample_object) )
	{
		RagDoll* rg = static_cast<RagDoll*>(object);
		rg->initRagDoll(sample_object);
		rg->updateRagDoll();
	}
}