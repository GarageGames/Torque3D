#pragma once

#include "console/engineAPI.h"

class GOAPAI_GoalManager : public SimObject
{
	DECLARE_STATIC_CLASS(BanList);
private:

protected:
	static GOAPAI_GoalManager* smInstance;

public:
	GOAPAI_GoalManager();

	static GOAPAI_GoalManager* instance() { return smInstance; }
};