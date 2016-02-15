#ifndef SLAG_STDLIB_H
#define SLAG_STDLIB_H
//=============================================================================
//  slag_stdlib.h
//
//  Slag Virtual Machine - functions for native I/O.
//
//  v3.5.0
//  ---------------------------------------------------------------------------
//
//  Copyright 2008-2010 Plasmaworks LLC
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
//    2008.12.27 / Abe Pralle - Created
//    2010.12.10 / Abe Pralle - Revamped for Slag VM 3.2
//=============================================================================

#ifdef ANDROID
#  include <jni.h>
#endif

/*
extern SlagReal64 Slag_pi;

struct SlagParseReader
{
  SlagTypeInfo* type;
  SlagInt32     line, column, pos, remaining, spaces_per_tab;
  SlagArray*    array;
};
*/

struct SlagProcess : SlagObject
{
  SlagInt32   exit_code;
  SlagLogical is_active;
};

//-----------------------------------------------------------------------------
//  GenericArray
//-----------------------------------------------------------------------------

void GenericArray__clear__Int32_Int32();
void GenericArray__count();
void GenericArray__copy_from__GenericArray_Int32_Int32_Int32();

//-----------------------------------------------------------------------------
//  Date
//-----------------------------------------------------------------------------
void Date__ms_to_ymdhmsms( SlagInt64 timestamp, 
    int* y, int* mo, int *d, int* h, int* m, int* s, int* ms );
SlagInt64 Date__ymdhms_to_ms( int y, int mo, int d, int h, int m, int s );

void Date__init__Int64();
void Date__timestamp();


//-----------------------------------------------------------------------------
//  Global
//-----------------------------------------------------------------------------
SlagInt64 slag_get_time_ms();
void Global__time_ms();
void Global__sleep__Int32();

//-----------------------------------------------------------------------------
//  File
//-----------------------------------------------------------------------------
SlagLogical slag_file_exists( SlagString* filepath );
SlagLogical slag_file_is_directory( SlagString* filepath );
void        slag_file_directory_listing( SlagString* filepath, SlagObject* list );
SlagString* slag_file_absolute_filepath( SlagString* filepath );
void        slag_file_rename( SlagString* filepath, SlagString* new_name );
void        slag_file_delete( SlagString* filepath );
bool        slag_get_file_timestamp( const char* filepath, SlagInt64* timestamp ); 
SlagInt64   slag_file_timestamp( SlagString* filepath );
void        slag_file_touch( SlagString* filepath );
void        slag_file_change_dir( SlagString* filepath );

void File__exists();
void File__is_directory();
void File__directory_listing__ArrayList_of_String();
void File__absolute_filepath();
void File__rename__String();
void File__delete();
void File__timestamp();
void File__touch();
void File__native_mkdir();
void File__change_dir();

//-----------------------------------------------------------------------------
//  FileReader
//-----------------------------------------------------------------------------
void slag_adjust_filename_for_os( char* filename, int len );

void FileReader__init__String();
void FileReader__close();
void FileReader__has_another();
void FileReader__peek();
void FileReader__read();
void FileReader__read__Array_of_Byte_Int32_Int32();
void FileReader__read__Array_of_Char_Int32_Int32();
void FileReader__remaining();
void FileReader__skip__Int32();
void FileReader__position();

//-----------------------------------------------------------------------------
//  FileWriter
//-----------------------------------------------------------------------------
void FileWriter__init__String_Logical();
void FileWriter__close();
void FileWriter__write__Char();
void FileWriter__write__Array_of_Char_Int32_Int32();
void FileWriter__write__Array_of_Byte_Int32_Int32();
void FileWriter__write__String();
void FileWriter__position();


//=============================================================================
//  NativeData
//=============================================================================
void NativeData__clean_up();


//=============================================================================
//  Object
//=============================================================================
void Object__hash_code();
//void Object__runtime_type();

//-----------------------------------------------------------------------------
//  ParseReader
//-----------------------------------------------------------------------------
void ParseReader__prep_data();
void ParseReader__has_another();
void ParseReader__peek();
void ParseReader__peek__Int32();
void ParseReader__read();
void ParseReader__consume__Char();
void ParseReader__consume__String();

//-----------------------------------------------------------------------------
//  Process
//-----------------------------------------------------------------------------
void Process__init__String();
void Process__update();
void Process__release();


//=============================================================================
//  Runtime
//=============================================================================

/*
void Runtime__find_type_index__String();

//-----------------------------------------------------------------------------
//  RuntimeMethod
//-----------------------------------------------------------------------------
void RuntimeMethod__name();
void RuntimeMethod__return_type();
void RuntimeMethod__parameter_types();
void RuntimeMethod__signature();
void RuntimeMethod__arg__Object();
void RuntimeMethod__arg__Int64();
void RuntimeMethod__arg__Int32();
void RuntimeMethod__call();
void RuntimeMethod__call_Object();
void RuntimeMethod__call_Int64();
void RuntimeMethod__call_Int32();


//-----------------------------------------------------------------------------
//  RuntimeMethods
//-----------------------------------------------------------------------------
void RuntimeMethods__count();


//-----------------------------------------------------------------------------
//  RuntimeProperties
//-----------------------------------------------------------------------------
void RuntimeProperties__count();


//-----------------------------------------------------------------------------
//  RuntimePropertyManager
//-----------------------------------------------------------------------------
void RuntimeProperty__type();
void RuntimeProperty__name();
void RuntimeProperty__as_Object();
void RuntimeProperty__as_Object__Object();
void RuntimeProperty__as_Int64();
void RuntimeProperty__as_Int64__Int64();
void RuntimeProperty__as_Int32();
void RuntimeProperty__as_Int32__Int32();
void RuntimeProperty__as_Char();
void RuntimeProperty__as_Char__Char();
void RuntimeProperty__as_Byte();
void RuntimeProperty__as_Byte__Byte();


//-----------------------------------------------------------------------------
//  RuntimeType
//-----------------------------------------------------------------------------
void RuntimeType__name();
void RuntimeType__instance_of__RuntimeType();
void RuntimeType__create_instance();
*/

//-----------------------------------------------------------------------------
//  Socket
//  SocketReader
//  SocketWriter
//  ServerSocket
//-----------------------------------------------------------------------------
void Socket__native_init__String_Int32();
void Socket__connection_pending();
void Socket__is_connected();
void Socket__native_remote_ip();
void Socket__close();
void SocketReader__available();
void SocketReader__peek();
void SocketReader__read();
void SocketWriter__write__Char();
void ServerSocket__native_init__Int32();
void ServerSocket__is_connected();
void ServerSocket__get_pending_info();
void ServerSocket__close();

//=============================================================================
//  StackTrace
//=============================================================================
void StackTrace__native_history();
void StackTrace__describe__Int64();

//-----------------------------------------------------------------------------
//  stdout, stdin
//-----------------------------------------------------------------------------
void StdInReader__native_read_char();

void StdOutWriter__flush();
void StdOutWriter__print__Char();
void StdOutWriter__print__String();
void StdOutWriter__write__Char();
void StdOutWriter__write__String();

//-----------------------------------------------------------------------------
//  String
//-----------------------------------------------------------------------------
void StringBuilder__native_copy__String_Array_of_Char_Int32();

void StringManager__create_from__Array_of_Char_Int32();
void StringManager__create_from__Char();

void String__count();
void String__get__Int32();
void String__hash_code();
void String__opCMP__String();
void String__opEQ__String();
void String__opADD__String();
void String__opADD__Char();
void String__substring__Int32_Int32();
void String__to_Array();

//=============================================================================
//  System
//=============================================================================
void System__catch_control_c__Logical();
void System__control_c_pressed();

void System__exit_program__Int32_String();

void System__force_garbage_collection();
void System__get__String();
void System__language();
void System__os();
void System__os_version();
void System__hardware_version();
void System__raw_exe_filepath();

//=============================================================================
//  WeakReference
//=============================================================================
void WeakReferenceManager__create_from__Object();
void WeakReference__object__Object();
void WeakReference__object();

#endif /* SLAG_STDLIB_H */
