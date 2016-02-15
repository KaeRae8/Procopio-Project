#ifndef SLAG_XC_H
#define SLAG_XC_H
//=============================================================================
//  slag_xc.h
//
//  Cross-Compiled Slag Header
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
//    2010.12.25 / Abe Pralle - Created for v3.2 revamp
//=============================================================================

#define SLAG_PUSH_REF( obj ) sxc.ref_stack_ptr[-1] = (obj); --sxc.ref_stack_ptr
#define SLAG_PEEK_REF() (*(sxc.ref_stack_ptr))
#define SLAG_POP_REF() *(sxc.ref_stack_ptr++)
#define SLAG_DUPLICATE_REF() sxc.ref_stack_ptr[-1] = *sxc.ref_stack_ptr; --sxc.ref_stack_ptr

#define SLAG_POP_INTEGER()   *(sxc.data_stack_ptr++)
#define SLAG_PUSH_INTEGER(i) *(--sxc.data_stack_ptr) = i
#define SLAG_PEEK_INTEGER() *(sxc.data_stack_ptr)

#define SLAG_POP_REAL()   *(SlagReal64*)(sxc.data_stack_ptr++)
#define SLAG_PUSH_REAL(r) *(SlagReal64*)(--sxc.data_stack_ptr) = r
#define SLAG_PEEK_REAL() *(SlagReal64*)(sxc.data_stack_ptr)

#define SLAG_PUSH(type,value) *((type*)(sxc.data_stack_ptr -= sizeof(type)/8)) = (value)
#define SLAG_PEEK(type) *((type*)sxc.data_stack_ptr)
#define SLAG_POP(type) ((type*)(sxc.data_stack_ptr += sizeof(type)/8))[-1]


#define SLAG_TYPE_STRING &type_ClassString
#define SLAG_TYPE_WEAK_REFERENCE &type_ClassWeakReference
#define SLAG_TYPE_ARRAY_OF_CHAR (&type_ClassArray_of_Char)

#define SLAG_REF_STACK_PTR sxc.ref_stack_ptr
#define SLAG_REF_STACK_LIMIT sxc.ref_stack_limit
#define SLAG_SINGLETONS sxc_singletons
#define SLAG_SINGLETONS_COUNT sxc_singletons_count

#define SLAG_CALL( type, sig ) \
  {\
    static int method_index = -1; \
    if (method_index == -1) method_index = type->must_find_method(sig)->index; \
    SlagMethodInfo* _m_info = &sxc_methods[method_index]; \
    ((void(*)(void*))_m_info->method_caller)(_m_info->method_ptr); \
  }

// Null check is defined out
#define SVM_NULL_CHECK( expr, cmd )

#define SLAG_FIND_TYPE( var_name, type_name ) \
  SlagTypeInfo* var_name; \
  { \
    static int type_index = -1; \
    if (type_index == -1) type_index = sxc.must_find_type(type_name)->index; \
    var_name = sxc_types[type_index]; \
  }

#define SLAG_GET_REF(var_name,obj,name) \
    SLAG_GET(SlagObject*,var_name,obj,name);

#define SLAG_SET_REF(obj,name,value) \
  {\
    static int property_offset = -1; \
    if (property_offset == -1) property_offset = (obj)->type->must_find_property(name)->offset; \
    SlagObject** _dest_ptr = (SlagObject**)(((char*)(obj)) + property_offset); \
    if (*_dest_ptr) --(*_dest_ptr)->reference_count; \
    *_dest_ptr = value; \
    if (value) ++(*_dest_ptr)->reference_count; \
  }

#define SLAG_GET(vartype,var_name,obj,name) \
  vartype var_name; \
  {\
    static int property_offset = -1; \
    if (property_offset == -1) property_offset = (obj)->type->must_find_property(name)->offset; \
    var_name = *((vartype*)(((char*)(obj)) + property_offset)); \
  }

#define SLAG_SET(vartype,obj,name,value) \
  {\
    static int property_offset = -1; \
    if (property_offset == -1) property_offset = (obj)->type->must_find_property(name)->offset; \
    *((vartype*)(((char*)(obj)) + property_offset)) = (vartype) value; \
  }

struct SlagMethodInfo;

extern SlagTypeInfo*   sxc_types[];
extern int             sxc_types_count;
extern SlagObject*     sxc_singletons[];
extern int             sxc_singletons_count;
extern char*           sxc_raw_exe_filepath;
extern const char*     sxc_identifiers[];
extern SlagTypeInfo*   sxc_main_class;
extern void*           sxc_method_pointers[];
extern SlagMethodInfo  sxc_methods[];
extern void*           sxc_caller_table[];

//extern SlagLiteralString* sxc_literal_strings;
//static int sxc_literal_strings_count = 0;

struct SlagXC
{
  SlagObject** ref_stack;
  SlagObject** ref_stack_ptr;
  SlagObject** ref_stack_limit;

  SlagInt64*   data_stack;
  SlagInt64*   data_stack_ptr;
  SlagInt64*   data_stack_limit;

  bool initialized;

  SlagXC()
  {
    initialized = false;
    init();
  }

  ~SlagXC()
  {
    shut_down();
  }

  void init()
  {
    if (initialized) return;
    initialized = true;

    ref_stack = new SlagObject*[SLAG_STACK_SIZE];
    ref_stack_limit = ref_stack + SLAG_STACK_SIZE;
    ref_stack_ptr = ref_stack_limit;

    // The data stack is only used when calling native methods.
    data_stack = new SlagInt64[512];
    data_stack_limit = data_stack + 512;
    data_stack_ptr = data_stack_limit;

    for (int i=0; i<sxc_singletons_count; ++i)
    {
      sxc_singletons[i] = NULL;
    }
  }

  void shut_down();

  void set_up_method_info( int* info_table, int count );
  void create_method_lists( int* info_table, int count );

  SlagTypeInfo* find_type( const char* name );
  SlagTypeInfo* must_find_type( const char* name );

};

extern SlagXC sxc;

/*
struct SXCRef
{
  SlagObject** stack_pos;

  SXCRef()
  {
    *(stack_pos = --sxc.ref_stack_ptr) = NULL;
  }

  SXCRef( SlagObject* object )
  {
    *(stack_pos = --sxc.ref_stack_ptr) = object;
  }

  SXCRef( const SXCRef& other )
  {
    *(stack_pos = --sxc.ref_stack_ptr) = *(other.stack_pos);
  }

  ~SXCRef()
  {
    ++sxc.ref_stack_ptr;
  }

  operator SlagObject*() { return (SlagObject*) *stack_pos; }
  operator SlagArray*() { return (SlagArray*) *stack_pos; }
  operator SlagString*() { return (SlagString*) *stack_pos; }

  void operator=( SlagObject* object )
  {
    *stack_pos = object;
  }

  void operator=( const SXCRef& other )
  {
    *stack_pos = *(other.stack_pos);
  }

  SlagObject* operator*() { return *stack_pos; }
  SlagObject* operator->() { return *stack_pos; }
};
*/

typedef void (*SlagFn)(SlagObject*);

struct SlagPropertyInfo
{
  SlagTypeInfo* type;
  const char*   name;
  int           offset;

  SlagPropertyInfo() { }

  SlagPropertyInfo( SlagTypeInfo* type, const char* name, int offset )
    : type(type), name(name), offset(offset)
  {
  }
};

struct SlagMethodInfo
{
  const char* name;
  const char* parameter_signature;
  void* method_ptr;
  void* method_caller;
  int   index;

  SlagMethodInfo() { }
};

struct SlagTypeInfo : SlagTypeInfoBase
{
  SlagFn method_init_object;
  SlagFn method_clean_up;
  ArrayList<SlagPropertyInfo> properties;
  ArrayList<SlagMethodInfo*>  methods;

  void** dispatch_table;

  SlagTypeInfo( int name_index, int qualifiers, int index, int singleton_index, 
      int base_type_count, int object_size,
      int reference_property_count )
    : method_init_object(NULL), method_clean_up(NULL), dispatch_table(NULL)
  {
    this->qualifiers = qualifiers;
    this->index = index;
    this->singleton_index = singleton_index;
    this->object_size = object_size;
    name = (char*) sxc_identifiers[name_index];
    sxc_types[index] = this;
    base_types.ensure_capacity( base_type_count );
    reference_property_offsets.ensure_capacity( reference_property_count );
  }

  void set_base_types( int* table, int offset )
  {
    base_types.clear();
    for (int i=0; i<base_types.capacity; ++i)
    {
      base_types.add( sxc_types[ table[offset+i] ] );
    }
  }

  void set_property_info( int* table, int index, int count )
  {
    int offset;
    if (this->is_reference()) offset = sizeof(SlagObject);
    else offset = 0;

    properties.clear();
    properties.ensure_capacity(count);
    reference_property_offsets.clear();

    for (int i=0; i<count; ++i)
    {
      SlagTypeInfo* type = sxc_types[ table[index++] ];
      const char*   name = sxc_identifiers[ table[index++] ];
      int size;
      if (type->is_reference())  size = sizeof(void*);
      else size = type->object_size;
      if ((offset & 1) && size >= 2) ++offset;
      if ((offset & 2) && size >= 4) offset += 2;

      if (offsetof(SlagAlignmentInfo,real)==8 && (offset & 4) && size >= 8) offset += 4;

      properties.add( SlagPropertyInfo( type, name, offset ) );

      if (type->is_reference()) reference_property_offsets.add( offset );

      offset += size;
    }
  }

  SlagObject* create();
  SlagObject* create_without_init();
  SlagArray*  create( int count );

  SlagObject* singleton() { return sxc_singletons[singleton_index]; }

  SlagPropertyInfo* find_property( const char* name )
  {
    for (int i=0; i<properties.count; ++i)
    {
      if (0 == strcmp(name,properties[i].name))
      {
        return &properties[i];
      }
    }
    return NULL;
  }

  SlagPropertyInfo* must_find_property( const char* name )
  {
    SlagPropertyInfo* result = find_property(name);
    if (result == NULL) slag_throw_fatal_error( "No such property \"", name, "\"" );
    return result;
  }

  SlagMethodInfo* find_method( const char* sig )
  {
    int pos;
    for (pos=0; sig[pos]; ++pos)
    {
      if (sig[pos] == '(') break;
    }

    if (sig[pos] == 0)
    {
      slag_throw_fatal_error( "Signature string is missing parens: \"", sig, "\"." );
    }

    for (int i=0; i<methods.count; ++i)
    {
      SlagMethodInfo* m = methods[i];
      if (0 == strncmp(sig,m->name,pos) && 0 == strcmp(sig+pos,m->parameter_signature))
      {
        return m;
      }
    }
    return NULL;
  }

  SlagMethodInfo* must_find_method( const char* sig )
  {
    SlagMethodInfo* result = find_method(sig);
    if (result == NULL) slag_throw_fatal_error( "No such method \"", sig, "\"" );
    return result;
  }
};

extern SlagTypeInfo type_ClassObject;
extern SlagTypeInfo type_ClassString;
extern SlagTypeInfo type_ClassArray_of_Char;
extern SlagTypeInfo type_ClassFileError;
extern SlagTypeInfo type_ClassFileNotFoundError;
extern SlagTypeInfo type_ClassNoNextValueError;
struct ClassString;

struct SlagLiteralString
{
  SlagString* value;

  SlagLiteralString( const char* utf8, int original_len )
  {
    // Don't use SlagString::create() since there's no guarantee
    // the memory manager has been set up yet.
    value = (SlagString*) new char[ sizeof(SlagString)-2 + original_len*2 ];
    value->type = SLAG_TYPE_STRING;
    value->reference_count = 0;
    value->next = NULL;
    value->count = original_len;

    --utf8;
    for (int i=0; i<original_len; ++i)
    {
      value->characters[i] = (SlagChar) read_utf8( &utf8 );
    }

    value->set_hash_code();
  }

  ~SlagLiteralString()
  {
    delete value;
    value = NULL;
  }

  operator SlagObject*()
  {
    return (SlagObject*) value;
  }

  int read_utf8( const char** ptrptr )
  {
    int ch = *(++(*ptrptr));

    if ((ch & 0x80) != 0)
    {
      int ch2 = *(++(*ptrptr));

      if ((ch & 0x20) == 0)
      {
        // %110xxxxx 10xxxxxx
        ch  &= 0x1f;
        ch2 &= 0x3f;
        ch = (ch << 6) | ch2;
      }
      else
      {
        // %1110xxxx 10xxxxxx 10xxxxxx
        int ch3 = *(++(*ptrptr));
        ch  &= 15;
        ch2 &= 0x3f;
        ch3 &= 0x3f;
        ch = (ch << 12) | (ch2 << 6) | ch3;
      }
    }

    return ch;
  }
};

void slag_set_raw_exe_filepath( char* filepath );
void sxc_configure();
void slag_set_command_line_args( char** argv, int argc );
void slag_throw_file_error();
void slag_throw_file_error( char* filename );
void slag_throw_file_not_found_error( char* filename );
void slag_throw_no_next_value_error();
void slag_throw_socket_error();
void slag_throw_divide_by_zero_error();
void slag_throw_invalid_operand_error();

void        sxc_write_ref( SlagObject** dest, SlagObject* obj );
SlagObject* sxc_as( SlagObject* obj, SlagTypeInfo* as_type );
SlagLogical sxc_instance_of( SlagObject* obj, SlagTypeInfo* of_type );
SlagInt64   sxc_real64_as_int64( SlagReal64 );
SlagInt32   sxc_real32_as_int32( SlagReal32 );
SlagReal64  sxc_int64_as_real64( SlagInt64 );
SlagReal32  sxc_int32_as_real32( SlagInt32 );
SlagReal64  sxc_abs( SlagReal64 n );
SlagReal64  sxc_mod( SlagReal64 a, SlagReal64 b );
SlagReal32  sxc_mod( SlagReal32 a, SlagReal32 b );
SlagInt64   sxc_mod( SlagInt64 a, SlagInt64 b );
SlagInt32   sxc_mod( SlagInt32 a, SlagInt32 b );
SlagInt64   sxc_shr( SlagInt64 n, SlagInt32 bits );
SlagInt32   sxc_shr( SlagInt32 n, SlagInt32 bits );

SlagObject* ArrayList_of_Object__add__Object( SlagObject* context, SlagObject* object );

#endif // SLAG_XC_H

