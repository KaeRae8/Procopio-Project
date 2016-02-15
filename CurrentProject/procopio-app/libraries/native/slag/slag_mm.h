#ifndef SLAG_MM_H
#define SLAG_MM_H
//=============================================================================
//  slag_mm.h
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

#ifndef SLAGMM_SMALL_OBJECT_PAGE_SIZE
#  define SLAGMM_PERMALLOC_PAGE_SIZE (512*1024)
#endif

#define SLAGMM_NUM_GENERATIONS 4
#define SLAGMM_NUM_ALLOC_POOLS 4

struct SlagGlobalRef
{
  SlagObject* object;

  SlagGlobalRef() : object(NULL) { }
  SlagGlobalRef( SlagObject* object );
  ~SlagGlobalRef();

  void operator=( SlagObject* obj );

  SlagObject* operator*() { return object; }
  SlagObject* operator->() { return object; }
  operator SlagObject*() { return object; }
  operator SlagArray*() { return (SlagArray*) object; }
  operator SlagString*() { return (SlagString*) object; }
};

struct SlagObjectManager
{
  int count;
  int limit;
  int gen;
  SlagObject*       objects;
  SlagObjectManager* next_gen;

  SlagObjectManager() : count(0), limit(1000), objects(NULL), next_gen(NULL) { }

  virtual ~SlagObjectManager()
  {
    clean_up();
  }

  virtual void clean_up();

  void add( SlagObject* object );
  void check_gc();

  virtual void delete_object( SlagObject* obj );
  virtual void collect();

  void zero_reference_counts();
  void delete_unreferenced_objects();
};

struct SlagObjectManagerWithCleanup : SlagObjectManager
{
  void collect();
};

struct SlagSmallObjectManager : SlagObjectManager
{
  int bin_index;

  void clean_up()
  {
    objects = NULL;
    count = 0;
  }

  void delete_object( SlagObject* obj );
};

struct SlagMM
{
  SlagObjectManager generations[SLAGMM_NUM_GENERATIONS];
  SlagSmallObjectManager generations32[SLAGMM_NUM_GENERATIONS];
  SlagSmallObjectManager generations64[SLAGMM_NUM_GENERATIONS];
  SlagSmallObjectManager generations96[SLAGMM_NUM_GENERATIONS];
  SlagSmallObjectManager generations128[SLAGMM_NUM_GENERATIONS];
  SlagObjectManagerWithCleanup objects_requiring_cleanup;
  SlagObject* objects_pending_cleanup;

  AllocList<char*> permalloc_pages;
  char* cur_permalloc_page;
  int   available_permalloc_size;

  SlagObject* allocation_pools[SLAGMM_NUM_ALLOC_POOLS];

  ArrayList<SlagGlobalRef*> global_refs;
  ArrayList<SlagWeakRef*>   weak_refs;
  SlagObject*               local_refs[256];
  SlagObject**              local_ref_limit;  // hard-coded to 256 in init()
  SlagObject**              local_ref_ptr;

  int stack_retain_count;

  bool initialized;
  bool force_gc;

  SlagMM() { initialized=false; init(); }
  ~SlagMM();

  void init();
  void shut_down();

  void check_gc();
  void gc();
  void retain_stack();
  void release_stack();
  void zero_weak_refs_to_unreferenced_objects();
  void zero_all_reference_counts();
  void trace_accessible_objects();
  static void trace( SlagObject* object );

  SlagObject* create_object( SlagTypeInfo* type, int byte_size );
  SlagArray*  create_array( SlagTypeInfo* type, int count );

  SlagObject* permalloc( int bytes );
};

extern SlagMM mm;
extern SlagGlobalRef slag_main_object;

//=============================================================================
//  SlagLocalRef
//=============================================================================
struct SlagLocalRef
{
  SlagObject** stack_pos;

  SlagLocalRef( SlagObject* object )
  {
    *(stack_pos = --mm.local_ref_ptr) = object;
#if defined(SLAG_VM)
    if (stack_pos < mm.local_refs) slag_throw_fatal_error("Local ref stack limit exceeded.");
#endif
  }

  SlagLocalRef( const SlagLocalRef& other )
  {
    *(stack_pos = --mm.local_ref_ptr) = *(other.stack_pos);
#if defined(SLAG_VM)
    if (stack_pos < mm.local_refs) slag_throw_fatal_error("Local ref stack limit exceeded.");
#endif
  }

  ~SlagLocalRef()
  {
    ++mm.local_ref_ptr;
  }

  void operator=( SlagObject* object )
  {
    *stack_pos = object;
  }

  SlagObject* operator*() { return *stack_pos; }
  SlagObject* operator->() { return *stack_pos; }

  operator SlagObject*() { return (SlagObject*) *stack_pos; }
  operator SlagArray*() { return (SlagArray*) *stack_pos; }
  operator SlagString*() { return (SlagString*) *stack_pos; }
};


#endif

