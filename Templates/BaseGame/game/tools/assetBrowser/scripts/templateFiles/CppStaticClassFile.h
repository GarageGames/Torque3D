#pragma once

#include "console/engineAPI.h"

class @
{
	DECLARE_STATIC_CLASS(@);
private:

protected:
	static @* smInstance;

public:
	@();

	static @* instance() { return smInstance; }
};