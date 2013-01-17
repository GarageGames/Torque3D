#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <windows.h>

#include "NvConvexDecomposition.h"
#include "wavefront.h"

#pragma warning(disable:4996)


static bool getBool(const char *str)
{
	bool ret = false;

	if ( stricmp(str,"true") == 0 || atoi(str) > 0 )
		ret = true;

	return ret;
}

void main(int argc,const char **argv)
{
	if ( argc < 2 )
	{
		printf("Usage: convexdecomposition <options> <filename.obj>\r\n");
		printf("\r\n");
		printf("Where <filename> is the name of a Wavefront OBJ file to perform Convex Decomposition on.\r\n");
		printf("The following options are valid:\r\n");
		printf("\r\n");
		printf("-s  <skinWidth>\r\n");
		printf("-d  <decompositionDepth>\r\n");
		printf("-m  <maxHullVertices>\r\n");
		printf("-c  <concavityThresholdPercent>\r\n");
		printf("-g  <mergeThresholdPercdent>\r\n");
		printf("-v  <volumeSplitThresholdPercent>\r\n");
		printf("-ii <useInitialIslandGeneration>\r\n");
		printf("-i  <useIslandGeneration>\r\n");
		printf("-t  <useBackgroundThreads>\r\n");
	}
	else
	{
		NxF32 skinWidth = 0;
		NxU32 decompositionDepth = 4;
		NxU32 maxHullVertices    = 64;
		NxF32 concavityThresholdPercent = 0.1f;
		NxF32 mergeThresholdPercent = 20;
		NxF32 volumeSplitThresholdPercent = 2;
		bool useInitialIslandGeneration = true;
		bool useIslandGeneration = false;
		bool useBackgroundThreads = true;

		NxI32 index = 1;
		const char *scan = argv[index];
		while ( *scan == '-' && index < (argc-1) )
		{
			const char *value = argv[index+1];
			if ( strcmp(scan,"-s") == 0 )
			{
				skinWidth = (NxF32)atof( value );
			}
			else if ( strcmp(scan,"-d") == 0 )
			{
				decompositionDepth = atoi(value);
			}
			else if ( strcmp(scan,"-m") == 0 )
			{
				maxHullVertices = atoi( value );
			}
			else if ( strcmp(scan,"-c") == 0 )
			{
				concavityThresholdPercent =(NxF32)atof(value);
			}
			else if ( strcmp(scan,"-g") == 0 )
			{
				mergeThresholdPercent = (NxF32)atof(value);
			}
			else if ( strcmp(scan,"-v") == 0 )
			{
				volumeSplitThresholdPercent = (NxF32)atof(value);
			}
			else if ( strcmp(scan,"-ii") == 0 )
			{
				useInitialIslandGeneration = getBool(value);
			}
			else if ( strcmp(scan,"-i") == 0 )
			{
				useIslandGeneration = getBool(value);
			}
			else if ( strcmp(scan,"-t") == 0 )
			{
				useBackgroundThreads = getBool(value);
			}
			else
			{
				printf("Unknown option: %s\r\n", scan );
			}
			index+=2;
			if ( index < (argc-1) )
			{
				scan = argv[index];
			}
			else
				break;
		}

		const char *fname = argv[index];
		WavefrontObj w;
		NxU32 tcount = w.loadObj(fname,false);
		if ( tcount )
		{
			printf("Found %d triangles and %d vertices in wavefront file '%s'\r\n", tcount, w.mVertexCount, fname );

			CONVEX_DECOMPOSITION::iConvexDecomposition *ic = CONVEX_DECOMPOSITION::createConvexDecomposition();
			for (NxU32 i=0; i<tcount; i++)
			{
				NxU32 i1 = w.mIndices[i*3+0];
				NxU32 i2 = w.mIndices[i*3+1];
				NxU32 i3 = w.mIndices[i*3+2];

				const NxF32 *p1 = &w.mVertices[i1*3];
				const NxF32 *p2 = &w.mVertices[i2*3];
				const NxF32 *p3 = &w.mVertices[i3*3];

				ic->addTriangle(p1,p2,p3);
			}

			ic->computeConvexDecomposition(skinWidth,
										 decompositionDepth,
										 maxHullVertices,
										 concavityThresholdPercent,
										 mergeThresholdPercent,
										 volumeSplitThresholdPercent,
										 useInitialIslandGeneration,
										 useIslandGeneration,
										 useBackgroundThreads);

			while ( !ic->isComputeComplete() )
			{
				printf("Computing the convex decomposition in a background thread.\r\n");
				Sleep(1000);
			}
			NxU32 hullCount = ic->getHullCount();
			printf("Convex Decomposition produced %d hulls.\r\n", hullCount );

			printf("Saving the convex hulls into a single Wavefront OBJ file 'hulls.obj'\r\n");
			FILE *fph = fopen("hulls.obj", "wb");
			if ( fph )
			{
				NxU32 vcount_base = 1;
				NxU32 vcount_total = 0;
				NxU32 tcount_total = 0;
				for (NxU32 i=0; i<hullCount; i++)
				{
					CONVEX_DECOMPOSITION::ConvexHullResult result;
					ic->getConvexHullResult(i,result);
					vcount_total+=result.mVcount;
					tcount_total+=result.mTcount;
					for (NxU32 i=0; i<result.mVcount; i++)
					{
						const NxF32 *pos = &result.mVertices[i*3];
						fprintf(fph,"v %0.9f %0.9f %0.9f\r\n", pos[0], pos[1], pos[2] );
					}
					for (NxU32 i=0; i<result.mTcount; i++)
					{
						NxU32 i1 = result.mIndices[i*3+0];
						NxU32 i2 = result.mIndices[i*3+1];
						NxU32 i3 = result.mIndices[i*3+2];
						fprintf(fph,"f %d %d %d\r\n", i1+vcount_base, i2+vcount_base, i3+vcount_base );
					}
					vcount_base+=result.mVcount;
				}
				fclose(fph);
				printf("Output contains %d vertices and %d triangles.\r\n", vcount_total, tcount_total );
			}

			CONVEX_DECOMPOSITION::releaseConvexDecomposition(ic);
		}
		else
		{
			printf("No triangles found in file '%s'\r\n", fname );
		}

	}
}
