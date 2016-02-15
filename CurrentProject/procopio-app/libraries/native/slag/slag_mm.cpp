//=============================================================================
//  slag_mm.cpp
//
//  Slag Memory Manager
//
//  v3.5.0
//  ---------------------------------------------------------------------------
//
//  Copyright 2008-2011 Plasmaworks LLC
//
//  Licensed under the Apache License, Version 2.0 (the "License"); 
//  you may not use this file except in compliance with the License. 
//  You may obtain a copy of the License at 
//
//    http://www.apache.org/licenses/LICENSE-2.0 
//
//  Unless required by applicable law or agreed to in writing, 
//  software distributed under the License is distributed on an 
//  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
//  either express or implied. See the License for the specific 
//  language governing permissions and limitations under the License.
//
//  ---------------------------------------------------------------------------
//
//  History:
//    2010.12.24 / Abe Pralle - v3.2 revamp
//    2008.12.17 / Abe Pralle - Created
//=============================================================================

#include "slag.h"

#if defined(SLAG_VM)
#  define SLAGMM_CALL( ref, m ) SLAG_PUSH_REF(ref); svm.call(m)
#else
#  define SLAGMM_CALL( ref, m ) m(ref)
#endif

SlagMM mm;
SlagGlobalRef slag_main_object;

//=============================================================================
// SlagGlobalRef
//=============================================================================
SlagGlobalRef::SlagGlobalRef( SlagObject* object ) : object(object)
{
  if (object) mm.global_refs.add( this );
}

SlagGlobalRef::~SlagGlobalRef()
{
  if (object) mm.global_refs.remove_value(this);
}

void SlagGlobalRef::operator=( SlagObject* obj )
{
  if (object && obj)
  {
    // Already tracked, don't need to change global ref list.
    object = obj;
  }
  else
  {
    if (object) mm.global_refs.remove_value(this);
    object = obj;
    if (object) mm.global_refs.add( this );
  }
}

//=============================================================================
// SlagObjectManager
//=============================================================================

void SlagObjectManager::clean_up()
{
  SlagObject* cur = objects;
  while (cur)
  {
    SlagObject* next = cur->next;
    delete cur;
    cur = next;
  }
  objects = NULL;
  count = 0;
}

void SlagObjectManager::add( SlagObject* object )
{
  //if (count >= limit) collect();
  ++count;
  object->next = objects;
  objects = object;
}

void SlagObjectManager::check_gc()
{
  if (count >= limit) 
  {
    collect();
  }
}

void SlagObjectManager::delete_object( SlagObject* obj )
{
  delete obj;
}

void SlagObjectManager::collect()
{
  mm.retain_stack();

  int remaining_count = count;
  count = 0;

  SlagObject* cur = objects;
  objects = NULL;
  SlagObject* dest = next_gen->objects;

  while (cur)
  {
    SlagObject* next = cur->next;
    if (cur->reference_count)
    {
      // double check refs are good (DEBUG)
      /*
      {
        SlagTypeInfo* type = cur->type;
        if (type->is_array())
        {
          if (type->is_reference_array())
          {
            int c = ((SlagArray*)cur)->array_count + 1;
            SlagObject** ref_ptr = ((SlagObject**) ((SlagArray*)cur)->data) - 1;
            while (--c)
            {
              if (*(++ref_ptr))
              {
                if (!((*ref_ptr)->reference_count))
                {
                  printf( "A %s has a bad reference to a %s\n",type->name,(*ref_ptr)->type->name );
                }
              }
            }
          }
        }
        else
        {
          int c = type->reference_property_offsets.count;
          int* offsets = type->reference_property_offsets.data;
          for (int i=c-1; i>=0; --i)
          {
            SlagObject** ref_ptr = (SlagObject**)(((char*)cur) + offsets[i]);
            if (*ref_ptr)
            {
              if (!((*ref_ptr)->reference_count))
              {
                printf( "A %s has a bad reference to a %s\n",type->name,(*ref_ptr)->type->name );
              }
            }
          }
        }
      }
      */

      cur->next = dest;
      dest = cur;
    }
    else
    {
      --remaining_count;

      // Release references held by this object
      SlagTypeInfo* type = cur->type;
      if (type->is_array())
      {
        if (type->is_reference_array())
        {
          int c = ((SlagArray*)cur)->array_count + 1;
          SlagObject** ref_ptr = ((SlagObject**) ((SlagArray*)cur)->data) - 1;
          while (--c)
          {
            if (*(++ref_ptr))
            {
              --((*ref_ptr)->reference_count);
              *ref_ptr = NULL;
            }
          }
        }
      }
      else
      {
        int c = type->reference_property_offsets.count;
        int* offsets = type->reference_property_offsets.data;
        for (int i=c-1; i>=0; --i)
        {
          SlagObject** ref_ptr = (SlagObject**)(((char*)cur) + offsets[i]);
          if (*ref_ptr)
          {
            --(*ref_ptr)->reference_count;
            *ref_ptr = NULL;
          }
        }
      }

      delete_object(cur);
    }
    cur = next;
  }

  next_gen->objects = dest;
  next_gen->count += remaining_count;

  if (remaining_count*2 > limit) limit = remaining_count * 2;

  if (next_gen->count >= next_gen->limit)
  {
    next_gen->collect();
  }

  mm.release_stack();
}

void SlagObjectManager::zero_reference_counts()
{
  SlagObject* cur = objects;
  while (cur)
  {
    cur->reference_count = 0;
    cur = cur->next;
  }
}

void SlagObjectManager::delete_unreferenced_objects()
{
  SlagObject* cur = objects;
  SlagObject* dest = NULL;
  while (cur)
  {
    SlagObject* next = cur->next;
    if (cur->reference_count)
    {
      cur->next = dest;
      dest = cur;
    }
    else
    {
      --count;
      delete_object( cur );
    }
    cur = next;
  }
  objects = dest;
}

void SlagObjectManagerWithCleanup::collect()
{
  mm.retain_stack();

  int remaining_count = count;
  count = 0;

  SlagObject* cur = objects;
  objects = NULL;
  SlagObject* dest = next_gen->objects;

  while (cur)
  {
    SlagObject* next = cur->next;
    if (cur->reference_count)
    {
      // double check refs are good (DEBUG)
      /*
      {
        SlagTypeInfo* type = cur->type;
        if (type->is_array())
        {
          if (type->is_reference_array())
          {
            int c = ((SlagArray*)cur)->array_count + 1;
            SlagObject** ref_ptr = ((SlagObject**) ((SlagArray*)cur)->data) - 1;
            while (--c)
            {
              if (*(++ref_ptr))
              {
                if (!((*ref_ptr)->reference_count))
                {
                  printf( "A %s has a bad reference to a %s\n",type->name,(*ref_ptr)->type->name );
                }
              }
            }
          }
        }
        else
        {
          int c = type->reference_property_offsets.count;
          int* offsets = type->reference_property_offsets.data;
          for (int i=c-1; i>=0; --i)
          {
            SlagObject** ref_ptr = (SlagObject**)(((char*)cur) + offsets[i]);
            if (*ref_ptr)
            {
              if (!((*ref_ptr)->reference_count))
              {
                printf( "A %s has a bad reference to a %s\n",type->name,(*ref_ptr)->type->name );
              }
            }
          }
        }
      }
      */

      cur->next = dest;
      dest = cur;
    }
    else
    {
      --remaining_count;
      cur->next = mm.objects_pending_cleanup;
      mm.objects_pending_cleanup = cur;
    }
    cur = next;
  }

  next_gen->objects = dest;
  next_gen->count += remaining_count;

  if (remaining_count*2 > limit) limit = remaining_count * 2;

  if (next_gen->count >= next_gen->limit)
  {
    next_gen->collect();
  }

  mm.release_stack();
}

void SlagSmallObjectManager::delete_object( SlagObject* obj )
{
  *((SlagObject**)obj) = mm.allocation_pools[bin_index];
  mm.allocation_pools[bin_index] = obj;
}

SlagMM::~SlagMM()
{
  shut_down();
}

void SlagMM::init()
{
  if (initialized) return;
  initialized = true;

  force_gc = false;

  cur_permalloc_page = NULL;
  available_permalloc_size = 0;

  for (int i=0; i<SLAGMM_NUM_ALLOC_POOLS; ++i) allocation_pools[i] = NULL;

  objects_pending_cleanup = NULL;
  stack_retain_count = 0;

  for (int i=0; i<(SLAGMM_NUM_GENERATIONS-1); ++i)
  {
    generations[i].next_gen = &generations[i+1];
    generations[i].gen = i+1;
    generations32[i].next_gen = &generations32[i+1];
    generations32[i].gen = i+1;
    generations64[i].next_gen = &generations64[i+1];
    generations64[i].gen = i+1;
    generations96[i].next_gen = &generations96[i+1];
    generations96[i].gen = i+1;
    generations128[i].next_gen = &generations128[i+1];
    generations128[i].gen = i+1;
  }

  for (int i=0; i<SLAGMM_NUM_GENERATIONS; ++i)
  {
    generations32[i].bin_index  = 0;
    generations64[i].bin_index  = 1;
    generations96[i].bin_index  = 2;
    generations128[i].bin_index = 3;
  }

  generations[SLAGMM_NUM_GENERATIONS-1].next_gen = &generations[SLAGMM_NUM_GENERATIONS-1];
  generations[SLAGMM_NUM_GENERATIONS-1].gen = SLAGMM_NUM_GENERATIONS;
  generations32[SLAGMM_NUM_GENERATIONS-1].next_gen = &generations32[SLAGMM_NUM_GENERATIONS-1];
  generations32[SLAGMM_NUM_GENERATIONS-1].gen = SLAGMM_NUM_GENERATIONS;
  generations64[SLAGMM_NUM_GENERATIONS-1].next_gen = &generations64[SLAGMM_NUM_GENERATIONS-1];
  generations64[SLAGMM_NUM_GENERATIONS-1].gen = SLAGMM_NUM_GENERATIONS;
  generations96[SLAGMM_NUM_GENERATIONS-1].next_gen = &generations96[SLAGMM_NUM_GENERATIONS-1];
  generations96[SLAGMM_NUM_GENERATIONS-1].gen = SLAGMM_NUM_GENERATIONS;
  generations128[SLAGMM_NUM_GENERATIONS-1].next_gen = &generations128[SLAGMM_NUM_GENERATIONS-1];
  generations128[SLAGMM_NUM_GENERATIONS-1].gen = SLAGMM_NUM_GENERATIONS;

  objects_requiring_cleanup.next_gen = &objects_requiring_cleanup;
  objects_requiring_cleanup.gen = 0;

  local_ref_limit = local_refs + 256;
  local_ref_ptr   = local_ref_limit;
}

void SlagMM::shut_down()
{
  initialized = false;

  // Before exiting, call clean_up on every object that requires it.
  SlagObject* cur = objects_requiring_cleanup.objects;
  objects_requiring_cleanup.objects = NULL;
  objects_requiring_cleanup.count = 0;

  while (cur)
  {
    SlagObject* next = cur->next;

    cur->next = generations[0].objects;
    generations[0].objects = cur;
    ++generations[0].count;

    SLAGMM_CALL( cur, cur->type->method_clean_up );

    cur = next;
  }

  // Release global references
  for (int i=0; i<mm.global_refs.count; ++i)
  {
    mm.global_refs[i]->object = NULL;
  }
  mm.global_refs.clear();

  // Clean up object lists
  for (int i=SLAGMM_NUM_GENERATIONS-1; i>=0; --i)
  {
    generations[i].clean_up();
    generations32[i].clean_up();
    generations64[i].clean_up();
    generations96[i].clean_up();
    generations128[i].clean_up();
  }
  objects_requiring_cleanup.clean_up();

  permalloc_pages.clear();
  available_permalloc_size = 0;
  cur_permalloc_page = NULL;
}

void SlagMM::check_gc()
{
  if (force_gc)
  {
    force_gc = false;
    gc();
  }
  else
  {
    generations[0].check_gc();
    generations32[0].check_gc();
    generations64[0].check_gc();
    generations96[0].check_gc();
    generations128[0].check_gc();
    objects_requiring_cleanup.check_gc();
  }
}


void SlagMM::gc()
{
//printf("GC\n");
  // Full mark & sweep.
  zero_all_reference_counts();
  trace_accessible_objects();
  
  // Don't need to actually call retain_stack() since we
  // just did a full trace.
  stack_retain_count++;  

  for (int i=SLAGMM_NUM_GENERATIONS-1; i>=0; --i)
  {
    generations[i].collect();
    generations32[i].collect();
    generations64[i].collect();
    generations96[i].collect();
    generations128[i].collect();
  }
  objects_requiring_cleanup.collect();

  release_stack();
}

void SlagMM::retain_stack()
{
  if (stack_retain_count++) return;

  {
    SlagObject** cur   = (SlagObject**) (SLAG_REF_STACK_PTR - 1);
    SlagObject** limit = (SlagObject**) SLAG_REF_STACK_LIMIT;
    while (++cur < limit)
    {
      SlagObject* obj = *cur;
      if (obj) ++obj->reference_count;
    }
  }

  // Global Refs
  {
    SlagGlobalRef** cur = mm.global_refs.data - 1;
    int count = mm.global_refs.count + 1;
    while (--count)
    {
      SlagObject* obj = (*(++cur))->object;
      if (obj) ++(obj->reference_count);
    }
  }

  // Note: weak refs are not retained or released

  // Local Refs
  {
    SlagObject** cur = mm.local_ref_ptr - 1;
    SlagObject** limit = mm.local_ref_limit;
    while (++cur < limit)
    {
      SlagObject* obj = *cur;
      if (obj) ++obj->reference_count;
    }
  }

  zero_weak_refs_to_unreferenced_objects();
}

void SlagMM::release_stack()
{
  if (--stack_retain_count) return;

  // Release stack references
  {
    SlagObject** cur   = (SlagObject**) (SLAG_REF_STACK_PTR - 1);
    SlagObject** limit = (SlagObject**) SLAG_REF_STACK_LIMIT;
    while (++cur < limit)
    {
      SlagObject* obj = *cur;
      if (obj) --obj->reference_count;
    }
  }

  // Release global references
  {
    SlagGlobalRef** cur = mm.global_refs.data - 1;
    int count = mm.global_refs.count + 1;
    while (--count)
    {
      SlagObject* obj = (*(++cur))->object;
      if (obj) --(obj->reference_count);
    }
  }

  // Note: weak refs are not retained or released

  // Release local references
  {
    SlagObject** cur = mm.local_ref_ptr - 1;
    SlagObject** limit = mm.local_ref_limit;
    while (++cur < limit)
    {
      SlagObject* obj = *cur;
      if (obj) --obj->reference_count;
    }
  }

  // The GC is fully over; now call clean_up on each unreferenced
  // object requiring cleanup, then move it to the new objects
  // list (not requiring cleanup) for probable deletion next gc.
  SlagObject* cur = objects_pending_cleanup;
  objects_pending_cleanup = NULL;
  while (cur)
  {
    SlagObject* next = cur->next;
    cur->next = generations[0].objects;
    generations[0].objects = cur;
    ++generations[0].count;
    SLAGMM_CALL( cur, cur->type->method_clean_up );
    cur = next;
  }
}

void SlagMM::zero_weak_refs_to_unreferenced_objects()
{
  for (int i=weak_refs.count-1; i>=0; --i)
  {
    SlagWeakRef* ref = weak_refs[i];
    SlagObject* object = ref->object;
    if (object && object->reference_count == 0)
    {
      ref->set( NULL );  // Also removes from weak_refs[]
    }
  }
}

void SlagMM::zero_all_reference_counts()
{
  for (int i=SLAGMM_NUM_GENERATIONS-1; i>=0; --i)
  {
    generations[i].zero_reference_counts();
    generations32[i].zero_reference_counts();
    generations64[i].zero_reference_counts();
    generations96[i].zero_reference_counts();
    generations128[i].zero_reference_counts();
  }
  objects_requiring_cleanup.zero_reference_counts();
}

void SlagMM::trace_accessible_objects()
{
  // singletons
  for (int i=SLAG_SINGLETONS_COUNT-1; i>=0; --i)
  {
    SlagMM::trace( SLAG_SINGLETONS[i] );
  }

  // stack
  {
    SlagObject** cur   = (SlagObject**) (SLAG_REF_STACK_PTR - 1);
    SlagObject** limit = (SlagObject**) SLAG_REF_STACK_LIMIT;
    while (++cur < limit)
    {
      SlagMM::trace( *cur );
    }
  }

  // global references
  {
    SlagGlobalRef** cur = mm.global_refs.data - 1;
    int count = mm.global_refs.count + 1;
    while (--count)
    {
      SlagMM::trace( (*(++cur))->object );
    }
  }

#if defined(SLAG_VM)
  // string table
  {
    SlagObject** strings_data = svm.strings.data - 1;
    int count = svm.strings.count + 1;
    while (--count)
    {
      ++(*(++strings_data))->reference_count;
    }
  }
#endif

  zero_weak_refs_to_unreferenced_objects();

  // Any unreferenced objects requiring cleanup should
  // be traced and added to the pending cleanup list.
  {
    SlagObject* cur = mm.objects_requiring_cleanup.objects;
    SlagObject* dest = NULL;

    while (cur)
    {
      SlagObject* next = cur->next;
      if (cur->reference_count)
      {
        cur->next = dest;
        dest = cur;
      }
      else
      {
        --mm.objects_requiring_cleanup.count;
        cur->next = mm.objects_pending_cleanup;
        mm.objects_pending_cleanup = cur;
        SlagMM::trace( cur );
      }
      cur = next;
    }
    mm.objects_requiring_cleanup.objects = dest;
  }

  // All unreferenced objects should be immediately deleted - otherwise
  // they'll go through standard collection which will try to adjust
  // property reference counts of other objects that are being deleted.
  for (int i=SLAGMM_NUM_GENERATIONS-1; i>=0; --i)
  {
    generations[i].delete_unreferenced_objects();
    generations32[i].delete_unreferenced_objects();
    generations64[i].delete_unreferenced_objects();
    generations96[i].delete_unreferenced_objects();
    generations128[i].delete_unreferenced_objects();
  }
}

void SlagMM::trace( SlagObject* object )
{
  if ( !object ) return;

  // If already referenced, increment the reference but
  // then return without tracing through.
  if (object->reference_count++) return;

  SlagTypeInfo* type = object->type;
  if (type->is_array())
  {
    if (type->is_reference_array())
    {
      int c = ((SlagArray*)object)->array_count + 1;
      SlagObject** ref_ptr = ((SlagObject**) ((SlagArray*)object)->data) - 1;
      while (--c)
      {
        SlagMM::trace( *(++ref_ptr) );
      }
    }
  }
  else
  {
    int c = type->reference_property_offsets.count;
    int* offsets = type->reference_property_offsets.data;
    for (int i=c-1; i>=0; --i)
    {
      SlagMM::trace( *((SlagObject**)(((char*)object) + offsets[i])) );
    }
  }
}

SlagObject* SlagMM::create_object( SlagTypeInfo* type, int byte_size )
{
  if (type->requires_cleanup())
  {
    SlagObject* obj = (SlagObject*) new char[byte_size];
    memset( obj, 0, byte_size );
    obj->type = type;
    objects_requiring_cleanup.add( obj );
    return obj;
  }
  else
  {
    if (byte_size <= 128)
    {
      byte_size = (byte_size + 31) & ~31;  // make 32, 64, 96, or 128
      if (byte_size == 0) byte_size = 32;
      SlagObject* obj = permalloc( byte_size );
      memset( obj, 0, byte_size );
      obj->type = type;
      switch (byte_size)
      {
        case 32:   generations32[0].add( obj ); break;
        case 64:   generations64[0].add( obj ); break;
        case 96:   generations96[0].add( obj ); break;
        case 128: generations128[0].add( obj ); break;
      }
      return obj;
    }
    else
    {
      SlagObject* obj = (SlagObject*) new char[byte_size];
      memset( obj, 0, byte_size );
      obj->type = type;
      generations[0].add( obj );
      return obj;
    }
  }
}

SlagArray* SlagMM::create_array( SlagTypeInfo* type, int count )
{
  int bytes = (sizeof(SlagArray) - 8) + count*type->element_size;
  SlagArray* result = (SlagArray*) create_object( type, bytes );
  result->array_count = count;
  return result;
}

SlagObject* SlagMM::permalloc( int bytes )
{
  // requires: bytes is 32, 64, 96, or 128
  int bin = (bytes >> 5) - 1;
  if (allocation_pools[bin])
  {
    SlagObject* result = allocation_pools[bin];
    allocation_pools[bin] = *((SlagObject**)result);
    return result;
  }

  if (bytes > available_permalloc_size)
  {
    cur_permalloc_page = new char[SLAGMM_PERMALLOC_PAGE_SIZE];
    permalloc_pages.add( cur_permalloc_page );
    available_permalloc_size = SLAGMM_PERMALLOC_PAGE_SIZE;
  }

  char* result = cur_permalloc_page;
  cur_permalloc_page += bytes;
  available_permalloc_size -= bytes;

  return (SlagObject*) result;
}

