//=============================================================================
//  slag_runtime.cpp
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
//    2010.12.24 / Abe Pralle - v3.2 revamp
//    2008.12.17 / Abe Pralle - Created
//=============================================================================

#include "slag.h"

//=============================================================================
//  Globals
//=============================================================================
ASCIIString slag_error_message;
SlagGlobalRef slag_language_ref;
SlagGlobalRef slag_os_ref;
SlagGlobalRef slag_os_version_ref;
SlagGlobalRef slag_hw_version_ref;

ArrayList<SlagNativeFn> slag_custom_shutdown_functions;


//=============================================================================
//  SlagString
//=============================================================================
SlagString* SlagString::create( int count )
{
  SlagString* string = (SlagString*) 
    mm.create_object( SLAG_TYPE_STRING, (sizeof(SlagString)-2) + count*2 );
  string->count = count;
  return string;
}

SlagString* SlagString::create( SlagChar* data, int count )
{
  SlagString* string = create(count);
  memcpy( string->characters, data, count*2 );
  string->set_hash_code();
  return string;
};

SlagString* SlagString::create( const char* data, int count )
{
  if (count == -1) count = strlen(data);

  SlagString* string = create(count);
  SlagChar* characters = string->characters;
  for (int i=0; i<count; ++i)
  {
    characters[i] = data[i];
  }
  string->set_hash_code();
  return string;
};

void SlagString::set_hash_code()
{
  SlagChar* data = characters - 1;
  int c = count + 1;

  int code = 0;
  while (--c)
  {
    // left rotate by 7 bits 
    code = (code << 7) | ((code >> (16-7)) & ((1<<7) - 1));
    
    code += *(++data);
  }

  hash_code = code;
}

void SlagString::to_ascii( char* buffer, int buffer_len )
{
  int c = buffer_len - 1;
  if (c > count) c = count;

  for (int i=0; i<c; ++i) buffer[i] = (char) characters[i];
  buffer[c] =  0;
}

char* SlagString::to_new_ascii()
{
  char* result = new char[count+1];
  for (int i=0; i<count; ++i) result[i] = (char) characters[i];
  result[count] = 0;
  return result;
}

#if defined(SLAG_XC)
  extern SlagTypeInfo type_ClassWeakReference;
#endif

//=============================================================================
//  SlagWeakRef
//=============================================================================
SlagWeakRef* SlagWeakRef::create( SlagObject* object )
{
  SlagWeakRef* ref = (SlagWeakRef*) 
    mm.create_object( SLAG_TYPE_WEAK_REFERENCE, (sizeof(SlagWeakRef)) );
  ref->set( object );
  return ref;
}

SlagWeakRef::SlagWeakRef( SlagObject* object ) : object(object)
{
  if (object) mm.weak_refs.add( this );
}

void SlagWeakRef::set( SlagObject* new_object )
{
  if (object)
  {
    if (new_object)
    {
      // No need to adjust weak ref list
      object = new_object;
    }
    else
    {
      object = NULL;
      mm.weak_refs.remove_value( this );
    }
  }
  else
  {
    if (new_object)
    {
      object = new_object;
      mm.weak_refs.add( this );
    }
  }
}

//=============================================================================
//  SlagNativeData
//=============================================================================
void SlagNativeDataNoDelete( void* data )
{
}

void SlagNativeDataGenericDelete( void* data )
{
  delete (char*) data;
}

void SlagNativeDataDeleteResource( void* data )
{
  delete ((SlagResource*) data);
}

#if defined(SLAG_VM)
SlagNativeData* SlagNativeData::create( void* data, SlagNativeDataDeleteFn delete_fn )
{
  SlagNativeData* result = (SlagNativeData*) mm.create_object( 
      svm.type_native_data, sizeof(SlagNativeData) );
  result->init( data, delete_fn );
  return result;
}
#else
extern SlagTypeInfo type_ClassNativeData;

SlagNativeData* SlagNativeData::create( void* data, SlagNativeDataDeleteFn delete_fn )
{
  SlagNativeData* result = (SlagNativeData*) 
    mm.create_object( &type_ClassNativeData, sizeof(SlagNativeData) );
  result->init( data, delete_fn );
  return result;
}
#endif

#ifdef _WIN32
  WSADATA winsock_data;
#endif

void slag_shut_down()
{
  for (int i=0; i<slag_custom_shutdown_functions.count; ++i)
  {
    slag_custom_shutdown_functions[i]();
  }
  slag_custom_shutdown_functions.clear();

#if defined(SLAG_VM)
  svm.shut_down();
#else
  sxc.shut_down();
#endif
}

void slag_init()
{
  mm.init();
#if defined(SLAG_VM)
  svm.init();
#else
  sxc.init();
#endif
}

void slag_configure()
{
#if defined(UNIX)
  signal( SIGPIPE, SIG_IGN );  // don't terminate program on a socket error
#elif _WIN32
  if (0 != WSAStartup( MAKEWORD(2,2), &winsock_data ))
  {
    slag_error_message = "Unable to start WinSock.";
    throw 1;
  }
  /*
  else
  {
    if ( LOBYTE( winsock_data.wVersion ) != 2 ||
         HIBYTE( winsock_data.wVersion ) != 2 )
    {
      WSACleanup();
      slag_throw_fatal_error( "WinSock - invalid version." );
    }
  }
  */
#endif

#if defined(SLAG_VM)
  svm.configure();
#else
  sxc_configure();
#endif
}

void slag_launch()
{
  SLAG_FIND_TYPE( type_SignalManager, "SignalManager" );
  SLAG_PUSH_REF( type_SignalManager->singleton() );
  SLAG_DUPLICATE_REF();

  SLAG_PUSH_REF( SlagString::create("launch") );
  SLAG_PUSH_REF( slag_main_object );
  SLAG_CALL( type_SignalManager, "queue_native(String,Object)" );

  bool repeat = true;
  while (repeat)
  {
    repeat = false;

    mm.check_gc();

    SLAG_DUPLICATE_REF();
    SLAG_CALL( type_SignalManager, "raise_pending()" );
    repeat = SLAG_POP_LOGICAL();
  }

  SLAG_POP_REF();

  /*
  // init_object()
  SLAG_PUSH_REF( slag_main_object );
  svm.call( svm.main_class->method_init_object );

  // init()
  SlagMethodInfo* m_init = svm.main_class->find_method( "init()" );
  SLAG_PUSH_REF( slag_main_object  );
  svm.call( m_init );
  */
}


void slag_hook_native( const char* class_name, const char* method_name, SlagNativeFn fn_ptr )
{
#if defined(SLAG_VM)
  svm.hook_native( class_name, method_name, fn_ptr );
#endif
}

void slag_add_custom_shutdown( SlagNativeFn fn )
{
  slag_custom_shutdown_functions.add( fn );
}

#if defined(SLAG_USE_LONGJMP)
extern jmp_buf slag_fatal_jumppoint;
void slag_throw_fatal_error( const char* st )
{
  slag_error_message = st;
  longjmp( slag_fatal_jumppoint, 1 );
}
#else
void slag_throw_fatal_error( const char* st )
{
  slag_error_message = st;
  throw 1;
}
#endif

void slag_throw_fatal_error( const char* st1, const char* st2, const char* st3 )
{
  int len1 = strlen(st1);
  int len2 = strlen(st2);
  int len3 = strlen(st3);
  char* result = new char[len1+len2+len3+1];
  strcpy( result, st1 );
  strcat( result, st2 );
  strcat( result, st3 );
  slag_throw_fatal_error( result );
}

void slag_throw_fatal_error( const char* st1, const char* st2, const char* st3,
    const char* st4, const char* st5 )
{
  int len1 = strlen(st1);
  int len2 = strlen(st2);
  int len3 = strlen(st3);
  int len4 = strlen(st4);
  int len5 = strlen(st5);
  char* result = new char[len1+len2+len3+len4+len5+1];
  strcpy( result, st1 );
  strcat( result, st2 );
  strcat( result, st3 );
  strcat( result, st4 );
  strcat( result, st5 );
  slag_throw_fatal_error( result );
}

SlagArray*     slag_create_byte_array( char* data, int count )
{
  SLAG_FIND_TYPE( type_byte_array, "Array<<Byte>>" );
  SlagArray* array = type_byte_array->create( count );
  if (data)
  {
    memcpy( array->data, data, count );
  }
  return array;
}

SlagArrayList* slag_create_byte_list( char* data, int count )
{
  SLAG_FIND_TYPE( type_byte_list, "ArrayList<<Byte>>" );
  SlagLocalRef list = type_byte_list->create();

  SLAG_FIND_TYPE( type_byte_array, "Array<<Byte>>" );
  SlagArray* array = type_byte_array->create( count );
  if (data)
  {
    memcpy( array->data, data, count );
  }

  SLAG_SET_REF( *list, "data", array );
  SLAG_SET_INT32( *list, "count", count );

  return (SlagArrayList*) *list;
}

SlagArray*     slag_create_char_array( SlagChar* data, int count )
{
  SLAG_FIND_TYPE( type_char_array, "Array<<Char>>" );
  SlagArray* array = type_char_array->create( count );
  if (data)
  {
    memcpy( array->data, data, count*2 );
  }
  return array;
}

SlagArrayList* slag_create_char_list( SlagChar* data, int count )
{
  SLAG_FIND_TYPE( type_char_list, "ArrayList<<Char>>" );
  SlagLocalRef list = type_char_list->create();

  SLAG_FIND_TYPE( type_char_array, "Array<<Char>>" );
  SlagArray* array = type_char_array->create( count );
  if (data)
  {
    memcpy( array->data, data, count );
  }

  SLAG_SET_REF( *list, "data", array );
  SLAG_SET_INT32( *list, "count", count );

  return (SlagArrayList*) *list;
}

SlagArray* slag_duplicate_array( SlagObject* obj )
{
  if (obj == NULL) return NULL;

  SlagLocalRef gc_guard(obj);

  SlagArray* original = (SlagArray*) obj;
  int count = original->array_count;

  SlagArray* duplicate = original->type->create( count );

  memcpy( duplicate->data, original->data, count * original->type->element_size );

  return duplicate;
}

SlagObject* slag_thrown_error = NULL;
#ifdef SLAG_USE_LONGJMP
  jmp_buf slag_fatal_jumppoint;
  SlagCatchHandler* slag_cur_catch = NULL;

  SlagCatchHandler::SlagCatchHandler()
  {
    prev = slag_cur_catch;
    slag_cur_catch = this;
  }

  SlagCatchHandler::~SlagCatchHandler()
  {
    unhook();
  }

  void SlagCatchHandler::unhook()
  {
    if (slag_cur_catch == this) slag_cur_catch = prev;
  }
#endif

