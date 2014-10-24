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

#include "meshEmitter.h"
#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "platform/profiler.h"

#include "ts/tsShapeInstance.h"

IMPLEMENT_CO_DATABLOCK_V1(MeshEmitterData);

static const float sgDefaultEjectionOffset = 0.f;

MeshEmitterData::MeshEmitterData()
{
   mEmitMesh = NULL;
   mEjectionVelocity = 2.0f;
   mVelocityVariance = 1.0f;
   mEjectionOffset = sgDefaultEjectionOffset;
   mEjectionOffsetVariance = 0.0f;
}

ParticleEmitter* MeshEmitterData::CreateEmitter(ParticleSystem* system)
{
   MeshEmitter *emitter = new MeshEmitter(system);
   emitter->setDataBlock(this);
   return emitter;
}

void MeshEmitterData::initPersistFields()
{
   addField("emitMesh", TYPEID< StringTableEntry >(), Offset(mEmitMesh, MeshEmitterData),
      "The object that the emitter will use to emit particles on.");

   addField("evenEmission", TYPEID< bool >(), Offset(mEvenEmission, MeshEmitterData),
      "If true, particle emission will be spread evenly along the whole model "
      "if false then there will be more particles where the geometry is more dense. "
      "Different effects for per vertex and per triangle emission - Read docs!.");

   addField("emitOnFaces", TYPEID< bool >(), Offset(mEmitOnFaces, MeshEmitterData),
      "If true, particles will be emitted along the faces of the mesh. "
      "If false, particles will be emitted along the vertices of mesh. ");

   addField("EjectionVelocity", TypeF32, Offset(mEjectionVelocity, MeshEmitterData),
      "Particle ejection velocity.");

   addField("VelocityVariance", TypeF32, Offset(mVelocityVariance, MeshEmitterData),
      "Variance for ejection velocity, from 0 - ejectionVelocity.");

   addField("EjectionOffset", TypeF32, Offset(mEjectionOffset, MeshEmitterData),
      "Distance along ejection Z axis from which to eject particles.");

   addField("EjectionOffsetVariance", TypeF32, Offset(mEjectionOffsetVariance, MeshEmitterData),
      "Distance Padding along ejection Z axis from which to eject particles.");

   Parent::initPersistFields();
}

void MeshEmitterData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->writeString(mEmitMesh);
   stream->writeFlag(mEvenEmission);
   stream->writeFlag(mEmitOnFaces);

   stream->writeInt((S32)(mEjectionVelocity * 100), 16);
   stream->writeInt((S32)(mVelocityVariance * 100), 14);
   if (stream->writeFlag(mEjectionOffset != sgDefaultEjectionOffset))
      stream->writeInt((S32)(mEjectionOffset * 100), 16);
   if (stream->writeFlag(mEjectionOffsetVariance != 0.0f))
      stream->writeInt((S32)(mEjectionOffsetVariance * 100), 16);
}

void MeshEmitterData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   char buf[256];
   stream->readString(buf);
   mEmitMesh = dStrdup(buf);

   mEvenEmission = stream->readFlag();
   mEmitOnFaces = stream->readFlag();

   mEjectionVelocity = stream->readInt(16) / 100.0f;
   mVelocityVariance = stream->readInt(14) / 100.0f;
   if (stream->readFlag())
      mEjectionOffset = stream->readInt(16) / 100.0f;
   else
      mEjectionOffset = sgDefaultEjectionOffset;
   if (stream->readFlag())
      mEjectionOffsetVariance = stream->readInt(16) / 100.0f;
   else
      mEjectionOffsetVariance = 0.0f;
}

MeshEmitter::MeshEmitter(ParticleSystem* system) : ParticleEmitter(system)
{
   currentMesh = 0;
}

bool MeshEmitter::addParticle(Point3F const& pos,
   Point3F const& axis,
   Point3F const& vel,
   Point3F const& axisx)
{
   MeshEmitterData* DataBlock = getDataBlock();

   // This should never happen
   //  - But if it happens it will slow down the server.
   if (mParentSystem->isServerObject())
      return false;

   if (!DataBlock->mEmitMesh)
      return false;

   PROFILE_SCOPE(meshEmitAddPart);

   // Check if the emitMesh matches a name
   SimObject* SB = Sim::findObject(DataBlock->mEmitMesh);
   // If not then check if it matches an ID
   if (!SB)
      SB = Sim::findObject(atoi(DataBlock->mEmitMesh));

   if (!SB)
      return false;

   if (SB->getId() != currentMesh)
   {
      currentMesh = SB->getId();
      loadFaces();
   }

   Particle* pNew;
   mParentSystem->getParticlePool()->AddParticle(pNew);
   Point3F ejectionAxis = axis;

   // We define these here so we can reduce the amount of dynamic_cast 'calls'
   psMeshInterface* psMesh = NULL;
   if (SB){
      psMesh = dynamic_cast<psMeshInterface*>(SB);
   }
   // Make sure that we are dealing with some proper objects
   if (psMesh) {

      bool coHandled = false;
      // Per vertex
      if (!DataBlock->mEmitOnFaces)
      {
         coHandled = getPointOnVertex(SB, psMesh, pNew);
      }
      // Per triangle
      if (DataBlock->mEmitOnFaces)
      {
         coHandled = getPointOnFace(SB, psMesh, pNew);
      }

      if (DataBlock->mEvenEmission && mainTime == U32_MAX)
         mainTime = 0;
      if (!DataBlock->mEvenEmission && !coHandled)
         mainTime = 0;
   }

   pNew->orientDir = ejectionAxis;
   pNew->acc.set(0, 0, 0);
   pNew->currentAge = 0;

   // Calculate the constant accleration...
   pNew->vel += vel * mParentSystem->getDataBlock()->getInheritedVelFactor();
   pNew->acc = pNew->vel * mParentSystem->getDataBlock()->getConstantAcceleration();

   // Calculate this instance's lifetime...
   pNew->totalLifetime = mParentSystem->getDataBlock()->getPartLifetimeMS();
   if (mParentSystem->getDataBlock()->getPartLifetimeVarianceMS() != 0)
      pNew->totalLifetime += S32(gRandGen.randI() % (2 * mParentSystem->getDataBlock()->getPartLifetimeVarianceMS() + 1)) - S32(mParentSystem->getDataBlock()->getPartLifetimeVarianceMS());
   // assign spin amount
   pNew->spinSpeed = mParentSystem->getDataBlock()->getSpinSpeed() * gRandGen.randF(mParentSystem->getDataBlock()->getSpinRandomMin(), mParentSystem->getDataBlock()->getSpinRandomMax());

   return true;
}

bool MeshEmitter::getPointOnVertex(SimObject *SB, psMeshInterface *psMesh, Particle *pNew)
{
   PROFILE_SCOPE(meshEmitVertex);
   MeshEmitterData* DataBlock = getDataBlock();

   F32 initialVel = DataBlock->getEjectionVelocity();
   initialVel += (DataBlock->getVelocityVariance() * 2.0f * gRandGen.randF()) - DataBlock->getVelocityVariance();
   // Set our count value to mainTime.
   U32 co = mainTime;
   // If evenEmission is on, set the co to a random number for the per vertex emission.
   if (DataBlock->mEvenEmission && vertexCount != 0)
      co = gRandGen.randI() % vertexCount;
   mainTime++;

   const TSShapeInstance* model;
   TSShape* shape;
   if (psMesh)
      model = psMesh->getShapeInstance();
   shape = model->getShape();
   const TSShape::Detail& det = shape->details[model->getCurrentDetail()];
   S32 od = det.objectDetailNum;
   S32 start = shape->subShapeFirstObject[det.subShapeNum];
   S32 end = start + shape->subShapeNumObjects[det.subShapeNum];
   for (S32 meshIndex = start; meshIndex < end; meshIndex++)
   {
      const TSShape::Object &obj = shape->objects[meshIndex];
      TSMesh* mesh = (od < obj.numMeshes) ? shape->meshes[obj.startMeshIndex + od] : NULL;
      if (!mesh)
         continue;

      TSSkinMesh* sMesh = dynamic_cast<TSSkinMesh*>(mesh);
      //TSMesh::TSMeshVertexArray vertexList = shape->meshes[meshIndex]->mVertexData;
      S32 numVerts;
      numVerts = mesh->mVertexData.size();
      //if (sMesh)
      //   numVerts = sMesh[meshIndex].mVertexData.size();

      if (!numVerts)
         continue;

      if (co >= numVerts)
      {
         co -= numVerts;
         continue;
      }

      // Apparently, creating a TSMesh::TSMeshVertexArray vertexList instead of 
      //  - accessing the vertexdata directly can cause the program to crash on shutdown and startup.
      //  - It calls it's deconstructor when it goes out of scope. Seems like a bug.
      Point3F vertPos;
      Point3F vertNorm;
      if (sMesh)
      {
         vertPos = Point3F(sMesh->mVertexData[co].vert());
         vertNorm = Point3F(sMesh->mVertexData[co].normal());
      }
      else
      {
         vertPos = Point3F(mesh->mVertexData[co].vert());
         vertNorm = Point3F(mesh->mVertexData[co].normal());
      }

      // Transform the vertex position
      psMesh->transformVertex(vertPos);

      // Set the relative position for later use.
      pNew->relPos = vertPos + (vertNorm * DataBlock->mEjectionOffset);

      pNew->pos = psMesh->getShapePosition() + pNew->relPos;
      // Velocity is based on the normal of the vertex
      pNew->vel = vertNorm * initialVel;
      pNew->orientDir = vertNorm;

      // Exit the loop
      return true;
   }
   return false;
}

bool MeshEmitter::getPointOnFace(SimObject *SB, psMeshInterface *psMesh, Particle *pNew)
{
   PROFILE_SCOPE(meshEmitFace);
   MeshEmitterData* DataBlock = getDataBlock();

   F32 initialVel = DataBlock->mEjectionVelocity;
   initialVel += (DataBlock->mVelocityVariance * 2.0f * gRandGen.randF()) - DataBlock->mVelocityVariance;
   // Set our count value to mainTime.
   U32 co = mainTime;
   // If evenEmission is on, set the co to a random number for the per vertex emission.
   if (DataBlock->mEvenEmission && vertexCount != 0)
      co = gRandGen.randI() % vertexCount;
   mainTime++;
   const TSShapeInstance* model;
   TSShape* shape;
   model = psMesh->getShapeInstance();
   shape = model->getShape();
   const TSShape::Detail& det = shape->details[model->getCurrentDetail()];
   S32 od = det.objectDetailNum;
   S32 start = shape->subShapeFirstObject[det.subShapeNum];
   S32 end = start + shape->subShapeNumObjects[det.subShapeNum];

   S32 meshIndex;

   TSMesh* Mesh = NULL;
   bool accepted = false;
   bool skinmesh = false;
   for (meshIndex = start; meshIndex < end; meshIndex++)
   {
      const TSShape::Object &obj = shape->objects[meshIndex];
      TSMesh* mesh = (od < obj.numMeshes) ? shape->meshes[obj.startMeshIndex + od] : NULL;
      TSSkinMesh* sMesh = dynamic_cast<TSSkinMesh*>(mesh);
      if (sMesh)
      {
         if (sMesh->mVertexData.size()){
            skinmesh = true;
            break;
         }
      }
   }

   // We don't want to run with partly skinmesh, partly static mesh.
   //  - So here we filter out skinmeshes from static meshes.
   while (!accepted)
   {
      accepted = false;
      // Pick a random mesh and test it.
      //  - This prevents the uneven emission from 
      //  - being as linear as it is with per vertex.
      meshIndex = (gRandGen.randI() % (end - start)) + start;
      const TSShape::Object &obj = shape->objects[meshIndex];
      Mesh = (od < obj.numMeshes) ? shape->meshes[obj.startMeshIndex + od] : NULL;
      if (skinmesh)
      {
         TSSkinMesh* sMesh = dynamic_cast<TSSkinMesh*>(Mesh);
         if (sMesh)
         {
            if (sMesh->mVertexData.size()){
               accepted = true;
               skinmesh = true;
            }
            else
               accepted = false;
         }
         else
            accepted = false;
      }
      if (!skinmesh && Mesh)
      {
         if (Mesh->mVertexData.size() > 0)
            accepted = true;
         if (Mesh->mVertexData.size() <= 0)
            accepted = false;
      }
   }
   if (emitfaces.size())
   {
      // Get a random triangle
      U32 triStart;
      if (DataBlock->mEvenEmission)
      {
         // Get a random face from our emitfaces vector.
         //  - then follow basically the same procedure as above.
         //  - Just slightly simplified
         S32 faceIndex = gRandGen.randI() % emitfaces.size();
         psMeshParsing::face tris = emitfaces[faceIndex];
         meshIndex = tris.meshIndex;
         triStart = tris.triStart;
      }
      else{
         S32 numPrims = Mesh->primitives.size();
         S32 primIndex = rand() % numPrims;
         S16 numElements = Mesh->primitives[primIndex].numElements;
         triStart = Mesh->primitives[primIndex].start + (rand() % (numElements / 3));
      }
      const TSShape::Object &obj = shape->objects[meshIndex];
      TSMesh* Mesh = (od < obj.numMeshes) ? shape->meshes[obj.startMeshIndex + od] : NULL;

      TSMesh::__TSMeshVertexBase v1, v2, v3;
      Point3F p1, p2, p3;
      U8 indiceBool = triStart % 2;
      if (indiceBool == 0)
      {
         v1 = Mesh->mVertexData[Mesh->indices[triStart]];
         v2 = Mesh->mVertexData[Mesh->indices[triStart + 1]];
         v3 = Mesh->mVertexData[Mesh->indices[triStart + 2]];
      }
      else
      {
         v3 = Mesh->mVertexData[Mesh->indices[triStart]];
         v2 = Mesh->mVertexData[Mesh->indices[triStart + 1]];
         v1 = Mesh->mVertexData[Mesh->indices[triStart + 2]];
      }

      p1 = v1.vert();
      p2 = v2.vert();
      p3 = v3.vert();

      Point3F vec1;
      Point3F vec2;
      vec1 = p2 - p1;
      vec2 = p3 - p2;
      F32 K1 = rand() % 1000 + 1;
      F32 K2 = rand() % 1000 + 1;

      // Construct a vector from the 3 results
      Point3F vertPos;
      if (K2 <= K1)
         vertPos = p1 + (vec1 * (K1 / 1000)) + (vec2 * (K2 / 1000));
      else
         vertPos = p1 + (vec1 * (1 - (K1 / 1000))) + (vec2 * (1 - (K2 / 1000)));

      // Get the normal
      Point3F normalV = v1.normal() + v2.normal() + v3.normal();
      normalV.normalize();

      // Rotate our point by the rotation matrix
      psMesh->transformVertex(vertPos);

      pNew->relPos = vertPos + (normalV * DataBlock->mEjectionOffset);
      pNew->pos = psMesh->getShapePosition() + pNew->relPos;

      pNew->vel = normalV * initialVel;
      pNew->orientDir = normalV;

      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------
// loadFaces
//  - This function calculates the area of all the triangles in the mesh
//  - finds the average area, and adds the triangles one or more times
//  - to the emitfaces vector based on their area relative to the other faces
//  - not a perfect way to do it, but it works fine.
// Custom
//-----------------------------------------------------------------------------
void MeshEmitter::loadFaces()
{
   MeshEmitterData* DataBlock = getDataBlock();

   SimObject* SB = Sim::findObject(DataBlock->mEmitMesh);
   if (!SB)
      SB = Sim::findObject(atoi(DataBlock->mEmitMesh));
   psMeshInterface *psMesh = NULL;
   if (SB){
      psMesh = dynamic_cast<psMeshInterface*>(SB);
   }
   // Make sure that we are dealing with some proper objects
   if (psMesh){
      loadFaces(SB, psMesh);
   }
}

void MeshEmitter::loadFaces(SimObject *SB, psMeshInterface *psMesh)
{
   PROFILE_SCOPE(MeshEmitLoadFaces);
   emitfaces.clear();
   vertexCount = 0;
   if (SB && psMesh){
      TSShapeInstance* model = psMesh->getShapeInstance();
      Vector<psMeshParsing::face> triangles;
      bool skinmesh = false;
      for (S32 meshIndex = 0; meshIndex < model->getShape()->meshes.size(); meshIndex++)
      {
         TSSkinMesh* sMesh = dynamic_cast<TSSkinMesh*>(model->getShape()->meshes[meshIndex]);
         if (sMesh)
         {
            if (sMesh->mVertexData.size()){
               skinmesh = true;
               break;
            }
         }
      }
      TSShape* shape = model->getShape();
      const TSShape::Detail& det = shape->details[model->getCurrentDetail()];
      S32 od = det.objectDetailNum;
      S32 start = shape->subShapeFirstObject[det.subShapeNum];
      S32 end = start + shape->subShapeNumObjects[det.subShapeNum];
      for (S32 meshIndex = start; meshIndex < end; meshIndex++)
      {
         TSSkinMesh* sMesh;

         const TSShape::Object &obj = shape->objects[meshIndex];
         TSMesh* Mesh = (od < obj.numMeshes) ? shape->meshes[obj.startMeshIndex + od] : NULL;

         sMesh = dynamic_cast<TSSkinMesh*>(Mesh);
         if (!Mesh)
            continue;

         S32 numVerts = Mesh->mVertexData.size();
         if (!numVerts)
            continue;
         vertexCount += numVerts;

         S32 numPrims = Mesh->primitives.size();
         if (!numPrims)
            continue;

         S32 numIndices = Mesh->indices.size();
         if (!numIndices)
            continue;
         for (U32 primIndex = 0; primIndex < numPrims; primIndex++)
         {
            U32 start = Mesh->primitives[primIndex].start;

            U32 numElements = Mesh->primitives[primIndex].numElements;

            if ((Mesh->primitives[primIndex].matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Triangles)
            {

               TSMesh::__TSMeshVertexBase v1, v2, v3;
               Point3F p1, p2, p3;

               for (U32 triIndex = 0; triIndex < numElements; triIndex += 3)
               {
                  U32 triStart = start + triIndex;
                  v1 = Mesh->mVertexData[Mesh->indices[triStart]];
                  v2 = Mesh->mVertexData[Mesh->indices[triStart + 1]];
                  v3 = Mesh->mVertexData[Mesh->indices[triStart + 2]];

                  Point3F scale = psMesh->getShapeScale();
                  p1 = v1.vert() * scale;
                  p2 = v2.vert() * scale;
                  p3 = v3.vert() * scale;

                  Point3F veca = p1 - p2;
                  Point3F vecb = p2 - p3;
                  Point3F vecc = p3 - p1;

                  psMeshParsing::face tris;
                  tris.meshIndex = meshIndex;
                  tris.triStart = triStart;
                  tris.area = psMeshParsing::HeronsF(veca, vecb, vecc);
                  triangles.push_back(tris);
               }
            }
            else
            {
               if ((Mesh->primitives[primIndex].matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Fan)
                  Con::warnf("Was a fan DrawPrimitive not TrisDrawPrimitive");
               else if ((Mesh->primitives[primIndex].matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Strip)
                  Con::warnf("Was a Strip DrawPrimitive not TrisDrawPrimitive");
               else
                  Con::warnf("Couldn't determine primitive type");
            }
         }
      }

      F32 averageArea = 0;
      for (S32 index = 0; index < triangles.size(); index++)
      {
         averageArea += triangles[index].area;
      }
      averageArea = averageArea / triangles.size();
      // Reserve some space to prevent numerous
      //  reallocs, which takes a lot of time.
      // Note: Each face is 224 bits, memory
      //  usage can quickly raise to the skies.
      emitfaces.reserve(triangles.size() * 2);
      for (S32 index = 0; index < triangles.size(); index++)
      {
         float n = triangles[index].area / averageArea;
         float t;
         t = n - floor(n);
         if (t >= 0.5)
            n = ceil(n);
         else
            n = floor(n);
         for (int i = -1; i < n; i++)
         {
            emitfaces.push_back(triangles[index]);
         }
      }
   }
}