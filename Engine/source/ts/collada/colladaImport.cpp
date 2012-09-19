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

#include "platform/platform.h"

#include "core/volume.h"
#include "ts/collada/colladaUtils.h"
#include "ts/collada/colladaAppNode.h"
#include "ts/collada/colladaShapeLoader.h"

#include "gui/controls/guiTreeViewCtrl.h"

// Helper struct for counting nodes, meshes and polygons down through the scene
// hierarchy
struct SceneStats
{
   S32 numNodes;
   S32 numMeshes;
   S32 numPolygons;
   S32 numMaterials;
   S32 numLights;
   S32 numClips;

   SceneStats() : numNodes(0), numMeshes(0), numPolygons(0), numMaterials(0), numLights(0), numClips(0) { }
};

// Recurse through the <visual_scene> adding nodes and geometry to the GuiTreeView control
static void processNode(GuiTreeViewCtrl* tree, domNode* node, S32 parentID, SceneStats& stats)
{
   stats.numNodes++;
   S32 nodeID = tree->insertItem(parentID, _GetNameOrId(node), "node", "", 0, 0);

   // Update mesh and poly counts
   for (int i = 0; i < node->getContents().getCount(); i++)
   {
      domGeometry* geom = 0;
      const char* elemName = "";

      daeElement* child = node->getContents()[i];
      switch (child->getElementType())
      {
         case COLLADA_TYPE::INSTANCE_GEOMETRY:
         {
            domInstance_geometry* instgeom = daeSafeCast<domInstance_geometry>(child);
            if (instgeom)
            {
               geom = daeSafeCast<domGeometry>(instgeom->getUrl().getElement());
               elemName = _GetNameOrId(geom);
            }
            break;
         }

         case COLLADA_TYPE::INSTANCE_CONTROLLER:
         {
            domInstance_controller* instctrl = daeSafeCast<domInstance_controller>(child);
            if (instctrl)
            {
               domController* ctrl = daeSafeCast<domController>(instctrl->getUrl().getElement());
               elemName = _GetNameOrId(ctrl);
               if (ctrl && ctrl->getSkin())
                  geom = daeSafeCast<domGeometry>(ctrl->getSkin()->getSource().getElement());
               else if (ctrl && ctrl->getMorph())
                  geom = daeSafeCast<domGeometry>(ctrl->getMorph()->getSource().getElement());
            }
            break;
         }

         case COLLADA_TYPE::INSTANCE_LIGHT:
            stats.numLights++;
            tree->insertItem(nodeID, _GetNameOrId(node), "light", "", 0, 0);
            break;
      }

      if (geom && geom->getMesh())
      {
         const char* name = _GetNameOrId(node);
         if ( dStrEqual( name, "null" ) || dStrEndsWith( name, "PIVOT" ) )
            name = _GetNameOrId( daeSafeCast<domNode>(node->getParent()) );

         stats.numMeshes++;
         tree->insertItem(nodeID, name, "mesh", "", 0, 0);

         for (S32 j = 0; j < geom->getMesh()->getTriangles_array().getCount(); j++)
            stats.numPolygons += geom->getMesh()->getTriangles_array()[j]->getCount();
         for (S32 j = 0; j < geom->getMesh()->getTristrips_array().getCount(); j++)
            stats.numPolygons += geom->getMesh()->getTristrips_array()[j]->getCount();
         for (S32 j = 0; j < geom->getMesh()->getTrifans_array().getCount(); j++)
            stats.numPolygons += geom->getMesh()->getTrifans_array()[j]->getCount();
         for (S32 j = 0; j < geom->getMesh()->getPolygons_array().getCount(); j++)
            stats.numPolygons += geom->getMesh()->getPolygons_array()[j]->getCount();
         for (S32 j = 0; j < geom->getMesh()->getPolylist_array().getCount(); j++)
            stats.numPolygons += geom->getMesh()->getPolylist_array()[j]->getCount();
      }
   }

   // Recurse into child nodes
   for (S32 i = 0; i < node->getNode_array().getCount(); i++)
      processNode(tree, node->getNode_array()[i], nodeID, stats);

   for (S32 i = 0; i < node->getInstance_node_array().getCount(); i++)
   {
      domInstance_node* instnode = node->getInstance_node_array()[i];
      domNode* node = daeSafeCast<domNode>(instnode->getUrl().getElement());
      if (node)
         processNode(tree, node, nodeID, stats);
   }
}

ConsoleFunction( enumColladaForImport, bool, 3, 3,
   "(string shapePath, GuiTreeViewCtrl ctrl) Collect scene information from "
   "a COLLADA file and store it in a GuiTreeView control. This function is "
   "used by the COLLADA import gui to show a preview of the scene contents "
   "prior to import, and is probably not much use for anything else.\n"
   "@param shapePath COLLADA filename\n"
   "@param ctrl GuiTreeView control to add elements to\n"
   "@return true if successful, false otherwise\n"
   "@ingroup Editors\n"
   "@internal")
{
   GuiTreeViewCtrl* tree;
   if (!Sim::findObject(argv[2], tree))
   {
      Con::errorf("enumColladaScene::Could not find GuiTreeViewCtrl '%s'", argv[2]);
      return false;
   }

   // Check if a cached DTS is available => no need to import the collada file
   // if we can load the DTS instead
   Torque::Path path(argv[1]);
   if (ColladaShapeLoader::canLoadCachedDTS(path))
      return false;

   // Check if this is a Sketchup file (.kmz) and if so, mount the zip filesystem
   // and get the path to the DAE file.
   String mountPoint;
   Torque::Path daePath;
   bool isSketchup = ColladaShapeLoader::checkAndMountSketchup(path, mountPoint, daePath);

   // Load the Collada file into memory
   domCOLLADA* root = ColladaShapeLoader::getDomCOLLADA(daePath);
   if (!root)
   {
      TSShapeLoader::updateProgress(TSShapeLoader::Load_Complete, "Load complete");
      return false;
   }

   if (isSketchup)
   {
      // Unmount the zip if we mounted it
      Torque::FS::Unmount(mountPoint);
   }

   // Initialize tree
   tree->removeItem(0);
   S32 nodesID = tree->insertItem(0, "Shape", "", "", 0, 0);
   S32 matsID = tree->insertItem(0, "Materials", "", "", 0, 0);
   S32 animsID = tree->insertItem(0, "Animations", "", "", 0, 0);

   SceneStats stats;

   // Query DOM for shape summary details
   for (int i = 0; i < root->getLibrary_visual_scenes_array().getCount(); i++)
   {
      const domLibrary_visual_scenes* libScenes = root->getLibrary_visual_scenes_array()[i];
      for (int j = 0; j < libScenes->getVisual_scene_array().getCount(); j++)
      {
         const domVisual_scene* visualScene = libScenes->getVisual_scene_array()[j];
         for (int k = 0; k < visualScene->getNode_array().getCount(); k++)
            processNode(tree, visualScene->getNode_array()[k], nodesID, stats);
      }
   }

   // Get material count
   for (S32 i = 0; i < root->getLibrary_materials_array().getCount(); i++)
   {
      const domLibrary_materials* libraryMats = root->getLibrary_materials_array()[i];
      stats.numMaterials += libraryMats->getMaterial_array().getCount();
      for (S32 j = 0; j < libraryMats->getMaterial_array().getCount(); j++)
      {
         domMaterial* mat = libraryMats->getMaterial_array()[j];
         tree->insertItem(matsID, _GetNameOrId(mat), _GetNameOrId(mat), "", 0, 0);
      }
   }

   // Get animation count
   for (S32 i = 0; i < root->getLibrary_animation_clips_array().getCount(); i++)
   {
      const domLibrary_animation_clips* libraryClips = root->getLibrary_animation_clips_array()[i];
      stats.numClips += libraryClips->getAnimation_clip_array().getCount();
      for (S32 j = 0; j < libraryClips->getAnimation_clip_array().getCount(); j++)
      {
         domAnimation_clip* clip = libraryClips->getAnimation_clip_array()[j];
         tree->insertItem(animsID, _GetNameOrId(clip), "animation", "", 0, 0);
      }
   }
   if (stats.numClips == 0)
   {
      // No clips => check if there are any animations (these will be added to a default clip)
      for (S32 i = 0; i < root->getLibrary_animations_array().getCount(); i++)
      {
         const domLibrary_animations* libraryAnims = root->getLibrary_animations_array()[i];
         if (libraryAnims->getAnimation_array().getCount())
         {
            stats.numClips = 1;
            tree->insertItem(animsID, "ambient", "animation", "", 0, 0);
            break;
         }
      }
   }

   // Extract the global scale and up_axis from the top level <asset> element,
   F32 unit = 1.0f;
   domUpAxisType upAxis = UPAXISTYPE_Z_UP;
   if (root->getAsset()) {
      if (root->getAsset()->getUnit())
         unit = root->getAsset()->getUnit()->getMeter();
      if (root->getAsset()->getUp_axis())
         upAxis = root->getAsset()->getUp_axis()->getValue();
   }

   TSShapeLoader::updateProgress(TSShapeLoader::Load_Complete, "Load complete");

   // Store shape information in the tree control
   tree->setDataField(StringTable->insert("_nodeCount"), 0, avar("%d", stats.numNodes));
   tree->setDataField(StringTable->insert("_meshCount"), 0, avar("%d", stats.numMeshes));
   tree->setDataField(StringTable->insert("_polygonCount"), 0, avar("%d", stats.numPolygons));
   tree->setDataField(StringTable->insert("_materialCount"), 0, avar("%d", stats.numMaterials));
   tree->setDataField(StringTable->insert("_lightCount"), 0, avar("%d", stats.numLights));
   tree->setDataField(StringTable->insert("_animCount"), 0, avar("%d", stats.numClips));
   tree->setDataField(StringTable->insert("_unit"), 0, avar("%g", unit));

   if (upAxis == UPAXISTYPE_X_UP)
      tree->setDataField(StringTable->insert("_upAxis"), 0, "X_AXIS");
   else if (upAxis == UPAXISTYPE_Y_UP)
      tree->setDataField(StringTable->insert("_upAxis"), 0, "Y_AXIS");
   else
      tree->setDataField(StringTable->insert("_upAxis"), 0, "Z_AXIS");

   return true;
}
