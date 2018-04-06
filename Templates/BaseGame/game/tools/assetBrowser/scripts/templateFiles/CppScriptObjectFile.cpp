#include "@.h"

@::@()
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

/// Updating
void @::processTick()
{
	//This ensures that any scipt callbacks are fired off
	Parent::processTick();
}

void @::interpolateTick(F32 dt)
{
	//This ensures that any scipt callbacks are fired off
	Parent::interpolateTick(dt);
}

void @::advanceTime(F32 dt)
{
	//This ensures that any scipt callbacks are fired off
	Parent::advanceTime(dt);
}