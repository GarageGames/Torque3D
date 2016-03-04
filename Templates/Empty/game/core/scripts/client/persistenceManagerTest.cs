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

new PersistenceManager(TestPManager);

function runPManTest(%test)
{
   if (!isObject(TestPManager))
      return;

   if (%test $= "")
      %test = 100;

   switch(%test)
   {
      case 0:
         TestPManager.testFieldUpdates();
      case 1:
         TestPManager.testObjectRename();
      case 2:
         TestPManager.testNewObject();
      case 3:
         TestPManager.testNewGroup();
      case 4:
         TestPManager.testMoveObject();
      case 5:
         TestPManager.testObjectRemove();
      case 100:
         TestPManager.testFieldUpdates();
         TestPManager.testObjectRename();
         TestPManager.testNewObject();
         TestPManager.testNewGroup();
         TestPManager.testMoveObject();
         TestPManager.testObjectRemove();
   }
}

function TestPManager::testFieldUpdates(%doNotSave)
{
   // Set some objects as dirty
   TestPManager.setDirty(AudioGui);
   TestPManager.setDirty(AudioSim);
   TestPManager.setDirty(AudioMessage);

   // Alter some of the existing fields
   AudioEffect.isLooping         = true;
   AudioMessage.isLooping     = true;
   AudioEffect.is3D              = true;

   // Test removing a field
   TestPManager.removeField(AudioGui, "isLooping");

   // Alter some of the persistent fields
   AudioGui.referenceDistance     = 0.8;
   AudioMessage.referenceDistance = 0.8;

   // Add some new dynamic fields
   AudioGui.foo = "bar";
   AudioEffect.foo = "bar";

   // Remove an object from the dirty list
   // It shouldn't get updated in the file
   TestPManager.removeDirty(AudioEffect);

   // Dirty an object in another file as well
   TestPManager.setDirty(WarningMaterial);

   // Update a field that doesn't exist
   WarningMaterial.glow[0] = true;

   // Drity another object to test for crashes
   // when a dirty object is deleted
   TestPManager.setDirty(SFXPausedSet);

   // Delete the object
   SFXPausedSet.delete();

   // Unless %doNotSave is set (by a batch/combo test)
   // then go ahead and save now
   if (!%doNotSave)
      TestPManager.saveDirty();
}

function TestPManager::testObjectRename(%doNotSave)
{
   // Flag an object as dirty
   if (isObject(AudioGui))
      TestPManager.setDirty(AudioGui);
   else if (isObject(AudioGuiFoo))
      TestPManager.setDirty(AudioGuiFoo);

   // Rename it
   if (isObject(AudioGui))
      AudioGui.setName(AudioGuiFoo);
   else if (isObject(AudioGuiFoo))
      AudioGuiFoo.setName(AudioGui);

   // Unless %doNotSave is set (by a batch/combo test)
   // then go ahead and save now
   if (!%doNotSave)
      TestPManager.saveDirty();
}

function TestPManager::testNewObject(%doNotSave)
{
   // Test adding a new named object
   new SFXDescription(AudioNew)
   {
      volume = 0.5;
      isLooping = true;
      channel  = $GuiAudioType;
      foo = 2;
   };

   // Flag it as dirty
   TestPManager.setDirty(AudioNew, "core/scripts/client/audio.cs");

   // Test adding a new unnamed object
   %obj = new SFXDescription()
   {
      volume = 0.75;
      isLooping = true;
      bar = 3;
   };

   // Flag it as dirty
   TestPManager.setDirty(%obj, "core/scripts/client/audio.cs");

   // Test adding an "empty" object
   new SFXDescription(AudioEmpty);

   TestPManager.setDirty(AudioEmpty, "core/scripts/client/audio.cs");

   // Unless %doNotSave is set (by a batch/combo test)
   // then go ahead and save now
   if (!%doNotSave)
      TestPManager.saveDirty();
}

function TestPManager::testNewGroup(%doNotSave)
{
   // Test adding a new named SimGroup
   new SimGroup(TestGroup)
   {
      foo = "bar";

      new SFXDescription(TestObject)
      {
         volume = 0.5;
         isLooping = true;
         channel  = $GuiAudioType;
         foo = 1;
      };
      new SimGroup(SubGroup)
      {
         foo = 2;

         new SFXDescription(SubObject)
         {
            volume = 0.5;
            isLooping = true;
            channel  = $GuiAudioType;
            foo = 3;
         };
      };
   };

   // Flag this as dirty
   TestPManager.setDirty(TestGroup, "core/scripts/client/audio.cs");

   // Test adding a new unnamed SimGroup
   %group = new SimGroup()
   {
      foo = "bar";

      new SFXDescription()
      {
         volume = 0.75;
         channel  = $GuiAudioType;
         foo = 4;
      };
      new SimGroup()
      {
         foo = 5;

         new SFXDescription()
         {
            volume = 0.75;
            isLooping = true;
            channel  = $GuiAudioType;
            foo = 6;
         };
      };
   };

   // Flag this as dirty
   TestPManager.setDirty(%group, "core/scripts/client/audio.cs");

   // Test adding a new unnamed SimSet
   %set = new SimSet()
   {
      foo = "bar";

      new SFXDescription()
      {
         volume = 0.75;
         channel  = $GuiAudioType;
         foo = 7;
      };
      new SimGroup()
      {
         foo = 8;

         new SFXDescription()
         {
            volume = 0.75;
            isLooping = true;
            channel  = $GuiAudioType;
            foo = 9;
         };
      };
   };

   // Flag this as dirty
   TestPManager.setDirty(%set, "core/scripts/client/audio.cs");

   // Unless %doNotSave is set (by a batch/combo test)
   // then go ahead and save now
   if (!%doNotSave)
      TestPManager.saveDirty();
}

function TestPManager::testMoveObject(%doNotSave)
{
   // First add a couple of groups to the file
   new SimGroup(MoveGroup1)
   {
      foo = "bar";

      new SFXDescription(MoveObject1)
      {
         volume = 0.5;
         isLooping = true;
         channel  = $GuiAudioType;
         foo = 1;
      };

      new SimSet(SubGroup1)
      {
         new SFXDescription(SubObject1)
         {
            volume = 0.75;
            isLooping = true;
            channel  = $GuiAudioType;
            foo = 2;
         };
      };
   };

   // Flag this as dirty
   TestPManager.setDirty(MoveGroup1, "core/scripts/client/audio.cs");

   new SimGroup(MoveGroup2)
   {
      foo = "bar";

      new SFXDescription(MoveObject2)
      {
         volume = 0.5;
         isLooping = true;
         channel  = $GuiAudioType;
         foo = 3;
      };
   };

   // Flag this as dirty
   TestPManager.setDirty(MoveGroup2, "core/scripts/client/audio.cs");

   // Unless %doNotSave is set (by a batch/combo test)
   // then go ahead and save now
   if (!%doNotSave)
      TestPManager.saveDirty();

   // Set them as dirty again
   TestPManager.setDirty(MoveGroup1);
   TestPManager.setDirty(MoveGroup2);

   // Give the subobject an new value
   MoveObject1.foo = 4;

   // Move it into the other group
   MoveGroup1.add(MoveObject2);

   // Switch the other subobject
   MoveGroup2.add(MoveObject1);

   // Also add a new unnamed object to one of the groups
   %obj = new SFXDescription()
   {
      volume = 0.75;
      isLooping = true;
      bar = 5;
   };

   MoveGroup1.add(%obj);

   // Unless %doNotSave is set (by a batch/combo test)
   // then go ahead and save now
   if (!%doNotSave)
      TestPManager.saveDirty();
}

function TestPManager::testObjectRemove(%doNotSave)
{
   TestPManager.removeObjectFromFile(AudioSim);
}
