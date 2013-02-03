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
#include "core/stream/bitStream.h"

#include "core/strings/stringFunctions.h"
#include "math/mathIO.h"
#include "console/consoleObject.h"
#include "platform/platformNet.h"
#include "core/bitVector.h"


static BitStream gPacketStream(NULL, 0);
static U8 gPacketBuffer[Net::MaxPacketDataSize];

// bitstream utility functions

void BitStream::clearStringBuffer() 
{
   static char stringBuf[256];
   stringBuf[0] = 0; 
//   setStringBuffer( stringBuf ); 
}

void BitStream::setStringBuffer(char buffer[256])
{
//   stringBuffer = buffer;
}

BitStream *BitStream::getPacketStream(U32 writeSize)
{
   if(!writeSize)
      writeSize = Net::MaxPacketDataSize;

   gPacketStream.setBuffer(gPacketBuffer, writeSize, Net::MaxPacketDataSize);
   gPacketStream.setPosition(0);

   return &gPacketStream;
}

void BitStream::sendPacketStream(const NetAddress *addr)
{
   Net::sendto(addr, gPacketStream.getBuffer(), gPacketStream.getPosition());
}

// CodeReview WTF is this additional IsEqual? - BJG, 3/29/07

inline bool IsEqual(F32 a, F32 b) { return a == b; }

ResizeBitStream::ResizeBitStream(U32 minSpace, U32 initialSize) : BitStream(NULL, 0, 0)
{
   mMinSpace = minSpace;
   if(!initialSize)
      initialSize = minSpace * 2;
   U8 *buf = (U8 *) dMalloc(initialSize);
   setBuffer(buf, initialSize, initialSize);
}

ResizeBitStream::~ResizeBitStream()
{
   dFree(dataPtr);
}

void ResizeBitStream::validate()
{
   if(getPosition() + mMinSpace > bufSize)
   {
      bufSize = getPosition() + mMinSpace * 2;
      dataPtr = (U8 *) dRealloc(dataPtr, bufSize);

      maxReadBitNum = bufSize << 3;
      maxWriteBitNum = bufSize << 3;
   }
}


class HuffmanProcessor
{
   static const U32 csm_charFreqs[256];
   bool   m_tablesBuilt;

   void buildTables();

   struct HuffNode {
      U32 pop;

      S16 index0;
      S16 index1;
   };
   struct HuffLeaf {
      U32 pop;

      U8  numBits;
      U8  symbol;
      U32 code;   // no code should be longer than 32 bits.
   };
   // We have to be a bit careful with these, mSince they are pointers...
   struct HuffWrap {
      HuffNode* pNode;
      HuffLeaf* pLeaf;

     public:
      HuffWrap() : pNode(NULL), pLeaf(NULL) { }

      void set(HuffLeaf* in_leaf) { pNode = NULL; pLeaf = in_leaf; }
      void set(HuffNode* in_node) { pLeaf = NULL; pNode = in_node; }

      U32 getPop() { if (pNode) return pNode->pop; else return pLeaf->pop; }
   };

   Vector<HuffNode> m_huffNodes;
   Vector<HuffLeaf> m_huffLeaves;

   S16 determineIndex(HuffWrap&);

   void generateCodes(BitStream&, S32, S32);

  public:
   HuffmanProcessor() : m_tablesBuilt(false) { }

   static HuffmanProcessor g_huffProcessor;

   bool readHuffBuffer(BitStream* pStream, char* out_pBuffer);
   bool writeHuffBuffer(BitStream* pStream, const char* out_pBuffer, S32 maxLen);
};

HuffmanProcessor HuffmanProcessor::g_huffProcessor;

void BitStream::setBuffer(void *bufPtr, S32 size, S32 maxSize)
{
   dataPtr = (U8 *) bufPtr;
   bitNum = 0;
   bufSize = size;
   maxReadBitNum = size << 3;
   if(maxSize < 0)
      maxSize = size;
   maxWriteBitNum = maxSize << 3;
   error = false;
   clearCompressionPoint();
}

U32 BitStream::getPosition() const
{
   return (bitNum + 7) >> 3;
}


bool BitStream::setPosition(const U32 pos)
{
   bitNum = pos << 3;
   return (true);
}

U32 BitStream::getStreamSize()
{
   return bufSize;
}

U8 *BitStream::getBytePtr()
{
   return dataPtr + getPosition();
}


U32 BitStream::getReadByteSize()
{
   return (maxReadBitNum >> 3) - getPosition();
}

U32 BitStream::getWriteByteSize()
{
   return (maxWriteBitNum >> 3) - getPosition();
}

void BitStream::clear()
{
   dMemset(dataPtr, 0, bufSize);
}

void BitStream::writeClassId(U32 classId, U32 classType, U32 classGroup)
{
   AssertFatal(classType < NetClassTypesCount, "Out of range class type.");
   AssertFatal(classGroup < NetClassGroupsCount, "Out of range class group.");
   AssertFatal(classId < AbstractClassRep::NetClassCount[classGroup][classType], "Out of range class id.");
   AssertFatal(AbstractClassRep::NetClassCount[classGroup][classType] < (1 << AbstractClassRep::NetClassBitSize[classGroup][classType]), 
      "NetClassBitSize too small!");

   writeInt(classId, AbstractClassRep::NetClassBitSize[classGroup][classType]);
}

S32 BitStream::readClassId(U32 classType, U32 classGroup)
{
   AssertFatal(classType < NetClassTypesCount, "Out of range class type.");
   AssertFatal(classGroup < NetClassGroupsCount, "Out of range class group.");
   AssertFatal(AbstractClassRep::NetClassCount[classGroup][classType] < (1 << AbstractClassRep::NetClassBitSize[classGroup][classType]), 
      "NetClassBitSize too small!");

   S32 ret = readInt(AbstractClassRep::NetClassBitSize[classGroup][classType]);

   AssertFatal(ret < AbstractClassRep::NetClassCount[classGroup][classType], "BitStream::readClassId - unexpected class ID!");
   if(ret >= AbstractClassRep::NetClassCount[classGroup][classType])
      return -1;
   return ret;
}

void BitStream::writeBits(S32 bitCount, const void *bitPtr)
{
   if(!bitCount)
      return;

   if(bitCount + bitNum > maxWriteBitNum)
   {
      error = true;
      AssertFatal(false, "Out of range write");
      return;
   }

   // [tom, 8/17/2006] This is probably a lot lamer then it needs to be. However,
   // at least it doesnt clobber data or overrun the buffer like the old code did.
   const U8 *ptr = (U8 *)bitPtr;

   for(S32 srcBitNum = 0;srcBitNum < bitCount;srcBitNum++)
   {
      if((*(ptr + (srcBitNum >> 3)) & (1 << (srcBitNum & 0x7))) != 0)
         *(dataPtr + (bitNum >> 3)) |= (1 << (bitNum & 0x7));
      else
         *(dataPtr + (bitNum >> 3)) &= ~(1 << (bitNum & 0x7));
      bitNum++;
   }
}

void BitStream::setBit(S32 bitCount, bool set)
{
   if(set)
      *(dataPtr + (bitCount >> 3)) |= (1 << (bitCount & 0x7));
   else
      *(dataPtr + (bitCount >> 3)) &= ~(1 << (bitCount & 0x7));
}

bool BitStream::testBit(S32 bitCount)
{
   return (*(dataPtr + (bitCount >> 3)) & (1 << (bitCount & 0x7))) != 0;
}

bool BitStream::writeFlag(bool val)
{
   if(bitNum + 1 > maxWriteBitNum)
   {
      error = true;
      AssertFatal(false, "Out of range write");
      return false;
   }
   if(val)
      *(dataPtr + (bitNum >> 3)) |= (1 << (bitNum & 0x7));
   else
      *(dataPtr + (bitNum >> 3)) &= ~(1 << (bitNum & 0x7));
   bitNum++;
   return (val);
}

void BitStream::readBits(S32 bitCount, void *bitPtr)
{
   if(!bitCount)
      return;
   if(bitCount + bitNum > maxReadBitNum)
   {
      error = true;
      //AssertFatal(false, "Out of range read");
      AssertWarn(false, "Out of range read");
      return;
   }
   U8 *stPtr = dataPtr + (bitNum >> 3);
   S32 byteCount = (bitCount + 7) >> 3;

   U8 *ptr = (U8 *) bitPtr;

   S32 downShift = bitNum & 0x7;
   S32 upShift = 8 - downShift;

   U8 curB = *stPtr;
   const U8 *stEnd = dataPtr + bufSize;
   while(byteCount--)
   {
      stPtr++;
      U8 nextB = stPtr < stEnd ? *stPtr : 0;
      *ptr++ = (curB >> downShift) | (nextB << upShift);
      curB = nextB;
   }

   bitNum += bitCount;
}

bool BitStream::_read(U32 size, void *dataPtr)
{
   readBits(size << 3, dataPtr);
   return true;
}

bool BitStream::_write(U32 size, const void *dataPtr)
{
   writeBits(size << 3, dataPtr);
   return true;
}

S32 BitStream::readInt(S32 bitCount)
{
   S32 ret = 0;
   readBits(bitCount, &ret);
   ret = convertLEndianToHost(ret);
   if(bitCount == 32)
      return ret;
   else
      ret &= (1 << bitCount) - 1;
   return ret;
}

void BitStream::writeInt(S32 val, S32 bitCount)
{
   AssertWarn((bitCount == 32) || ((val >> bitCount) == 0), "BitStream::writeInt: value out of range");

   val = convertHostToLEndian(val);
   writeBits(bitCount, &val);
}

void BitStream::writeFloat(F32 f, S32 bitCount)
{
   writeInt((S32)(f * ((1 << bitCount) - 1)), bitCount);
}

F32 BitStream::readFloat(S32 bitCount)
{
   return readInt(bitCount) / F32((1 << bitCount) - 1);
}

void BitStream::writeSignedFloat(F32 f, S32 bitCount)
{
   writeInt((S32)(((f + 1) * .5) * ((1 << bitCount) - 1)), bitCount);
}

F32 BitStream::readSignedFloat(S32 bitCount)
{
   return readInt(bitCount) * 2 / F32((1 << bitCount) - 1) - 1.0f;
}

void BitStream::writeSignedInt(S32 value, S32 bitCount)
{
   if(writeFlag(value < 0))
      writeInt(-value, bitCount - 1);
   else
      writeInt(value, bitCount - 1);
}

S32 BitStream::readSignedInt(S32 bitCount)
{
   if(readFlag())
      return -readInt(bitCount - 1);
   else
      return readInt(bitCount - 1);
}

void BitStream::writeNormalVector(const Point3F& vec, S32 bitCount)
{
   F32 phi   = mAtan2(vec.x, vec.y) / M_PI;
   F32 theta = mAtan2(vec.z, mSqrt(vec.x*vec.x + vec.y*vec.y)) / (M_PI/2.0);

   writeSignedFloat(phi, bitCount+1);
   writeSignedFloat(theta, bitCount);
}

void BitStream::readNormalVector(Point3F *vec, S32 bitCount)
{
   F32 phi   = readSignedFloat(bitCount+1) * M_PI;
   F32 theta = readSignedFloat(bitCount) * (M_PI/2.0);

   vec->x = mSin(phi)*mCos(theta);
   vec->y = mCos(phi)*mCos(theta);
   vec->z = mSin(theta);
}

Point3F BitStream::dumbDownNormal(const Point3F& vec, S32 bitCount)
{
   U8 buffer[128];
   BitStream temp(buffer, 128);

   temp.writeNormalVector(vec, bitCount);
   temp.setCurPos(0);

   Point3F ret;
   temp.readNormalVector(&ret, bitCount);
   return ret;
}

void BitStream::writeVector( Point3F vec, F32 maxMag, S32 magBits, S32 normalBits )
{
   F32 mag = vec.len();

   // If its zero length then we're done.
   if ( !writeFlag( mag > 0.0f ) )
      return;

   // Write the magnitude compressed unless its greater than the maximum.
   if ( writeFlag( mag < maxMag ) )
      writeFloat( mag / maxMag, magBits );
   else
      write( mag );

   // Finally write the normal part.
   vec *= 1.0f / mag;
   writeNormalVector( vec, normalBits );
}

void BitStream::readVector( Point3F *outVec, F32 maxMag, S32 magBits, S32 normalBits )
{
   // Nothing more to do if we got a zero length vector.
   if ( !readFlag() )
   {
      outVec->set(0,0,0);
      return;
   }

   // Read the compressed or uncompressed magnitude.
   F32 mag;
   if ( readFlag() )
      mag = readFloat( magBits ) * maxMag;
   else
      read( &mag );

   // Finally read the normal and reconstruct the vector.
   readNormalVector( outVec, normalBits );
   *outVec *= mag;
}

void BitStream::writeAffineTransform(const MatrixF& matrix)
{
//   AssertFatal(matrix.isAffine() == true,
//               "BitStream::writeAffineTransform: Error, must write only affine transforms!");

   Point3F pos;
   matrix.getColumn(3, &pos);
   mathWrite(*this, pos);

   QuatF q(matrix);
   q.normalize();
   write(q.x);
   write(q.y);
   write(q.z);
   writeFlag(q.w < 0.0);
}

void BitStream::readAffineTransform(MatrixF* matrix)
{
   Point3F pos;
   QuatF   q;

   mathRead(*this, &pos);
   read(&q.x);
   read(&q.y);
   read(&q.z);
   q.w = mSqrt(1.0 - getMin(F32(((q.x * q.x) + (q.y * q.y) + (q.z * q.z))), 1.f));
   if (readFlag())
      q.w = -q.w;

   q.setMatrix(matrix);
   matrix->setColumn(3, pos);
//   AssertFatal(matrix->isAffine() == true,
//               "BitStream::readAffineTransform: Error, transform should be affine after this function!");
}

void BitStream::writeQuat( const QuatF& quat, U32 bitCount )
{
   writeSignedFloat( quat.x, bitCount );
   writeSignedFloat( quat.y, bitCount );
   writeSignedFloat( quat.z, bitCount );
   writeFlag( quat.w < 0.0f );
}

void BitStream::readQuat( QuatF *outQuat, U32 bitCount )
{
   outQuat->x = readSignedFloat( bitCount );
   outQuat->y = readSignedFloat( bitCount );
   outQuat->z = readSignedFloat( bitCount );

   outQuat->w = mSqrt( 1.0 - getMin(   mSquared( outQuat->x ) + 
                                       mSquared( outQuat->y ) + 
                                       mSquared( outQuat->z ),
                                       1.0f ) );
   if ( readFlag() )
      outQuat->w = -outQuat->w;
}

void BitStream::writeBits( const BitVector &bitvec )
{
   U32 size = bitvec.getSize();   
   if ( writeFlag( size <= 127 ) )
      writeInt( size, 7 );
   else
      write( size );

   writeBits( bitvec.getSize(), bitvec.getBits() );
}

void BitStream::readBits( BitVector *bitvec )
{
   U32 size;   
   if ( readFlag() ) // size <= 127
      size = readInt( 7 );
   else
      read( &size );

   bitvec->setSize( size );
   readBits( size, bitvec->getBits() );
}

//----------------------------------------------------------------------------

void BitStream::clearCompressionPoint()
{
   mCompressPoint.set(0,0,0);
}

void BitStream::setCompressionPoint(const Point3F& p)
{
   mCompressPoint = p;
}

static U32 gBitCounts[4] = {
   16, 18, 20, 32
};

void BitStream::writeCompressedPoint(const Point3F& p,F32 scale)
{
   // Same # of bits for all axis
   Point3F vec;
   F32 invScale = 1 / scale;
   U32 type;
   vec = p - mCompressPoint;
   F32 dist = vec.len() * invScale;
   if(dist < (1 << 15))
      type = 0;
   else if(dist < (1 << 17))
      type = 1;
   else if(dist < (1 << 19))
      type = 2;
   else
      type = 3;

   writeInt(type, 2);

   if (type != 3)
   {
      type = gBitCounts[type];
      writeSignedInt(S32(vec.x * invScale + 0.5f),type);
      writeSignedInt(S32(vec.y * invScale + 0.5f),type);
      writeSignedInt(S32(vec.z * invScale + 0.5f),type);
   }
   else
   {
      write(p.x);
      write(p.y);
      write(p.z);
   }
}

void BitStream::readCompressedPoint(Point3F* p,F32 scale)
{
   // Same # of bits for all axis
   U32 type = readInt(2);

   if(type == 3)
   {
      read(&p->x);
      read(&p->y);
      read(&p->z);
   }
   else
   {
      type = gBitCounts[type];
      p->x = (F32)readSignedInt(type);
      p->y = (F32)readSignedInt(type);
      p->z = (F32)readSignedInt(type);

      p->x = mCompressPoint.x + p->x * scale;
      p->y = mCompressPoint.y + p->y * scale;
      p->z = mCompressPoint.z + p->z * scale;
   }
}

//------------------------------------------------------------------------------

InfiniteBitStream::InfiniteBitStream()
{
   //
}

InfiniteBitStream::~InfiniteBitStream()
{
   //
}

void InfiniteBitStream::reset()
{
   // Rewing back to beginning
   setPosition(0);
}

void InfiniteBitStream::validate(U32 upcomingBytes)
{
   if(getPosition() + upcomingBytes + mMinSpace > bufSize)
   {
      bufSize = getPosition() + upcomingBytes + mMinSpace;
      dataPtr = (U8 *) dRealloc(dataPtr, bufSize);

      maxReadBitNum = bufSize << 3;
      maxWriteBitNum = bufSize << 3;
   }
}

void InfiniteBitStream::compact()
{
   // Prepare to copy...
   U32 oldSize = bufSize;
   U8 *tmp = (U8*)dMalloc(bufSize);

   // Copy things...
   bufSize = getPosition() + mMinSpace * 2;
   dMemcpy(tmp, dataPtr, oldSize);

   // And clean up.
   dFree(dataPtr);
   dataPtr = tmp;

   maxReadBitNum = bufSize << 3;
   maxWriteBitNum = bufSize << 3;
}

void InfiniteBitStream::writeToStream(Stream &s)
{
   s.write(getPosition(), dataPtr);
}

//------------------------------------------------------------------------------

void BitStream::readString(char buf[256])
{
   if(stringBuffer)
   {
      if(readFlag())
      {
         S32 offset = readInt(8);
         HuffmanProcessor::g_huffProcessor.readHuffBuffer(this, stringBuffer + offset);
         dStrcpy(buf, stringBuffer);
         return;
      }
   }
   HuffmanProcessor::g_huffProcessor.readHuffBuffer(this, buf);
   if(stringBuffer)
      dStrcpy(stringBuffer, buf);
}

void BitStream::writeString(const char *string, S32 maxLen)
{
   if(!string)
      string = "";
   if(stringBuffer)
   {
      S32 j;
      for(j = 0; j < maxLen && stringBuffer[j] == string[j] && string[j];j++)
         ;
      dStrncpy(stringBuffer, string, maxLen);
      stringBuffer[maxLen] = 0;

      if(writeFlag(j > 2))
      {
         writeInt(j, 8);
         HuffmanProcessor::g_huffProcessor.writeHuffBuffer(this, string + j, maxLen - j);
         return;
      }
   }
   HuffmanProcessor::g_huffProcessor.writeHuffBuffer(this, string, maxLen);
}

void HuffmanProcessor::buildTables()
{
   AssertFatal(m_tablesBuilt == false, "Cannot build tables twice!");
   m_tablesBuilt = true;

   S32 i;

   // First, construct the array of wraps...
   //
   m_huffLeaves.setSize(256);
   m_huffNodes.reserve(256);
   m_huffNodes.increment();
   for (i = 0; i < 256; i++) {
      HuffLeaf& rLeaf = m_huffLeaves[i];

      rLeaf.pop    = csm_charFreqs[i] + 1;
      rLeaf.symbol = U8(i);

      dMemset(&rLeaf.code, 0, sizeof(rLeaf.code));
      rLeaf.numBits = 0;
   }

   S32 currWraps = 256;
   HuffWrap* pWrap = new HuffWrap[256];
   for (i = 0; i < 256; i++) {
      pWrap[i].set(&m_huffLeaves[i]);
   }

   while (currWraps != 1) {
      U32 min1 = 0xfffffffe, min2 = 0xffffffff;
      S32 index1 = -1, index2 = -1;

      for (i = 0; i < currWraps; i++) {
         if (pWrap[i].getPop() < min1) {
            min2   = min1;
            index2 = index1;

            min1   = pWrap[i].getPop();
            index1 = i;
         } else if (pWrap[i].getPop() < min2) {
            min2   = pWrap[i].getPop();
            index2 = i;
         }
      }
      AssertFatal(index1 != -1 && index2 != -1 && index1 != index2, "hrph");

      // Create a node for this...
      m_huffNodes.increment();
      HuffNode& rNode = m_huffNodes.last();
      rNode.pop    = pWrap[index1].getPop() + pWrap[index2].getPop();
      rNode.index0 = determineIndex(pWrap[index1]);
      rNode.index1 = determineIndex(pWrap[index2]);

      S32 mergeIndex = index1 > index2 ? index2 : index1;
      S32 nukeIndex  = index1 > index2 ? index1 : index2;
      pWrap[mergeIndex].set(&rNode);

      if (index2 != (currWraps - 1)) {
         pWrap[nukeIndex] = pWrap[currWraps - 1];
      }
      currWraps--;
   }
   AssertFatal(currWraps == 1, "wrong wraps?");
   AssertFatal(pWrap[0].pNode != NULL && pWrap[0].pLeaf == NULL, "Wrong wrap type!");

   // Ok, now we have one wrap, which is a node.  we need to make sure that this
   //  is the first node in the node list.
   m_huffNodes[0] = *(pWrap[0].pNode);
   delete [] pWrap;

   U32 code = 0;
   BitStream bs(&code, 4);

   generateCodes(bs, 0, 0);
}

void HuffmanProcessor::generateCodes(BitStream& rBS, S32 index, S32 depth)
{
   if (index < 0) {
      // leaf node, copy the code in, and back out...
      HuffLeaf& rLeaf = m_huffLeaves[-(index + 1)];

      dMemcpy(&rLeaf.code, rBS.dataPtr, sizeof(rLeaf.code));
      rLeaf.numBits = depth;
   } else {
      HuffNode& rNode = m_huffNodes[index];

      S32 pos = rBS.getCurPos();

      rBS.writeFlag(false);
      generateCodes(rBS, rNode.index0, depth + 1);

      rBS.setCurPos(pos);
      rBS.writeFlag(true);
      generateCodes(rBS, rNode.index1, depth + 1);

      rBS.setCurPos(pos);
   }
}

S16 HuffmanProcessor::determineIndex(HuffWrap& rWrap)
{
   if (rWrap.pLeaf != NULL) {
      AssertFatal(rWrap.pNode == NULL, "Got a non-NULL pNode in a HuffWrap with a non-NULL leaf.");

      return -((rWrap.pLeaf - m_huffLeaves.address()) + 1);
   } else {
      AssertFatal(rWrap.pNode != NULL, "Got a NULL pNode in a HuffWrap with a NULL leaf.");

      return rWrap.pNode - m_huffNodes.address();
   }
}

bool HuffmanProcessor::readHuffBuffer(BitStream* pStream, char* out_pBuffer)
{
   if (m_tablesBuilt == false)
      buildTables();

   if (pStream->readFlag()) {
      S32 len = pStream->readInt(8);
      for (S32 i = 0; i < len; i++) {
         S32 index = 0;
         while (true) {
            if (index >= 0) {
               if (pStream->readFlag() == true) {
                  index = m_huffNodes[index].index1;
               } else {
                  index = m_huffNodes[index].index0;
               }
            } else {
               out_pBuffer[i] = m_huffLeaves[-(index+1)].symbol;
               break;
            }
         }
      }
      out_pBuffer[len] = '\0';
      return true;
   } else {
      // Uncompressed string...
      U32 len = pStream->readInt(8);
      pStream->read(len, out_pBuffer);
      out_pBuffer[len] = '\0';
      return true;
   }
}

bool HuffmanProcessor::writeHuffBuffer(BitStream* pStream, const char* out_pBuffer, S32 maxLen)
{
   if (out_pBuffer == NULL) {
      pStream->writeFlag(false);
      pStream->writeInt(0, 8);
      return true;
   }

   if (m_tablesBuilt == false)
      buildTables();

   S32 len = out_pBuffer ? dStrlen(out_pBuffer) : 0;
   AssertWarn(len <= 255, "String TOO long for writeString");
   AssertWarn(len <= 255, out_pBuffer);
   if (len > maxLen)
      len = maxLen;

   S32 numBits = 0;
   S32 i;
   for (i = 0; i < len; i++)
      numBits += m_huffLeaves[(unsigned char)out_pBuffer[i]].numBits;

   if (numBits >= (len * 8)) {
      pStream->writeFlag(false);
      pStream->writeInt(len, 8);
      pStream->write(len, out_pBuffer);
   } else {
      pStream->writeFlag(true);
      pStream->writeInt(len, 8);
      for (i = 0; i < len; i++) {
         HuffLeaf& rLeaf = m_huffLeaves[((unsigned char)out_pBuffer[i])];
         pStream->writeBits(rLeaf.numBits, &rLeaf.code);
      }
   }

   return true;
}

const U32 HuffmanProcessor::csm_charFreqs[256] = {
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
329   ,
21    ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
2809  ,
68    ,
0     ,
27    ,
0     ,
58    ,
3     ,
62    ,
4     ,
7     ,
0     ,
0     ,
15    ,
65    ,
554   ,
3     ,
394   ,
404   ,
189   ,
117   ,
30    ,
51    ,
27    ,
15    ,
34    ,
32    ,
80    ,
1     ,
142   ,
3     ,
142   ,
39    ,
0     ,
144   ,
125   ,
44    ,
122   ,
275   ,
70    ,
135   ,
61    ,
127   ,
8     ,
12    ,
113   ,
246   ,
122   ,
36    ,
185   ,
1     ,
149   ,
309   ,
335   ,
12    ,
11    ,
14    ,
54    ,
151   ,
0     ,
0     ,
2     ,
0     ,
0     ,
211   ,
0     ,
2090  ,
344   ,
736   ,
993   ,
2872  ,
701   ,
605   ,
646   ,
1552  ,
328   ,
305   ,
1240  ,
735   ,
1533  ,
1713  ,
562   ,
3     ,
1775  ,
1149  ,
1469  ,
979   ,
407   ,
553   ,
59    ,
279   ,
31    ,
0     ,
0     ,
0     ,
68    ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0
};

