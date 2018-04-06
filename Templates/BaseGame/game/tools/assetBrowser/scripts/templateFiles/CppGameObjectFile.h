#pragma once

#ifndef _ENGINEAPI_H_
#include "console/engineAPI.h"
#endif
#ifndef ENTITY_H
#include "T3D/entity.h"
#endif
#ifndef _MOVEMANAGER_H_
#include "T3D/gameBase/moveManager.h"
#endif
#ifndef COMPONENT_H
#include "T3D/components/component.h"
#endif

class @ : public Entity
{
	typedef Entity Parent;
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
	virtual void processTick(const Move* move);
    virtual void advanceTime(F32 dt);
    virtual void interpolateTick(F32 delta);

	/// Networking
	virtual U32 packUpdate(NetConnection *conn, U32 mask, BitStream *stream);
    virtual void unpackUpdate(NetConnection *conn, BitStream *stream);

    /// Editing
	void onInspect();
    void onEndInspect();
};