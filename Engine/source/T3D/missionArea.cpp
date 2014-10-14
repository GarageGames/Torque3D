//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "T3D/missionArea.h"
#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "math/mathIO.h"
#include "console/engineAPI.h"

IMPLEMENT_CO_NETOBJECT_V1(MissionArea);

ConsoleDocClass( MissionArea,
   "@brief Level object which defines the boundaries of the level.\n\n"

   "This is a simple box with starting points, width, depth, and height. It does not have "
   "any default functionality. Instead, when objects hit the boundaries certain "
   "script callbacks will be made allowing you to control the reaction.\n\n"

   "@tsexample\n"
   "new MissionArea(GlobalMissionArea)\n"
   "{\n"
   "	  Area = \"-152 -352 1008 864\";\n"
   "	  flightCeiling = \"300\";\n"
   "	  flightCeilingRange = \"20\";\n"
   "	  canSaveDynamicFields = \"1\";\n"
   "		 enabled = \"1\";\n"
   "		 TypeBool locked = \"false\";\n"
   "};\n"
   "@endtsexample\n\n"

   "@ingroup enviroMisc\n"
);

RectI MissionArea::smMissionArea(Point2I(768, 768), Point2I(512, 512));

MissionArea * MissionArea::smServerObject = NULL;

//------------------------------------------------------------------------------

MissionArea::MissionArea()
{
   mArea.set(Point2I(768, 768), Point2I(512, 512));
   mNetFlags.set(Ghostable | ScopeAlways);

   mFlightCeiling      = 2000;
   mFlightCeilingRange = 50;
}

//------------------------------------------------------------------------------

void MissionArea::setArea(const RectI & area)
{
   // set it
   mArea = MissionArea::smMissionArea = area;

   // pass along..
   if(isServerObject())
      mNetFlags.set(UpdateMask);
}

//------------------------------------------------------------------------------

MissionArea * MissionArea::getServerObject()
{
   return smServerObject;
}

//------------------------------------------------------------------------------

bool MissionArea::onAdd()
{
   if(isServerObject())
   {
      if(MissionArea::getServerObject())
      {
         Con::errorf(ConsoleLogEntry::General, "MissionArea::onAdd - MissionArea already instantiated!");
         return(false);
      }
      else
      {
         smServerObject = this;
      }
   }

   if(!Parent::onAdd())
      return(false);

   setArea(mArea);
   return(true);
}

void MissionArea::onRemove()
{
   if (smServerObject == this)
      smServerObject = NULL;

   Parent::onRemove();
}

//------------------------------------------------------------------------------

void MissionArea::inspectPostApply()
{
   // Set Parent.
   Parent::inspectPostApply();

   setMaskBits(UpdateMask);
}

//------------------------------------------------------------------------------

void MissionArea::initPersistFields()
{
   addGroup("Dimensions");	
   addField("area", TypeRectI, Offset(mArea, MissionArea), "Four corners (X1, X2, Y1, Y2) that makes up the level's boundaries");
   addField("flightCeiling", TypeF32, Offset(mFlightCeiling, MissionArea), "Represents the top of the mission area, used by FlyingVehicle. ");
   addField("flightCeilingRange", TypeF32, Offset(mFlightCeilingRange, MissionArea), "Distance from ceiling before FlyingVehicle thrust is cut off. ");
   endGroup("Dimensions");

   Parent::initPersistFields();
}


//------------------------------------------------------------------------------

void MissionArea::unpackUpdate(NetConnection *, BitStream * stream)
{
   // ghost (initial) and regular updates share flag..
   if(stream->readFlag())
   {
      mathRead(*stream, &mArea);
      stream->read(&mFlightCeiling);
      stream->read(&mFlightCeilingRange);
   }
}

U32 MissionArea::packUpdate(NetConnection *, U32 mask, BitStream * stream)
{
   if(stream->writeFlag(mask & UpdateMask))
   {
      mathWrite(*stream, mArea);
      stream->write(mFlightCeiling);
      stream->write(mFlightCeilingRange);
   }
   return(0);
}

//-----------------------------------------------------------------------------

DefineEngineFunction(getMissionAreaServerObject, MissionArea*, (),,
					 "Get the MissionArea object, if any.\n\n"
					 "@ingroup enviroMisc\n\n")
{
	return MissionArea::getServerObject();
}

DefineEngineMethod( MissionArea, getArea, const char *, (),,
              "Returns 4 fields: starting x, starting y, extents x, extents y.\n")
{
   static const U32 bufSize = 48;
   char* returnBuffer = Con::getReturnBuffer(bufSize);

   RectI area = object->getArea();
   dSprintf(returnBuffer, bufSize, "%d %d %d %d", area.point.x, area.point.y, area.extent.x, area.extent.y);
   return(returnBuffer);
}

DefineEngineMethod( MissionArea, setArea, void, (S32 x, S32 y, S32 width, S32 height),,
			  "@brief - Defines the size of the MissionArea\n\n"
			  "param x Starting X coordinate position for MissionArea\n"
			  "param y Starting Y coordinate position for MissionArea\n"
			  "param width New width of the MissionArea\n"
			  "param height New height of the MissionArea\n"
           "@note Only the server object may be set.\n"
			  )
{
   if(object->isClientObject())
   {
      Con::errorf(ConsoleLogEntry::General, "MissionArea::cSetArea - cannot alter client object!");
      return;
   }

   RectI rect;
   rect.point.x = x;
   rect.point.y = y;
   rect.extent.x = width;
   rect.extent.y = height;

   object->setArea(rect);
}

DefineEngineMethod( MissionArea, postApply, void, (),,
                   "Intended as a helper to developers and editor scripts.\n"
                   "Force trigger an inspectPostApply. This will transmit "
                   "material and other fields ( not including nodes ) to client objects."
                   )
{
   object->inspectPostApply();
}