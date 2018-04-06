#include "@.h"

IMPLEMENT_CO_NETOBJECT_V1(@);

@::@() : Entity()
{
}

@::~@()
{
}

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

/// Updating
void @::processTick(const Move* move)
{
	Parent::processTick(move);
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

/// Editing
void @::onInspect()
{
}

void @::onEndInspect()
{
}