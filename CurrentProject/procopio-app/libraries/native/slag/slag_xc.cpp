//=============================================================================
//  slag_xc.cpp
//
//  Cross-Compiled Slag Routines
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

#include "slag.h"

extern SlagTypeInfo type_ClassObject;
extern SlagTypeInfo type_ClassString;
extern SlagTypeInfo type_ClassGlobal;
extern SlagTypeInfo type_ClassSocketError;
extern SlagTypeInfo type_ClassDivideByZeroError;
extern SlagTypeInfo type_ClassInvalidOperandError;
SlagObject* sxc_get_global_cmd_line_args_list();

//=============================================================================
//  Global Variables
//=============================================================================
SlagXC sxc;
char* sxc_raw_exe_filepath = NULL;
SlagTypeInfo* sxc_main_class = NULL;

//=============================================================================
//  SlagXC
//=============================================================================
void SlagXC::shut_down()
{
  initialized = false;

  mm.shut_down();

  if (ref_stack)
  {
    delete ref_stack;
    ref_stack = NULL;
  }

  if (data_stack)
  {
    delete data_stack;
    data_stack = NULL;
  }
}

void SlagXC::set_up_method_info( int* info_table, int count )
{
  for (int i=0; i<count; ++i)
  {
    SlagMethodInfo* m = &sxc_methods[i];
    m->name = sxc_identifiers[*(info_table++)];
    m->parameter_signature = sxc_identifiers[*(info_table++)];
    m->method_ptr = sxc_method_pointers[i];
    m->method_caller = sxc_caller_table[*(info_table++)];
    m->index = i;
  }
}

void SlagXC::create_method_lists( int* info_table, int num_lists )
{
  for (int i=0; i<num_lists; ++i)
  {
    SlagTypeInfo* type = sxc_types[ *(info_table++) ];
    int m_count = *(info_table++);
    type->methods.clear();
    type->methods.ensure_capacity( m_count );
    for (int j=0; j<m_count; ++j)
    {
      type->methods.add( &sxc_methods[ *(info_table++) ] );
    }
  }
}

SlagTypeInfo* SlagXC::find_type( const char* name )
{
  for (int i=0; i<sxc_types_count; ++i)
  {
    SlagTypeInfo* type = sxc_types[i];
    if (type && 0 == strcmp(name,type->name))
    {
      return type;
    }
  }
  return NULL;
}

SlagTypeInfo* SlagXC::must_find_type( const char* name )
{
  SlagTypeInfo* result = find_type(name);
  if (result == NULL) slag_throw_fatal_error( "No such type \"", name, "\"" );
  return result;
}

//=============================================================================
//  SlagTypeInfo
//=============================================================================
SlagObject* SlagTypeInfo::create()
{
   SlagObject* object = mm.create_object( this, object_size );
   if (method_init_object) method_init_object( object );
   return object;
}

SlagObject* SlagTypeInfo::create_without_init()
{
   SlagObject* object = mm.create_object( this, object_size );
   return object;
}

SlagArray* SlagTypeInfo::create( int count )
{
   return mm.create_array( this, count );
}


//=============================================================================
//  SlagString
//=============================================================================
SlagTypeInfo* SlagString::string_type()
{
  return &type_ClassString;
}

//=============================================================================
//  Miscellaneous Methods
//=============================================================================
void slag_set_raw_exe_filepath( char* filepath )
{
  sxc_raw_exe_filepath = filepath;
}

void slag_set_command_line_args( char** argv, int argc )
{
  for (int i=0; i<argc; ++i)
  {
    ArrayList_of_Object__add__Object( 
        sxc_get_global_cmd_line_args_list(), 
        SlagString::create(argv[i])
      );

  }
}

SlagObject* FileError__init( SlagObject* context );
SlagObject* FileError__init__String( SlagObject* context, SlagObject* filename );
SlagObject* FileNotFoundError__init__String( SlagObject* context, SlagObject* filename );
SlagObject* NoNextValueError__init( SlagObject* context );
SlagObject* SocketError__init( SlagObject* context );
SlagObject* DivideByZeroError__init( SlagObject* context );
SlagObject* InvalidOperandError__init( SlagObject* context );

void slag_throw_file_error()
{
  SLAG_THROW( FileError__init( type_ClassFileError.create() ) );
}

void slag_throw_file_error( char* filename )
{
  SLAG_THROW( FileError__init__String(
      type_ClassFileError.create(), SlagString::create(filename) ) );
}

void slag_throw_file_not_found_error( char* filename )
{
  SLAG_THROW( FileNotFoundError__init__String(
      type_ClassFileNotFoundError.create(), SlagString::create(filename) ) );
}

void slag_throw_no_next_value_error()
{
  SLAG_THROW( NoNextValueError__init( type_ClassNoNextValueError.create() ) );
}

void slag_throw_socket_error()
{
  SLAG_THROW( SocketError__init( type_ClassSocketError.create() ) );
}

void slag_throw_divide_by_zero_error()
{
  SLAG_THROW( DivideByZeroError__init( type_ClassDivideByZeroError.create() ) );
}

void slag_throw_invalid_operand_error()
{
  SLAG_THROW( InvalidOperandError__init( type_ClassInvalidOperandError.create() ) );
}


void sxc_write_ref( SlagObject** dest, SlagObject* obj )
{
  if (*dest) --((*dest)->reference_count);
  if (obj) ++(obj->reference_count);
  *dest = obj;
}

SlagObject* sxc_as( SlagObject* obj, SlagTypeInfo* as_type )
{
  if (obj && obj->type->instance_of(as_type)) return obj;
  return NULL;
}

SlagLogical sxc_instance_of( SlagObject* obj, SlagTypeInfo* of_type )
{
  if (obj && obj->type->instance_of(of_type)) return (SlagLogical) 1;
  return (SlagLogical) 0;
}

SlagInt64 sxc_real64_as_int64( SlagReal64 n )
{
  return *((SlagInt64*)&n);
}

SlagInt32 sxc_real32_as_int32( SlagReal32 n )
{
  return *((SlagInt32*)&n);
}

SlagReal64 sxc_int64_as_real64( SlagInt64 n )
{
  return *((SlagReal64*)&n);
}

SlagReal32 sxc_int32_as_real32( SlagInt32 n )
{
  return *((SlagReal32*)&n);
}

SlagReal64  sxc_abs( SlagReal64 n )
{
  return (n >= 0) ? n : -n;
}

SlagReal64  sxc_mod( SlagReal64 a, SlagReal64 b )
{
  SlagReal64 q = a / b;
  return (a - (floor(q)) * b);
}

SlagReal32  sxc_mod( SlagReal32 a, SlagReal32 b )
{
  SlagReal32 q = a / b;
  return (a - (floor(q)) * b);
}

SlagInt64   sxc_mod( SlagInt64 a, SlagInt64 b )
{
  if (b == 0)
  {
    slag_throw_divide_by_zero_error();
  }

  if (b == 1)
  {
    return 0;
  }
  else if ((a ^ b) < 0)
  {
    SlagInt64 r = a % b;
    return (r ? (r+b) : r);
  }
  else
  {
    return (a % b);
  }
}

SlagInt32   sxc_mod( SlagInt32 a, SlagInt32 b )
{
  if (b == 0)
  {
    slag_throw_divide_by_zero_error();
  }

  if (b == 1)
  {
    return 0;
  }
  else if ((a ^ b) < 0)
  {
    SlagInt32 r = a % b;
    return (r ? (r+b) : r);
  }
  else
  {
    return (a % b);
  }
}

SlagInt64   sxc_shr( SlagInt64 n, SlagInt32 bits )
{
  if (bits == 0) return n;

  n = (n >> 1) & 0x7fffFFFFffffFFFFLL;
  if (--bits == 0) return n;

  return n >> bits;
}

SlagInt32   sxc_shr( SlagInt32 n, SlagInt32 bits )
{
  if (bits == 0) return n;

  n = (n >> 1) & 0x7fffFFFF;
  if (--bits == 0) return n;

  return n >> bits;
}

