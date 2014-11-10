

#pragma once

#ifndef _READXML_H_
#define _READXML_H_

#include "console/simobject.h"

#include "console/simBase.h"
#include "core/util/tVector.h"

class ReadXML : public SimObject
{
private:
	FileName mFileName;
public:
	ReadXML(void);
	~ReadXML(void);

	typedef SimObject Parent;
	DECLARE_CONOBJECT(ReadXML);
	static void initPersistFields();

	/// Read the file.
	bool readFile();
	void readLayer(SimXMLDocument *document);
};

#endif

