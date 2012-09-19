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

#include "unit/test.h"
#include "math/mPolyhedron.h"


#ifndef TORQUE_SHIPPING

using namespace UnitTesting;

#define TEST( x ) test( ( x ), "FAIL: " #x )
#define XTEST( t, x ) t->test( ( x ), "FAIL: " #x )


CreateUnitTest( TestMathPolyhedronBuildFromPlanes, "Math/Polyhedron/BuildFromPlanes" )
{
   void test_unitCube()
   {
      Vector< PlaneF > planes;

      // Build planes for a unit cube centered at the origin.
      // Note that the normals must be facing inwards.

      planes.push_back( PlaneF( Point3F( -0.5f, 0.f, 0.f ), Point3F( 1.f, 0.f, 0.f ) ) );
      planes.push_back( PlaneF( Point3F( 0.5f, 0.f, 0.f ), Point3F( -1.f, 0.f, 0.f ) ) );
      planes.push_back( PlaneF( Point3F( 0.f, -0.5f, 0.f ), Point3F( 0.f, 1.f, 0.f ) ) );
      planes.push_back( PlaneF( Point3F( 0.f, 0.5f, 0.f ), Point3F( 0.f, -1.f, 0.f ) ) );
      planes.push_back( PlaneF( Point3F( 0.f, 0.f, -0.5f ), Point3F( 0.f, 0.f, 1.f ) ) );
      planes.push_back( PlaneF( Point3F( 0.f, 0.f, 0.5f ), Point3F( 0.f, 0.f, -1.f ) ) );

      // Turn it into a polyhedron.

      Polyhedron polyhedron;
      polyhedron.buildFromPlanes(
         PlaneSetF( planes.address(), planes.size() )
      );

      // Check if we got a cube back.

      TEST( polyhedron.getNumPoints() == 8 );
      TEST( polyhedron.getNumPlanes() == 6 );
      TEST( polyhedron.getNumEdges() == 12 );
   }

   void test_extraPlane()
   {
      Vector< PlaneF > planes;

      // Build planes for a unit cube centered at the origin.
      // Note that the normals must be facing inwards.

      planes.push_back( PlaneF( Point3F( -0.5f, 0.f, 0.f ), Point3F( 1.f, 0.f, 0.f ) ) );
      planes.push_back( PlaneF( Point3F( 0.5f, 0.f, 0.f ), Point3F( -1.f, 0.f, 0.f ) ) );
      planes.push_back( PlaneF( Point3F( 0.f, -0.5f, 0.f ), Point3F( 0.f, 1.f, 0.f ) ) );
      planes.push_back( PlaneF( Point3F( 0.f, 0.5f, 0.f ), Point3F( 0.f, -1.f, 0.f ) ) );
      planes.push_back( PlaneF( Point3F( 0.f, 0.f, -0.5f ), Point3F( 0.f, 0.f, 1.f ) ) );
      planes.push_back( PlaneF( Point3F( 0.f, 0.f, 0.5f ), Point3F( 0.f, 0.f, -1.f ) ) );

      // Add extra plane that doesn't contribute a new edge.

      planes.push_back( PlaneF( Point3F( 0.5f, 0.5f, 0.5f ), Point3F( -1.f, -1.f, -1.f ) ) );

      // Turn it into a polyhedron.

      Polyhedron polyhedron;
      polyhedron.buildFromPlanes(
         PlaneSetF( planes.address(), planes.size() )
      );

      // Check if we got a cube back.

      TEST( polyhedron.getNumPoints() == 8 );
      TEST( polyhedron.getNumPlanes() == 6 );
      TEST( polyhedron.getNumEdges() == 12 );
   }

   void run()
   {
      test_unitCube();
      //test_extraPlane();
   }
};

#endif // !TORQUE_SHIPPING
