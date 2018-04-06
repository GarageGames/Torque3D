#pragma once

#ifndef _ENGINEAPI_H_
#include "console/engineAPI.h"
#endif
#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif

class @ : public GameBaseData
{
	typedef GameBaseData Parent;
public:
	// Because this is just a data container class, you can just put the fields in the public space

	/// Functions
	@();
	~@();

	//This registers the class type and namespace for the console system
	DECLARE_CONOBJECT(@);

    /// SimObject handling
    bool onAdd();
    bool preload(bool server, String &errorStr);

    static void initPersistFields();

	/// Networking
    virtual void packData(BitStream* stream);
    virtual void unpackData(BitStream* stream);
   
    // Triggers reload signal when changed
    void inspectPostApply();
};