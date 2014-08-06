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

#include "particle.h"

ParticlePool::ParticlePool()
{
   clear(0);
}

ParticlePool::ParticlePool(U32 initSize)
{
   clear(initSize);
}

bool ParticlePool::AddParticle(Particle*& pNew)
{
   bool result = false;
   n_parts++;
   if (n_parts > n_part_capacity)
   {
      // In an emergency we allocate additional particles in blocks of 16.
      // This should happen rarely.
      Particle* store_block = new Particle[16];
      part_store.push_back(store_block);
      n_part_capacity += 16;
      for (S32 i = 0; i < 16; i++)
      {
         store_block[i].next = part_freelist;
         part_freelist = &store_block[i];
      }
      result = true;
   }
   pNew = part_freelist;
   part_freelist = pNew->next;
   pNew->next = part_list_head.next;
   part_list_head.next = pNew;
   return result;
}

void ParticlePool::RemoveParticle(Particle* previousParticle)
{
   Particle *nextParticle = previousParticle->next->next;
   Particle *particle = previousParticle->next;
   n_parts--;
   previousParticle->next = nextParticle;
   particle->next = part_freelist;
   part_freelist = particle;
}

void ParticlePool::clear(U32 initSize)
{
   //   Allocate particle structures and init the freelist. Member part_store
   //   is a Vector so that we can allocate more particles if partListInitSize
   //   turns out to be too small. 
   //
   if (initSize > 0)
   {
      for (S32 i = 0; i < part_store.size(); i++)
      {
         delete[] part_store[i];
      }
      part_store.clear();
      n_part_capacity = initSize;
      Particle* store_block = new Particle[n_part_capacity];
      part_store.push_back(store_block);
      part_freelist = store_block;
      Particle* last_part = part_freelist;
      Particle* part = last_part + 1;
      for (S32 i = 1; i < n_part_capacity; i++, part++, last_part++)
      {
         last_part->next = part;
      }
      store_block[n_part_capacity - 1].next = NULL;
      part_list_head.next = NULL;
      n_parts = 0;
   }
}


void ParticlePool::AdvanceTime(U32 numMSToUpdate)
{
   // remove dead particles
   Particle* last_part = &part_list_head;
   for (Particle* part = part_list_head.next; part != NULL; part = part->next)
   {
      part->currentAge += numMSToUpdate;
      if (part->currentAge > part->totalLifetime)
      {
         n_parts--;
         last_part->next = part->next;
         part->next = part_freelist;
         part_freelist = part;
         part = last_part;
      }
      else
      {
         last_part = part;
      }
   }

   AssertFatal(n_parts >= 0, "ParticlePool: negative part count!");
}