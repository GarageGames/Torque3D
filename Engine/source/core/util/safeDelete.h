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

#ifndef _TORQUE_SAFEDELETE_H_
#define _TORQUE_SAFEDELETE_H_

/// @addtogroup utility_macros Utility Macros
// @{

#undef  SAFE_DELETE

//-----------------------------------------------------------------------------
/// @brief Safely delete an object and set the pointer to NULL
///
/// @param a Object to delete
/// @see #SAFE_DELETE_ARRAY(), #SAFE_DELETE_OBJECT(), #SAFE_FREE(), #SAFE_FREE_REFERENCE()
//-----------------------------------------------------------------------------
#define SAFE_DELETE(a) {delete (a); (a) = NULL; }

#undef  SAFE_DELETE_ARRAY

//-----------------------------------------------------------------------------
/// @brief Safely delete an array and set the pointer to NULL
///
/// @param a Array to delete
/// @see #SAFE_DELETE(), #SAFE_DELETE_OBJECT(), #SAFE_FREE(), #SAFE_FREE_REFERENCE()
//-----------------------------------------------------------------------------
#define SAFE_DELETE_ARRAY(a) { delete [] (a); (a) = NULL; }

#undef  SAFE_DELETE_OBJECT

//-----------------------------------------------------------------------------
/// @brief Safely delete a SimObject and set the pointer to NULL
///
/// @param a Object to delete
/// @see #SAFE_DELETE_ARRAY(), #SAFE_DELETE(), #SAFE_FREE(), #SAFE_FREE_REFERENCE()
//-----------------------------------------------------------------------------
#define SAFE_DELETE_OBJECT(a) { if( (a) != NULL ) (a)->deleteObject(); (a) = NULL; }

#undef  SAFE_FREE

//-----------------------------------------------------------------------------
/// @brief Safely free memory and set the pointer to NULL
///
/// @param a Pointer to memory to free
/// @see #SAFE_DELETE_ARRAY(), #SAFE_DELETE_OBJECT(), #SAFE_DELETE(), #SAFE_FREE_REFERENCE()
//-----------------------------------------------------------------------------
#define SAFE_FREE(a) { if( (a) != NULL ) dFree ((void *)a); (a) = NULL; }

// CodeReview: Is the NULL conditional needed? [5/14/2007 Pat]

#undef  SAFE_FREE_REFERENCE

//-----------------------------------------------------------------------------
/// @brief Safely free a reference to a Message and set the pointer to NULL
///
/// @param a Pointer to message to free
/// @see #SAFE_DELETE_ARRAY(), #SAFE_DELETE_OBJECT(), #SAFE_FREE(), #SAFE_DELETE()
//-----------------------------------------------------------------------------
#define SAFE_FREE_REFERENCE(a) { if((a) != NULL) (a)->freeReference(); (a) = NULL; }

#undef  SAFE_DELETE_MESSAGE

//-----------------------------------------------------------------------------
/// @brief Synonym for SAFE_FREE_REFERENCE()
///
/// @param a Object to delete
/// @see #SAFE_DELETE(), #SAFE_DELETE_ARRAY(), #SAFE_DELETE_OBJECT(), #SAFE_FREE(), #SAFE_FREE_REFERENCE()
//-----------------------------------------------------------------------------
#define SAFE_DELETE_MESSAGE   SAFE_FREE_REFERENCE

// @}

#endif // _TORQUE_SAFEDELETE_H_

