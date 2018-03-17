#include "catch.hpp"

#include "Recast.h"

TEST_CASE("rcSwap")
{
	SECTION("Swap two values")
	{
		int one = 1;
		int two = 2;
		rcSwap(one, two);
		REQUIRE(one == 2);
		REQUIRE(two == 1);
	}
}

TEST_CASE("rcMin")
{
	SECTION("Min returns the lowest value.")
	{
		REQUIRE(rcMin(1, 2) == 1);
		REQUIRE(rcMin(2, 1) == 1);
	}

	SECTION("Min with equal args")
	{
		REQUIRE(rcMin(1, 1) == 1);
	}
}

TEST_CASE("rcMax")
{
	SECTION("Max returns the greatest value.")
	{
		REQUIRE(rcMax(1, 2) == 2);
		REQUIRE(rcMax(2, 1) == 2);
	}

	SECTION("Max with equal args")
	{
		REQUIRE(rcMax(1, 1) == 1);
	}
}

TEST_CASE("rcAbs")
{
	SECTION("Abs returns the absolute value.")
	{
		REQUIRE(rcAbs(-1) == 1);
		REQUIRE(rcAbs(1) == 1);
		REQUIRE(rcAbs(0) == 0);
	}
}

TEST_CASE("rcSqr")
{
	SECTION("Sqr squares a number")
	{
		REQUIRE(rcSqr(2) == 4);
		REQUIRE(rcSqr(-4) == 16);
		REQUIRE(rcSqr(0) == 0);
	}
}

TEST_CASE("rcClamp")
{
	SECTION("Higher than range")
	{
		REQUIRE(rcClamp(2, 0, 1) == 1);
	}

	SECTION("Within range")
	{
		REQUIRE(rcClamp(1, 0, 2) == 1);
	}

	SECTION("Lower than range")
	{
		REQUIRE(rcClamp(0, 1, 2) == 1);
	}
}

TEST_CASE("rcSqrt")
{
	SECTION("Sqrt gets the sqrt of a number")
	{
		REQUIRE(rcSqrt(4) == Approx(2));
		REQUIRE(rcSqrt(81) == Approx(9));
	}
}

TEST_CASE("rcVcross")
{
	SECTION("Computes cross product")
	{
		float v1[3] = {3, -3, 1};
		float v2[3] = {4, 9, 2};
		float result[3];
		rcVcross(result, v1, v2);
		REQUIRE(result[0] == Approx(-15));
		REQUIRE(result[1] == Approx(-2));
		REQUIRE(result[2] == Approx(39));
	}

	SECTION("Cross product with itself is zero")
	{
		float v1[3] = {3, -3, 1};
		float result[3];
		rcVcross(result, v1, v1);
		REQUIRE(result[0] == Approx(0));
		REQUIRE(result[1] == Approx(0));
		REQUIRE(result[2] == Approx(0));
	}
}

TEST_CASE("rcVdot")
{
	SECTION("Dot normalized vector with itself")
	{
		float v1[] = { 1, 0, 0 };
		float result = rcVdot(v1, v1);
		REQUIRE(result == Approx(1));
	}

	SECTION("Dot zero vector with anything is zero")
	{
		float v1[] = { 1, 2, 3 };
		float v2[] = { 0, 0, 0 };

		float result = rcVdot(v1, v2);
		REQUIRE(result == Approx(0));
	}
}

TEST_CASE("rcVmad")
{
	SECTION("scaled add two vectors")
	{
		float v1[3] = {1, 2, 3};
		float v2[3] = {0, 2, 4};
		float result[3];
		rcVmad(result, v1, v2, 2);
		REQUIRE(result[0] == Approx(1));
		REQUIRE(result[1] == Approx(6));
		REQUIRE(result[2] == Approx(11));
	}

	SECTION("second vector is scaled, first is not")
	{
		float v1[3] = {1, 2, 3};
		float v2[3] = {5, 6, 7};
		float result[3];
		rcVmad(result, v1, v2, 0);
		REQUIRE(result[0] == Approx(1));
		REQUIRE(result[1] == Approx(2));
		REQUIRE(result[2] == Approx(3));
	}
}

TEST_CASE("rcVadd")
{
	SECTION("add two vectors")
	{
		float v1[3] = {1, 2, 3};
		float v2[3] = {5, 6, 7};
		float result[3];
		rcVadd(result, v1, v2);
		REQUIRE(result[0] == Approx(6));
		REQUIRE(result[1] == Approx(8));
		REQUIRE(result[2] == Approx(10));
	}
}

TEST_CASE("rcVsub")
{
	SECTION("subtract two vectors")
	{
		float v1[3] = {5, 4, 3};
		float v2[3] = {1, 2, 3};
		float result[3];
		rcVsub(result, v1, v2);
		REQUIRE(result[0] == Approx(4));
		REQUIRE(result[1] == Approx(2));
		REQUIRE(result[2] == Approx(0));
	}
}

TEST_CASE("rcVmin")
{
	SECTION("selects the min component from the vectors")
	{
		float v1[3] = {5, 4, 0};
		float v2[3] = {1, 2, 9};
		rcVmin(v1, v2);
		REQUIRE(v1[0] == Approx(1));
		REQUIRE(v1[1] == Approx(2));
		REQUIRE(v1[2] == Approx(0));
	}

	SECTION("v1 is min")
	{
		float v1[3] = {1, 2, 3};
		float v2[3] = {4, 5, 6};
		rcVmin(v1, v2);
		REQUIRE(v1[0] == Approx(1));
		REQUIRE(v1[1] == Approx(2));
		REQUIRE(v1[2] == Approx(3));
	}

	SECTION("v2 is min")
	{
		float v1[3] = {4, 5, 6};
		float v2[3] = {1, 2, 3};
		rcVmin(v1, v2);
		REQUIRE(v1[0] == Approx(1));
		REQUIRE(v1[1] == Approx(2));
		REQUIRE(v1[2] == Approx(3));
	}
}

TEST_CASE("rcVmax")
{
	SECTION("selects the max component from the vectors")
	{
		float v1[3] = {5, 4, 0};
		float v2[3] = {1, 2, 9};
		rcVmax(v1, v2);
		REQUIRE(v1[0] == Approx(5));
		REQUIRE(v1[1] == Approx(4));
		REQUIRE(v1[2] == Approx(9));
	}

	SECTION("v2 is max")
	{
		float v1[3] = {1, 2, 3};
		float v2[3] = {4, 5, 6};
		rcVmax(v1, v2);
		REQUIRE(v1[0] == Approx(4));
		REQUIRE(v1[1] == Approx(5));
		REQUIRE(v1[2] == Approx(6));
	}

	SECTION("v1 is max")
	{
		float v1[3] = {4, 5, 6};
		float v2[3] = {1, 2, 3};
		rcVmax(v1, v2);
		REQUIRE(v1[0] == Approx(4));
		REQUIRE(v1[1] == Approx(5));
		REQUIRE(v1[2] == Approx(6));
	}
}

TEST_CASE("rcVcopy")
{
	SECTION("copies a vector into another vector")
	{
		float v1[3] = {5, 4, 0};
		float result[3] = {1, 2, 9};
		rcVcopy(result, v1);
		REQUIRE(result[0] == Approx(5));
		REQUIRE(result[1] == Approx(4));
		REQUIRE(result[2] == Approx(0));
		REQUIRE(v1[0] == Approx(5));
		REQUIRE(v1[1] == Approx(4));
		REQUIRE(v1[2] == Approx(0));
	}
}

TEST_CASE("rcVdist")
{
	SECTION("distance between two vectors")
	{
		float v1[3] = {3, 1, 3};
		float v2[3] = {1, 3, 1};
		float result = rcVdist(v1, v2);

		REQUIRE(result == Approx(3.4641f));
	}

	SECTION("Distance from zero is magnitude")
	{
		float v1[3] = {3, 1, 3};
		float v2[3] = {0, 0, 0};
		float distance = rcVdist(v1, v2);
		float magnitude = rcSqrt(rcSqr(v1[0]) + rcSqr(v1[1]) + rcSqr(v1[2]));
		REQUIRE(distance == Approx(magnitude));
	}
}

TEST_CASE("rcVdistSqr")
{
	SECTION("squared distance between two vectors")
	{
		float v1[3] = {3, 1, 3};
		float v2[3] = {1, 3, 1};
		float result = rcVdistSqr(v1, v2);

		REQUIRE(result == Approx(12));
	}

	SECTION("squared distance from zero is squared magnitude")
	{
		float v1[3] = {3, 1, 3};
		float v2[3] = {0, 0, 0};
		float distance = rcVdistSqr(v1, v2);
		float magnitude = rcSqr(v1[0]) + rcSqr(v1[1]) + rcSqr(v1[2]);
		REQUIRE(distance == Approx(magnitude));
	}
}

TEST_CASE("rcVnormalize")
{
	SECTION("normalizing reduces magnitude to 1")
	{
		float v[3] = {3, 3, 3};
		rcVnormalize(v);
		REQUIRE(v[0] == Approx(rcSqrt(1.0f / 3.0f)));
		REQUIRE(v[1] == Approx(rcSqrt(1.0f / 3.0f)));
		REQUIRE(v[2] == Approx(rcSqrt(1.0f / 3.0f)));
		float magnitude = rcSqrt(rcSqr(v[0]) + rcSqr(v[1]) + rcSqr(v[2]));
		REQUIRE(magnitude == Approx(1));
	}
}

TEST_CASE("rcCalcBounds")
{
	SECTION("bounds of one vector")
	{
		float verts[] = {1, 2, 3};
		float bmin[3];
		float bmax[3];
		rcCalcBounds(verts, 1, bmin, bmax);

		REQUIRE(bmin[0] == Approx(verts[0]));
		REQUIRE(bmin[1] == Approx(verts[1]));
		REQUIRE(bmin[2] == Approx(verts[2]));

		REQUIRE(bmax[0] == Approx(verts[0]));
		REQUIRE(bmax[1] == Approx(verts[1]));
		REQUIRE(bmax[2] == Approx(verts[2]));
	}

	SECTION("bounds of more than one vector")
	{
		float verts[] = {
			1, 2, 3,
			0, 2, 5
		};
		float bmin[3];
		float bmax[3];
		rcCalcBounds(verts, 2, bmin, bmax);

		REQUIRE(bmin[0] == Approx(0));
		REQUIRE(bmin[1] == Approx(2));
		REQUIRE(bmin[2] == Approx(3));

		REQUIRE(bmax[0] == Approx(1));
		REQUIRE(bmax[1] == Approx(2));
		REQUIRE(bmax[2] == Approx(5));
	}
}

TEST_CASE("rcCalcGridSize")
{
	SECTION("computes the size of an x & z axis grid")
	{
		float verts[] = {
			1, 2, 3,
			0, 2, 6
		};
		float bmin[3];
		float bmax[3];
		rcCalcBounds(verts, 2, bmin, bmax);

		float cellSize = 1.5f;

		int width;
		int height;

		rcCalcGridSize(bmin, bmax, cellSize, &width, &height);

		REQUIRE(width == 1);
		REQUIRE(height == 2);
	}
}

TEST_CASE("rcCreateHeightfield")
{
	SECTION("create a heightfield")
	{
		float verts[] = {
			1, 2, 3,
			0, 2, 6
		};
		float bmin[3];
		float bmax[3];
		rcCalcBounds(verts, 2, bmin, bmax);

		float cellSize = 1.5f;
		float cellHeight = 2;

		int width;
		int height;

		rcCalcGridSize(bmin, bmax, cellSize, &width, &height);

		rcHeightfield heightfield;

		bool result = rcCreateHeightfield(0, heightfield, width, height, bmin, bmax, cellSize, cellHeight);

		REQUIRE(result);

		REQUIRE(heightfield.width == width);
		REQUIRE(heightfield.height == height);

		REQUIRE(heightfield.bmin[0] == Approx(bmin[0]));
		REQUIRE(heightfield.bmin[1] == Approx(bmin[1]));
		REQUIRE(heightfield.bmin[2] == Approx(bmin[2]));

		REQUIRE(heightfield.bmax[0] == Approx(bmax[0]));
		REQUIRE(heightfield.bmax[1] == Approx(bmax[1]));
		REQUIRE(heightfield.bmax[2] == Approx(bmax[2]));

		REQUIRE(heightfield.cs == Approx(cellSize));
		REQUIRE(heightfield.ch == Approx(cellHeight));

		REQUIRE(heightfield.spans != 0);
		REQUIRE(heightfield.pools == 0);
		REQUIRE(heightfield.freelist == 0);
	}
}

TEST_CASE("rcMarkWalkableTriangles")
{
	rcContext* ctx = 0;
	float walkableSlopeAngle = 45;
	float verts[] = {
		0, 0, 0,
		1, 0, 0,
		0, 0, -1
	};
	int nv = 3;
	int walkable_tri[] = { 0, 1, 2 };
	int unwalkable_tri[] = { 0, 2, 1 };
	int nt = 1;
	unsigned char areas[] = { RC_NULL_AREA };

	SECTION("One walkable triangle")
	{
		rcMarkWalkableTriangles(ctx, walkableSlopeAngle, verts, nv, walkable_tri, nt, areas);
		REQUIRE(areas[0] == RC_WALKABLE_AREA);
	}

	SECTION("One non-walkable triangle")
	{
		rcMarkWalkableTriangles(ctx, walkableSlopeAngle, verts, nv, unwalkable_tri, nt, areas);
		REQUIRE(areas[0] == RC_NULL_AREA);
	}

	SECTION("Non-walkable triangle area id's are not modified")
	{
		areas[0] = 42;
		rcMarkWalkableTriangles(ctx, walkableSlopeAngle, verts, nv, unwalkable_tri, nt, areas);
		REQUIRE(areas[0] == 42);
	}

	SECTION("Slopes equal to the max slope are considered unwalkable.")
	{
		walkableSlopeAngle = 0;
		rcMarkWalkableTriangles(ctx, walkableSlopeAngle, verts, nv, walkable_tri, nt, areas);
		REQUIRE(areas[0] == RC_NULL_AREA);
	}
}

TEST_CASE("rcClearUnwalkableTriangles")
{
	rcContext* ctx = 0;
	float walkableSlopeAngle = 45;
	float verts[] = {
		0, 0, 0,
		1, 0, 0,
		0, 0, -1
	};
	int nv = 3;
	int walkable_tri[] = { 0, 1, 2 };
	int unwalkable_tri[] = { 0, 2, 1 };
	int nt = 1;
	unsigned char areas[] = { 42 };

	SECTION("Sets area ID of unwalkable triangle to RC_NULL_AREA")
	{
		rcClearUnwalkableTriangles(ctx, walkableSlopeAngle, verts, nv, unwalkable_tri, nt, areas);
		REQUIRE(areas[0] == RC_NULL_AREA);
	}

	SECTION("Does not modify walkable triangle aread ID's")
	{
		rcClearUnwalkableTriangles(ctx, walkableSlopeAngle, verts, nv, walkable_tri, nt, areas);
		REQUIRE(areas[0] == 42);
	}

	SECTION("Slopes equal to the max slope are considered unwalkable.")
	{
		walkableSlopeAngle = 0;
		rcClearUnwalkableTriangles(ctx, walkableSlopeAngle, verts, nv, walkable_tri, nt, areas);
		REQUIRE(areas[0] == RC_NULL_AREA);
	}
}

TEST_CASE("rcAddSpan")
{
	rcContext ctx(false);

	float verts[] = {
		1, 2, 3,
		0, 2, 6
	};
	float bmin[3];
	float bmax[3];
	rcCalcBounds(verts, 2, bmin, bmax);

	float cellSize = 1.5f;
	float cellHeight = 2;

	int width;
	int height;

	rcCalcGridSize(bmin, bmax, cellSize, &width, &height);

	rcHeightfield hf;
	REQUIRE(rcCreateHeightfield(&ctx, hf, width, height, bmin, bmax, cellSize, cellHeight));

	int x = 0;
	int y = 0;
	unsigned short smin = 0;
	unsigned short smax = 1;
	unsigned char area = 42;
	int flagMergeThr = 1;

	SECTION("Add a span to an empty heightfield.")
	{
		bool result = rcAddSpan(&ctx, hf, x, y, smin, smax, area, flagMergeThr);
		REQUIRE(result);
		REQUIRE(hf.spans[0] != 0);
		REQUIRE(hf.spans[0]->smin == smin);
		REQUIRE(hf.spans[0]->smax == smax);
		REQUIRE(hf.spans[0]->area == area);
	}

	SECTION("Add a span that gets merged with an existing span.")
	{
		bool result = rcAddSpan(&ctx, hf, x, y, smin, smax, area, flagMergeThr);
		REQUIRE(result);
		REQUIRE(hf.spans[0] != 0);
		REQUIRE(hf.spans[0]->smin == smin);
		REQUIRE(hf.spans[0]->smax == smax);
		REQUIRE(hf.spans[0]->area == area);

		smin = 1;
		smax = 2;
		result = rcAddSpan(&ctx, hf, x, y, smin, smax, area, flagMergeThr);
		REQUIRE(result);
		REQUIRE(hf.spans[0] != 0);
		REQUIRE(hf.spans[0]->smin == 0);
		REQUIRE(hf.spans[0]->smax == 2);
		REQUIRE(hf.spans[0]->area == area);
	}

	SECTION("Add a span that merges with two spans above and below.")
	{
		smin = 0;
		smax = 1;
		REQUIRE(rcAddSpan(&ctx, hf, x, y, smin, smax, area, flagMergeThr));
		REQUIRE(hf.spans[0] != 0);
		REQUIRE(hf.spans[0]->smin == smin);
		REQUIRE(hf.spans[0]->smax == smax);
		REQUIRE(hf.spans[0]->area == area);
		REQUIRE(hf.spans[0]->next == 0);

		smin = 2;
		smax = 3;
		REQUIRE(rcAddSpan(&ctx, hf, x, y, smin, smax, area, flagMergeThr));
		REQUIRE(hf.spans[0]->next != 0);
		REQUIRE(hf.spans[0]->next->smin == smin);
		REQUIRE(hf.spans[0]->next->smax == smax);
		REQUIRE(hf.spans[0]->next->area == area);

		smin = 1;
		smax = 2;
		REQUIRE(rcAddSpan(&ctx, hf, x, y, smin, smax, area, flagMergeThr));
		REQUIRE(hf.spans[0] != 0);
		REQUIRE(hf.spans[0]->smin == 0);
		REQUIRE(hf.spans[0]->smax == 3);
		REQUIRE(hf.spans[0]->area == area);
		REQUIRE(hf.spans[0]->next == 0);
	}
}

TEST_CASE("rcRasterizeTriangle")
{
	rcContext ctx;
	float verts[] = {
		0, 0, 0,
		1, 0, 0,
		0, 0, -1
	};
	float bmin[3];
	float bmax[3];
	rcCalcBounds(verts, 3, bmin, bmax);

	float cellSize = .5f;
	float cellHeight = .5f;

	int width;
	int height;

	rcCalcGridSize(bmin, bmax, cellSize, &width, &height);

	rcHeightfield solid;
	REQUIRE(rcCreateHeightfield(&ctx, solid, width, height, bmin, bmax, cellSize, cellHeight));

	unsigned char area = 42;
	int flagMergeThr = 1;

	SECTION("Rasterize a triangle")
	{
		REQUIRE(rcRasterizeTriangle(&ctx, &verts[0], &verts[3], &verts[6], area, solid, flagMergeThr));

		REQUIRE(solid.spans[0 + 0 * width]);
		REQUIRE(!solid.spans[1 + 0 * width]);
		REQUIRE(solid.spans[0 + 1 * width]);
		REQUIRE(solid.spans[1 + 1 * width]);

		REQUIRE(solid.spans[0 + 0 * width]->smin == 0);
		REQUIRE(solid.spans[0 + 0 * width]->smax == 1);
		REQUIRE(solid.spans[0 + 0 * width]->area == area);
		REQUIRE(!solid.spans[0 + 0 * width]->next);

		REQUIRE(solid.spans[0 + 1 * width]->smin == 0);
		REQUIRE(solid.spans[0 + 1 * width]->smax == 1);
		REQUIRE(solid.spans[0 + 1 * width]->area == area);
		REQUIRE(!solid.spans[0 + 1 * width]->next);

		REQUIRE(solid.spans[1 + 1 * width]->smin == 0);
		REQUIRE(solid.spans[1 + 1 * width]->smax == 1);
		REQUIRE(solid.spans[1 + 1 * width]->area == area);
		REQUIRE(!solid.spans[1 + 1 * width]->next);
	}
}

TEST_CASE("rcRasterizeTriangles")
{
	rcContext ctx;
	float verts[] = {
		0, 0, 0,
		1, 0, 0,
		0, 0, -1,
		0, 0, 1
	};
	int tris[] = {
		0, 1, 2,
		0, 3, 1
	};
	unsigned char areas[] = {
		1,
		2
	};
	float bmin[3];
	float bmax[3];
	rcCalcBounds(verts, 4, bmin, bmax);

	float cellSize = .5f;
	float cellHeight = .5f;

	int width;
	int height;

	rcCalcGridSize(bmin, bmax, cellSize, &width, &height);

	rcHeightfield solid; 
	REQUIRE(rcCreateHeightfield(&ctx, solid, width, height, bmin, bmax, cellSize, cellHeight));

	int flagMergeThr = 1;

	SECTION("Rasterize some triangles")
	{
		REQUIRE(rcRasterizeTriangles(&ctx, verts, 4, tris, areas, 2, solid, flagMergeThr));

		REQUIRE(solid.spans[0 + 0 * width]);
		REQUIRE(solid.spans[0 + 1 * width]);
		REQUIRE(solid.spans[0 + 2 * width]);
		REQUIRE(solid.spans[0 + 3 * width]);
		REQUIRE(!solid.spans[1 + 0 * width]);
		REQUIRE(solid.spans[1 + 1 * width]);
		REQUIRE(solid.spans[1 + 2 * width]);
		REQUIRE(!solid.spans[1 + 3 * width]);

		REQUIRE(solid.spans[0 + 0 * width]->smin == 0);
		REQUIRE(solid.spans[0 + 0 * width]->smax == 1);
		REQUIRE(solid.spans[0 + 0 * width]->area == 1);
		REQUIRE(!solid.spans[0 + 0 * width]->next);

		REQUIRE(solid.spans[0 + 1 * width]->smin == 0);
		REQUIRE(solid.spans[0 + 1 * width]->smax == 1);
		REQUIRE(solid.spans[0 + 1 * width]->area == 1);
		REQUIRE(!solid.spans[0 + 1 * width]->next);

		REQUIRE(solid.spans[0 + 2 * width]->smin == 0);
		REQUIRE(solid.spans[0 + 2 * width]->smax == 1);
		REQUIRE(solid.spans[0 + 2 * width]->area == 2);
		REQUIRE(!solid.spans[0 + 2 * width]->next);

		REQUIRE(solid.spans[0 + 3 * width]->smin == 0);
		REQUIRE(solid.spans[0 + 3 * width]->smax == 1);
		REQUIRE(solid.spans[0 + 3 * width]->area == 2);
		REQUIRE(!solid.spans[0 + 3 * width]->next);

		REQUIRE(solid.spans[1 + 1 * width]->smin == 0);
		REQUIRE(solid.spans[1 + 1 * width]->smax == 1);
		REQUIRE(solid.spans[1 + 1 * width]->area == 1);
		REQUIRE(!solid.spans[1 + 1 * width]->next);

		REQUIRE(solid.spans[1 + 2 * width]->smin == 0);
		REQUIRE(solid.spans[1 + 2 * width]->smax == 1);
		REQUIRE(solid.spans[1 + 2 * width]->area == 2);
		REQUIRE(!solid.spans[1 + 2 * width]->next);
	}

	SECTION("Unsigned short overload")
	{
		unsigned short utris[] = {
			0, 1, 2,
			0, 3, 1
		};
		REQUIRE(rcRasterizeTriangles(&ctx, verts, 4, utris, areas, 2, solid, flagMergeThr));

		REQUIRE(solid.spans[0 + 0 * width]);
		REQUIRE(solid.spans[0 + 1 * width]);
		REQUIRE(solid.spans[0 + 2 * width]);
		REQUIRE(solid.spans[0 + 3 * width]);
		REQUIRE(!solid.spans[1 + 0 * width]);
		REQUIRE(solid.spans[1 + 1 * width]);
		REQUIRE(solid.spans[1 + 2 * width]);
		REQUIRE(!solid.spans[1 + 3 * width]);

		REQUIRE(solid.spans[0 + 0 * width]->smin == 0);
		REQUIRE(solid.spans[0 + 0 * width]->smax == 1);
		REQUIRE(solid.spans[0 + 0 * width]->area == 1);
		REQUIRE(!solid.spans[0 + 0 * width]->next);

		REQUIRE(solid.spans[0 + 1 * width]->smin == 0);
		REQUIRE(solid.spans[0 + 1 * width]->smax == 1);
		REQUIRE(solid.spans[0 + 1 * width]->area == 1);
		REQUIRE(!solid.spans[0 + 1 * width]->next);

		REQUIRE(solid.spans[0 + 2 * width]->smin == 0);
		REQUIRE(solid.spans[0 + 2 * width]->smax == 1);
		REQUIRE(solid.spans[0 + 2 * width]->area == 2);
		REQUIRE(!solid.spans[0 + 2 * width]->next);

		REQUIRE(solid.spans[0 + 3 * width]->smin == 0);
		REQUIRE(solid.spans[0 + 3 * width]->smax == 1);
		REQUIRE(solid.spans[0 + 3 * width]->area == 2);
		REQUIRE(!solid.spans[0 + 3 * width]->next);

		REQUIRE(solid.spans[1 + 1 * width]->smin == 0);
		REQUIRE(solid.spans[1 + 1 * width]->smax == 1);
		REQUIRE(solid.spans[1 + 1 * width]->area == 1);
		REQUIRE(!solid.spans[1 + 1 * width]->next);

		REQUIRE(solid.spans[1 + 2 * width]->smin == 0);
		REQUIRE(solid.spans[1 + 2 * width]->smax == 1);
		REQUIRE(solid.spans[1 + 2 * width]->area == 2);
		REQUIRE(!solid.spans[1 + 2 * width]->next);
	}

	SECTION("Triangle list overload")
	{
		float vertsList[] = {
			0, 0, 0,
			1, 0, 0,
			0, 0, -1,
			0, 0, 0,
			0, 0, 1,
			1, 0, 0,
		};

		REQUIRE(rcRasterizeTriangles(&ctx, vertsList, areas, 2, solid, flagMergeThr));

		REQUIRE(solid.spans[0 + 0 * width]);
		REQUIRE(solid.spans[0 + 1 * width]);
		REQUIRE(solid.spans[0 + 2 * width]);
		REQUIRE(solid.spans[0 + 3 * width]);
		REQUIRE(!solid.spans[1 + 0 * width]);
		REQUIRE(solid.spans[1 + 1 * width]);
		REQUIRE(solid.spans[1 + 2 * width]);
		REQUIRE(!solid.spans[1 + 3 * width]);

		REQUIRE(solid.spans[0 + 0 * width]->smin == 0);
		REQUIRE(solid.spans[0 + 0 * width]->smax == 1);
		REQUIRE(solid.spans[0 + 0 * width]->area == 1);
		REQUIRE(!solid.spans[0 + 0 * width]->next);

		REQUIRE(solid.spans[0 + 1 * width]->smin == 0);
		REQUIRE(solid.spans[0 + 1 * width]->smax == 1);
		REQUIRE(solid.spans[0 + 1 * width]->area == 1);
		REQUIRE(!solid.spans[0 + 1 * width]->next);

		REQUIRE(solid.spans[0 + 2 * width]->smin == 0);
		REQUIRE(solid.spans[0 + 2 * width]->smax == 1);
		REQUIRE(solid.spans[0 + 2 * width]->area == 2);
		REQUIRE(!solid.spans[0 + 2 * width]->next);

		REQUIRE(solid.spans[0 + 3 * width]->smin == 0);
		REQUIRE(solid.spans[0 + 3 * width]->smax == 1);
		REQUIRE(solid.spans[0 + 3 * width]->area == 2);
		REQUIRE(!solid.spans[0 + 3 * width]->next);

		REQUIRE(solid.spans[1 + 1 * width]->smin == 0);
		REQUIRE(solid.spans[1 + 1 * width]->smax == 1);
		REQUIRE(solid.spans[1 + 1 * width]->area == 1);
		REQUIRE(!solid.spans[1 + 1 * width]->next);

		REQUIRE(solid.spans[1 + 2 * width]->smin == 0);
		REQUIRE(solid.spans[1 + 2 * width]->smax == 1);
		REQUIRE(solid.spans[1 + 2 * width]->area == 2);
		REQUIRE(!solid.spans[1 + 2 * width]->next);
	}
}
