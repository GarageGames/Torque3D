#pragma once

#ifndef _ENGINEAPI_H_
#include "console/engineAPI.h"
#endif
#ifndef _SCRIPTOBJECTS_H_
#include "console/scriptObjects.h"
#endif

class @ : public ScriptTickObject
{
	typedef ScriptTickObject Parent;
private:

protected:

public:
	@();
	virtual ~@();

	//This registers the class type and namespace for the console system
	DECLARE_CONOBJECT(@);

    /// SimObject handling
	virtual bool onAdd();
    virtual void onRemove();

    static void initPersistFields();

	/// Updating
	virtual void processTick();
    virtual void interpolateTick(F32 dt);
    virtual void advanceTime(F32 dt);
};