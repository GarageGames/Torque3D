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

#include "sim/actionMap.h"
#include "console/console.h"
#include "platform/platform.h"
#include "platform/platformInput.h"
#include "platform/platformAssert.h"
#include "core/stream/fileStream.h"
#include "math/mMathFn.h"
#include "console/engineAPI.h"
#include "math/mQuat.h"
#include "math/mAngAxis.h"

#define CONST_E 2.7182818284590452353602874f

IMPLEMENT_CONOBJECT(ActionMap);

ConsoleDocClass( ActionMap,
   "@brief ActionMaps assign platform input events to console commands.\n\n"

   "Any platform input event can be bound in a single, generic way. In theory, the game doesn't need to know if the event came from the keyboard, mouse, joystick "
   "or some other input device. This allows users of the game to map keys and actions according to their own preferences. "
   "Game action maps are arranged in a stack for processing so individual parts of the game can define specific "
   "actions. For example, when the player jumps into a vehicle it could push a vehicle action map and pop the default player action map.\n\n"

   "@section ActionMap_creation Creating an ActionMap\n"

   "The input system allows for the creation of multiple ActionMaps, so long as they have unique names and do not already exist. It's a simple "
   "three step process.\n\n"
   "1. Check to see if the ActionMap exists\n"
   "2. Delete it if it exists\n"
   "3. Instantiate the ActionMap\n\n"

   "The following is an example of how to create a new ActionMap:\n"

   "@tsexample\n"
   "if ( isObject( moveMap ) )\n"
   "  moveMap.delete();\n"
   "new ActionMap(moveMap);"
   "@endtsexample\n\n\n"
   
   "@section ActionMap_binding Binding Functions\n"
   "Once you have created an ActionMap, you can start binding functionality to events. Currently, Torque 3D supports the following devices out of the box\n\n"
   "* Mouse\n\n"
   "* Keyboard\n\n"
   "* Joystick/Gamepad\n\n"
   "* Xbox 360 Controller\n\n"

   "The two most commonly used binding methods are bind() and bindCmd(). Both are similar in that they will bind functionality to a device and event, "
   "but different in how the event is interpreted. With bind(), "
   "you specify a device, action to bind, then a function to be called when the event happens.\n\n"

   "@tsexample\n"
   "// Simple function that prints to console\n"
   "// %val - Sent by the device letting the user know\n"
   "// if an input was pressed (true) or released (false)\n"
   "function testInput(%val)\n"
   "{\n"
   "   if(%val)\n"
   "    echo(\"Key is down\");\n"
   "   else\n"
   "    echo(\"Key was released\");\n"
   "}\n\n"
   "// Bind the \'K\' key to the testInput function\n"
   "moveMap.bind(keyboard, \"k\", testInput);\n\n"
   "@endtsexample\n\n\n"

   "bindCmd is an alternative method for binding commands. This function is similar to bind(), "
   "except two functions are set to be called when the event is processed.\n\n"
   "One will be called when the event is activated (input down), while the other is activated when the event is broken (input release). "
   "When using bindCmd(), pass the functions as strings rather than the function names.\n\n"

   "@tsexample\n"
   "// Print to the console when the spacebar is pressed\n"
   "function onSpaceDown()\n"
   "{\n"
   "   echo(\"Space bar down!\");\n"
   "}\n\n"

   "// Print to the console when the spacebar is released\n"
   "function onSpaceUp()\n"
   "{\n"
   "   echo(\"Space bar up!\");\n"
   "}\n\n"

   "// Bind the commands onSpaceDown and onSpaceUp to spacebar events\n"
   "moveMap.bindCmd(keyboard, \"space\", \"onSpaceDown();\", \"onSpaceUp();\");\n"
   "@endtsexample\n\n"
   
   "@section ActionMap_switching Switching ActionMaps\n"
   "Let's say you want to have different ActionMaps activated based on game play situations. A classic example would be first person shooter controls and racing controls "
   "in the same game. On foot, spacebar may cause your player to jump. In a vehicle, it may cause some kind of \"turbo charge\". You simply need to push/pop the ActionMaps appropriately:\n\n"

   "First, create two separate ActionMaps:\n\n"
   "@tsexample\n"
   "// Create the two ActionMaps\n"
   "if ( isObject( moveMap ) )\n"
   "  moveMap.delete();\n"
   "new ActionMap(moveMap);\n\n"
   "if ( isObject( carMap ) )\n"
   "  carMap.delete();\n"
   "new ActionMap(carMap);\n\n"
   "@endtsexample\n\n"

   "Next, create the two separate functions. Both will be bound to spacebar, but not the same ActionMap:\n\n"
   "@tsexample\n"
   "// Print to the console the player is jumping\n"
   "function playerJump(%val)\n"
   "{\n"
   "   if(%val)\n"
   "    echo(\"Player jumping!\");\n"
   "}\n\n"
   "// Print to the console the vehicle is charging\n"
   "function turboCharge()\n"
   "{\n"
   "   if(%val)\n"
   "    echo(\"Vehicle turbo charging!\");\n"
   "}\n"
   "@endtsexample\n\n"
   
   "You are now ready to bind functions to your ActionMaps' devices:\n\n"

   "@tsexample\n"
   "// Bind the spacebar to the playerJump function\n"
   "// when moveMap is the active ActionMap\n"
   "moveMap.bind(keyboard, \"space\", playerJump);\n\n"
   "// Bind the spacebar to the turboCharge function\n"
   "// when carMap is the active ActionMap\n"
   "carMap.bind(keyboard, \"space\", turboCharge);\n"
   "@endtsexample\n"

   "Finally, you can use the push() and pop() commands on each ActionMap to toggle activation. To activate an ActionMap, use push():\n\n"

   "@tsexample\n"
   "// Make moveMap the active action map\n"
   "// You should now be able to activate playerJump with spacebar\n"
   "moveMap.push();\n"
   "@endtsexample\n\n"

   "To switch ActionMaps, first pop() the old one. Then you can push() the new one:\n\n"

   "@tsexample\n"
   "// Deactivate moveMap\n"
   "moveMap.pop();\n\n"
   "// Activate carMap\n"
   "carMap.push();\n\n"
   "@endtsexample\n\n\n"

   "@ingroup Input"
   
);

// This is used for determing keys that have ascii codes for the foreign keyboards. IsAlpha doesn't work on foreign keys.
static inline bool dIsDecentChar(U8 c)
{
   return ((U8(0xa0) <= c) || (( U8(0x21) <= c) && (c <= U8(0x7e))) || ((U8(0x91) <= c) && (c <= U8(0x92))));
}

struct AsciiMapping
{
   const char* pDescription;
   U16         asciiCode;
};

extern AsciiMapping gAsciiMap[];

//------------------------------------------------------------------------------
//-------------------------------------- Action maps
//
Vector<ActionMap::BreakEntry> ActionMap::smBreakTable(__FILE__, __LINE__);


//------------------------------------------------------------------------------
ActionMap::ActionMap()
{
   VECTOR_SET_ASSOCIATION(mDeviceMaps);
}

//------------------------------------------------------------------------------
ActionMap::~ActionMap()
{
   for (U32 i = 0; i < mDeviceMaps.size(); i++)
      delete mDeviceMaps[i];
   mDeviceMaps.clear();
}

//------------------------------------------------------------------------------
ActionMap::DeviceMap::~DeviceMap()
{
   for(U32 i = 0; i < nodeMap.size(); i++)
   {
      dFree(nodeMap[i].makeConsoleCommand);
      dFree(nodeMap[i].breakConsoleCommand);
   }
}

//------------------------------------------------------------------------------
bool ActionMap::onAdd()
{
   if (Parent::onAdd() == false)
      return false;

   Sim::getActionMapGroup()->addObject(this);

   return true;
}

//--------------------------------------------------------------------------
void ActionMap::dumpActionMap(const char* fileName, const bool append) const
{
   if (fileName != NULL) {
      // Dump the deletion, and creation script commands, followed by all the binds
      //  to a script.

      FileStream *iostrm;
      if((iostrm = FileStream::createAndOpen( fileName, append ? Torque::FS::File::WriteAppend : Torque::FS::File::Write )) == NULL)
      {
         Con::errorf( "Unable to open file '%s' for writing.", fileName );
         return;
      }

      char lineBuffer[1024];
      if ( append )
         iostrm->setPosition( iostrm->getStreamSize() );
      else
      {
         // IMPORTANT -- do NOT change the following line, it identifies the file as an input map file
         dStrcpy( lineBuffer, "// Torque Input Map File\n" );
         iostrm->write( dStrlen( lineBuffer ), lineBuffer );
      }

      dSprintf(lineBuffer, 1023, "if (isObject(%s)) %s.delete();\n"
                                 "new ActionMap(%s);\n", getName(), getName(), getName());
      iostrm->write(dStrlen(lineBuffer), lineBuffer);

      // Dump all the binds to the console...
      for (S32 i = 0; i < mDeviceMaps.size(); i++) {
         const DeviceMap* pDevMap = mDeviceMaps[i];

         char devbuffer[32];
         getDeviceName(pDevMap->deviceType, pDevMap->deviceInst, devbuffer);

         for (S32 j = 0; j < pDevMap->nodeMap.size(); j++) {
            const Node& rNode = pDevMap->nodeMap[j];

            const char* pModifierString = getModifierString(rNode.modifiers);

            char objectbuffer[64];
            if (getKeyString(rNode.action, objectbuffer) == false)
               continue;

            const char* command = (rNode.flags & Node::BindCmd) ? "bindCmd" : "bind";

            dSprintf(lineBuffer, 1023, "%s.%s(%s, \"%s%s\"",
                                        getName(),
                                        command,
                                        devbuffer,
                                        pModifierString, objectbuffer);

            if (rNode.flags & (Node::HasScale|Node::HasDeadZone|Node::Ranged|Node::Inverted)) {
               char buff[10];
               U32 curr = 0;
               buff[curr++] = ',';
               buff[curr++] = ' ';
               if (rNode.flags & Node::HasScale)
                  buff[curr++] = 'S';
               if (rNode.flags & Node::Ranged)
                  buff[curr++] = 'R';
               if (rNode.flags & Node::HasDeadZone)
                  buff[curr++] = 'D';
               if (rNode.flags & Node::Inverted)
                  buff[curr++] = 'I';
               buff[curr] = '\0';

               dStrcat(lineBuffer, buff);
            }

            if (rNode.flags & Node::HasDeadZone) {
               char buff[64];
               dSprintf(buff, 63, ", \"%g %g\"", rNode.deadZoneBegin, rNode.deadZoneEnd);
               dStrcat(lineBuffer, buff);
            }

            if (rNode.flags & Node::HasScale) {
               char buff[64];
               dSprintf(buff, 63, ", %g", rNode.scaleFactor);
               dStrcat(lineBuffer, buff);
            }

            if (rNode.flags & Node::BindCmd) {
               if (rNode.makeConsoleCommand) {
                  dStrcat(lineBuffer, ", \"");
                  U32 pos = dStrlen(lineBuffer);
                  expandEscape(lineBuffer + pos, rNode.makeConsoleCommand);
                  dStrcat(lineBuffer, "\"");
               } else {
                  dStrcat(lineBuffer, ", \"\"");
               }
               if (rNode.breakConsoleCommand) {
                  dStrcat(lineBuffer, ", \"");
                  U32 pos = dStrlen(lineBuffer);
                  expandEscape(lineBuffer + pos, rNode.breakConsoleCommand);
                  dStrcat(lineBuffer, "\"");
               }
               else
                  dStrcat(lineBuffer, ", \"\"");
            } else {
               dStrcat(lineBuffer, ", ");
               dStrcat(lineBuffer, rNode.consoleFunction);
            }

            dStrcat(lineBuffer, ");\n");
            iostrm->write(dStrlen(lineBuffer), lineBuffer);
         }
      }

      delete iostrm;
   }
   else {
      // Dump all the binds to the console...
      for (S32 i = 0; i < mDeviceMaps.size(); i++) {
         const DeviceMap* pDevMap = mDeviceMaps[i];

         char devbuffer[32];
         getDeviceName(pDevMap->deviceType, pDevMap->deviceInst, devbuffer);

         for (S32 j = 0; j < pDevMap->nodeMap.size(); j++) {
            const Node& rNode = pDevMap->nodeMap[j];

            const char* pModifierString = getModifierString(rNode.modifiers);

            char keybuffer[64];
            if (getKeyString(rNode.action, keybuffer) == false)
               continue;

            const char* command = (rNode.flags & Node::BindCmd) ? "bindCmd" : "bind";

            char finalBuffer[1024];
            dSprintf(finalBuffer, 1023, "%s.%s(%s, \"%s%s\"",
                                        getName(),
                                        command,
                                        devbuffer,
                                        pModifierString, keybuffer);

            if (rNode.flags & (Node::HasScale|Node::HasDeadZone|Node::Ranged|Node::Inverted)) {
               char buff[10];
               U32 curr = 0;
               buff[curr++] = ',';
               buff[curr++] = ' ';
               if (rNode.flags & Node::HasScale)
                  buff[curr++] = 'S';
               if (rNode.flags & Node::Ranged)
                  buff[curr++] = 'R';
               if (rNode.flags & Node::HasDeadZone)
                  buff[curr++] = 'D';
               if (rNode.flags & Node::Inverted)
                  buff[curr++] = 'I';
               buff[curr] = '\0';

               dStrcat(finalBuffer, buff);
            }

            if (rNode.flags & Node::HasDeadZone) {
               char buff[64];
               dSprintf(buff, 63, ", \"%g %g\"", rNode.deadZoneBegin, rNode.deadZoneEnd);
               dStrcat(finalBuffer, buff);
            }

            if (rNode.flags & Node::HasScale) {
               char buff[64];
               dSprintf(buff, 63, ", %g", rNode.scaleFactor);
               dStrcat(finalBuffer, buff);
            }

            if (rNode.flags & Node::BindCmd) {
               if (rNode.makeConsoleCommand) {
                  dStrcat(finalBuffer, ", \"");
                  dStrcat(finalBuffer, rNode.makeConsoleCommand);
                  dStrcat(finalBuffer, "\"");
               } else {
                  dStrcat(finalBuffer, ", \"\"");
               }
               if (rNode.breakConsoleCommand) {
                  dStrcat(finalBuffer, ", \"");
                  dStrcat(finalBuffer, rNode.breakConsoleCommand);
                  dStrcat(finalBuffer, "\"");
               }
               else
                  dStrcat(finalBuffer, ", \"\"");
            } else {
               dStrcat(finalBuffer, ", ");
               dStrcat(finalBuffer, rNode.consoleFunction);
            }

            dStrcat(finalBuffer, ");");
            Con::printf(finalBuffer);
         }
      }
   }
}

//--------------------------------------------------------------------------
bool ActionMap::createEventDescriptor(const char* pEventString, EventDescriptor* pDescriptor)
{
   char copyBuffer[256];
   dStrcpy(copyBuffer, pEventString);

   // Do we have modifiers?
   char* pSpace = dStrchr(copyBuffer, ' ');
   char* pObjectString;
   if (pSpace != NULL) {
      // Yes.  Parse them out...
      //
      pDescriptor->flags = 0;
      pObjectString      = pSpace + 1;
      pSpace[0]          = '\0';

      char* pModifier = dStrtok(copyBuffer, "-");
      while (pModifier != NULL) {
         if (dStricmp(pModifier, "shift") == 0) {
            pDescriptor->flags |= SI_SHIFT;
         } else if (dStricmp(pModifier, "ctrl") == 0) {
            pDescriptor->flags |= SI_CTRL;
         } else if (dStricmp(pModifier, "alt") == 0) {
            pDescriptor->flags |= SI_ALT;
         } else if (dStricmp(pModifier, "cmd") == 0) {
            pDescriptor->flags |= SI_ALT;
         } else if (dStricmp(pModifier, "opt") == 0) {
            pDescriptor->flags |= SI_MAC_OPT;
         }

         pModifier = dStrtok(NULL, "-");
      }
   } else {
      // No.
      pDescriptor->flags = 0;
      pObjectString      = copyBuffer;
   }

   // Now we need to map the key string to the proper KEY code from event.h
   //
   AssertFatal(dStrlen(pObjectString) != 0, "Error, no key was specified!");

   if (dStrlen(pObjectString) == 1)
   {
      if (dIsDecentChar(*pObjectString)) // includes foreign chars
      {
         U16 asciiCode = (*pObjectString);
         // clear out the FF in upper 8bits for foreign keys??
         asciiCode &= 0xFF;
         U16 keyCode = Input::getKeyCode(asciiCode);
         if ( keyCode >= KEY_0 )
         {
            pDescriptor->eventType = SI_KEY;
            pDescriptor->eventCode = keyCode;
            return true;
         }
         else if (dIsalpha(*pObjectString) == true)
         {
            pDescriptor->eventType = SI_KEY;
            pDescriptor->eventCode = KEY_A+dTolower(*pObjectString)-'a';
            return true;
         }
         else if (dIsdigit(*pObjectString) == true)
         {
            pDescriptor->eventType = SI_KEY;
            pDescriptor->eventCode = KEY_0+(*pObjectString)-'0';
            return true;
         }
      }
      return false;
   }
   else
   {
      pDescriptor->eventCode = 0;
      // Gotta search through the Ascii table...
      for (U16 i = 0; gAsciiMap[i].asciiCode != 0xFFFF; i++)
      {
         if (dStricmp(pObjectString, gAsciiMap[i].pDescription) == 0)
         {
            U16 asciiCode = gAsciiMap[i].asciiCode;
            U16 keyCode   = Input::getKeyCode(asciiCode);
            if ( keyCode >= KEY_0 )
            {
               pDescriptor->eventType = SI_KEY;
               pDescriptor->eventCode = keyCode;
               return(true);

            }
            else
            {
               break;
            }
         }
      }
      // Didn't find an ascii match. Check the virtual map table
      //for (U32 j = 0; gVirtualMap[j].code != 0xFFFFFFFF; j++)
      //{
      //   if (dStricmp(pObjectString, gVirtualMap[j].pDescription) == 0)
      //   {
      //      pDescriptor->eventType = gVirtualMap[j].type;
      //      pDescriptor->eventCode = gVirtualMap[j].code;
      //      return true;
      //   }
      //}
      InputEventManager::VirtualMapData* data = INPUTMGR->findVirtualMap(pObjectString);
      if(data)
      {
         pDescriptor->eventType = data->type;
         pDescriptor->eventCode = data->code;
         return true;
      }
   }
   return false;
}

//------------------------------------------------------------------------------
ActionMap::Node* ActionMap::getNode(const U32 inDeviceType, const U32 inDeviceInst,
                   const U32 inModifiers,  const U32 inAction,SimObject* object /*= NULL*/)
{
   // DMMTODO - Slow INITIAL implementation.  Replace with a faster version...
   //
   DeviceMap* pDeviceMap = NULL;
   U32 i;
   for (i = 0; i < mDeviceMaps.size(); i++) 
   {
      if (mDeviceMaps[i]->deviceType == inDeviceType &&
          mDeviceMaps[i]->deviceInst == inDeviceInst) {
         pDeviceMap = mDeviceMaps[i];
         break;
      }
   }
   if (pDeviceMap == NULL) 
   {
      mDeviceMaps.increment();
      mDeviceMaps.last() = new DeviceMap;
      pDeviceMap = mDeviceMaps.last();

      pDeviceMap->deviceInst = inDeviceInst;
      pDeviceMap->deviceType = inDeviceType;
   }

   for (i = 0; i < pDeviceMap->nodeMap.size(); i++) 
   {
      if (pDeviceMap->nodeMap[i].modifiers == inModifiers &&
          pDeviceMap->nodeMap[i].action    == inAction &&
          ( (object != NULL) ? object == pDeviceMap->nodeMap[i].object : true )) // Check for an object match if the object exists 
      {
         return &pDeviceMap->nodeMap[i];
      }
   }

   // If we're here, the node doesn't exist.  create it.
   pDeviceMap->nodeMap.increment();

   Node* pRetNode = &pDeviceMap->nodeMap.last();
   pRetNode->modifiers = inModifiers;
   pRetNode->action    = inAction;

   pRetNode->flags         = 0;
   pRetNode->deadZoneBegin = 0.0;
   pRetNode->deadZoneEnd   = 0.0;
   pRetNode->scaleFactor   = 1.0;

   pRetNode->consoleFunction = NULL;
   pRetNode->makeConsoleCommand = NULL;
   pRetNode->breakConsoleCommand = NULL;

   //[neob, 5/7/2007 - #2975]
   pRetNode->object = 0;

   return pRetNode;
}

//------------------------------------------------------------------------------
void ActionMap::removeNode(const U32 inDeviceType, const U32 inDeviceInst, const U32 inModifiers, const U32 inAction, SimObject* object /*= NULL*/)
{
   // DMMTODO - Slow INITIAL implementation.  Replace with a faster version...
   //
   DeviceMap* pDeviceMap = NULL;
   U32 i;
   for (i = 0; i < mDeviceMaps.size(); i++) {
      if (mDeviceMaps[i]->deviceType == inDeviceType &&
          mDeviceMaps[i]->deviceInst == inDeviceInst) {
         pDeviceMap = mDeviceMaps[i];
         break;
      }
   }

   if (pDeviceMap == NULL)
      return;

   U32 realMods = inModifiers;
   if (realMods & SI_SHIFT)
      realMods |= SI_SHIFT;
   if (realMods & SI_CTRL)
      realMods |= SI_CTRL;
   if (realMods & SI_ALT)
      realMods |= SI_ALT;
   if (realMods & SI_MAC_OPT)
      realMods |= SI_MAC_OPT;

   for (i = 0; i < pDeviceMap->nodeMap.size(); i++) {
      if (pDeviceMap->nodeMap[i].modifiers == realMods &&
          pDeviceMap->nodeMap[i].action    == inAction &&
          ( (object != NULL) ? object == pDeviceMap->nodeMap[i].object : true )) 
      {
          dFree(pDeviceMap->nodeMap[i].makeConsoleCommand);
          dFree(pDeviceMap->nodeMap[i].breakConsoleCommand);
          pDeviceMap->nodeMap.erase(i);
      }
   }
}

//------------------------------------------------------------------------------
const ActionMap::Node* ActionMap::findNode(const U32 inDeviceType, const U32 inDeviceInst,
                    const U32 inModifiers,  const U32 inAction)
{
   // DMMTODO - Slow INITIAL implementation.  Replace with a faster version...
   //
   DeviceMap* pDeviceMap = NULL;
   U32 i;
   for (i = 0; i < mDeviceMaps.size(); i++)
   {
      if (mDeviceMaps[i]->deviceType == inDeviceType && mDeviceMaps[i]->deviceInst == inDeviceInst)
      {
         pDeviceMap = mDeviceMaps[i];
         break;
      }
   }

   if (pDeviceMap == NULL)
      return NULL;

   U32 realMods = inModifiers;
   if (realMods & SI_SHIFT)
      realMods |= SI_SHIFT;
   if (realMods & SI_CTRL)
      realMods |= SI_CTRL;
   if (realMods & SI_ALT)
      realMods |= SI_ALT;
   if (realMods & SI_MAC_OPT)
      realMods |= SI_MAC_OPT;

   for (i = 0; i < pDeviceMap->nodeMap.size(); i++)
   {
      // Special case for an ANYKEY bind...
      if (pDeviceMap->nodeMap[i].action == KEY_ANYKEY 
         && pDeviceMap->nodeMap[i].modifiers == realMods 
         && dIsDecentChar(inAction)
         && inAction <= U8_MAX)
         return &pDeviceMap->nodeMap[i];

      if (pDeviceMap->nodeMap[i].modifiers == realMods 
         && pDeviceMap->nodeMap[i].action == inAction)
         return &pDeviceMap->nodeMap[i];
   }

   return NULL;
}

//------------------------------------------------------------------------------
bool ActionMap::findBoundNode( const char* function, U32 &devMapIndex, U32 &nodeIndex )
{
   devMapIndex = 0;
   nodeIndex = 0;
   return nextBoundNode( function, devMapIndex, nodeIndex );
}

bool ActionMap::nextBoundNode( const char* function, U32 &devMapIndex, U32 &nodeIndex )
{
   // Loop through all of the existing nodes to find the one mapped to the
   // given function:
   for ( U32 i = devMapIndex; i < mDeviceMaps.size(); i++ )
   {
      const DeviceMap* dvcMap = mDeviceMaps[i];

      for ( U32 j = nodeIndex; j < dvcMap->nodeMap.size(); j++ )
      {
         const Node* node = &dvcMap->nodeMap[j];
         if ( !( node->flags & Node::BindCmd ) && ( dStricmp( function, node->consoleFunction ) == 0 ) )
         {
            devMapIndex = i;
            nodeIndex = j;
            return( true );
         }
      }

      nodeIndex = 0;
   }

   return( false );
}

//------------------------------------------------------------------------------
bool ActionMap::processUnbind(const char *device, const char *action, SimObject* object /*= NULL*/)
{
   U32 deviceType;
   U32 deviceInst;

   if(!getDeviceTypeAndInstance(device, deviceType, deviceInst))
      return false;
   EventDescriptor eventDescriptor;
   if (!createEventDescriptor(action, &eventDescriptor))
      return false;

   removeNode(deviceType, deviceInst, eventDescriptor.flags,eventDescriptor.eventCode, object);
   return true;
}

//------------------------------------------------------------------------------
// This function is for the use of the control remapper.
// It will only check against the console function (since all remappable commands are
// bound using bind and not bindCmd).
//
const char* ActionMap::getBinding( const char* command )
{
   char* returnString = Con::getReturnBuffer( 1024 );
   returnString[0] = 0;

   char buffer[256];
   char deviceBuffer[32];
   char keyBuffer[64];
 
   U32 devMapIndex = 0, nodeIndex = 0;
   while ( nextBoundNode( command, devMapIndex, nodeIndex ) )
   {
      const DeviceMap* deviceMap = mDeviceMaps[devMapIndex];

      if ( getDeviceName( deviceMap->deviceType, deviceMap->deviceInst, deviceBuffer ) )
      {
         const Node* node = &deviceMap->nodeMap[nodeIndex];
         const char* modifierString = getModifierString( node->modifiers );

         if ( getKeyString( node->action, keyBuffer ) )
         {
            dSprintf( buffer, sizeof( buffer ), "%s\t%s%s", deviceBuffer, modifierString, keyBuffer );
            if ( returnString[0] )
               dStrcat( returnString, "\t" );
            dStrcat( returnString, buffer );
         }
      }

      ++nodeIndex;
   }

   return returnString;
}

//------------------------------------------------------------------------------
// This function is for the use of the control remapper.
// The intent of this function is to determine if the given event descriptor is already
// bound in this action map.  If so, this function returns the command it is bound to.
// If not, it returns NULL.
//
const char* ActionMap::getCommand( const char* device, const char* action )
{
   U32 deviceType;
   U32 deviceInst;
   if ( getDeviceTypeAndInstance( device, deviceType, deviceInst ) )
   {
      EventDescriptor eventDescriptor;
      if ( createEventDescriptor( action, &eventDescriptor ) )
      {
         const ActionMap::Node* mapNode = findNode( deviceType, deviceInst, eventDescriptor.flags, eventDescriptor.eventCode );
         if ( mapNode )
         {
            if ( mapNode->flags & Node::BindCmd )
            {
               S32 bufferLen = dStrlen( mapNode->makeConsoleCommand ) + dStrlen( mapNode->breakConsoleCommand ) + 2;
               char* returnString = Con::getReturnBuffer( bufferLen );
               dSprintf( returnString, bufferLen, "%s\t%s",
                     ( mapNode->makeConsoleCommand ? mapNode->makeConsoleCommand : "" ),
                     ( mapNode->breakConsoleCommand ? mapNode->breakConsoleCommand : "" ) );             
               return( returnString );
            }              
            else
               return( mapNode->consoleFunction );             
         }
      }
   }

   return( "" );
}

//------------------------------------------------------------------------------
// This function returns whether or not the mapping specified is inverted.
// Obviously, this should only be used for axes.
bool ActionMap::isInverted( const char* device, const char* action )
{
   U32 deviceType;
   U32 deviceInst;
   if ( getDeviceTypeAndInstance( device, deviceType, deviceInst ) )
   {
      EventDescriptor eventDescriptor;
      if ( createEventDescriptor( action, &eventDescriptor ) )
      {
         const ActionMap::Node* mapNode = findNode( deviceType, deviceInst, eventDescriptor.flags, eventDescriptor.eventCode );
         if ( mapNode )
            return( mapNode->flags & Node::Inverted );
      }
   }

   Con::errorf( "The input event specified by %s %s is not in this action map!", device, action );
   return( false );
}

//------------------------------------------------------------------------------
F32 ActionMap::getScale( const char* device, const char* action )
{
   U32 deviceType;
   U32 deviceInst;
   if ( getDeviceTypeAndInstance( device, deviceType, deviceInst ) )
   {
      EventDescriptor eventDescriptor;
      if ( createEventDescriptor( action, &eventDescriptor ) )
      {
         const ActionMap::Node* mapNode = findNode( deviceType, deviceInst, eventDescriptor.flags, eventDescriptor.eventCode );
         if ( mapNode )
         {
            if ( mapNode->flags & Node::HasScale )
               return( mapNode->scaleFactor );
            else
               return( 1.0f );
         }
      }
   }

   Con::errorf( "The input event specified by %s %s is not in this action map!", device, action );
   return( 1.0f );
}

//------------------------------------------------------------------------------
const char* ActionMap::getDeadZone( const char* device, const char* action )
{
   U32 deviceType;
   U32 deviceInst;
   if ( getDeviceTypeAndInstance( device, deviceType, deviceInst ) )
   {
      EventDescriptor eventDescriptor;
      if ( createEventDescriptor( action, &eventDescriptor ) )
      {
         const ActionMap::Node* mapNode = findNode( deviceType, deviceInst, eventDescriptor.flags, eventDescriptor.eventCode );
         if ( mapNode )
         {
            if ( mapNode->flags & Node::HasDeadZone )
            {
               char buf[64];
               dSprintf( buf, sizeof( buf ), "%g %g", mapNode->deadZoneBegin, mapNode->deadZoneEnd );
               char* returnString = Con::getReturnBuffer( dStrlen( buf ) + 1 );
               dStrcpy( returnString, buf );
               return( returnString );
            }
            else
               return( "0 0" );                    
         }
      }
   }

   Con::errorf( "The input event specified by %s %s is not in this action map!", device, action );
   return( "" );
}

//------------------------------------------------------------------------------
const char* ActionMap::buildActionString( const InputEventInfo* event )
{
   const char* modifierString = getModifierString( event->modifier );

   char objectBuffer[64];
   if ( !getKeyString( event->objInst, objectBuffer ) )
      return( "" );

   U32 returnLen = dStrlen( modifierString ) + dStrlen( objectBuffer ) + 2;   
   char* returnString = Con::getReturnBuffer( returnLen );
   dSprintf( returnString, returnLen - 1, "%s%s", modifierString, objectBuffer );
   return( returnString );
}

//------------------------------------------------------------------------------
bool ActionMap::getDeviceTypeAndInstance(const char *pDeviceName, U32 &deviceType, U32 &deviceInstance)
{
   U32 offset = 0;
   U32 inputMgrDeviceType = 0;

   if (dStrnicmp(pDeviceName, "keyboard", dStrlen("keyboard")) == 0) 
   {
      deviceType      = KeyboardDeviceType;
      offset = dStrlen("keyboard");
   } 
   else if (dStrnicmp(pDeviceName, "mouse", dStrlen("mouse")) == 0) 
   {
      deviceType      = MouseDeviceType;
      offset = dStrlen("mouse");
   } 
   else if (dStrnicmp(pDeviceName, "joystick", dStrlen("joystick")) == 0) 
   {
      deviceType      = JoystickDeviceType;
      offset = dStrlen("joystick");
   }
   else if (dStrnicmp(pDeviceName, "gamepad", dStrlen("gamepad")) == 0)
   {
      deviceType = GamepadDeviceType;
      offset     = dStrlen("gamepad");
   }
   else if(INPUTMGR->isRegisteredDeviceWithAttributes(pDeviceName, inputMgrDeviceType, offset))
   {
      deviceType = inputMgrDeviceType;
   }
   else 
   {
      return false;
   }

   if (dStrlen(pDeviceName) > offset) 
   {
      const char* pInst = pDeviceName + offset;
      S32 instNum = dAtoi(pInst);
      
      if (instNum < 0)
         deviceInstance = 0;
      else   
         deviceInstance = instNum;
   }
   else 
   {
      deviceInstance = 0;
   }

   return true;
}

//------------------------------------------------------------------------------
bool ActionMap::getDeviceName(const U32 deviceType, const U32 deviceInstance, char* buffer)
{
   switch (deviceType) {
     case KeyboardDeviceType:
      dStrcpy(buffer, "keyboard");
      break;

     case MouseDeviceType:
      dSprintf(buffer, 16, "mouse%d", deviceInstance);
      break;

     case JoystickDeviceType:
      dSprintf(buffer, 16, "joystick%d", deviceInstance);
      break;

     case GamepadDeviceType:
      dSprintf(buffer, 16, "gamepad%d", deviceInstance);
      break;

     default:
      {
         const char* name = INPUTMGR->getRegisteredDeviceName(deviceType);
         if(!name)
         {
            Con::errorf( "ActionMap::getDeviceName: unknown device type specified, %d (inst: %d)", deviceType, deviceInstance);
            return false;
         }

         dSprintf(buffer, 16, "%s%d", name, deviceInstance);
         break;
      }
   }

   return true;
}

//------------------------------------------------------------------------------
const char* ActionMap::getModifierString(const U32 modifiers)
{
   U32 realModifiers = modifiers;
   if ( modifiers & SI_LSHIFT || modifiers & SI_RSHIFT )
      realModifiers |= SI_SHIFT;
   if ( modifiers & SI_LCTRL || modifiers & SI_RCTRL )
      realModifiers |= SI_CTRL;
   if ( modifiers & SI_LALT || modifiers & SI_RALT )
      realModifiers |= SI_ALT;
   if ( modifiers & SI_MAC_LOPT || modifiers & SI_MAC_ROPT )
      realModifiers |= SI_MAC_OPT;

   switch (realModifiers & (SI_SHIFT|SI_CTRL|SI_ALT|SI_MAC_OPT)) 
   {
#if defined(TORQUE_OS_MAC)
      // optional code, to output alt as cmd on mac.
      // interpreter sees them as the same...
     case (SI_SHIFT|SI_CTRL|SI_ALT):
      return "cmd-shift-ctrl ";

     case (SI_SHIFT|SI_ALT):
      return "cmd-shift ";

     case (SI_CTRL|SI_ALT):
      return "cmd-ctrl ";

     case (SI_ALT):
      return "cmd ";
#else
     case (SI_SHIFT|SI_CTRL|SI_ALT):
      return "shift-ctrl-alt ";

     case (SI_SHIFT|SI_ALT):
      return "shift-alt ";

     case (SI_CTRL|SI_ALT):
      return "ctrl-alt ";

     case (SI_ALT):
      return "alt ";
#endif
     case (SI_SHIFT|SI_CTRL):
      return "shift-ctrl ";

     case (SI_SHIFT):
      return "shift ";

     case (SI_CTRL):
      return "ctrl ";

// plus new mac cases:
     case (SI_ALT|SI_SHIFT|SI_CTRL|SI_MAC_OPT):
      return "cmd-shift-ctrl-opt ";

     case (SI_ALT|SI_SHIFT|SI_MAC_OPT):
      return "cmd-shift-opt ";

     case (SI_ALT|SI_CTRL|SI_MAC_OPT):
      return "cmd-ctrl-opt ";

     case (SI_ALT|SI_MAC_OPT):
      return "cmd-opt ";

     case (SI_SHIFT|SI_CTRL|SI_MAC_OPT):
      return "shift-ctrl-opt ";

     case (SI_SHIFT|SI_MAC_OPT):
      return "shift-opt ";

     case (SI_CTRL|SI_MAC_OPT):
      return "ctrl-opt ";

     case (SI_MAC_OPT):
      return "opt ";
      
     case 0:
      return "";

     default:
      AssertFatal(false, "Error, should never reach the default case in getModifierString");
      return "";
   }
}

//------------------------------------------------------------------------------
bool ActionMap::getKeyString(const U32 action, char* buffer)
{
   U16 asciiCode = 0;

   // This is a special case.... numpad keys do have ascii values
   // but for the purposes of this method we want to return the 
   // description from the gVirtualMap.
   if ( !( KEY_NUMPAD0 <= action && action <= KEY_NUMPAD9 ) )
      asciiCode = Input::getAscii( action, STATE_LOWER );

//   if (action >= KEY_A && action <= KEY_Z) {
//      buffer[0] = char(action - KEY_A + 'a');
//      buffer[1] = '\0';
//      return true;
//   } else if (action >= KEY_0 && action <= KEY_9) {
//      buffer[0] = char(action - KEY_0 + '0');
//      buffer[1] = '\0';
   if ( (asciiCode != 0) && dIsDecentChar((char)asciiCode))
   {
      for (U32 i = 0; gAsciiMap[i].asciiCode != 0xFFFF; i++) {
         if (gAsciiMap[i].asciiCode == asciiCode)
         {
            dStrcpy(buffer, gAsciiMap[i].pDescription);
            return true;
         }
      }
      // Must not have found a string for that ascii code just record the char
      buffer[0] = char(asciiCode);
      buffer[1] = '\0';
      return true;
   }
   else
   {
      if (action >= KEY_A && action <= KEY_Z)
      {
         buffer[0] = char(action - KEY_A + 'a');
         buffer[1] = '\0';
         return true;
      }
      else if (action >= KEY_0 && action <= KEY_9) {
         buffer[0] = char(action - KEY_0 + '0');
         buffer[1] = '\0';
         return true;
      }
      //for (U32 i = 0; gVirtualMap[i].code != 0xFFFFFFFF; i++) {
      //   if (gVirtualMap[i].code == action) {
      //      dStrcpy(buffer, gVirtualMap[i].pDescription);
      //      return true;
      //   }
      //}
      const char* desc = INPUTMGR->findVirtualMapDescFromCode(action);
      if(desc)
      {
         dStrcpy(buffer, desc);
         return true;
      }
   }

   Con::errorf( "ActionMap::getKeyString: no string for action %d", action );
   return false;
}

//--------------------------------------------------------------------------
bool ActionMap::processBindCmd(const char *device, const char *action, const char *makeCmd, const char *breakCmd)
{
   U32 deviceType;
   U32 deviceInst;

   if(!getDeviceTypeAndInstance(device, deviceType, deviceInst))
   {
      Con::printf("processBindCmd: unknown device: %s", device);
      return false;
   }

   // Ok, we now have the deviceType and instance.  Create an event descriptor
   //  for the bind...
   //
   EventDescriptor eventDescriptor;
   if (createEventDescriptor(action, &eventDescriptor) == false) {
      Con::printf("Could not create a description for binding: %s", action);
      return false;
   }

   // SI_POV == SI_MOVE, and the POV works fine with bindCmd, so we have to add these manually.
   if( ( eventDescriptor.eventCode == SI_XAXIS ) ||
       ( eventDescriptor.eventCode == SI_YAXIS ) ||
       ( eventDescriptor.eventCode == SI_ZAXIS ) ||
       ( eventDescriptor.eventCode == SI_RXAXIS ) ||
       ( eventDescriptor.eventCode == SI_RYAXIS ) ||
       ( eventDescriptor.eventCode == SI_RZAXIS ) ||
       ( eventDescriptor.eventCode == SI_SLIDER ) ||
       ( eventDescriptor.eventCode == SI_XPOV ) ||
       ( eventDescriptor.eventCode == SI_YPOV ) ||
       ( eventDescriptor.eventCode == SI_XPOV2 ) ||
       ( eventDescriptor.eventCode == SI_YPOV2 ) )
   {
      Con::warnf( "ActionMap::processBindCmd - Cannot use 'bindCmd' with a move event type. Use 'bind' instead." );
      return false;
   }

   // Create the full bind entry, and place it in the map
   //
   // DMMTODO
   Node* pBindNode = getNode(deviceType, deviceInst,
                             eventDescriptor.flags,
                             eventDescriptor.eventCode);

   pBindNode->flags           = Node::BindCmd;
   pBindNode->deadZoneBegin   = 0;
   pBindNode->deadZoneEnd     = 0;
   pBindNode->scaleFactor     = 1;

   if( pBindNode->makeConsoleCommand )
      dFree( pBindNode->makeConsoleCommand );
   if( pBindNode->breakConsoleCommand )
      dFree( pBindNode->breakConsoleCommand );

   if(makeCmd[0])
      pBindNode->makeConsoleCommand = dStrdup(makeCmd);
   else
      pBindNode->makeConsoleCommand = dStrdup("");

   if(breakCmd[0])
      pBindNode->breakConsoleCommand = dStrdup(breakCmd);
   else
      pBindNode->breakConsoleCommand = dStrdup("");
   return true;
}

//------------------------------------------------------------------------------
bool ActionMap::processBind(const U32 argc, const char** argv, SimObject* object)
{
   // Ok, the bind will come in the following format:
   //  [device] [key or button] <[param spec] [param] ...> [fnName]
   //
   const char* pDeviceName = argv[0];
   const char* pEvent      = argv[1];
   const char* pFnName     = argv[argc - 1];

   // Determine the device
   U32 deviceType;
   U32 deviceInst;

   if(!getDeviceTypeAndInstance(argv[0], deviceType, deviceInst))
   {
      Con::printf("processBind: unknown device: %s", pDeviceName);
      return false;
   }

   // Ok, we now have the deviceType and instance.  Create an event descriptor
   //  for the bind...
   //
   EventDescriptor eventDescriptor;
   if (createEventDescriptor(pEvent, &eventDescriptor) == false) 
   {
      Con::printf("Could not create a description for binding: %s", pEvent);
      return false;
   }

   // Event has now been described, and device determined.  we need now to extract
   //  any modifiers that the action map will apply to incoming events before
   //  calling the bound function...
   //
   // DMMTODO
   U32 assignedFlags = 0;
   F32 deadZoneBegin = 0.0f;
   F32 deadZoneEnd   = 0.0f;
   F32 scaleFactor   = 1.0f;

   if (argc != 3) {
      // We have the following: "[DSIR]" [deadZone] [scale]
      //
      const char* pSpec = argv[2];

      for (U32 i = 0; pSpec[i] != '\0'; i++) {
         switch (pSpec[i]) {
           case 'r': case 'R':
            assignedFlags |= Node::Ranged;
            break;
           case 's': case 'S':
            assignedFlags |= Node::HasScale;
            break;
           case 'd': case 'D':
            assignedFlags |= Node::HasDeadZone;
            break;
           case 'i': case 'I':
            assignedFlags |= Node::Inverted;
            break;
           case 'n': case 'N':
            assignedFlags |= Node::NonLinear;
            break;

           default:
            AssertFatal(false, avar("Misunderstood specifier in bind (spec string: %s)",
                                    pSpec));
         }
      }

      // Ok, we have the flags.  Scan the dead zone and scale, if any.
      //
      U32 curArg = 3;
      if (assignedFlags & Node::HasDeadZone) {
         dSscanf(argv[curArg], "%g %g", &deadZoneBegin, &deadZoneEnd);
         curArg++;
      }
      if (assignedFlags & Node::HasScale) {
         scaleFactor = dAtof(argv[curArg]);
         curArg++;
      }

      if (curArg != (argc - 1)) {
         AssertFatal(curArg == (argc - 1), "error in bind spec somewhere...");
         Con::printf("Improperly specified bind for key: %s", argv[2]);
         return false;
      }
   }

   // Ensure that the console function is properly specified?
   //
   // DMMTODO

   // Create the full bind entry, and place it in the map
   //
   // DMMTODO
   Node* pBindNode = getNode(deviceType, deviceInst,
                             eventDescriptor.flags,
                             eventDescriptor.eventCode, object);

   pBindNode->flags           = assignedFlags;
   pBindNode->deadZoneBegin   = deadZoneBegin;
   pBindNode->deadZoneEnd     = deadZoneEnd;
   pBindNode->scaleFactor     = scaleFactor;
   pBindNode->object          = object;
   pBindNode->consoleFunction = StringTable->insert(pFnName);

   return true;
}

//------------------------------------------------------------------------------
bool ActionMap::processAction(const InputEventInfo* pEvent)
{
   // Suppress excluded input events, like alt-tab.
   if(Platform::checkKeyboardInputExclusion(pEvent))
      return false;

   static const char *argv[5];
   if (pEvent->action == SI_MAKE) {
      const Node* pNode = findNode(pEvent->deviceType, pEvent->deviceInst,
                                   pEvent->modifier,   pEvent->objInst);

      if( pNode == NULL )
         return false;

      // Enter the break into the table if this is a make event...
      // Do this now rather than after command is processed because
      // command might add a binding which can move the vector of nodes.
      enterBreakEvent(pEvent, pNode);

      // Whadda ya know, we have this bound.  Set up, and call the console
      //  function associated with it...
      //
      F32 value = pEvent->fValue;
      if (pNode->flags & Node::Ranged) {
         value = (value * 2.0f) - 1.0f;
         if (pNode->flags & Node::Inverted)
            value *= -1.0f;
      } else {
         if (pNode->flags & Node::Inverted)
            value = 1.0f - value;
      }

      if (pNode->flags & Node::HasScale)
         value *= pNode->scaleFactor;

      if ( pNode->flags & Node::HasDeadZone )
      {
         if ( value >= pNode->deadZoneBegin && value <= pNode->deadZoneEnd ) 
            value = 0.0f;
         else
         {
            if( value > 0 )
               value = ( value - pNode->deadZoneBegin ) * ( 1.f / ( 1.f - pNode->deadZoneBegin ) );
            else
               value = ( value + pNode->deadZoneBegin ) * ( 1.f / ( 1.f - pNode->deadZoneBegin ) );
         }
      }

      if( pNode->flags & Node::NonLinear )
         value = ( value < 0.f ? -1.f : 1.f ) * mPow( mFabs( value ), CONST_E );

      // Ok, we're all set up, call the function.
      if(pNode->flags & Node::BindCmd)
      {
         // it's a bind command
         if(pNode->makeConsoleCommand)
            Con::evaluate(pNode->makeConsoleCommand);
      }
      else if ( pNode->consoleFunction[0] )
      {
         argv[0] = pNode->consoleFunction;
         argv[1] = Con::getFloatArg(value);
         if (pNode->object)
            Con::executef(pNode->object, argv[0], argv[1]);
         else
            Con::execute(2, argv);
      }
      return true;
   } else if (pEvent->action == SI_MOVE) {
      if (pEvent->deviceType == MouseDeviceType) {
         const Node* pNode = findNode(pEvent->deviceType, pEvent->deviceInst,
                                      pEvent->modifier,   pEvent->objInst);

         if( pNode == NULL )
            return false;

         // "Do nothing" bind:
         if ( !pNode->consoleFunction[0] )
            return( true );

         // Whadda ya know, we have this bound.  Set up, and call the console
         //  function associated with it.  Mouse events ignore range and dead
         //  zone params.
         //
         F32 value = pEvent->fValue;
         if (pNode->flags & Node::Inverted)
            value *= -1.0f;
         if (pNode->flags & Node::HasScale)
            value *= pNode->scaleFactor;

         // Ok, we're all set up, call the function.
         argv[0] = pNode->consoleFunction;
         argv[1] = Con::getFloatArg(value);
         if (pNode->object)
            Con::executef(pNode->object, argv[0], argv[1]);
         else
            Con::execute(2, argv);

         return true;
      }
      else if ( (pEvent->objType == SI_POS || pEvent->objType == SI_FLOAT || pEvent->objType == SI_ROT || pEvent->objType == SI_INT)
                && INPUTMGR->isRegisteredDevice(pEvent->deviceType)
              )
      {
         const Node* pNode = findNode(pEvent->deviceType, pEvent->deviceInst,
                                      pEvent->modifier,   pEvent->objInst);

         if( pNode == NULL )
            return false;

         // Ok, we're all set up, call the function.
         argv[0] = pNode->consoleFunction;
         S32 argc = 1;

         if (pEvent->objType == SI_INT)
         {
            // Handle the integer as some sort of motion such as a
            // single component to an absolute position
            argv[1] = Con::getIntArg( pEvent->iValue );
            argc += 1;
         }
         else if (pEvent->objType == SI_FLOAT)
         {
            // Handle float as some sort of motion such as a
            // single component to an absolute position
            argv[1] = Con::getFloatArg( pEvent->fValue );
            argc += 1;
         }
         else if (pEvent->objType == SI_POS)
         {
            // Handle Point3F type position
            argv[1] = Con::getFloatArg( pEvent->fValue );
            argv[2] = Con::getFloatArg( pEvent->fValue2 );
            argv[3] = Con::getFloatArg( pEvent->fValue3 );

            argc += 3;
         }
         else
         {
            // Handle rotation (AngAxisF)
            AngAxisF aa(Point3F(pEvent->fValue, pEvent->fValue2, pEvent->fValue3), pEvent->fValue4);
            aa.axis.normalize();
            argv[1] = Con::getFloatArg( aa.axis.x );
            argv[2] = Con::getFloatArg( aa.axis.y );
            argv[3] = Con::getFloatArg( aa.axis.z );
            argv[4] = Con::getFloatArg( mRadToDeg(aa.angle) );

            argc += 4;
         }

         if (pNode->object)
         {
            Con::execute(pNode->object, argc, argv);
         }
         else
         {
            Con::execute(argc, argv);
         }

         return true;
      }
      else if ( pEvent->deviceType == JoystickDeviceType 
                || pEvent->deviceType == GamepadDeviceType
                || INPUTMGR->isRegisteredDevice(pEvent->deviceType)
              )
      {
         // Joystick events...
         const Node* pNode = findNode( pEvent->deviceType, pEvent->deviceInst,
                                       pEvent->modifier,   pEvent->objInst );

         if( pNode == NULL )
            return false;

         // "Do nothing" bind:
         if ( !pNode->consoleFunction[0] )
            return( true );

         // Whadda ya know, we have this bound.  Set up, and call the console
         //  function associated with it.  Joystick move events are the same as mouse
         //  move events except that they don't ignore dead zone.
         //
         F32 value = pEvent->fValue;
         if ( pNode->flags & Node::Inverted )
            value *= -1.0f;

         if ( pNode->flags & Node::HasScale )
            value *= pNode->scaleFactor;

         if ( pNode->flags & Node::HasDeadZone )
         {
            if ( value >= pNode->deadZoneBegin &&
                 value <= pNode->deadZoneEnd )
               value = 0.0f;
            else
            {
               if( value > 0 )
                  value = ( value - pNode->deadZoneEnd ) * ( 1.f / ( 1.f - pNode->deadZoneEnd ) );
               else
                  value = ( value - pNode->deadZoneBegin ) * ( 1.f / ( 1.f + pNode->deadZoneBegin ) );
            }
         }

         if( pNode->flags & Node::NonLinear )
            value = ( value < 0.f ? -1.f : 1.f ) * mPow( mFabs( value ), CONST_E );

         // Ok, we're all set up, call the function.
         argv[0] = pNode->consoleFunction;
         argv[1] = Con::getFloatArg( value );
         if (pNode->object)
            Con::executef(pNode->object, argv[0], argv[1]);
         else
            Con::execute(2, argv);

         return true;
      }
   }
   else if (pEvent->action == SI_BREAK)
   {
      return checkBreakTable(pEvent);
   }
   else if (pEvent->action == SI_VALUE)
   {
      if ( (pEvent->objType == SI_FLOAT || pEvent->objType == SI_INT)
                && INPUTMGR->isRegisteredDevice(pEvent->deviceType)
              )
      {
         const Node* pNode = findNode(pEvent->deviceType, pEvent->deviceInst,
                                      pEvent->modifier,   pEvent->objInst);

         if( pNode == NULL )
            return false;

         // Ok, we're all set up, call the function.
         argv[0] = pNode->consoleFunction;
         S32 argc = 1;

         if (pEvent->objType == SI_INT)
         {
            // Handle the integer as some sort of motion such as a
            // single component to an absolute position
            argv[1] = Con::getIntArg( pEvent->iValue );
            argc += 1;
         }
         else if (pEvent->objType == SI_FLOAT)
         {
            // Handle float as some sort of motion such as a
            // single component to an absolute position
            argv[1] = Con::getFloatArg( pEvent->fValue );
            argc += 1;
         }

         if (pNode->object)
         {
            Con::execute(pNode->object, argc, argv);
         }
         else
         {
            Con::execute(argc, argv);
         }

         return true;
      }
   }

   return false;
}

//------------------------------------------------------------------------------
bool ActionMap::isAction( U32 deviceType, U32 deviceInst, U32 modifiers, U32 action )
{
   return ( findNode( deviceType, deviceInst, modifiers, action ) != NULL );
}

//------------------------------------------------------------------------------
ActionMap* ActionMap::getGlobalMap()
{
   SimSet* pActionMapSet = Sim::getActiveActionMapSet();
   AssertFatal( pActionMapSet && pActionMapSet->size() != 0,
                "error, no ActiveMapSet or no global action map...");

   return ( ( ActionMap* ) pActionMapSet->first() );
}

//------------------------------------------------------------------------------
void ActionMap::enterBreakEvent(const InputEventInfo* pEvent, const Node* pNode)
{
   // There aren't likely to be many breaks outstanding at any one given time,
   //  so a simple linear search is probably sufficient.  Note that the break table
   //  is static to the class, all breaks are directed to the action map that received
   //  the make.
   //
   S32 entry = -1;
   for (U32 i = 0; i < smBreakTable.size(); i++) {
      if (smBreakTable[i].deviceType == U32(pEvent->deviceType) &&
          smBreakTable[i].deviceInst == U32(pEvent->deviceInst) &&
          smBreakTable[i].objInst    == U32(pEvent->objInst)) {
         // Match.
         entry = i;
         break;
      }
   }
   if (entry == -1) {
      smBreakTable.increment();
      entry = smBreakTable.size() - 1;

      smBreakTable[entry].deviceType = pEvent->deviceType;
      smBreakTable[entry].deviceInst = pEvent->deviceInst;
      smBreakTable[entry].objInst    = pEvent->objInst;
   }

   // Ok, we now have the entry, and know that the device desc. and the objInst match.
   //  Copy out the node information...
   //
   smBreakTable[entry].object = pNode->object;
   smBreakTable[entry].consoleFunction = pNode->consoleFunction;
   if(pNode->breakConsoleCommand)
      smBreakTable[entry].breakConsoleCommand = dStrdup(pNode->breakConsoleCommand);
   else
      smBreakTable[entry].breakConsoleCommand = NULL;

   smBreakTable[entry].flags         = pNode->flags;
   smBreakTable[entry].deadZoneBegin = pNode->deadZoneBegin;
   smBreakTable[entry].deadZoneEnd   = pNode->deadZoneEnd;
   smBreakTable[entry].scaleFactor   = pNode->scaleFactor;
}

//------------------------------------------------------------------------------
bool ActionMap::checkBreakTable(const InputEventInfo* pEvent)
{
   for (U32 i = 0; i < smBreakTable.size(); i++) 
   {
      if (smBreakTable[i].deviceType == U32(pEvent->deviceType) &&
          smBreakTable[i].deviceInst == U32(pEvent->deviceInst) &&
          smBreakTable[i].objInst    == U32(pEvent->objInst)) 
      {
         fireBreakEvent(i, pEvent->fValue);
         return true;
      }
   }

   return false;
}

//------------------------------------------------------------------------------
bool ActionMap::handleEvent(const InputEventInfo* pEvent)
{
   // Interate through the ActionMapSet until we get a map that
   //  handles the event or we run out of maps...
   //
   SimSet* pActionMapSet = Sim::getActiveActionMapSet();
   AssertFatal(pActionMapSet && pActionMapSet->size() != 0,
               "error, no ActiveMapSet or no global action map...");
               
   for (SimSet::iterator itr = pActionMapSet->end() - 1;
        itr > pActionMapSet->begin(); itr--) {
      ActionMap* pMap = static_cast<ActionMap*>(*itr);
      if (pMap->processAction(pEvent) == true)
         return true;
   }
   
   // Found no matching action.  Try with the modifiers stripped.

   InputEventInfo eventNoModifiers = *pEvent;
   eventNoModifiers.modifier = ( InputModifiers ) 0;
   
   for (SimSet::iterator itr = pActionMapSet->end() - 1;
        itr > pActionMapSet->begin(); itr--) {
      ActionMap* pMap = static_cast<ActionMap*>(*itr);
      if( pMap->processAction( &eventNoModifiers ) )
         return true;
   }

   return false;
}

//------------------------------------------------------------------------------
bool ActionMap::handleEventGlobal(const InputEventInfo* pEvent)
{
   return getGlobalMap()->processAction( pEvent );
}

//------------------------------------------------------------------------------
bool ActionMap::checkAsciiGlobal( U16 key, U32 modifiers )
{
   // Does this ascii map to a key?
   U16 keyCode = Input::getKeyCode(key);
   if(keyCode == 0)
      return false;

   // Grab the action map set.
   SimSet* pActionMapSet = Sim::getActiveActionMapSet();
   AssertFatal(pActionMapSet && pActionMapSet->size() != 0,
      "error, no ActiveMapSet or no global action map...");

   // Grab the device maps for the first ActionMap.
   Vector<DeviceMap*> &maps = ((ActionMap*)pActionMapSet->first())->mDeviceMaps;

   // Find the keyboard.
   DeviceMap *keyMap = NULL;
   for(S32 i=0; i<maps.size(); i++)
   {
      // CodeReview Doesn't deal with multiple keyboards [bjg, 5/16/07]
      if(maps[i]->deviceType == KeyboardDeviceType)
      {
         keyMap = maps[i];
         break;
      }
   }

   if(!keyMap)
      return false;

   // Normalize modifiers.
   U32 realMods = modifiers;
   if (realMods & SI_SHIFT)
      realMods |= SI_SHIFT;
   if (realMods & SI_CTRL)
      realMods |= SI_CTRL;
   if (realMods & SI_ALT)
      realMods |= SI_ALT;
   if (realMods & SI_MAC_OPT)
      realMods |= SI_MAC_OPT;

   // Now find a matching node, if there is one.
   for(S32 i=0; i<keyMap->nodeMap.size(); i++)
   {
      Node &n = keyMap->nodeMap[i];

      if(n.action == keyCode && (n.modifiers == modifiers))
         return true;
   }

   return false;
}

void ActionMap::clearAllBreaks()
{
   while(smBreakTable.size())
      fireBreakEvent(smBreakTable.size()-1);
}

void ActionMap::fireBreakEvent( U32 i, F32 fValue )
{
   // Match.  Issue the break event...
   //
   F32 value = fValue;
   if (smBreakTable[i].flags & Node::Ranged) {
      value = (value * 2.0f) - 1.0f;
      if (smBreakTable[i].flags & Node::Inverted)
         value *= -1.0f;
   } else {
      if (smBreakTable[i].flags & Node::Inverted)
         value = 1.0f - value;
   }

   if (smBreakTable[i].flags & Node::HasScale)
      value *= smBreakTable[i].scaleFactor;

   if (smBreakTable[i].flags & Node::HasDeadZone)
   {
      if (value >= smBreakTable[i].deadZoneBegin &&
         value <= smBreakTable[i].deadZoneEnd)
         value = 0.0f;
      else
      {
         if( value > 0 )
            value = ( value - smBreakTable[i].deadZoneBegin ) * ( 1.f / ( 1.f - smBreakTable[i].deadZoneBegin ) );
         else
            value = ( value + smBreakTable[i].deadZoneBegin ) * ( 1.f / ( 1.f - smBreakTable[i].deadZoneBegin ) );
      }
   }

   if( smBreakTable[i].flags & Node::NonLinear )
      value = ( value < 0.f ? -1.f : 1.f ) * mPow( mFabs( value ), CONST_E );

   // Ok, we're all set up, call the function.
   if(smBreakTable[i].consoleFunction)
   {
      if ( smBreakTable[i].consoleFunction[0] )
      {
         static const char *argv[2];
         argv[0] = smBreakTable[i].consoleFunction;
         argv[1] = Con::getFloatArg(value);
         if (smBreakTable[i].object)
            Con::executef(smBreakTable[i].object, argv[0], argv[1]);
         else
            Con::execute(2, argv);
      }
   }
   else if(smBreakTable[i].breakConsoleCommand)
   {
      Con::evaluate(smBreakTable[i].breakConsoleCommand);
      dFree(smBreakTable[i].breakConsoleCommand);
   }
   smBreakTable.erase(i);
}

//------------------------------------------------------------------------------

// Console interop version.

static ConsoleDocFragment _ActionMapbind1(
   "@brief Associates a function to an input event.\n\n"
   "When the input event is raised, the specified function will be called.\n\n"
   "@param device The input device, such as mouse or keyboard.\n"
   "@param action The input event, such as space, button0, etc.\n"
   "@param command The function to bind to the action. Function must have a single boolean argument.\n"
   "@return True if the binding was successful, false if the device was unknown or description failed.\n\n"
   "@tsexample\n"
   "// Simple function that prints to console\n"
   "// %val - Sent by the device letting the user know\n"
   "// if an input was pressed (true) or released (false)\n"
   "function testInput(%val)\n"
   "{\n"
   "   if(%val)\n"
   "    echo(\"Key is down\");\n"
   "   else\n"
   "    echo(\"Key was released\");\n"
   "}\n\n"
   "// Bind the \'K\' key to the testInput function\n"
   "moveMap.bind(keyboard, k, testInput);\n\n"
   "@endtsexample\n\n\n",
   "ActionMap",
   "bool bind( string device, string action, string command );");

static ConsoleDocFragment _ActionMapbind2(
   "@brief Associates a function and input parameters to an input event.\n\n"
   "When the input event is raised, the specified function will be called. Modifier flags may be specified to process "
   "dead zones, input inversion, and more.\n\n"
   "Valid modifier flags:\n\n"
   " - R - Input is Ranged.\n"
   " - S - Input is Scaled.\n"
   " - I - Input is inverted.\n"
   " - D - Dead zone is present.\n"
   " - N - Input should be re-fit to a non-linear scale.\n\n"
   "@param device The input device, such as mouse or keyboard.\n"
   "@param action The input event, such as space, button0, etc.\n"
   "@param flag Modifier flag assigned during binding, letting event know there are additional parameters to consider. \n"
   "@param deadZone Restricted region in which device motion will not be acknowledged.\n"
   "@param scale Modifies the deadZone region.\n"
   "@param command The function bound to the action. Must take in a single argument.\n"
   "@return True if the binding was successful, false if the device was unknown or description failed.\n\n"
   "@tsexample\n"
   "// Simple function that adjusts the pitch of the camera based on the "
   "mouse's movement along the X axis.\n"
   "function testPitch(%val)\n"
   "{\n"
   "   %pitchAdj = getMouseAdjustAmount(%val);\n"
   "   $mvPitch += %pitchAdj;\n"
   "}\n\n"
   "// Bind the mouse's X axis to the testPitch function\n"
   "// DI is flagged, meaning input is inverted and has a deadzone\n"
   "%this.bind( mouse, \"xaxis\", \"DI\", \"-0.23 0.23\", testPitch );\n"
   "@endtsexample\n\n\n",
   "ActionMap",
   "bool bind( string device, string action, string flag, string deadZone, string scale, string command );");

ConsoleMethod( ActionMap, bind, bool, 5, 10, "actionMap.bind( device, action, [modifier spec, mod...], command )" 
           "@hide")
{
   StringStackWrapper args(argc - 2, argv + 2);
   return object->processBind( args.count(), args, NULL );
}

static ConsoleDocFragment _ActionMapbindObj1(
   "@brief Associates a function to an input event for a specified class or object.\n\n"
   "You must specify a device, the action to bind, a function, and an object to be called when the event happens. "
   "The function specified must be set to receive a single boolean value passed.\n\n"
   "@param device The input device, such as mouse or keyboard.\n"
   "@param action The input event, such as space, button0, etc.\n"
   "@param command The function bound to the action.\n"
   "@param object The object or class bound to the action.\n"
   "@return True if the binding was successful, false if the device was unknown or description failed.\n\n"
   "@tsexample\n"
   "moveMap.bindObj(keyboard, \"numpad1\", \"rangeChange\", %player);"
   "@endtsexample\n\n",
   "ActionMap",
   "bool bindObj( string device, string action, string command, SimObjectID object );");

static ConsoleDocFragment _ActionMapbindObj2(
   "@brief Associates a function to an input event for a specified class or object.\n\n"
   "You must specify a device, the action to bind, a function, and an object to be called when the event happens. "
   "The function specified must be set to receive a single boolean value passed. Modifier flags may be specified to process "
   "dead zones, input inversion, and more.\n\n"
   "Valid modifier flags:\n\n"
   " - R - Input is Ranged.\n"
   " - S - Input is Scaled.\n"
   " - I - Input is inverted.\n"
   " - D - Dead zone is present.\n"
   " - N - Input should be re-fit to a non-linear scale.\n\n"
   "@param device The input device, such as mouse or keyboard.\n"
   "@param action The input event, such as space, button0, etc.\n"
   "@param flag Modifier flag assigned during binding, letting event know there are additional parameters to consider.\n"
   "@param deadZone [Required only when flag is set] Restricted region in which device motion will not be acknowledged.\n"
   "@param scale [Required only when flag is set] Modifies the deadZone region.\n"
   "@param command The function bound to the action.\n"
   "@param object The object or class bound to the action.\n"
   "@return True if the binding was successful, false if the device was unknown or description failed.\n\n"
   "@tsexample\n"
   "// Bind the mouse's movement along the x-axis to the testInput function of the Player class\n"
   "// DSI is flagged, meaning input is inverted, has scale and has a deadzone\n"
   "%this.bindObj( mouse, \"xaxis\", \"DSI\", %deadZone, %scale, \"testInput\", %player );\n"
   "@endtsexample\n\n\n",
   "ActionMap",
   "bool bindObj( string device, string action, string flag, string deadZone, string scale, string command, SimObjectID object );");

ConsoleMethod( ActionMap, bindObj, bool, 6, 11, "(device, action, [modifier spec, mod...], command, object)"
           "@hide")
{
   SimObject* simObject = Sim::findObject(argv[argc - 1]);
   if ( simObject == NULL )
   {
      Con::warnf("ActionMap::bindObj() - Cannot bind, specified object was not found!");
      return false;
   }

   StringStackWrapper args(argc - 3, argv + 2);
   return object->processBind( args.count(), args, simObject );
}

//------------------------------------------------------------------------------

DefineEngineMethod( ActionMap, bindCmd, bool, ( const char* device, const char* action, const char* makeCmd, const char* breakCmd ), ( "" ),
    "@brief Associates a make command and optional break command to a specified input device action.\n\n"
    "Must include parenthesis and semicolon in the make and break command strings.\n\n"
    "@param device The device to bind to. Can be a keyboard, mouse, joystick or gamepad.\n"
    "@param action The device action to bind to. The action is dependant upon the device. Specify a key for keyboards.\n"
    "@param makeCmd The command to execute when the device/action is made.\n"
    "@param breakCmd [optional] The command to execute when the device or action is unmade.\n"
    "@return True the bind was successful, false if the device was unknown or description failed.\n"
   "@tsexample\n"
   "// Print to the console when the spacebar is pressed\n"
   "function onSpaceDown()\n"
   "{\n"
   "   echo(\"Space bar down!\");\n"
   "}\n\n"
   "// Print to the console when the spacebar is released\n"
   "function onSpaceUp()\n"
   "{\n"
   "   echo(\"Space bar up!\");\n"
   "}\n\n"
   "// Bind the commands onSpaceDown() and onSpaceUp() to spacebar events\n\n"
   "moveMap.bindCmd(keyboard, \"space\", \"onSpaceDown();\", \"onSpaceUp();\");\n"
   "@endtsexample\n\n")
{
   return object->processBindCmd( device, action, makeCmd, breakCmd );
}

DefineEngineMethod( ActionMap, unbind, bool, ( const char* device, const char* action ),,
   "@brief Removes the binding on an input device and action.\n"
   "@param device The device to unbind from. Can be a keyboard, mouse, joystick or a gamepad.\n"
   "@param action The device action to unbind from. The action is dependant upon the device. Specify a key for keyboards.\n"
   "@return True if the unbind was successful, false if the device was unknown or description failed.\n\n"
   "@tsexample\n"
   "moveMap.unbind(\"keyboard\", \"space\");\n"
   "@endtsexample\n\n")
{
   return object->processUnbind( device, action );
}

DefineEngineMethod( ActionMap, unbindObj, bool, ( const char* device, const char* action, const char* obj ),,
   "@brief Remove any object-binding on an input device and action.\n"
   "@param device The device to bind to.  Can be keyboard, mouse, joystick or gamepad.\n"
   "@param action The device action to unbind from. The action is dependant upon the device. Specify a key for keyboards.\n"
   "@param obj The object to perform unbind against.\n"
   "@return True if the unbind was successful, false if the device was unknown or description failed.\n"
   "@tsexample\n"
   "moveMap.unbindObj(\"keyboard\", \"numpad1\", \"rangeChange\", %player);"
   "@endtsexample\n\n\n")
{
    SimObject* simObject = Sim::findObject(obj);
    if ( simObject == NULL )
    {
        Con::warnf("ActionMap::unbindObj() - Cannot unbind, specified object was not found!");
        return false;
    }

    return object->processUnbind( device, action, simObject );
}

DefineEngineMethod( ActionMap, save, void, ( const char* fileName, bool append ), ( nullAsType<const char*>(), false ),
   "@brief Saves the ActionMap to a file or dumps it to the console.\n\n"
   "@param fileName The file path to save the ActionMap to. If a filename is not specified "
   " the ActionMap will be dumped to the console.\n"
   "@param append Whether to write the ActionMap at the end of the file or overwrite it.\n"
   "@tsexample\n"
   "// Write out the actionmap into the config.cs file\n"
   "moveMap.save( \"scripts/client/config.cs\" );"
   "@endtsexample\n\n")
{
   char buffer[1024];

   if(fileName)
   {
      if(Con::expandScriptFilename(buffer, sizeof(buffer), fileName))
         fileName = buffer;
   }

   object->dumpActionMap( fileName, append );
}

DefineEngineFunction( getCurrentActionMap, ActionMap*, (),,
   "@brief Returns the current %ActionMap.\n"
   "@see ActionMap"
   "@ingroup Input")
{
   SimSet* pActionMapSet = Sim::getActiveActionMapSet();
   return dynamic_cast< ActionMap* >( pActionMapSet->last() );
}

DefineEngineMethod( ActionMap, push, void, (),,
   "@brief Push the ActionMap onto the %ActionMap stack.\n\n"
   "Activates an ActionMap and placees it at the top of the ActionMap stack.\n\n"
   "@tsexample\n"
   "// Make moveMap the active action map\n"
   "moveMap.push();\n"
   "@endtsexample\n\n"
   "@see ActionMap")
{
   SimSet* pActionMapSet = Sim::getActiveActionMapSet();
   pActionMapSet->pushObject( object );
}

DefineEngineMethod( ActionMap, pop, void, (),,
   "@brief Pop the ActionMap off the %ActionMap stack.\n\n"
   "Deactivates an %ActionMap and removes it from the @ActionMap stack.\n"
   "@tsexample\n"
   "// Deactivate moveMap\n"
   "moveMap.pop();\n"
   "@endtsexample\n\n"
   "@see ActionMap")
{
   SimSet* pActionMapSet = Sim::getActiveActionMapSet();
   pActionMapSet->removeObject( object );
}

DefineEngineMethod( ActionMap, getBinding, const char*, ( const char* command ),,
   "@brief Gets the ActionMap binding for the specified command.\n\n"
   "Use getField() on the return value to get the device and action of the binding.\n"
   "@param command The function to search bindings for.\n"
   "@return The binding against the specified command. Returns an empty string(\"\") "
   "if a binding wasn't found.\n"
   "@tsexample\n"
   "// Find what the function \"jump()\" is bound to in moveMap\n"
   "%bind = moveMap.getBinding( \"jump\" );\n\n"
   "if ( %bind !$= \"\" )\n"
   "{\n"
   "// Find out what device is used in the binding\n"
   "  %device = getField( %bind, 0 );\n\n"
   "// Find out what action (such as a key) is used in the binding\n"
   "  %action = getField( %bind, 1 );\n"
   "}\n"
   "@endtsexample\n\n"
   "@see getField")
{
   return object->getBinding( command );  
}

DefineEngineMethod( ActionMap, getCommand, const char*, ( const char* device, const char* action ),,
   "@brief Gets ActionMap command for the device and action.\n\n"
   "@param device The device that was bound. Can be a keyboard, mouse, joystick or a gamepad.\n"
   "@param action The device action that was bound.  The action is dependant upon the device. Specify a key for keyboards.\n"
   "@return The command against the specified device and action.\n"
   "@tsexample\n"
   "// Find what function is bound to a device\'s action\n"
   "// In this example, \"jump()\" was assigned to the space key in another script\n"
   "%command = moveMap.getCommand(\"keyboard\", \"space\");\n\n"
   "// Should print \"jump\" in the console\n"
   "echo(%command)\n"
   "@endtsexample\n\n")
{
   return object->getCommand( device, action ); 
}

DefineEngineMethod( ActionMap, isInverted, bool, ( const char* device, const char* action ),,
   "@brief Determines if the specified device and action is inverted.\n\n"
   "Should only be used for scrolling devices or gamepad/joystick axes."
   "@param device The device that was bound. Can be a keyboard, mouse, joystick or a gamepad.\n"
   "@param action The device action that was bound.  The action is dependant upon the device. Specify a key for keyboards.\n"
   "@return True if the specified device and action is inverted.\n"
   "@tsexample\n"
   "%if ( moveMap.isInverted( \"mouse\", \"xaxis\"))\n"
   "   echo(\"Mouse's xAxis is inverted\");"
   "@endtsexample\n\n")
{
   return object->isInverted( device, action ); 
}

DefineEngineMethod( ActionMap, getScale, F32, ( const char* device, const char* action ),,
   "@brief Get any scaling on the specified device and action.\n\n"
   "@param device The device that was bound. Can be keyboard, mouse, joystick or gamepad.\n"
   "@param action The device action that was bound. The action is dependant upon the device. Specify a key for keyboards.\n"
   "@return Any scaling applied to the specified device and action.\n"
   "@tsexample\n"
   "%scale = %moveMap.getScale( \"gamepad\", \"thumbrx\");\n"
   "@endtsexample\n\n")
{
   return object->getScale( device, action );   
}

DefineEngineMethod( ActionMap, getDeadZone, const char*, ( const char* device, const char* action ),,
   "@brief Gets the Dead zone for the specified device and action.\n\n"
   "@param device The device that was bound.  Can be a keyboard, mouse, joystick or a gamepad.\n"
   "@param action The device action that was bound. The action is dependant upon the device. Specify a key for keyboards.\n"
   "@return The dead zone for the specified device and action. Returns \"0 0\" if there is no dead zone " 
   "or an empty string(\"\") if the mapping was not found.\n"
   "@tsexample\n"
   "%deadZone = moveMap.getDeadZone( \"gamepad\", \"thumbrx\");\n"
   "@endtsexample\n\n")
{
   return object->getDeadZone( device, action );   
}

//------------------------------------------------------------------------------
AsciiMapping gAsciiMap[] =
{
   //--- KEYBOARD EVENTS
   //
   { "space",           0x0020 },
   //{ "exclamation",     0x0021 },
   { "doublequote",     0x0022 },
   //{ "pound",           0x0023 },
   //{ "ampersand",       0x0026 },
   { "apostrophe",      0x0027 },
   //{ "lparen",          0x0028 },
   //{ "rparen",          0x0029 },
   { "comma",           0x002c },
   { "minus",           0x002d },
   { "period",          0x002e },
   //{ "slash",           0x002f },
   //{ "colon",           0x003a },
   //{ "semicolon",       0x003b },
   //{ "lessthan",        0x003c },
   //{ "equals",          0x003d },
   //{ "morethan",        0x003e },
   //{ "lbracket",        0x005b },
   { "backslash",       0x005c },
   //{ "rbracket",        0x005d },
   //{ "circumflex",      0x005e },
   //{ "underscore",      0x005f },
   { "grave",           0x0060 },
   //{ "tilde",           0x007e },
   //{ "vertbar",         0x007c },
   //{ "exclamdown",      0x00a1 },
   //{ "cent",            0x00a2 },
   //{ "sterling",        0x00a3 },
   //{ "currency",        0x00a4 },
   //{ "brokenbar",       0x00a6 },
   //{ "ring",            0x00b0 },
   //{ "plusminus",       0x00b1 },
   { "super2",          0x00b2 },
   { "super3",          0x00b3 },
   { "acute",           0x00b4 },
   //{ "mu",              0x00b5 },
   //{ "ordmasculine",    0x00ba },
   //{ "questiondown",    0x00bf },
   //{ "gemandbls",       0x00df },
   //{ "agrave",          0x00e0 },
   //{ "aacute",          0x00e1 },
   //{ "acircumflex",     0x00e2 },
   //{ "atilde",          0x00e3 },
   //{ "adieresis",       0x00e4 },
   //{ "aring",           0x00e5 },
   //{ "ae",              0x00e6 },
   //{ "ccedille",        0x00e7 },
   //{ "egrave",          0x00e8 },
   //{ "eacute",          0x00e9 },
   //{ "ecircumflex",     0x00ea },
   //{ "edieresis",       0x00eb },
   //{ "igrave",          0x00ec },
   //{ "iacute",          0x00ed },
   //{ "icircumflex",     0x00ee },
   //{ "idieresis",       0x00ef },
   //{ "ntilde",          0x00f1 },
   //{ "ograve",          0x00f2 },
   //{ "oacute",          0x00f3 },
   //{ "ocircumflex",     0x00f4 },
   //{ "otilde",          0x00f5 },
   //{ "odieresis",       0x00f6 },
   //{ "divide",          0x00f7 },
   //{ "oslash",          0x00f8 },
   //{ "ugrave",          0x00f9 },
   //{ "uacute",          0x00fa },
   //{ "ucircumflex",     0x00fb },
   //{ "udieresis",       0x00fc },
   //{ "ygrave",          0x00fd },
   //{ "thorn",           0x00fe },
   //{ "ydieresis",       0x00ff },
   { "nomatch",         0xFFFF }
};
