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

#include "component/dynamicConsoleMethodComponent.h"

IMPLEMENT_CO_NETOBJECT_V1(DynamicConsoleMethodComponent);

ConsoleDocClass( DynamicConsoleMethodComponent,
				"@brief Console object used for calling methods defined in script, from within other classes.\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

//-----------------------------------------------------------
// Function name:  SimComponent::handlesConsoleMethod
// Summary:
//-----------------------------------------------------------
bool DynamicConsoleMethodComponent::handlesConsoleMethod( const char *fname, S32 *routingId )
{
   // CodeReview: Host object is now given priority over components for method
   // redirection. [6/23/2007 Pat]

   // On this object?
   if( isMethod( fname ) )
   {
      *routingId = -1; // -1 denotes method on object
#ifdef TORQUE_DEBUG
      // Inject Method.
      injectMethodCall( fname );
#endif
      return true;
   }

   // on this objects components?
   S32 nI = 0;
   VectorPtr<SimComponent*> &componentList = lockComponentList();
   for( SimComponentIterator nItr = componentList.begin(); nItr != componentList.end(); nItr++, nI++ )
   {
      SimObject *pComponent = dynamic_cast<SimObject*>(*nItr);
      if( pComponent != NULL && pComponent->isMethod( fname ) )
      {
         *routingId = -2; // -2 denotes method on component
         unlockComponentList();

#ifdef TORQUE_DEBUG
         // Inject Method.
         injectMethodCall( fname );
#endif
         return true;
      }
   }
   unlockComponentList();

   return false;
}

const char *DynamicConsoleMethodComponent::callMethod( S32 argc, const char* methodName, ... )
{
   const char *argv[128];
   methodName = StringTable->insert( methodName );

   argc++;

   va_list args;
   va_start(args, methodName);
   for(S32 i = 0; i < argc; i++)
      argv[i+2] = va_arg(args, const char *);
   va_end(args);

   // FIXME: the following seems a little excessive. I wonder why it's needed?
   argv[0] = methodName;
   argv[1] = methodName;
   argv[2] = methodName;

   return callMethodArgList( argc , argv );
}

#ifdef TORQUE_DEBUG
/// Inject Method Call.
void DynamicConsoleMethodComponent::injectMethodCall( const char* method )
{
   // Get Call Method.
   StringTableEntry callMethod = StringTable->insert( method );

   // Find Call Method Metric.
   callMethodMetricType::Iterator itr = mCallMethodMetrics.find( callMethod );

   // Did we find the method?
   if ( itr == mCallMethodMetrics.end() )
   {
      // No, so set the call count to initially be 1.
      itr = mCallMethodMetrics.insert( callMethod, 1 );
   }
   else
   {
      // Increment Call Count.
      itr->value++;
   }
}
#endif

const char* DynamicConsoleMethodComponent::callMethodArgList( U32 argc, const char *argv[], bool callThis /* = true  */ )
{
#ifdef TORQUE_DEBUG
   injectMethodCall( argv[0] );
#endif

   return _callMethod( argc, argv, callThis );
}

// Call all components that implement methodName giving them a chance to operate
// Components are called in reverse order of addition
const char *DynamicConsoleMethodComponent::_callMethod( U32 argc, const char *argv[], bool callThis /* = true  */ )
{
   // Set Owner
   SimObject *pThis = dynamic_cast<SimObject *>( this );
   AssertFatal( pThis, "DynamicConsoleMethodComponent::callMethod : this should always exist!" );

   const char *cbName = StringTable->insert(argv[0]);

   if( getComponentCount() > 0 )
   {
      lockComponentList();
      for( int i = getComponentCount() - 1; i >= 0; i-- )
      //for( SimComponentIterator nItr = componentList.end();  nItr != componentList.begin(); nItr-- )
      {
         argv[0] = cbName;

         SimComponent *pComponent = dynamic_cast<SimComponent *>( getComponent( i ) );
         AssertFatal( pComponent, "DynamicConsoleMethodComponent::callMethod - NULL component in list!" );

         DynamicConsoleMethodComponent *pThisComponent = dynamic_cast<DynamicConsoleMethodComponent*>( pComponent );
         AssertFatal( pThisComponent, "DynamicConsoleMethodComponent::callMethod - Non DynamicConsoleMethodComponent component attempting to callback!");

         // Only call on first depth components
         // Should isMethod check these calls?  [11/22/2006 justind]
         if(pComponent->isEnabled())
            Con::execute( pThisComponent, argc, argv );

         // Bail if this was the first element
         //if( nItr == componentList.begin() )
         //   break;
      }
      unlockComponentList();
   }
   
   // Set Owner Field
   const char* result = "";
   if(callThis)
      result = Con::execute( pThis, argc, argv, true ); // true - exec method onThisOnly, not on DCMCs

   return result;
}

ConsoleMethod( DynamicConsoleMethodComponent, callMethod, void, 3, 64 , "(methodName, argi) Calls script defined method\n"
			  "@param methodName The method's name as a string\n"
			  "@param argi Any arguments to pass to the method\n"
			  "@return No return value"
			  "@note %obj.callMethod( %methodName, %arg1, %arg2, ... );\n")

{
   object->callMethodArgList( argc - 1, argv + 2 );
}

//////////////////////////////////////////////////////////////////////////

