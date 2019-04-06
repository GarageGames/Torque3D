#include "core/module.h"
#include "console/engineAPI.h"

MODULE_BEGIN(@_Module)

MODULE_INIT_AFTER(Sim)
MODULE_SHUTDOWN_BEFORE(Sim)

MODULE_INIT
{
	// Setup anything needed when the engine initializes here
}
MODULE_SHUTDOWN
{
	// Cleanup anything that was initialized before here
}
MODULE_END;

