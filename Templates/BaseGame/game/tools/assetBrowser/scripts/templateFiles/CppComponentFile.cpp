#include "@.h"
#include "core/stream/bitStream.h"

@::@() : Component()
{
}

@::~@()
{
}

IMPLEMENT_CONOBJECT(@);

/// SimObject handling
bool @::onAdd()
{
	if(!Parent::onAdd())
		return false;

	return true;
}

void @::onRemove()
{
	Parent::onRemove();
}

void @::initPersistFields()
{
	Parent::initPersistFields();
}

/// Management

void @::onComponentAdd()
{
	Parent::onComponentAdd();
}             

void @::onComponentRemove()
{
	Parent::onComponentRemove();
}   

void @::componentAddedToOwner(Component *comp)
{
	Parent::componentAddedToOwner(comp);
}  

void @::componentRemovedFromOwner(Component *comp)
{
	Parent::componentRemovedFromOwner(comp);
} 

/// Updating
void @::processTick()
{
	Parent::processTick();
}

void @::interpolateTick(F32 dt)
{
}

void @::advanceTime(F32 dt)
{
}

/// Networking
U32 @::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
	U32 ret = Parent::packUpdate(con, mask, stream);

	return ret;
}

void @::unpackUpdate(NetConnection *con, BitStream *stream)
{
	Parent::unpackUpdate(con, stream);
}