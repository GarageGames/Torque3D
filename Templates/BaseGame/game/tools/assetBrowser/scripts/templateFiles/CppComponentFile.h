#pragma once

#ifndef _ENGINEAPI_H_
#include "console/engineAPI.h"
#endif
#ifndef COMPONENT_H
#include "T3D/components/component.h"
#endif

class @ : public Component
{
	typedef Component Parent;
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

    /// Management
	//This is called when we are added to an entity
    virtual void onComponentAdd();            
    //This is called when we are removed from an entity
    virtual void onComponentRemove();   

	//This is called when a different component is added to our owner entity
    virtual void componentAddedToOwner(Component *comp);  
    //This is called when a different component is removed from our owner entity
    virtual void componentRemovedFromOwner(Component *comp); 

	/// Updating
	virtual void processTick();
    virtual void interpolateTick(F32 dt);
    virtual void advanceTime(F32 dt);

	/// Networking
	virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
    virtual void unpackUpdate(NetConnection *con, BitStream *stream);
};