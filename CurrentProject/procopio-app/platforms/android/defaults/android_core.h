#ifndef ANDROID_CORE_H
#define ANDROID_CORE_H
//=============================================================================
// android_core.h
//
// $(PLASMACORE_VERSION)
//
// -----------------------------------------------------------------------------
//
// Copyright 2010-2011 Plasmaworks LLC
//
//   http://plasmaworks.com/plasmacore
//
// Licensed under the Apache License, Version 2.0 (the "License"); 
// you may not use this file except in compliance with the License. 
// You may obtain a copy of the License at 
//
//   http://www.apache.org/licenses/LICENSE-2.0 
//
// Unless required by applicable law or agreed to in writing, 
// software distributed under the License is distributed on an 
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific 
// language governing permissions and limitations under the License.
//=============================================================================
#include "plasmacore.h"
#include <jni.h>

extern JNIEnv* jvm;

void LOG( const char* st );
void LOG( SlagString* string_obj );

struct JavaByteArray
{
  jbyteArray array;
  int count;
  char* data;

  JavaByteArray()
  {
    count = 0;
    array = NULL;
    data = NULL;
  }

  JavaByteArray( jbyteArray array )
  {
    data = NULL;
    retain( array );
  }

  ~JavaByteArray() { release(); }

  void retain( jbyteArray array )
  {
    // retain is automatically called for you on creation
    this->array = array;
    count = jvm->GetArrayLength(array);
    if ( !data ) data = (char*) jvm->GetByteArrayElements( array, NULL );
  }

  void release()
  {
    if (data)
    {
      jvm->ReleaseByteArrayElements( array, (jbyte*)data, 0 );
      data = NULL;
    }
  }
};

struct JavaIntArray
{
  jintArray array;
  int count;
  jint* data;

  JavaIntArray()
  {
    count = 0;
    array = NULL;
    data = NULL;
  }

  JavaIntArray( jintArray array )
  {
    data = NULL;
    retain( array );
  }

  ~JavaIntArray() { release(); }

  void retain( jintArray array )
  {
    // retain is automatically called for you on creation
    this->array = array;
    count = jvm->GetArrayLength(array);
    if ( !data ) data = (jint*) jvm->GetIntArrayElements( array, NULL );
  }

  void release()
  {
    if (data)
    {
      jvm->ReleaseIntArrayElements( array, (jint*)data, 0 );
      data = NULL;
    }
  }
};

struct JavaStringArray
{
  jobjectArray array;
  int count;

  JavaStringArray( jobjectArray array )
  {
    this->array = array;
    count = jvm->GetArrayLength(array);
  }

  SlagString* operator[]( int index )
  {
    jstring jstr = (jstring) jvm->GetObjectArrayElement( array, index );
    const char* string_value = jvm->GetStringUTFChars( jstr, NULL );
    SlagString* result = (SlagString*) SlagString::create((char*)string_value);
    jvm->ReleaseStringUTFChars( jstr, string_value );
    jvm->DeleteLocalRef( jstr );
    return result;
  }
};

SlagString* to_slag_string( jstring jst );
jstring to_jstring( SlagObject* string_obj );

#endif // ANDROID_CORE_H
