//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
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

#ifndef _FACTORY_CACHE_H_
#define _FACTORY_CACHE_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

//-----------------------------------------------------------------------------

class IFactoryObjectReset
{
public:
    virtual void resetState( void ) = 0;
};

//-----------------------------------------------------------------------------

template<class T>
class FactoryCache : private Vector<T*>
{
public:
    FactoryCache()
    {
    }

    virtual ~FactoryCache()
    {
        purgeCache();
    }

    T* createObject( void )
    {
        // Create a new object if cache is empty.
        if ( this->size() == 0 )
            return new T();

        // Return a cached object.
        T* pObject = this->back();
        this->pop_back();
        return pObject;
    }

    void cacheObject( T* pObject )
    {
        // Cache object.
        this->push_back( pObject );

        // Reset object state if available.
        IFactoryObjectReset* pResetStateObject = dynamic_cast<IFactoryObjectReset*>( pObject );
        if ( pResetStateObject != NULL )
            pResetStateObject->resetState();
    }

    void purgeCache( void )
    {
        while( this->size() > 0 )
        {
            delete this->back();
            this->pop_back();
        }
    }
};

#endif // _FACTORY_CACHE_H_