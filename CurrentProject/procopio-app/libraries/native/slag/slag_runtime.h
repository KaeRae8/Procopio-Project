#ifndef SLAG_RUNTIME_H
#define SLAG_RUNTIME_H
//=============================================================================
//  slag_runtime.h
//
//  Runtime info common to both VM and XC builds.
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
//    2010.12.25 / Abe Pralle - Created
//=============================================================================

#define SLAG_PUSH_REAL64  SLAG_PUSH_REAL
#define SLAG_PUSH_REAL32  SLAG_PUSH_REAL
#define SLAG_PUSH_INT64   SLAG_PUSH_INTEGER
#define SLAG_PUSH_INT32   SLAG_PUSH_INTEGER
#define SLAG_PUSH_CHAR    SLAG_PUSH_INTEGER
#define SLAG_PUSH_BYTE    SLAG_PUSH_INTEGER
#define SLAG_PUSH_LOGICAL SLAG_PUSH_INTEGER

#define SLAG_POP_REAL64()  SLAG_POP_REAL()
#define SLAG_POP_REAL32()  ((SlagReal32)SLAG_POP_REAL())
#define SLAG_POP_INT64()   SLAG_POP_INTEGER()
#define SLAG_POP_INT32()   ((SlagInt32)SLAG_POP_INTEGER())
#define SLAG_POP_CHAR()    ((SlagChar)SLAG_POP_INTEGER())
#define SLAG_POP_BYTE()    ((SlagByte)SLAG_POP_INTEGER())
#define SLAG_POP_LOGICAL() (SLAG_POP_INTEGER() != 0)

#define SLAG_GET_REAL64( var_name, obj, name ) SLAG_GET(SlagReal64,var_name,obj,name)
#define SLAG_GET_REAL32( var_name, obj, name ) SLAG_GET(SlagReal32,var_name,obj,name)
#define SLAG_GET_INT64( var_name, obj, name ) SLAG_GET(SlagInt64,var_name,obj,name)
#define SLAG_GET_INT32( var_name, obj, name ) SLAG_GET(SlagInt32,var_name,obj,name)
#define SLAG_GET_CHAR( var_name, obj, name ) SLAG_GET(SlagChar,var_name,obj,name)
#define SLAG_GET_BYTE( var_name, obj, name ) SLAG_GET(SlagByte,var_name,obj,name)
#define SLAG_GET_LOGICAL( var_name, obj, name ) SLAG_GET(SlagLogical,var_name,obj,name)

#define SLAG_GET_NATIVE_DATA( type, varname, context, property_name ) \
  type varname = NULL; \
  { \
    SLAG_GET( SlagNativeData*, _native_data_obj_temp, context, property_name ); \
    if ( _native_data_obj_temp ) varname = (type) _native_data_obj_temp->data; \
  }

#define SLAG_RELEASE_NATIVE_DATA( context, property_name ) \
  { \
    SLAG_GET( SlagNativeData*, _native_data_obj_temp, context, property_name ); \
    if (_native_data_obj_temp) \
    { \
      SLAG_SET_REF( context, property_name, NULL ); \
      _native_data_obj_temp->release(); \
    } \
  }

#define SLAG_SET_REAL64( obj, name, value ) SLAG_SET(SlagReal64,obj,name,value)
#define SLAG_SET_REAL32( obj, name, value ) SLAG_SET(SlagReal32,obj,name,value)
#define SLAG_SET_INT64( obj, name, value )  SLAG_SET(SlagInt64,obj,name,value)
#define SLAG_SET_INT32( obj, name, value )  SLAG_SET(SlagInt32,obj,name,value)
#define SLAG_SET_CHAR( obj, name, value )   SLAG_SET(SlagChar,obj,name,value)
#define SLAG_SET_BYTE( obj, name, value )   SLAG_SET(SlagByte,obj,name,value)
#define SLAG_SET_LOGICAL( obj, name,value ) SLAG_SET(SlagLogical,obj,name,value)

#define SLAG_QUALIFIER_CLASS         1
#define SLAG_QUALIFIER_ASPECT       (1 << 1)
#define SLAG_QUALIFIER_PRIMITIVE    (1 << 2)
#define SLAG_QUALIFIER_COMPOUND     (1 << 3)
#define SLAG_QUALIFIER_SINGLETON    (1 << 4)
#define SLAG_QUALIFIER_MANAGED      (1 << 5)
#define SLAG_QUALIFIER_DEFERRED     (1 << 6)
#define SLAG_QUALIFIER_UNDERLYING   (1 << 7)
#define SLAG_QUALIFIER_OVERLAYING   (1 << 8)
#define SLAG_QUALIFIER_REQUISITE    (1 << 9)
#define SLAG_QUALIFIER_LIMITED      (1 <<10)
#define SLAG_QUALIFIER_NATIVE       (1 <<11)
#define SLAG_QUALIFIER_ABSTRACT     (1 <<12)
#define SLAG_QUALIFIER_AUTOMATIC    (1 <<13)
#define SLAG_QUALIFIER_PROPAGATED   (1 <<14)
#define SLAG_QUALIFIER_PUBLIC       (1 <<15)
#define SLAG_QUALIFIER_PRIVATE      (1 <<16)
#define SLAG_QUALIFIER_READONLY     (1 <<17)
#define SLAG_QUALIFIER_WRITEONLY    (1 <<18)
#define SLAG_QUALIFIER_CONTAINS_THROW (1 <<19)
#define SLAG_QUALIFIER_EMPTY_BODY     (1 <<20)
#define SLAG_QUALIFIER_AUGMENT          (1<<21)
#define SLAG_QUALIFIER_ENUM             (1<<22)
#define SLAG_QUALIFIER_GENERIC          (1<<23)
#define SLAG_QUALIFIER_CONSTANT         (1<<24)
#define SLAG_QUALIFIER_REQUIRES_CLEANUP (1<<25)
#define SLAG_QUALIFIER_ARRAY            (1<<26)
#define SLAG_QUALIFIER_REFERENCE_ARRAY  (1<<27)
#define SLAG_QUALIFIER_RUNTIME          (1<<28)

#define SLAG_QUALIFIER_REFERENCE  (SLAG_QUALIFIER_CLASS | SLAG_QUALIFIER_ASPECT)
#define SLAG_QUALIFIER_KIND       (SLAG_QUALIFIER_REFERENCE | SLAG_QUALIFIER_PRIMITIVE | SLAG_QUALIFIER_COMPOUND)

extern ASCIIString slag_error_message;

struct SlagTypeInfo;

struct SlagObject
{
  SlagTypeInfo* type;
  SlagObject*   next;
  SlagInt32     reference_count;

};

struct SlagArray : SlagObject
{
  SlagInt32 array_count;
  SlagInt64 data[1];  // Int64 to give this member 8-byte alignment.
};

struct SlagString : SlagObject
{
  static SlagString* create( int count );
  static SlagString* create( SlagChar* data, int count );
  static SlagString* create( const char* data, int count=-1 );
  static SlagTypeInfo* string_type();

  SlagInt32     count;
  SlagInt32     hash_code;
  SlagChar      characters[1];

  void  set_hash_code();
  void  to_ascii( char* buffer, int buffer_len );
  char* to_new_ascii();
};

struct SlagWeakRef : SlagObject
{
  static SlagWeakRef* create( SlagObject* obj );

  SlagObject* object;

  SlagWeakRef( SlagObject* object );
  void set( SlagObject* object );
};


struct SlagArrayList : SlagObject
{
  SlagArray* array;
  SlagInt32  count;
  SlagInt32  modification_count;
};

struct SlagParseReader : SlagObject
{
  SlagInt32  property_line, property_column, property_pos, property_remaining;
  SlagInt32  property_spaces_per_tab;
  SlagArray* property_data;
};

struct SlagException : SlagObject
{
  SlagString* message;
  SlagObject* stack_trace;
};

struct SlagDate : SlagObject
{
  SlagInt32 year;
  SlagInt32 month;
  SlagInt32 day;
  SlagInt32 hour;
  SlagInt32 minute;
  SlagInt32 second;
  SlagInt32 millisecond;
};

struct SlagFile : SlagObject
{
  SlagString* property_filepath;
};

struct SlagFileReader : SlagObject
{
  SlagObject* property_native_data;
};

struct SlagFileWriter : SlagObject
{
  SlagObject* property_native_data;
};

typedef void (*SlagNativeDataDeleteFn)(void*);

void SlagNativeDataNoDelete( void* data );
void SlagNativeDataGenericDelete( void* data );
void SlagNativeDataDeleteResource( void* data );

struct SlagNativeData : SlagObject
{
  static SlagNativeData* create( void* data, SlagNativeDataDeleteFn delete_fn );

  void* data;
  SlagNativeDataDeleteFn delete_fn;

  void init( void* data, SlagNativeDataDeleteFn delete_fn )
  {
    this->data = data;
    this->delete_fn = delete_fn;
  }

  ~SlagNativeData()
  {
    release();
  }

  void release()
  {
    if (data)
    {
      // Set pointer to null before calling delete function avoid any chance of
      // calling it twice.
      void* ptr = data;
      data = 0;
      delete_fn(ptr);
    }
  }
};


struct SlagResource
{
  virtual ~SlagResource() { }
};

struct SlagNativeDataWrapper : SlagObject
{
  SlagNativeData* native_data;
};

struct SlagSocket : SlagObject
{
  SlagNativeData* native_data;
  SlagString*     address;
  SlagInt32       port;
  SlagNativeDataWrapper *reader;
  SlagNativeDataWrapper *writer;
  // There are more properties but these are all we need on the native side.
};

//=============================================================================
//  SlagTypeInfoBase
//=============================================================================
struct SlagTypeInfoBase
{
  int qualifiers;
  int index;
  int singleton_index;
  int object_size;  // size of actual data
  int element_size; // byte size of an array element (if array)

  char* name;

  ArrayList<SlagTypeInfoBase*> base_types;
  ArrayList<int>               reference_property_offsets;

  SlagTypeInfoBase() : singleton_index(0), element_size(0)
  {
  }

  inline bool is_reference() { return (qualifiers & SLAG_QUALIFIER_REFERENCE) != 0; }
  inline bool is_primitive() { return (qualifiers & SLAG_QUALIFIER_PRIMITIVE) != 0; }
  inline bool is_singleton() { return (qualifiers & SLAG_QUALIFIER_SINGLETON) != 0; }
  inline bool is_managed() { return (qualifiers & SLAG_QUALIFIER_MANAGED) != 0; }
  inline bool is_compound()  { return (qualifiers & SLAG_QUALIFIER_COMPOUND)  != 0; }
  inline bool requires_cleanup() { return (qualifiers & SLAG_QUALIFIER_REQUIRES_CLEANUP) != 0; }
  inline bool is_array() { return (qualifiers & SLAG_QUALIFIER_ARRAY) != 0; }
  inline bool is_reference_array() { return (qualifiers & SLAG_QUALIFIER_REFERENCE_ARRAY) != 0; }

  bool instance_of( SlagTypeInfoBase* base_type )
  {
    if (this == base_type) return true;

    int count = base_types.count + 1;
    SlagTypeInfoBase** base_types = this->base_types.data - 1;
    while (--count)
    {
      if (base_type == *(++base_types)) return true;
    }

    return false;
  }
};


void slag_shut_down();
void slag_init();
void slag_configure();
void slag_launch();

void slag_throw_fatal_error( const char* st );
void slag_throw_fatal_error( const char* st1, const char* st2, const char* st3 );
void slag_throw_fatal_error( const char* st1, const char* st2, const char* st3,
    const char* st4, const char* st5 );

typedef void (*SlagNativeFn)();
void slag_hook_native( const char* class_name, const char* method_name, SlagNativeFn fn_ptr );

extern ArrayList<SlagNativeFn> slag_custom_shutdown_functions;
void  slag_add_custom_shutdown( SlagNativeFn fn );

SlagArray*     slag_create_byte_array( char* data, int count );
SlagArrayList* slag_create_byte_list( char* data, int count );
SlagArray*     slag_create_char_array( SlagChar* data, int count );
SlagArrayList* slag_create_char_list( SlagChar* data, int count );
SlagArray*     slag_duplicate_array( SlagObject* obj );

#ifdef SLAG_USE_LONGJMP
  struct SlagCatchHandler
  {
    SlagCatchHandler* prev;
    jmp_buf env;

    SlagCatchHandler();
    ~SlagCatchHandler();
    void unhook();
  };

  extern jmp_buf slag_fatal_jumppoint;
  extern SlagCatchHandler* slag_cur_catch;;
# define SLAG_TRY if (!setjmp(slag_cur_catch->env))
# define SLAG_CATCH(err_name) else
# define SLAG_THROW(err) slag_thrown_error = err; longjmp( slag_cur_catch->env, 1 )

#else
# define SLAG_TRY try
# define SLAG_CATCH(err_name) catch (SlagObject* err_name)
# define SLAG_THROW(err) throw err
#endif

extern SlagObject* slag_thrown_error;


#include "slag_mm.h"
#if defined(SLAG_VM)
#  include "slag_vm.h"
#else
#  include "slag_xc.h"
#endif
#include "slag_stdlib.h"

extern SlagGlobalRef slag_language_ref;
extern SlagGlobalRef slag_os_ref;
extern SlagGlobalRef slag_os_version_ref;
extern SlagGlobalRef slag_hw_version_ref;

#define SLAG_WRITE_REF( old_ref, new_ref ) \
  if (old_ref) --old_ref->reference_count; \
  old_ref = new_ref; \
  if (new_ref) ++new_ref->reference_count;

#endif // SLAG_RUNTIME_H
