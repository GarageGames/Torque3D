#include "@.h"
#include "core/stream/bitStream.h"

@::@()
{
}

@::~@()
{
}

IMPLEMENT_CO_DATABLOCK_V1(@);

/// SimObject handling
bool @::onAdd()
{
	if(!Parent::onAdd())
		return false;

	return true;
}

bool @::preload(bool server, String &errorStr)
{
	if (!Parent::preload(server, errorStr))
      return false;

	return true;
}

void @::initPersistFields()
{
	Parent::initPersistFields();
}

/// Networking
void @::packData(BitStream* stream)
{
   Parent::packData(stream);
}

void @::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
}


void @::inspectPostApply()
{
   Parent::inspectPostApply();
}