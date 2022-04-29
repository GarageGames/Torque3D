#include "@.h"

IMPLEMENT_STATIC_CLASS(@, , "Functions for maintaing a list of banned users.");

@* @::smInstance;

@::@()
{
	smInstance = this;
}

DefineEngineStaticMethod(@, doSomething, void, (), , "")
{
	//@::instance()->doSomething();
}