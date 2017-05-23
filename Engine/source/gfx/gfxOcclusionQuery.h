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

#ifndef _GFXOCCLUSIONQUERY_H_
#define _GFXOCCLUSIONQUERY_H_

#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif


/// A geometry visibility query object.
/// @see GFXDevice::createOcclusionQuery
class GFXOcclusionQuery : public GFXResource
{
protected:

   GFXDevice *mDevice;
   
   GFXOcclusionQuery( GFXDevice *device ) 
      : mDevice( device )
   {
   }   

public:

   /// The states returned by getStatus()
   /// If you modify this enum you should also modify statusToString()
   enum OcclusionQueryStatus
   {
      Unset,         ///<      
      Waiting,       ///< 
      Error,         ///< 
      Occluded,
      NotOccluded
   };   
      
   virtual ~GFXOcclusionQuery() {}
   
   /// Prepares the query returning true if the last query
   /// has been processed and more geometry can be issued.
   /// @see getStatus
   virtual bool begin() = 0;
   
   /// Called after your geometry is drawn to submit
   /// the query for processing.
   virtual void end() = 0;   
   
   /// Returns the status of the last submitted query.  In general
   /// you should avoid blocking for the result until the frame 
   /// following your query to keep from stalling the CPU.
   /// @return       Status    
   /// @param block  If true CPU will block until the query finishes.
   /// @param data   Number of pixels rendered, valid only if status returned is NotOccluded.
   virtual OcclusionQueryStatus getStatus( bool block, U32 *data = NULL ) = 0;
   
   /// Returns a status string.
   static String statusToString( OcclusionQueryStatus status );

   // GFXResource
   virtual void zombify() = 0;   
   virtual void resurrect() = 0;
   virtual const String describeSelf() const = 0;
};

/// Handle for GFXOcclusionQuery than store last valid state
class GFXOcclusionQueryHandle
{
public:

    GFXOcclusionQueryHandle() 
        : mLastStatus(GFXOcclusionQuery::Unset), mLastData(0), mWaiting(false) , mQuery(NULL)
    {}

    ~GFXOcclusionQueryHandle()
    {
        SAFE_DELETE(mQuery);
    }

    bool getLastStatus( bool block, GFXOcclusionQuery::OcclusionQueryStatus *statusPtr = NULL, U32 *data = NULL );
    GFXOcclusionQuery* getQuery() const { return mQuery; }

    void clearLastStatus()
    {
        mLastStatus = GFXOcclusionQuery::Unset;
        mLastData = 0;
        mWaiting = false;

        if( !mQuery )
            return;

        mQuery->begin();
        mQuery->end();
    }

    bool isWaiting() const { return mWaiting; }
protected:
    GFXOcclusionQuery::OcclusionQueryStatus mLastStatus;
    U32 mLastData;
    bool mWaiting;
    GFXOcclusionQuery *mQuery;
};

inline bool GFXOcclusionQueryHandle::getLastStatus( bool block, GFXOcclusionQuery::OcclusionQueryStatus *statusPtr, U32 *data )
{
    if( !mQuery )
        mQuery = GFX->createOcclusionQuery();

    GFXOcclusionQuery::OcclusionQueryStatus status = mQuery->getStatus( block, data );

    if( status == GFXOcclusionQuery::Waiting )
    {
        mWaiting = true;
        if( statusPtr )
            *statusPtr = mLastStatus;
        if( data )
            *data = mLastData;

        return true;
    }

    if( statusPtr )
        *statusPtr = status;

    mWaiting = false;
    mLastStatus = status;
    mLastData = *data;

    return true;
}


#endif // _GFXOCCLUSIONQUERY_H_