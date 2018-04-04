#include "core/module.h"
#include "console/engineAPI.h"
#include "GOAPAI_GoalManager.h"

MODULE_BEGIN(GOAPAI)

MODULE_INIT_AFTER(Sim)
MODULE_SHUTDOWN_BEFORE(Sim)

MODULE_INIT
{
	new GOAPAI_GoalManager;
}
MODULE_SHUTDOWN
{
	delete GOAPAI_GoalManager::instance();
}
MODULE_END;

