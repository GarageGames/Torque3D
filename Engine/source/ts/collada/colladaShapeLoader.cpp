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

/*
   Resource stream -> Buffer
   Buffer -> Collada DOM
   Collada DOM -> TSShapeLoader
   TSShapeLoader installed into TSShape
*/

//-----------------------------------------------------------------------------

#include "platform/platform.h"

#include "ts/collada/colladaShapeLoader.h"
#include "ts/collada/colladaUtils.h"
#include "ts/collada/colladaAppNode.h"
#include "ts/collada/colladaAppMesh.h"
#include "ts/collada/colladaAppMaterial.h"
#include "ts/collada/colladaAppSequence.h"

#include "core/util/tVector.h"
#include "core/strings/findMatch.h"
#include "core/stream/fileStream.h"
#include "core/fileObject.h"
#include "ts/tsShape.h"
#include "ts/tsShapeInstance.h"
#include "materials/materialManager.h"
#include "console/persistenceManager.h"
#include "ts/tsShapeConstruct.h"
#include "core/util/zip/zipVolume.h"
#include "gfx/bitmap/gBitmap.h"

MODULE_BEGIN( ColladaShapeLoader )
   MODULE_INIT_AFTER( ShapeLoader )
   MODULE_INIT
   {
      TSShapeLoader::addFormat("Collada", "dae");
      TSShapeLoader::addFormat("Google Earth", "kmz");
   }
MODULE_END;

// 
static DAE sDAE;                 // Collada model database (holds the last loaded file)
static Torque::Path sLastPath;   // Path of the last loaded Collada file
static FileTime sLastModTime;    // Modification time of the last loaded Collada file

//-----------------------------------------------------------------------------
// Custom warning/error message handler
class myErrorHandler : public daeErrorHandler
{
	void handleError( daeString msg )
   {
      Con::errorf("Error: %s", msg);
   }

	void handleWarning( daeString msg )
   {
      Con::errorf("Warning: %s", msg);
   }
} sErrorHandler;

//-----------------------------------------------------------------------------

ColladaShapeLoader::ColladaShapeLoader(domCOLLADA* _root)
   : root(_root)
{
   // Extract the global scale and up_axis from the top level <asset> element,
   F32 unit = 1.0f;
   domUpAxisType upAxis = UPAXISTYPE_Z_UP;
   if (root->getAsset()) {
      if (root->getAsset()->getUnit())
         unit = root->getAsset()->getUnit()->getMeter();
      if (root->getAsset()->getUp_axis())
         upAxis = root->getAsset()->getUp_axis()->getValue();
   }

   // Set import options (if they are not set to override)
   if (ColladaUtils::getOptions().unit <= 0.0f)
      ColladaUtils::getOptions().unit = unit;

   if (ColladaUtils::getOptions().upAxis == UPAXISTYPE_COUNT)
      ColladaUtils::getOptions().upAxis = upAxis;
}

ColladaShapeLoader::~ColladaShapeLoader()
{
   // Delete all of the animation channels
   for (S32 iAnim = 0; iAnim < animations.size(); iAnim++) {
      for (S32 iChannel = 0; iChannel < animations[iAnim]->size(); iChannel++)
         delete (*animations[iAnim])[iChannel];
      delete animations[iAnim];
   }
   animations.clear();
}

void ColladaShapeLoader::processAnimation(const domAnimation* anim, F32& maxEndTime, F32& minFrameTime)
{
   const char* sRGBANames[] =   { ".R", ".G", ".B", ".A", "" };
   const char* sXYZNames[] =    { ".X", ".Y", ".Z", "" };
   const char* sXYZANames[] =   { ".X", ".Y", ".Z", ".ANGLE" };
   const char* sLOOKATNames[] = { ".POSITIONX", ".POSITIONY", ".POSITIONZ", ".TARGETX", ".TARGETY", ".TARGETZ", ".UPX", ".UPY", ".UPZ", "" };
   const char* sSKEWNames[] =   { ".ROTATEX", ".ROTATEY", ".ROTATEZ", ".AROUNDX", ".AROUNDY", ".AROUNDZ", ".ANGLE", "" };
   const char* sNullNames[] =   { "" };

   for (S32 iChannel = 0; iChannel < anim->getChannel_array().getCount(); iChannel++) {

      // Get the animation elements: <channel>, <sampler>
      domChannel* channel = anim->getChannel_array()[iChannel];
      domSampler* sampler = daeSafeCast<domSampler>(channel->getSource().getElement());
      if (!sampler)
         continue;

      // Find the animation channel target
      daeSIDResolver resolver(channel, channel->getTarget());
      daeElement* target = resolver.getElement();
      if (!target) {
         daeErrorHandler::get()->handleWarning(avar("Failed to resolve animation "
            "target: %s", channel->getTarget()));
         continue;
      }
/*
      // If the target is a <source>, point it at the array instead
      // @todo:Only support targeting float arrays for now...
      if (target->getElementType() == COLLADA_TYPE::SOURCE)
      {
         domSource* source = daeSafeCast<domSource>(target);
         if (source->getFloat_array())
            target = source->getFloat_array();
      }
*/
      // Get the target's animation channels (create them if not already)
      if (!AnimData::getAnimChannels(target)) {
         animations.push_back(new AnimChannels(target));
      }
      AnimChannels* targetChannels = AnimData::getAnimChannels(target);

      // Add a new animation channel to the target
      targetChannels->push_back(new AnimData());
      channel->setUserData(targetChannels->last());
      AnimData& data = *targetChannels->last();

      for (S32 iInput = 0; iInput < sampler->getInput_array().getCount(); iInput++) {

         const domInputLocal* input = sampler->getInput_array()[iInput];
         const domSource* source = daeSafeCast<domSource>(input->getSource().getElement());
         if (!source)
            continue;

         // @todo:don't care about the input param names for now. Could
         // validate against the target type....
         if (dStrEqual(input->getSemantic(), "INPUT")) {
            data.input.initFromSource(source);
            // Adjust the maximum sequence end time
            maxEndTime = getMax(maxEndTime, data.input.getFloatValue((S32)data.input.size()-1));

            // Detect the frame rate (minimum time between keyframes)
            for (S32 iFrame = 1; iFrame < data.input.size(); iFrame++)
            {
               F32 delta = data.input.getFloatValue( iFrame ) - data.input.getFloatValue( iFrame-1 );
               if ( delta < 0 )
               {
                  daeErrorHandler::get()->handleError(avar("<animation> INPUT '%s' "
                     "has non-monotonic keys. Animation is unlikely to be imported correctly.", source->getID()));
                  break;
               }
               minFrameTime = getMin( minFrameTime, delta );
            }
         }
         else if (dStrEqual(input->getSemantic(), "OUTPUT"))
            data.output.initFromSource(source);
         else if (dStrEqual(input->getSemantic(), "IN_TANGENT"))
            data.inTangent.initFromSource(source);
         else if (dStrEqual(input->getSemantic(), "OUT_TANGENT"))
            data.outTangent.initFromSource(source);
         else if (dStrEqual(input->getSemantic(), "INTERPOLATION"))
            data.interpolation.initFromSource(source);
      }

      // Set initial value for visibility targets that were added automatically (in colladaUtils.cpp
      if (dStrEqual(target->getElementName(), "visibility"))
      {
         domAny* visTarget = daeSafeCast<domAny>(target);
         if (visTarget && dStrEqual(visTarget->getValue(), ""))
            visTarget->setValue(avar("%g", data.output.getFloatValue(0)));
      }

      // Ignore empty animations
      if (data.input.size() == 0) {
         channel->setUserData(0);
         delete targetChannels->last();
         targetChannels->pop_back();
         continue;
      }

      // Determine the number and offset the elements of the target value
      // targeted by this animation
      switch (target->getElementType()) {
         case COLLADA_TYPE::COLOR:        data.parseTargetString(channel->getTarget(), 4, sRGBANames);   break;
         case COLLADA_TYPE::TRANSLATE:    data.parseTargetString(channel->getTarget(), 3, sXYZNames);    break;
         case COLLADA_TYPE::ROTATE:       data.parseTargetString(channel->getTarget(), 4, sXYZANames);   break;
         case COLLADA_TYPE::SCALE:        data.parseTargetString(channel->getTarget(), 3, sXYZNames);    break;
         case COLLADA_TYPE::LOOKAT:       data.parseTargetString(channel->getTarget(), 3, sLOOKATNames); break;
         case COLLADA_TYPE::SKEW:         data.parseTargetString(channel->getTarget(), 3, sSKEWNames);   break;
         case COLLADA_TYPE::MATRIX:       data.parseTargetString(channel->getTarget(), 16, sNullNames);  break;
         case COLLADA_TYPE::FLOAT_ARRAY:  data.parseTargetString(channel->getTarget(), daeSafeCast<domFloat_array>(target)->getCount(), sNullNames); break;
         default:                         data.parseTargetString(channel->getTarget(), 1, sNullNames);   break;
      }
   }

   // Process child animations
   for (S32 iAnim = 0; iAnim < anim->getAnimation_array().getCount(); iAnim++)
      processAnimation(anim->getAnimation_array()[iAnim], maxEndTime, minFrameTime);
}

void ColladaShapeLoader::enumerateScene()
{
   // Get animation clips
   Vector<const domAnimation_clip*> animationClips;
   for (S32 iClipLib = 0; iClipLib < root->getLibrary_animation_clips_array().getCount(); iClipLib++) {
      const domLibrary_animation_clips* libraryClips = root->getLibrary_animation_clips_array()[iClipLib];
      for (S32 iClip = 0; iClip < libraryClips->getAnimation_clip_array().getCount(); iClip++)
         appSequences.push_back(new ColladaAppSequence(libraryClips->getAnimation_clip_array()[iClip]));
   }

   // Process all animations => this attaches animation channels to the targeted
   // Collada elements, and determines the length of the sequence if it is not
   // already specified in the Collada <animation_clip> element
   for (S32 iSeq = 0; iSeq < appSequences.size(); iSeq++) {
      ColladaAppSequence* appSeq = dynamic_cast<ColladaAppSequence*>(appSequences[iSeq]);
      F32 maxEndTime = 0;
      F32 minFrameTime = 1000.0f;
      for (S32 iAnim = 0; iAnim < appSeq->getClip()->getInstance_animation_array().getCount(); iAnim++) {
         domAnimation* anim = daeSafeCast<domAnimation>(appSeq->getClip()->getInstance_animation_array()[iAnim]->getUrl().getElement());
         if (anim)
            processAnimation(anim, maxEndTime, minFrameTime);
      }
      if (appSeq->getEnd() == 0)
         appSeq->setEnd(maxEndTime);

      // Collada animations can be stored as sampled frames or true keyframes. For
      // sampled frames, use the same frame rate as the DAE file. For true keyframes,
      // resample at a fixed frame rate.
      appSeq->fps = mClamp(1.0f / minFrameTime + 0.5f, TSShapeLoader::MinFrameRate, TSShapeLoader::MaxFrameRate);
   }

   // First grab all of the top-level nodes
   Vector<domNode*> sceneNodes;
   for (S32 iSceneLib = 0; iSceneLib < root->getLibrary_visual_scenes_array().getCount(); iSceneLib++) {
      const domLibrary_visual_scenes* libScenes = root->getLibrary_visual_scenes_array()[iSceneLib];
      for (S32 iScene = 0; iScene < libScenes->getVisual_scene_array().getCount(); iScene++) {
         const domVisual_scene* visualScene = libScenes->getVisual_scene_array()[iScene];
         for (S32 iNode = 0; iNode < visualScene->getNode_array().getCount(); iNode++)
            sceneNodes.push_back(visualScene->getNode_array()[iNode]);
      }
   }

   // Set LOD option
   bool singleDetail = true;
   switch (ColladaUtils::getOptions().lodType)
   {
      case ColladaUtils::ImportOptions::DetectDTS:
         // Check for a baseXX->startXX hierarchy at the top-level, if we find
         // one, use trailing numbers for LOD, otherwise use a single size
         for (S32 iNode = 0; singleDetail && (iNode < sceneNodes.size()); iNode++) {
            domNode* node = sceneNodes[iNode];
            if (dStrStartsWith(_GetNameOrId(node), "base")) {
               for (S32 iChild = 0; iChild < node->getNode_array().getCount(); iChild++) {
                  domNode* child = node->getNode_array()[iChild];
                  if (dStrStartsWith(_GetNameOrId(child), "start")) {
                     singleDetail = false;
                     break;
                  }
               }
            }
         }
         break;

      case ColladaUtils::ImportOptions::SingleSize:
         singleDetail = true;
         break;

      case ColladaUtils::ImportOptions::TrailingNumber:
         singleDetail = false;
         break;
         
      default:
         break;
   }

   ColladaAppMesh::fixDetailSize( singleDetail, ColladaUtils::getOptions().singleDetailSize );

   // Process the top level nodes
   for (S32 iNode = 0; iNode < sceneNodes.size(); iNode++) {
      ColladaAppNode* node = new ColladaAppNode(sceneNodes[iNode], 0);
      if (!processNode(node))
         delete node;
   }

   // Make sure that the scene has a bounds node (for getting the root scene transform)
   if (!boundsNode)
   {
      domVisual_scene* visualScene = root->getLibrary_visual_scenes_array()[0]->getVisual_scene_array()[0];
      domNode* dombounds = daeSafeCast<domNode>( visualScene->createAndPlace( "node" ) );
      dombounds->setName( "bounds" );
      ColladaAppNode *appBounds = new ColladaAppNode(dombounds, 0);
      if (!processNode(appBounds))
         delete appBounds;
   }
}

bool ColladaShapeLoader::ignoreNode(const String& name)
{
   if (FindMatch::isMatchMultipleExprs(ColladaUtils::getOptions().alwaysImport, name, false))
      return false;
   else
      return FindMatch::isMatchMultipleExprs(ColladaUtils::getOptions().neverImport, name, false);
}

bool ColladaShapeLoader::ignoreMesh(const String& name)
{
   if (FindMatch::isMatchMultipleExprs(ColladaUtils::getOptions().alwaysImportMesh, name, false))
      return false;
   else
      return FindMatch::isMatchMultipleExprs(ColladaUtils::getOptions().neverImportMesh, name, false);
}

void ColladaShapeLoader::computeBounds(Box3F& bounds)
{
   TSShapeLoader::computeBounds(bounds);

   // Check if the model origin needs adjusting
   if ( bounds.isValidBox() &&
       (ColladaUtils::getOptions().adjustCenter ||
        ColladaUtils::getOptions().adjustFloor) )
   {
      // Compute shape offset
      Point3F shapeOffset = Point3F::Zero;
      if ( ColladaUtils::getOptions().adjustCenter )
      {
         bounds.getCenter( &shapeOffset );
         shapeOffset = -shapeOffset;
      }
      if ( ColladaUtils::getOptions().adjustFloor )
         shapeOffset.z = -bounds.minExtents.z;

      // Adjust bounds
      bounds.minExtents += shapeOffset;
      bounds.maxExtents += shapeOffset;

      // Now adjust all positions for root level nodes (nodes with no parent)
      for (S32 iNode = 0; iNode < shape->nodes.size(); iNode++)
      {
         if ( !appNodes[iNode]->isParentRoot() )
            continue;

         // Adjust default translation
         shape->defaultTranslations[iNode] += shapeOffset;

         // Adjust animated translations
         for (S32 iSeq = 0; iSeq < shape->sequences.size(); iSeq++)
         {
            const TSShape::Sequence& seq = shape->sequences[iSeq];
            if ( seq.translationMatters.test(iNode) )
            {
               for (S32 iFrame = 0; iFrame < seq.numKeyframes; iFrame++)
               {
                  S32 index = seq.baseTranslation + seq.translationMatters.count(iNode)*seq.numKeyframes + iFrame;
                  shape->nodeTranslations[index] += shapeOffset;
               }
            }
         }
      }
   }
}

//-----------------------------------------------------------------------------
/// Find the file extension for an extensionless texture
String findTextureExtension(const Torque::Path &texPath)
{
   Torque::Path path(texPath);
   for(S32 i = 0;i < GBitmap::sRegistrations.size();++i)
   {
      GBitmap::Registration &reg = GBitmap::sRegistrations[i];
      for(S32 j = 0;j < reg.extensions.size();++j)
      {
         path.setExtension(reg.extensions[j]);
         if (Torque::FS::IsFile(path))
            return path.getExtension();
      }
   }

   return String();
}

//-----------------------------------------------------------------------------
/// Copy a texture from a KMZ to a cache. Note that the texture filename is modified
void copySketchupTexture(const Torque::Path &path, String &textureFilename)
{
   if (textureFilename.isEmpty())
      return;

   Torque::Path texturePath(textureFilename);
   texturePath.setExtension(findTextureExtension(texturePath));

   String cachedTexFilename = String::ToString("%s_%s.cached",
      TSShapeLoader::getShapePath().getFileName().c_str(), texturePath.getFileName().c_str());

   Torque::Path cachedTexPath;
   cachedTexPath.setRoot(path.getRoot());
   cachedTexPath.setPath(path.getPath());
   cachedTexPath.setFileName(cachedTexFilename);
   cachedTexPath.setExtension(texturePath.getExtension());

   FileStream *source;
   FileStream *dest;
   if ((source = FileStream::createAndOpen(texturePath.getFullPath(), Torque::FS::File::Read)) == NULL)
      return;

   if ((dest = FileStream::createAndOpen(cachedTexPath.getFullPath(), Torque::FS::File::Write)) == NULL)
   {
      delete source;
      return;
   }

   dest->copyFrom(source);

   delete dest;
   delete source;

   // Update the filename in the material
   cachedTexPath.setExtension("");
   textureFilename = cachedTexPath.getFullPath();
}

//-----------------------------------------------------------------------------
/// Add collada materials to materials.cs
void updateMaterialsScript(const Torque::Path &path, bool copyTextures = false)
{
#ifdef DAE2DTS_TOOL
   if (!ColladaUtils::getOptions().forceUpdateMaterials)
      return;
#endif

   Torque::Path scriptPath(path);
   scriptPath.setFileName("materials");
   scriptPath.setExtension("cs");

   // First see what materials we need to update
   PersistenceManager persistMgr;
   for ( U32 iMat = 0; iMat < AppMesh::appMaterials.size(); iMat++ )
   {
      ColladaAppMaterial *mat = dynamic_cast<ColladaAppMaterial*>( AppMesh::appMaterials[iMat] );
      if ( mat )
      {
         Material *mappedMat;
         if ( Sim::findObject( MATMGR->getMapEntry( mat->getName() ), mappedMat ) )
         {
            // Only update existing materials if forced to
            if ( ColladaUtils::getOptions().forceUpdateMaterials )
               persistMgr.setDirty( mappedMat );
         }
         else
         {
            // Create a new material definition
            persistMgr.setDirty( mat->createMaterial( scriptPath ), scriptPath.getFullPath() );
         }
      }
   }

   if ( persistMgr.getDirtyList().empty() )
      return;

   // If importing a sketchup file, the paths will point inside the KMZ so we need to cache them.
   if (copyTextures)
   {
      for (S32 iMat = 0; iMat < persistMgr.getDirtyList().size(); iMat++)
      {
         Material *mat = dynamic_cast<Material*>( persistMgr.getDirtyList()[iMat].getObject() );

         copySketchupTexture(path, mat->mDiffuseMapFilename[0]);
         copySketchupTexture(path, mat->mNormalMapFilename[0]);
         copySketchupTexture(path, mat->mSpecularMapFilename[0]);
      }
   }

   persistMgr.saveDirty();
}

//-----------------------------------------------------------------------------
/// Check if an up-to-date cached DTS is available for this DAE file
bool ColladaShapeLoader::canLoadCachedDTS(const Torque::Path& path)
{
   // Generate the cached filename
   Torque::Path cachedPath(path);
   cachedPath.setExtension("cached.dts");

   // Check if a cached DTS newer than this file is available
   FileTime cachedModifyTime;
   if (Platform::getFileTimes(cachedPath.getFullPath(), NULL, &cachedModifyTime))
   {
      bool forceLoadDAE = Con::getBoolVariable("$collada::forceLoadDAE", false);

      FileTime daeModifyTime;
      if (!Platform::getFileTimes(path.getFullPath(), NULL, &daeModifyTime) ||
         (!forceLoadDAE && (Platform::compareFileTimes(cachedModifyTime, daeModifyTime) >= 0) ))
      {
         // DAE not found, or cached DTS is newer
         return true;
      }
   }

   //assume the dts is good since it was zipped on purpose
   Torque::FS::FileSystemRef ref = Torque::FS::GetFileSystem(cachedPath);
   if (ref && !dStrcmp("Zip", ref->getTypeStr()))
   {
      bool forceLoadDAE = Con::getBoolVariable("$collada::forceLoadDAE", false);

      if (!forceLoadDAE && Torque::FS::IsFile(cachedPath))
          return true;
   }
     
   return false;
}

bool ColladaShapeLoader::checkAndMountSketchup(const Torque::Path& path, String& mountPoint, Torque::Path& daePath)
{
   bool isSketchup = path.getExtension().equal("kmz", String::NoCase);
   if (isSketchup)
   {
      // Mount the zip so files can be found (it will be unmounted before we return)
      mountPoint = String("sketchup_") + path.getFileName();
      String zipPath = path.getFullPath();
      if (!Torque::FS::Mount(mountPoint, new Torque::ZipFileSystem(zipPath)))
         return false;

      Vector<String> daeFiles;
      Torque::Path findPath;
      findPath.setRoot(mountPoint);
      S32 results = Torque::FS::FindByPattern(findPath, "*.dae", true, daeFiles);
      if (results == 0 || daeFiles.size() == 0)
      {
         Torque::FS::Unmount(mountPoint);
         return false;
      }

      daePath = daeFiles[0];
   }
   else
   {
      daePath = path;
   }

   return isSketchup;
}

//-----------------------------------------------------------------------------
/// Get the root collada DOM element for the given DAE file
domCOLLADA* ColladaShapeLoader::getDomCOLLADA(const Torque::Path& path)
{
   daeErrorHandler::setErrorHandler(&sErrorHandler);

   TSShapeLoader::updateProgress(TSShapeLoader::Load_ReadFile, path.getFullFileName().c_str());

   // Check if we can use the last loaded file
   FileTime daeModifyTime;
   if (Platform::getFileTimes(path.getFullPath(), NULL, &daeModifyTime))
   {
      if ((path == sLastPath) && (Platform::compareFileTimes(sLastModTime, daeModifyTime) >= 0))
         return sDAE.getRoot(path.getFullPath().c_str());
   }

   sDAE.clear();
   sDAE.setBaseURI("");

   TSShapeLoader::updateProgress(TSShapeLoader::Load_ParseFile, "Parsing XML...");
   domCOLLADA* root = readColladaFile(path.getFullPath());
   if (!root)
   {
      TSShapeLoader::updateProgress(TSShapeLoader::Load_Complete, "Import failed");
      sDAE.clear();
      return NULL;
   }

   sLastPath = path;
   sLastModTime = daeModifyTime;

   return root;
}

domCOLLADA* ColladaShapeLoader::readColladaFile(const String& path)
{
   // Check if this file is already loaded into the database
   domCOLLADA* root = sDAE.getRoot(path.c_str());
   if (root)
      return root;

   // Load the Collada file into memory
   FileObject fo;
   if (!fo.readMemory(path))
   {
      daeErrorHandler::get()->handleError(avar("Could not read %s into memory", path.c_str()));
      return NULL;
   }

   root = sDAE.openFromMemory(path.c_str(), (const char*)fo.buffer());
   if (!root || !root->getLibrary_visual_scenes_array().getCount()) {
      daeErrorHandler::get()->handleError(avar("Could not parse %s", path.c_str()));
      return NULL;
   }

   // Fixup issues in the model
   ColladaUtils::applyConditioners(root);

   // Recursively load external DAE references
   TSShapeLoader::updateProgress(TSShapeLoader::Load_ExternalRefs, "Loading external references...");
   for (S32 iRef = 0; iRef < root->getDocument()->getReferencedDocuments().getCount(); iRef++) {
      String refPath = (daeString)root->getDocument()->getReferencedDocuments()[iRef];
      if (refPath.endsWith(".dae") && !readColladaFile(refPath))
         daeErrorHandler::get()->handleError(avar("Failed to load external reference: %s", refPath.c_str()));
   }
   return root;
}

//-----------------------------------------------------------------------------
/// This function is invoked by the resource manager based on file extension.
TSShape* loadColladaShape(const Torque::Path &path)
{
#ifndef DAE2DTS_TOOL
   // Generate the cached filename
   Torque::Path cachedPath(path);
   cachedPath.setExtension("cached.dts");

   // Check if an up-to-date cached DTS version of this file exists, and
   // if so, use that instead.
   if (ColladaShapeLoader::canLoadCachedDTS(path))
   {
      FileStream cachedStream;
      cachedStream.open(cachedPath.getFullPath(), Torque::FS::File::Read);
      if (cachedStream.getStatus() == Stream::Ok)
      {
         TSShape *shape = new TSShape;
         bool readSuccess = shape->read(&cachedStream);
         cachedStream.close();

         if (readSuccess)
         {
         #ifdef TORQUE_DEBUG
            Con::printf("Loaded cached Collada shape from %s", cachedPath.getFullPath().c_str());
         #endif
            return shape;
         }
         else
            delete shape;
      }

      Con::warnf("Failed to load cached COLLADA shape from %s", cachedPath.getFullPath().c_str());
   }
#endif // DAE2DTS_TOOL

   if (!Torque::FS::IsFile(path))
   {
      // DAE file does not exist, bail.
      return NULL;
   }

#ifdef DAE2DTS_TOOL
   ColladaUtils::ImportOptions cmdLineOptions = ColladaUtils::getOptions();
#endif

   // Allow TSShapeConstructor object to override properties
   ColladaUtils::getOptions().reset();
   TSShapeConstructor* tscon = TSShapeConstructor::findShapeConstructor(path.getFullPath());
   if (tscon)
   {
      ColladaUtils::getOptions() = tscon->mOptions;

#ifdef DAE2DTS_TOOL
      // Command line overrides certain options
      ColladaUtils::getOptions().forceUpdateMaterials = cmdLineOptions.forceUpdateMaterials;
      ColladaUtils::getOptions().useDiffuseNames = cmdLineOptions.useDiffuseNames;
#endif
   }

   // Check if this is a Sketchup file (.kmz) and if so, mount the zip filesystem
   // and get the path to the DAE file.
   String mountPoint;
   Torque::Path daePath;
   bool isSketchup = ColladaShapeLoader::checkAndMountSketchup(path, mountPoint, daePath);

   // Load Collada model and convert to 3space
   TSShape* tss = 0;
   domCOLLADA* root = ColladaShapeLoader::getDomCOLLADA(daePath);
   if (root)
   {
      ColladaShapeLoader loader(root);
      tss = loader.generateShape(daePath);
      if (tss)
      {
#ifndef DAE2DTS_TOOL
         // Cache the Collada model to a DTS file for faster loading next time.
         FileStream dtsStream;
         
         if (dtsStream.open(cachedPath.getFullPath(), Torque::FS::File::Write))
         {
            Torque::FS::FileSystemRef ref = Torque::FS::GetFileSystem(daePath);
            if (ref && !dStrcmp("Zip", ref->getTypeStr()))
               Con::errorf("No cached dts file found in archive for %s. Forcing cache to disk.", daePath.getFullFileName().c_str());

            Con::printf("Writing cached COLLADA shape to %s", cachedPath.getFullPath().c_str());
            tss->write(&dtsStream);
         }

#endif // DAE2DTS_TOOL

         // Add collada materials to materials.cs
         updateMaterialsScript(path, isSketchup);
      }
   }

   // Close progress dialog
   TSShapeLoader::updateProgress(TSShapeLoader::Load_Complete, "Import complete");

   if (isSketchup)
   {
      // Unmount the zip if we mounted it
      Torque::FS::Unmount(mountPoint);
   }

   return tss;
}
