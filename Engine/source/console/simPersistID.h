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

#ifndef _SIMPERSISTID_H_
#define _SIMPERSISTID_H_

#ifndef _TORQUE_UUID_H_
   #include "core/util/uuid.h"
#endif
#ifndef _REFBASE_H_
   #include "core/util/refBase.h"
#endif


/// @file
/// Persistent IDs for SimObjects.


class SimObject;
template< typename, typename > class HashTable;


/// A globally unique persistent ID for a SimObject.
class SimPersistID : public StrongRefBase
{
   public:
   
      typedef void Parent;
      friend class SimObject;
      
   protected:
   
      typedef HashTable< Torque::UUID, SimPersistID* > LookupTableType;
   
      /// Reference to the SimObject.  Will be NULL for as long as the
      /// persistent ID is not resolved.
      SimObject* mObject;
   
      /// The UUID assigned to the object.  Never changes.
      Torque::UUID mUUID;
      
      /// Table of persistent object IDs.
      static LookupTableType* smLookupTable;

      /// Construct a new persistent ID for "object" by generating a fresh
      /// unique identifier.
      SimPersistID( SimObject* object );
      
      /// Construct a persistent ID stub for the given unique identifier.
      /// The stub remains not bound to any object until it is resolved.
      SimPersistID( const Torque::UUID& uuid );
      
      ///
      ~SimPersistID();
      
      /// Bind this unresolved PID to the given object.
      void resolve( SimObject* object );
      
      ///
      void unresolve() { mObject = NULL; }

      /// Create a persistent ID for the given object.
      static SimPersistID* create( SimObject* object );
         
   public:
   
      /// Initialize the persistent ID system.
      static void init();
      
      /// Uninitialize the persistent ID system.
      static void shutdown();
      
      /// Look up a persistent ID by its UUID.  Return NULL if no PID is bound to the given UUID.
      static SimPersistID* find( const Torque::UUID& uuid );

      /// Look up a persistent ID by its UUID.  If no PID is bound to the given UUID yet, create a
      /// new PID and bind it to the UUID.
      static SimPersistID* findOrCreate( const Torque::UUID& uuid );
      
      /// Find a SimObject by the UUID assigned to its PID.  Return NULL if either no PID is bound
      /// to the given UUID or if the PID bound to it is not yet resolved.
      static SimObject* findObjectByUUID( const Torque::UUID& uuid );
            
      /// Return the object that is bound to this PID.  If the PID has not yet been resolved,
      /// return NULL.
      SimObject* getObject() const { return mObject; }
      
      /// Return the UUID bound to this PID.
      const Torque::UUID& getUUID() const { return mUUID; }
};

#endif // !_SIMPERSISTID_H_
