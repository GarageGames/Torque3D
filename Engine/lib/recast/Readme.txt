
Recast & Detour
===============

## Recast

Recast is state of the art navigation mesh construction toolset for games.

* It is automatic, which means that you can throw any level geometry at it and you will get robust mesh out
* It is fast which means swift turnaround times for level designers
* It is open source so it comes with full source and you can customize it to your heart's content. 

The Recast process starts with constructing a voxel mold from a level geometry 
and then casting a navigation mesh over it. The process consists of three steps, 
building the voxel mold, partitioning the mold into simple regions, peeling off 
the regions as simple polygons.

1. The voxel mold is build from the input triangle mesh by rasterizing the triangles into a multi-layer heightfield. Some simple filters are  then applied to the mold to prune out locations where the character would not be able to move.
2. The walkable areas described by the mold are divided into simple overlayed 2D regions. The resulting regions have only one non-overlapping contour, which simplifies the final step of the process tremendously.
3. The navigation polygons are peeled off from the regions by first tracing the boundaries and then simplifying them. The resulting polygons are finally converted to convex polygons which makes them perfect for pathfinding and spatial reasoning about the level. 

## Detour

Recast is accompanied with Detour, path-finding and spatial reasoning toolkit. You can use any navigation mesh with Detour, but of course the data generated with Recast fits perfectly.

Detour offers simple static navigation mesh which is suitable for many simple cases, as well as tiled navigation mesh which allows you to plug in and out pieces of the mesh. The tiled mesh allows you to create systems where you stream new navigation data in and out as the player progresses the level, or you may regenerate tiles as the world changes. 

## Integrating with your own project

It is recommended to add the source directories `DebugUtils`, `Detour`, `DetourCrowd`, `DetourTileCache`, and `Recast` into your own project depending on which parts of the project you need. For example your level building tool could include DebugUtils, Recast, and Detour, and your game runtime could just include Detour.

## Discuss

- Discuss Recast & Detour: http://groups.google.com/group/recastnavigation
- Development blog: http://digestingduck.blogspot.com/

## License

Recast & Detour is licensed under ZLib license, see License.txt for more information.

## Download
Latest code available at https://github.com/recastnavigation/recastnavigation
