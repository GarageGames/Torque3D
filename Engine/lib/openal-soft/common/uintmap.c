
#include "config.h"

#include "uintmap.h"

#include <stdlib.h>
#include <string.h>

#include "almalloc.h"


extern inline void LockUIntMapRead(UIntMap *map);
extern inline void UnlockUIntMapRead(UIntMap *map);
extern inline void LockUIntMapWrite(UIntMap *map);
extern inline void UnlockUIntMapWrite(UIntMap *map);


void InitUIntMap(UIntMap *map, ALsizei limit)
{
    map->keys = NULL;
    map->values = NULL;
    map->size = 0;
    map->capacity = 0;
    map->limit = limit;
    RWLockInit(&map->lock);
}

void ResetUIntMap(UIntMap *map)
{
    WriteLock(&map->lock);
    al_free(map->keys);
    map->keys = NULL;
    map->values = NULL;
    map->size = 0;
    map->capacity = 0;
    WriteUnlock(&map->lock);
}

ALenum InsertUIntMapEntry(UIntMap *map, ALuint key, ALvoid *value)
{
    ALsizei pos = 0;

    WriteLock(&map->lock);
    if(map->size > 0)
    {
        ALsizei count = map->size;
        do {
            ALsizei step = count>>1;
            ALsizei i = pos+step;
            if(!(map->keys[i] < key))
                count = step;
            else
            {
                pos = i+1;
                count -= step+1;
            }
        } while(count > 0);
    }

    if(pos == map->size || map->keys[pos] != key)
    {
        if(map->size >= map->limit)
        {
            WriteUnlock(&map->lock);
            return AL_OUT_OF_MEMORY;
        }

        if(map->size == map->capacity)
        {
            ALuint *keys = NULL;
            ALvoid **values;
            ALsizei newcap, keylen;

            newcap = (map->capacity ? (map->capacity<<1) : 4);
            if(map->limit > 0 && newcap > map->limit)
                newcap = map->limit;
            if(newcap > map->capacity)
            {
                /* Round the memory size for keys up to a multiple of the
                 * pointer size.
                 */
                keylen = newcap * sizeof(map->keys[0]);
                keylen += sizeof(map->values[0]) - 1;
                keylen -= keylen%sizeof(map->values[0]);

                keys = al_malloc(16, keylen + newcap*sizeof(map->values[0]));
            }
            if(!keys)
            {
                WriteUnlock(&map->lock);
                return AL_OUT_OF_MEMORY;
            }
            values = (ALvoid**)((ALbyte*)keys + keylen);

            if(map->keys)
            {
                memcpy(keys, map->keys, map->size*sizeof(map->keys[0]));
                memcpy(values, map->values, map->size*sizeof(map->values[0]));
            }
            al_free(map->keys);
            map->keys = keys;
            map->values = values;
            map->capacity = newcap;
        }

        if(pos < map->size)
        {
            memmove(&map->keys[pos+1], &map->keys[pos],
                    (map->size-pos)*sizeof(map->keys[0]));
            memmove(&map->values[pos+1], &map->values[pos],
                    (map->size-pos)*sizeof(map->values[0]));
        }
        map->size++;
    }
    map->keys[pos] = key;
    map->values[pos] = value;
    WriteUnlock(&map->lock);

    return AL_NO_ERROR;
}

ALvoid *RemoveUIntMapKey(UIntMap *map, ALuint key)
{
    ALvoid *ptr = NULL;
    WriteLock(&map->lock);
    if(map->size > 0)
    {
        ALsizei pos = 0;
        ALsizei count = map->size;
        do {
            ALsizei step = count>>1;
            ALsizei i = pos+step;
            if(!(map->keys[i] < key))
                count = step;
            else
            {
                pos = i+1;
                count -= step+1;
            }
        } while(count > 0);
        if(pos < map->size && map->keys[pos] == key)
        {
            ptr = map->values[pos];
            if(pos < map->size-1)
            {
                memmove(&map->keys[pos], &map->keys[pos+1],
                        (map->size-1-pos)*sizeof(map->keys[0]));
                memmove(&map->values[pos], &map->values[pos+1],
                        (map->size-1-pos)*sizeof(map->values[0]));
            }
            map->size--;
        }
    }
    WriteUnlock(&map->lock);
    return ptr;
}

ALvoid *LookupUIntMapKey(UIntMap *map, ALuint key)
{
    ALvoid *ptr = NULL;
    ReadLock(&map->lock);
    if(map->size > 0)
    {
        ALsizei pos = 0;
        ALsizei count = map->size;
        do {
            ALsizei step = count>>1;
            ALsizei i = pos+step;
            if(!(map->keys[i] < key))
                count = step;
            else
            {
                pos = i+1;
                count -= step+1;
            }
        } while(count > 0);
        if(pos < map->size && map->keys[pos] == key)
            ptr = map->values[pos];
    }
    ReadUnlock(&map->lock);
    return ptr;
}
