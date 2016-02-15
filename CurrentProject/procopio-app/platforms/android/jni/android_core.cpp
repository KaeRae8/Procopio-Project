//=============================================================================
// android_core.cpp
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

#include <jni.h>
#include <string.h>
#include <fcntl.h>

#include "android_core.h"
#include "png.h"

void perform_custom_setup();  // defined in custom.cpp

void Android__is_tablet();
void Android__memory_class();

void ObjC__showDialog();
void ObjC__showSpinner();
void ObjC__openSite__Int32();
void ObjC__removeSpinner();
void ObjC__exploreAction__Int32_Int32();
void ObjC__addSiteForCategory__String_String_Real64_Real64_String();
void ObjC__loadExpansionBitmapData__String();
void ObjC__fileExisted__String();

void VideoPlayer__play__String();
void VideoPlayer__update__NativeData();

void WebView__view__URL();
void WebView__view__String();
void WebView__close();
void WebView__bounds__Box();
void WebView__visible__Logical();
void WebView__visible();
void WebView__loaded();
void WebView__failed();


#define NATIVE_DEF(m) Java_com_plasmaworks_procopio_AndroidCore_##m

JNIEnv* jvm = 0;
jobject android_core_obj = 0;
jclass  class_AndroidCore = 0;
char*   android_etc_data=0;
int     android_etc_size=0;
SlagGlobalRef android_setup_ref;

bool plasmacore_initialized = false;

Archive data_archive( DATA_ARCHIVE );
Archive image_archive( IMAGES_ARCHIVE );

jmethodID m_jniLog = 0;
jmethodID m_jniAndroidMemoryClass = 0;
jmethodID m_jniAndroidIsTablet    = 0;
jmethodID m_jniExitProgram = 0;

jmethodID m_showDialog = 0;
jmethodID m_exploreAction = 0;
jmethodID m_openSite = 0;
jmethodID m_addSiteForCategory = 0;
jmethodID m_showSpinner = 0;
jmethodID m_removeSpinner = 0;
jmethodID m_loadBM = 0;
jmethodID m_fileExisted = 0;

jmethodID m_jniDecodeBitmapData = 0;
jmethodID m_jniEncodeBitmapData = 0;

jmethodID m_jniGetDeviceID = 0;
jmethodID m_jniGetCountryName = 0;
jmethodID m_jniOpenURL = 0;

jmethodID m_jniLoadResource = 0;
jmethodID m_jniIsDirectory = 0;
jmethodID m_jniFileExists = 0;
jmethodID m_jniDirectoryListing = 0;
jmethodID m_jniAbsoluteFilePath = 0;
jmethodID m_jniFileCopy = 0;
jmethodID m_jniFileRename = 0;
jmethodID m_jniFileDelete = 0;
jmethodID m_jniFileTimestamp = 0;
jmethodID m_jniFileTouch = 0;
jmethodID m_jniFileMkdir = 0;
jmethodID m_jniFileReaderOpen = 0;
jmethodID m_jniFileReaderClose = 0;
jmethodID m_jniFileReaderReadBytes = 0;
jmethodID m_jniFileReaderAvailable = 0;
jmethodID m_jniGetIOBuffer = 0;
jmethodID m_jniFileWriterOpen = 0;
jmethodID m_jniFileWriterClose = 0;
jmethodID m_jniFileWriterWriteBytes = 0;

jmethodID m_jniLoadGamestate = 0;
jmethodID m_jniSaveGamestate = 0;
jmethodID m_jniDeleteGamestate = 0;


jmethodID m_jniSoundLoad = 0;
jmethodID m_jniSoundDuplicate = 0;
jmethodID m_jniSoundPlay = 0;
jmethodID m_jniSoundPause = 0;
jmethodID m_jniSoundIsPlaying = 0;
jmethodID m_jniSoundSetVolume = 0;
jmethodID m_jniSoundSetRepeats = 0;
jmethodID m_jniSoundGetCurrentTime = 0;
jmethodID m_jniSoundSetCurrentTime = 0;
jmethodID m_jniSoundDuration = 0;
jmethodID m_jniSoundRelease = 0;

jmethodID m_jniShowKeyboard = 0;
jmethodID m_jniKeyboardVisible = 0;

jmethodID m_jniVideoPlay = 0;
jmethodID m_jniVideoUpdate = 0;
jmethodID m_jniVideoStop = 0;

jmethodID m_jniWebViewGet = 0;
jmethodID m_jniWebViewURL = 0;
jmethodID m_jniWebViewHTML = 0;
jmethodID m_jniWebViewClose = 0;
jmethodID m_jniWebViewSetBounds = 0;
jmethodID m_jniWebViewSetVisible = 0;
jmethodID m_jniWebViewGetVisible = 0;
jmethodID m_jniWebViewGetLoaded = 0;
jmethodID m_jniWebViewGetFailed = 0;


//-----------------------------------------------------------------------------

void LOG( const char* st )
{
  jobject mesg_obj = jvm->NewStringUTF(st);
  jvm->CallVoidMethod( android_core_obj, m_jniLog, mesg_obj );
  jvm->DeleteLocalRef( mesg_obj );
}

void LOG( SlagString* string_obj )
{
  int count = string_obj->count;
  if (count >= 512)
  {
    char* buffer = string_obj->to_new_ascii();
    LOG( buffer );
    delete buffer;
  }
  else
  {
    char buffer[512];
    string_obj->to_ascii( buffer, 512 );
    LOG( buffer );
  }
}

void GLTexture::on_textures_lost()
{
}

GLTexture::GLTexture( int w, int h, bool offscreen_buffer )
{
  next_texture = all_textures;
  all_textures = this;

  frame_buffer = 0;
  if (offscreen_buffer)
  {
    glGenFramebuffersOES( 1, &frame_buffer );
    glBindFramebufferOES( GL_FRAMEBUFFER_OES, frame_buffer );
  }
  glGenTextures( 1, &id );
  glBindTexture( GL_TEXTURE_2D, id );
  texture_width = texture_height = 0;
  resize( w, h );
}

void GLTexture::destroy()
{
  // unlink from all_textures list
  if (this == all_textures)
  {
    all_textures = this->next_texture;
  }
  else
  {
    GLTexture* prev = all_textures;
    GLTexture* cur =  all_textures->next_texture;
    while (cur != this)
    {
      prev = cur;
      cur = cur->next_texture;
    }
    prev->next_texture = this->next_texture;
  }

  if (id)
  {
    glDeleteTextures( 1, &id );
    if (frame_buffer)
    {
      glDeleteFramebuffersOES( 1, &frame_buffer );
    }
  }
}

//-----------------------------------------------------------------------------

SlagString* to_slag_string( jstring jst )
{
  if (jst == NULL) return NULL;

  int count = jvm->GetStringLength(jst);
  const jchar* char_data = jvm->GetStringChars( jst, NULL );

  SlagString* result = SlagString::create( count );
  memcpy( result->characters, char_data, count*2 );
  result->set_hash_code();

  jvm->ReleaseStringChars( jst, char_data );
  return result;
}

jstring to_jstring( SlagObject* string_obj )
{
  if (string_obj == NULL) return NULL;

  SlagString* str_obj = (SlagString*) string_obj;
  int count = str_obj->count;
  return (jstring) jvm->NewString((jchar*)(str_obj->characters),count);
}

void reset_gl()
{
  draw_buffer.reset();

  glViewport(0, 0, plasmacore.display_width, plasmacore.display_height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrthof( 0, plasmacore.display_width, plasmacore.display_height, 0, -1, 1 );
  glMatrixMode(GL_MODELVIEW);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
  glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
  glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);

  // Sets up pointers and enables states needed for using vertex arrays and textures
  glClientActiveTexture(GL_TEXTURE0);
  glVertexPointer( 2, GL_FLOAT, 0, draw_buffer.vertices );
  glTexCoordPointer( 2, GL_FLOAT, 0, draw_buffer.uv);
  glColorPointer( 4, GL_UNSIGNED_BYTE, 0, draw_buffer.colors);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  glClientActiveTexture(GL_TEXTURE1);
  glTexCoordPointer( 2, GL_FLOAT, 0, draw_buffer.alpha_uv);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glClientActiveTexture(GL_TEXTURE0);
}

//-----------------------------------------------------------------------------

extern "C" jboolean NATIVE_DEF(slagCreate)( JNIEnv* env, jobject THIS,
    int screen_width, int screen_height, jbyteArray etc_data )
{
  jvm = env;

  android_core_obj = (jobject) jvm->NewGlobalRef( THIS );

  // Get a global ref to class AndroidCore for finding Java methods.
  jclass local_class = jvm->FindClass( 
      "com/plasmaworks/procopio/Procopio" );
  class_AndroidCore = (jclass) jvm->NewGlobalRef( local_class );
  jvm->DeleteLocalRef( local_class );

  // Get info for the Java methods we need.
	m_jniLog = jvm->GetMethodID( class_AndroidCore, "jniLog", "(Ljava/lang/String;)V" );
  m_jniAndroidIsTablet    = jvm->GetMethodID( class_AndroidCore, "jniAndroidIsTablet", "()I" );
  m_jniAndroidMemoryClass = jvm->GetMethodID( class_AndroidCore, "jniAndroidMemoryClass", "()I" );
  m_jniExitProgram = jvm->GetMethodID( class_AndroidCore, "jniExitProgram", "()V" );
    
    m_showDialog = jvm->GetMethodID(class_AndroidCore, "showDialog", "()V");
    m_openSite = jvm->GetMethodID(class_AndroidCore, "openSite", "(I)V");
    m_exploreAction = jvm->GetMethodID(class_AndroidCore, "exploreAction", "(II)V");
    m_addSiteForCategory = jvm->GetMethodID(class_AndroidCore, "addSiteForCategory", "(Ljava/lang/String;Ljava/lang/String;DDLjava/lang/String;)V");
    m_showSpinner = jvm->GetMethodID( class_AndroidCore, "showSpinner","()V");
    m_removeSpinner = jvm->GetMethodID( class_AndroidCore, "removeSpinner","()V");
    m_loadBM = jvm->GetMethodID(class_AndroidCore,"loadExpansionBitmap","(Ljava/lang/String;)Ljava/nio/IntBuffer;");
    m_fileExisted = jvm->GetMethodID( class_AndroidCore, "fileExisted", "(Ljava/lang/String;)I" );

  m_jniDecodeBitmapData = jvm->GetMethodID( class_AndroidCore, "jniDecodeBitmapData", "([B)[I" );
  m_jniEncodeBitmapData = jvm->GetMethodID( class_AndroidCore, "jniEncodeBitmapData", "(II[III)[B" );

	m_jniGetDeviceID = jvm->GetMethodID( class_AndroidCore,    "jniGetDeviceID", "()Ljava/lang/String;" );
	m_jniGetCountryName = jvm->GetMethodID( class_AndroidCore, "jniGetCountryName", "()Ljava/lang/String;" );
	m_jniOpenURL = jvm->GetMethodID( class_AndroidCore, "jniOpenURL", "(Ljava/lang/String;)V" );

  m_jniLoadResource = jvm->GetMethodID( class_AndroidCore, "jniLoadResource", 
      "(ILjava/lang/String;)[B" );
  m_jniIsDirectory = jvm->GetMethodID(class_AndroidCore, "jniIsDirectory", "(Ljava/lang/String;)Z");
  m_jniFileExists = jvm->GetMethodID(class_AndroidCore, "jniFileExists", "(Ljava/lang/String;)Z");
  m_jniDirectoryListing = jvm->GetMethodID(class_AndroidCore, "jniDirectoryListing", 
      "(Ljava/lang/String;)[Ljava/lang/String;");
  m_jniAbsoluteFilePath = jvm->GetMethodID(class_AndroidCore, "jniAbsoluteFilePath", 
      "(Ljava/lang/String;)Ljava/lang/String;");
  m_jniFileCopy = jvm->GetMethodID(class_AndroidCore, "jniFileCopy", 
      "(Ljava/lang/String;Ljava/lang/String;)V");
  m_jniFileRename = jvm->GetMethodID(class_AndroidCore, "jniFileRename", 
      "(Ljava/lang/String;Ljava/lang/String;)V");
  m_jniFileDelete = jvm->GetMethodID(class_AndroidCore, "jniFileDelete", 
      "(Ljava/lang/String;)V");
  m_jniFileTimestamp = jvm->GetMethodID(class_AndroidCore, "jniFileTimestamp", 
      "(Ljava/lang/String;)J");
  m_jniFileTouch = jvm->GetMethodID(class_AndroidCore, "jniFileTouch", "(Ljava/lang/String;)V");
  m_jniFileMkdir = jvm->GetMethodID(class_AndroidCore, "jniFileMkdir", "(Ljava/lang/String;)V");

  m_jniFileReaderOpen = jvm->GetMethodID(class_AndroidCore,"jniFileReaderOpen",
      "(Ljava/lang/String;)I");
  m_jniFileReaderClose     = jvm->GetMethodID(class_AndroidCore,"jniFileReaderClose","(I)V");
  m_jniFileReaderReadBytes = jvm->GetMethodID(class_AndroidCore,"jniFileReaderReadBytes","(II)[B");
  m_jniFileReaderAvailable = jvm->GetMethodID(class_AndroidCore,"jniFileReaderAvailable","(I)I");

  m_jniGetIOBuffer = jvm->GetMethodID(class_AndroidCore,"jniGetIOBuffer","(I)[B");
  m_jniFileWriterOpen = jvm->GetMethodID(class_AndroidCore,"jniFileWriterOpen",
      "(Ljava/lang/String;Z)I");
  m_jniFileWriterClose     = jvm->GetMethodID(class_AndroidCore,"jniFileWriterClose","(I)V");
  m_jniFileWriterWriteBytes = jvm->GetMethodID(class_AndroidCore,"jniFileWriterWriteBytes",
      "(I[BI)V");

	m_jniLoadGamestate = jvm->GetMethodID( class_AndroidCore, "jniLoadGamestate", "(Ljava/lang/String;)[B" );
	m_jniSaveGamestate = jvm->GetMethodID( class_AndroidCore, "jniSaveGamestate", "(Ljava/lang/String;Ljava/lang/String;)Z" );
	m_jniDeleteGamestate = jvm->GetMethodID( class_AndroidCore, "jniDeleteGamestate", "(Ljava/lang/String;)Z" );
	

	m_jniSoundLoad = jvm->GetMethodID( class_AndroidCore, "jniSoundLoad", "(Ljava/lang/String;)I" );
	m_jniSoundDuplicate = jvm->GetMethodID( class_AndroidCore, "jniSoundDuplicate", "(I)I" );
	m_jniSoundPlay = jvm->GetMethodID( class_AndroidCore, "jniSoundPlay", "(I)V" );
	m_jniSoundPause = jvm->GetMethodID( class_AndroidCore, "jniSoundPause", "(I)V" );
	m_jniSoundIsPlaying = jvm->GetMethodID( class_AndroidCore, "jniSoundIsPlaying", "(I)Z" );
	m_jniSoundSetVolume = jvm->GetMethodID( class_AndroidCore, "jniSoundSetVolume", "(ID)V" );
	m_jniSoundSetRepeats = jvm->GetMethodID( class_AndroidCore, "jniSoundSetRepeats", "(IZ)V" );
	m_jniSoundGetCurrentTime = jvm->GetMethodID( class_AndroidCore, "jniSoundGetCurrentTime", "(I)D" );
	m_jniSoundSetCurrentTime = jvm->GetMethodID( class_AndroidCore, "jniSoundSetCurrentTime", "(ID)V" );
	m_jniSoundDuration = jvm->GetMethodID( class_AndroidCore, "jniSoundDuration", "(I)D" );
	m_jniSoundRelease = jvm->GetMethodID( class_AndroidCore, "jniSoundRelease", "(I)V" );

	m_jniShowKeyboard = jvm->GetMethodID( class_AndroidCore, "jniShowKeyboard", "(Z)V" );
	m_jniKeyboardVisible = jvm->GetMethodID( class_AndroidCore, "jniKeyboardVisible", "()Z" );

  m_jniVideoPlay = jvm->GetMethodID( class_AndroidCore,"jniVideoPlay",
      "(Ljava/lang/String;)I");
  m_jniVideoUpdate = jvm->GetMethodID( class_AndroidCore,"jniVideoUpdate", "(I)Z");
  m_jniVideoStop = jvm->GetMethodID( class_AndroidCore,"jniVideoStop", "(I)V");

  m_jniWebViewGet = jvm->GetMethodID( class_AndroidCore, "jniWebViewGet", "(I)I" );
  m_jniWebViewURL = jvm->GetMethodID( class_AndroidCore, "jniWebViewURL", "(ILjava/lang/String;)V" );
  m_jniWebViewHTML = jvm->GetMethodID( class_AndroidCore, "jniWebViewHTML", "(ILjava/lang/String;)V" );
  m_jniWebViewClose = jvm->GetMethodID( class_AndroidCore, "jniWebViewClose", "(I)V" );
  m_jniWebViewSetBounds = jvm->GetMethodID( class_AndroidCore, "jniWebViewSetBounds", "(IIIII)V" );
  m_jniWebViewSetVisible = jvm->GetMethodID( class_AndroidCore, "jniWebViewSetVisible", "(IZ)V" );
  m_jniWebViewGetVisible = jvm->GetMethodID( class_AndroidCore, "jniWebViewGetVisible", "(I)Z" );
  m_jniWebViewGetLoaded = jvm->GetMethodID( class_AndroidCore, "jniWebViewGetLoaded", "(I)Z" );
  m_jniWebViewGetFailed = jvm->GetMethodID( class_AndroidCore, "jniWebViewGetFailed", "(I)Z" );

  android_setup_ref = NULL;

  int err = setjmp(slag_fatal_jumppoint);
  if ( !err )
  {
    plasmacore_initialized = true;

    if (etc_data)
    {
      LOG( "game.etc loaded" );
      JavaByteArray array( etc_data );
      android_etc_data = array.data;
      android_etc_size = array.count;
    }

    plasmacore_init();

    slag_hook_native( "Android", "memory_class()", Android__memory_class );
    slag_hook_native( "Android", "is_tablet()",    Android__is_tablet );
      
      slag_hook_native( "ObjC", "showDialog()", ObjC__showDialog );
	  
    slag_hook_native( "VideoPlayer", "play(String)", VideoPlayer__play__String );
    slag_hook_native( "VideoPlayer", "update(NativeData)", VideoPlayer__update__NativeData );

    slag_hook_native( "WebView", "view(URL)",        WebView__view__URL );
    slag_hook_native( "WebView", "view(String)",     WebView__view__String );
    slag_hook_native( "WebView", "close()",          WebView__close );
    slag_hook_native( "WebView", "bounds(Box)",      WebView__bounds__Box );
    slag_hook_native( "WebView", "visible(Logical)", WebView__visible__Logical );
    slag_hook_native( "WebView", "visible()",        WebView__visible );
    slag_hook_native( "WebView", "loaded()",         WebView__loaded );
    slag_hook_native( "WebView", "failed()",         WebView__failed );

    perform_custom_setup();

    plasmacore_configure(screen_width,screen_height,true,false);

    //if (is_fast_hardware) plasmacore.fast_hardware = 1;
    //else plasmacore.fast_hardware = 0;

    reset_gl();

    plasmacore_launch();
    draw_buffer.render();
  }
  else
  {
    LOG( "------------------FATAL ERROR------------------" );
    LOG( slag_error_message.value );
  }
}

extern "C" jint NATIVE_DEF(slagUpdateDrawEvent)( JNIEnv* env, jobject THIS )
{
  jvm = env;
  if ( plasmacore_update() ) plasmacore_draw();
  return plasmacore.target_fps;
}

extern "C" void NATIVE_DEF(slagTexturesLostEvent)( JNIEnv* env, jobject THIS )
{
  jvm = env;
  GLTexture::on_textures_lost();
  reset_gl();
  plasmacore_queue_event( plasmacore.event_textures_lost );
  plasmacore_queue_event( plasmacore.event_resume );
  plasmacore_dispatch_pending_events();
}

extern "C" void NATIVE_DEF(slagKeyEvent)( JNIEnv* env, jobject THIS, jboolean is_press, 
    jint code, jboolean is_unicode)
{
  jvm = env;
  if (is_press) 
  {
    plasmacore_queue_data_event( plasmacore.event_key, is_unicode ? 1 : 0, code, true  );
  }
  else
  {
    plasmacore_queue_data_event( plasmacore.event_key, is_unicode ? 1 : 0, code, false  );
  }
}

extern "C" void NATIVE_DEF(slagTouchEvent)( JNIEnv* env, jobject THIS, jint stage,
    jint id, jdouble x, jdouble y )
{
  jvm = env;
  if (id == 0) return;

  x = (x - plasmacore.border_x) / plasmacore.scale_factor;
  y = (y - plasmacore.border_y) / plasmacore.scale_factor;

  if (stage == 0)
  {
    plasmacore_queue_data_event( plasmacore.event_mouse_button, id, 1, true, x, y );
  }
  else if (stage == 1)
  {
    plasmacore_queue_data_event( plasmacore.event_mouse_move, id, 1, true, x, y );
  }
  else
  {
    plasmacore_queue_data_event( plasmacore.event_mouse_button, id, 1, false, x, y );
  }
}

extern "C" void NATIVE_DEF(slagCustomEvent)( JNIEnv* env, jobject THIS, jobject jcustom_id,
    jdouble value, jobject jmessage )
{
  jvm = env;
  plasmacore_queue_object_event( 
      to_slag_string( (jstring) jcustom_id ),
      to_slag_string( (jstring) jmessage ),
      value, 0.0 );
}

extern "C" void NATIVE_DEF(slagAccelerationEvent)( JNIEnv* env, jobject THIS,
    jdouble x, jdouble y, jdouble z )
{
  jvm = env;
  if (plasmacore.original_orientation == 1)
  {
    double temp = x;
    x = y;
    y = -temp;
  }

  SLAG_FIND_TYPE( type_input, "Input" );
  SlagObject* input_obj = type_input->singleton();
  SLAG_SET_REAL64( input_obj, "acceleration_x", x );
  SLAG_SET_REAL64( input_obj, "acceleration_y", y );
  SLAG_SET_REAL64( input_obj, "acceleration_z", z );
}

extern "C" void NATIVE_DEF(slagOnResourceDownloaderProgress)( JNIEnv* env, jobject THIS, jdouble progress )
{
  jvm = env;
  if (*android_setup_ref == NULL) return;

  if (progress == -1.0)
  {
    // signal that actual downloading is taking place
    SLAG_SET_LOGICAL( *android_setup_ref, "downloading", 1 );
  }
  else
  {
    SLAG_SET_REAL64( *android_setup_ref, "progress", progress );
  }
}

extern "C" void NATIVE_DEF(slagOnPause)( JNIEnv* env, jobject THIS )
{
  jvm = env;
  //plasmacore_on_exit_request();
  plasmacore_queue_event( plasmacore.event_suspend );
  plasmacore_dispatch_pending_events();

  // Droid X doesn't resume when offscreen buffers exist AND textures
  // will need to be recreated on resume anyways - just delete all 
  // textures before pausing.  We'll give Plasmacore a notification
  // to recreate them.
  GLTexture* cur = all_textures;
  while (cur)
  {
    glDeleteTextures( 1, &(cur->id) );
    if (cur->frame_buffer) glDeleteFramebuffersOES( 1, &(cur->frame_buffer) );
    cur->id = 0;
    cur->frame_buffer = 0;
    cur = cur->next_texture;
  }
}

extern "C" void NATIVE_DEF(slagOnShutDown)( JNIEnv* env, jobject THIS )
{
  jvm = env;

  LOG( "Plasmacore shutting down" );

  if (plasmacore_initialized)
  {
    plasmacore_initialized = false;

    plasmacore_queue_event( plasmacore.event_shut_down );
    plasmacore_dispatch_pending_events();
    NativeLayer_shut_down();
    slag_shut_down();
  }
}

//-----------------------------------------------------------------------------

// local helper
void NativeLayer_init_bitmap( SlagObject* bitmap_obj, char* raw_data, int data_size )
{
  jbyteArray src_obj = jvm->NewByteArray( data_size );
  JavaByteArray src_wrapper( src_obj );
  memcpy( src_wrapper.data, raw_data, data_size );
  src_wrapper.release();
  jintArray decoded_obj = (jintArray) jvm->CallObjectMethod( android_core_obj,
      m_jniDecodeBitmapData , src_obj );

  jvm->DeleteLocalRef( src_obj );

  if (decoded_obj)
  {
    JavaIntArray decoded_array( decoded_obj );
    int width  = decoded_array.data[decoded_array.count-1];
    int height = (decoded_array.count - 1) / width;

    // premultiply the alpha and swap red with blue
    int count = (width * height) + 1;
    SlagInt32* cur = ((SlagInt32*) decoded_array.data);
    while (--count)
    {
      SlagInt32 color = *cur;
      int a = (color >> 24) & 255;
      int r = (color >> 16) & 255;
      int g = (color >> 8) & 255;
      int b = color & 255;

      r = (r * a) / 255;
      g = (g * a) / 255;
      b = (b * a) / 255;

      *(cur++) = (a<<24) | (r<<16) | (g<<8) | b;
    }

    SLAG_PUSH_REF( bitmap_obj );
    SLAG_PUSH_REF( bitmap_obj );
    SLAG_PUSH_INT32( width );
    SLAG_PUSH_INT32( height );
    SLAG_CALL( bitmap_obj->type, "init(Int32,Int32)" );
    SLAG_GET( SlagArray*, array, bitmap_obj, "data" );
    memcpy( array->data, decoded_array.data, width*height*4 );
		SLAG_POP_REF();
  }
  else
  {
    LOG("Bitmap not found\n");
    slag_throw_file_error();
  }
}

void NativeLayer_shut_down()
{
  // TODO
}

void NativeLayer_begin_draw()
{
  // Prepare for drawing.
  glDisable( GL_SCISSOR_TEST );

  SLAG_FIND_TYPE( type_display, "Display" );
  SLAG_GET_INT32( argb, type_display->singleton(), "background_color" );
  int alpha = (argb >> 24) & 255;
  if (alpha)
  {
    glClearColor( ((argb>>16)&255)/255.0f,
        ((argb>>8)&255)/255.0f,
        ((argb)&255)/255.0f,
        alpha/255.0f );

    glClear(GL_COLOR_BUFFER_BIT);
  }

  glEnable( GL_BLEND );

  draw_buffer.set_draw_target( NULL );
}

void NativeLayer_end_draw()
{
  draw_buffer.render();
}


//====================================================================

Archive::Archive( int type )
{
  archive_type = type;
}


char* Archive::load( const char* filename, int *size )
{
  if (strncmp(filename,"internal:",9) == 0)
  {
    filename += 9;
    if (0 == strncmp(filename,"font_system_17",14))
    {
      *size = embedded_font_system_17_size;
      char* buffer = (char*) new char[( *size )];
      memcpy( buffer, embedded_font_system_17, *size );
      return buffer;
    }


    return NULL;
  }
	
  jobject filename_obj = jvm->NewStringUTF(filename);
  jbyteArray array_obj = (jbyteArray) jvm->CallObjectMethod( android_core_obj, m_jniLoadResource, 
      archive_type, filename_obj );
  jvm->DeleteLocalRef( filename_obj );

  if (array_obj)
  {
		JavaByteArray wrapper = JavaByteArray(array_obj);
		char* data = (char*)new char[(wrapper.count)];
		memcpy(data, wrapper.data, wrapper.count);
		*size = wrapper.count;
		
    return data;
  }
  else
  {
    return NULL;
  }
}

void Android__memory_class()
{
  SLAG_POP_REF();  // singleton
  int result = jvm->CallIntMethod( android_core_obj, m_jniAndroidMemoryClass );
  SLAG_PUSH_INT32( result );
}

void Android__is_tablet()
{
  SLAG_POP_REF();  // singleton
  int result = jvm->CallIntMethod( android_core_obj, m_jniAndroidIsTablet );
  SLAG_PUSH_INT32( result );
}

void ObjC__showDialog()
{
    SLAG_POP_REF();
    jvm->CallVoidMethod( android_core_obj, m_showDialog);
}

void ObjC__showSpinner()
{
    SLAG_POP_REF();
    jvm->CallVoidMethod( android_core_obj, m_showSpinner);
}

void ObjC__removeSpinner()
{
    SLAG_POP_REF();
    jvm->CallVoidMethod( android_core_obj, m_removeSpinner);
}

void ObjC__openSite__Int32()
{
    int index = SLAG_POP_INT32();
    SLAG_POP_REF();
    jvm->CallVoidMethod( android_core_obj, m_openSite, index);
}

void ObjC__exploreAction__Int32_Int32()
{
    int index = SLAG_POP_INT32();
    int cat = SLAG_POP_INT32();
    SLAG_POP_REF();
    jvm->CallVoidMethod( android_core_obj, m_exploreAction, index, cat);
}

void ObjC__addSiteForCategory__String_String_Real64_Real64_String()
{
    jstring site = to_jstring(SLAG_POP_REF());
    double lon = SLAG_POP_REAL64();
    double lat = SLAG_POP_REAL64();
    jstring name = to_jstring(SLAG_POP_REF());
    jstring category = to_jstring(SLAG_POP_REF());
    SLAG_POP_REF();
    jvm->CallVoidMethod( android_core_obj, m_addSiteForCategory, name, category, lat, lon, site);
}

void ObjC__loadExpansionBitmapData__String(){
    //Pop string param and cast as a jstring or regular string?
    jstring fileName = to_jstring(SLAG_POP_REF());
    SLAG_POP_REF();
    jobject buffer = jvm->CallObjectMethod(android_core_obj, m_loadBM, fileName);
    SlagInt32* data = (SlagInt32*) jvm->GetDirectBufferAddress( buffer );
    SlagInt32 width = data[0];
    SlagInt32 height = data[1];
    
    SLAG_FIND_TYPE( type_Int32Array, "Array<<Int32>>" );
    int total_size = width * height + 2;
    SlagArray* array = type_Int32Array->create( total_size );
    
    // When copying the bitmap data to the array, move the width and height
    // to the end so that we don't have to scoot all the integers over on the
    // Slag side
    SlagInt32* array_data = (SlagInt32*) array->data;
    memcpy( array_data, data+8, total_size*4 ); // copy pixel data
    array_data[ total_size-2 ] = width;
    array_data[ total_size-1 ] = height;
    
    SLAG_PUSH_REF( array );
    
}

void ObjC__fileExisted__String(){
    jstring fileName = to_jstring(SLAG_POP_REF());
    SLAG_POP_REF();
    int result = jvm->CallIntMethod( android_core_obj, m_fileExisted,  fileName);
    SLAG_PUSH_INT32( result );
}

/*
void Android__fast_hardware()
{
  SLAG_POP_REF();
  SLAG_PUSH_INT32(plasmacore.fast_hardware);
}

void Android__email__String_String_String_String_String()
{
  jstring jmime_type = to_jstring( SLAG_POP_REF() );
  jstring jfilename   = to_jstring( SLAG_POP_REF() );
  jstring jrecipient = to_jstring( SLAG_POP_REF() );
  jstring jbody      = to_jstring( SLAG_POP_REF() );
  jstring jsubject   = to_jstring( SLAG_POP_REF() );
  SLAG_POP_REF();  // singleton

  jvm->CallVoidMethod( plasmacore, m_jniEmail, jsubject, jbody, jrecipient, jfilename, jmime_type );

  if (jsubject)   jvm->DeleteLocalRef( jsubject );
  if (jbody)      jvm->DeleteLocalRef( jbody );
  if (jrecipient) jvm->DeleteLocalRef( jrecipient );
  if (jfilename)  jvm->DeleteLocalRef( jfilename );
  if (jmime_type) jvm->DeleteLocalRef( jmime_type );
}

void Android__choose_photo__PhotoChoiceListener()
{
  photo_chooser_delegate.retain( SLAG_POP_REF() );
  SLAG_POP_REF(); // discard singleton
	jvm->CallVoidMethod( plasmacore, m_jniChoosePhoto );
}

void AdMob__init()
{
}

void AdMob__configure__String_Logical_Color_Color_Color()
{
}

void AdMob__bounds__Vector2_Vector2_Radians()
{
}

void AdMob__visible__Logical()
{
}
*/

void Application__log__String()
{
  SlagString* mesg = (SlagString*) SLAG_POP_REF();  // ignore title string 
  SLAG_POP_REF();  // discard singleton
  LOG( mesg );
}

void Application__title__String()
{
  // Application::title(String) 
  SlagString* mesg = (SlagString*) SLAG_POP_REF();  // ignore title string 
  SLAG_POP_REF();  // discard singleton

  if ( !mesg ) return;
}

void encode_bitmap( int to_type, double quality )
{
  // to_type: 1=png, 2=jpg
  // quality: 0.0 to 1.0
  SlagBitmap* bitmap_obj = (SlagBitmap*) SLAG_POP_REF();
  int w = bitmap_obj->width;
  int h = bitmap_obj->height;

  jintArray src_obj = jvm->NewIntArray( w*h );
  JavaIntArray src_wrapper( src_obj );

  // unmultiply the color components by the alpha and swap red with blue
  SlagInt32* src = ((SlagInt32*) bitmap_obj->pixels->data) - 1;
  jint* dest = ((jint*) src_wrapper.data) - 1;
  int count = w*h + 1;
  while (--count)
  {
    SlagInt32 color = *(++src);
    int a = (color >> 24) & 255;
    int r = (color >> 16) & 255;
    int g = (color >> 8) & 255;
    int b = color & 255;

    if (a)
    {
      r = (r * 255) / a;
      g = (g * 255) / a;
      b = (b * 255) / a;
    }

    *(++dest) = (a<<24) | (b<<16) | (g<<8) | r;
  }

  src_wrapper.release();

  jbyteArray png_bytes = (jbyteArray) jvm->CallObjectMethod( android_core_obj, 
      m_jniEncodeBitmapData , w, h, src_obj, to_type, (int)(quality*100) );

  jvm->DeleteLocalRef( src_obj );

  if (png_bytes)
  {
    JavaByteArray encoded_array( png_bytes );
    count = encoded_array.count;

    SlagArrayList* list = slag_create_byte_list( encoded_array.data, count );
    SLAG_PUSH_REF( list );
  }
  else
  {
    SLAG_PUSH_REF(NULL);
  }
}

void Bitmap__to_png_bytes()
{
  encode_bitmap(1,1.0);
}

void Bitmap__to_jpg_bytes__Real64()
{
  double quality = SLAG_POP_REAL64();
  encode_bitmap(2,quality);
}

void Display__fullscreen()
{
  // Application::fullscreen().Logical 
  SLAG_POP_REF();
  SLAG_PUSH_INT32( 1 );
}

void Display__fullscreen__Logical()
{
  // Application::fullscreen(Logical) 
  SLAG_POP_INT32();  // ignore fullscreen setting 
  SLAG_POP_REF();

  // no action
}

void Input__keyboard_visible__Logical()
{
  SlagInt32 setting = SLAG_POP_INT32();
  SLAG_POP_REF();  // discard singleton
	
  if (setting)
  {
    jvm->CallVoidMethod( android_core_obj, m_jniShowKeyboard, true );
  }
  else
  {
    jvm->CallVoidMethod( android_core_obj, m_jniShowKeyboard, false );
  }
}

void Input__keyboard_visible()
{
  jboolean result = jvm->CallBooleanMethod( android_core_obj, m_jniKeyboardVisible );
  SLAG_PUSH_INT32( result ? 1 : 0 );
}

void Input__mouse_visible__Logical()
{
  SLAG_POP_INT32();
  SLAG_POP_REF();      // discard singleton

  // no action
}

void Input__input_capture__Logical()
{
  SLAG_POP_INT32(); // ignore setting
  SLAG_POP_REF();      // discard singleton
}


/*
void Email__available()
{
  SLAG_POP_REF();
  SLAG_PUSH_INT32( 1 );
}
*/

struct AndroidSoundInfo : SlagResource
{
  int id;

  AndroidSoundInfo( int id ) : id(id)
  {
  }
  
  ~AndroidSoundInfo()
  {
    jvm->CallVoidMethod( android_core_obj, m_jniSoundRelease, id );
    id = 0;
  }
};


void NativeSound__init__ArrayList_of_Byte()
{
  // NativeSound::init(Byte[])

  SlagArrayList* list = (SlagArrayList*) SLAG_POP_REF();
  SlagObject* sound_obj = SLAG_POP_REF();

  // no action - can't make a sound from a byte list
}

void NativeSound__init__String()
{
  SlagString* filename = (SlagString*) SLAG_POP_REF();
  SlagObject* sound_obj = SLAG_POP_REF();
	
  int count = filename->count;
	char* buffer = new char[count+1];
	filename->to_ascii( buffer, count+1 );
		
	jobject filename_obj = jvm->NewStringUTF(buffer);

  jint sound_id = jvm->CallIntMethod( android_core_obj, m_jniSoundLoad, filename_obj );
  jvm->DeleteLocalRef( filename_obj );
	
  if (sound_id > 0)
  {
    SlagLocalRef gc_guard( sound_obj );
    AndroidSoundInfo* info = new AndroidSoundInfo(sound_id);
    SlagNativeData* data_obj = SlagNativeData::create( info, SlagNativeDataDeleteResource );
    SLAG_SET_REF(sound_obj,"native_data",data_obj);
  }
  else
  {
    LOG("Sound file not found\n");
    slag_throw_file_not_found_error( buffer );
  }
	delete buffer;
}

int get_sound_id( SlagObject* sound_obj )
{
  if ( !sound_obj ) return 0;
  SLAG_GET( SlagNativeData*, data_obj, sound_obj, "native_data" );
  if (data_obj == NULL) return 0;
  return ((AndroidSoundInfo*)data_obj->data)->id;
}


void NativeSound__create_duplicate()
{
  // NativeSound::create_duplicate().Sound 
	SlagObject* sound_obj = SLAG_POP_REF();

  int sound_id = get_sound_id(sound_obj);
	
	jint new_sound_id = jvm->CallIntMethod( android_core_obj, m_jniSoundDuplicate, sound_id );
	
  if (new_sound_id != 0)
	{
		SLAG_FIND_TYPE( type_sound, "NativeSound" );
		SlagObject* new_sound_obj = type_sound->create();
    SLAG_PUSH_REF( new_sound_obj );

    AndroidSoundInfo* info = new AndroidSoundInfo(new_sound_id);
    SlagNativeData* data_obj = SlagNativeData::create( info, SlagNativeDataDeleteResource );
    SLAG_SET_REF(new_sound_obj,"native_data",data_obj);
	}
	else 
	{
		SLAG_PUSH_REF( NULL );
	}
}

void NativeSound__play()
{
	SlagObject* sound_obj = SLAG_POP_REF();
	jvm->CallVoidMethod(android_core_obj, m_jniSoundPlay, get_sound_id(sound_obj));
}

void NativeSound__pause()
{
  // NativeSound::pause() 
	SlagObject* sound_obj = SLAG_POP_REF();
	jvm->CallVoidMethod(android_core_obj, m_jniSoundPause, get_sound_id(sound_obj));
}

void NativeSound__is_playing()
{
  // NativeSound::is_playing().Logical 
	SlagObject* sound_obj = SLAG_POP_REF();
	jboolean playing = jvm->CallBooleanMethod(android_core_obj, m_jniSoundIsPlaying, get_sound_id(sound_obj));

  if (playing)
  {
    SLAG_PUSH_INT32(1);
  }
  else
  {
    SLAG_PUSH_INT32(0);
  }
}

void NativeSound__volume__Real64()
{
  // NativeSound::volume(Real64) 
	double volume = SLAG_POP_REAL64();
	SlagObject* sound_obj = SLAG_POP_REF();
	jvm->CallVoidMethod(android_core_obj, m_jniSoundSetVolume, get_sound_id(sound_obj), volume);
}

void NativeSound__pan__Real64()
{
  // NativeSound::pan(Real64) 

  SLAG_POP_REAL64();
  SLAG_POP_REF(); // sound object 

  // no action
}

void NativeSound__pitch__Real64()
{
  // NativeSound::pitch(Real64) 

  SLAG_POP_REAL64();
  SLAG_POP_REF(); // sound object 

  // no action
}

void NativeSound__repeats__Logical()
{
  // NativeSound::repeats(Logical) 
  int setting = SLAG_POP_INT32();
	SlagObject* sound_obj = SLAG_POP_REF();
	jvm->CallVoidMethod(android_core_obj, m_jniSoundSetRepeats, get_sound_id(sound_obj), setting);
}

void NativeSound__current_time()
{
  // NativeSound::current_time().Real64 
	SlagObject* sound_obj = SLAG_POP_REF();
	jdouble curtime = jvm->CallDoubleMethod( android_core_obj, m_jniSoundGetCurrentTime, get_sound_id(sound_obj) );
  SLAG_PUSH_REAL64( curtime );
}

void NativeSound__current_time__Real64()
{
  // NativeSound::current_time(Real64) 
	SlagReal64 new_time = SLAG_POP_REAL64();
	SlagObject* sound_obj = SLAG_POP_REF();
	jvm->CallVoidMethod(android_core_obj, m_jniSoundSetCurrentTime, get_sound_id(sound_obj), new_time);
}

void NativeSound__duration()
{
  // NativeSound::duration().Real64 
	SlagObject* sound_obj = SLAG_POP_REF();
	jdouble duration = jvm->CallDoubleMethod( android_core_obj, m_jniSoundDuration, get_sound_id(sound_obj) );
  SLAG_PUSH_REAL64( duration );
}


GLTexture* NativeLayer_get_native_texture_data( SlagObject* texture_obj );
// defined in gl_core.cpp


void OffscreenBuffer__clear__Color()
{
  // OffscreenBuffer::clear(Color)
  draw_buffer.render();

  SlagInt32 color = SLAG_POP_INT32();
  SlagObject* buffer_obj = SLAG_POP_REF();

  SVM_NULL_CHECK( buffer_obj, return );

  SLAG_GET_REF( texture_obj, buffer_obj, "texture" );
  SVM_NULL_CHECK( texture_obj, return );

  GLTexture* texture = NativeLayer_get_native_texture_data( texture_obj );
  if ( !texture || !texture->frame_buffer ) return;

  glBindFramebufferOES( GL_FRAMEBUFFER_OES, texture->frame_buffer );
  glDisable( GL_SCISSOR_TEST );
  glClearColor( ((color>>16)&255)/255.0f,
      ((color>>8)&255)/255.0f,
      ((color)&255)/255.0f,
      ((color>>24)&255)/255.0f );
  glClear(GL_COLOR_BUFFER_BIT);
  if (use_scissor) glEnable( GL_SCISSOR_TEST );

  if (draw_buffer.draw_target)
  {
    glBindFramebufferOES( GL_FRAMEBUFFER_OES, draw_buffer.draw_target->frame_buffer );
  }
  else
  {
    glBindFramebufferOES( GL_FRAMEBUFFER_OES, 0 );
  }
}

void ResourceManager__load_gamestate__String()
{
	SlagString* filename = (SlagString*) SLAG_POP_REF();
  SLAG_POP_REF();   // discard singleton

	int count = filename->count;
	char* filename_buffer = new char[count+1];
	filename->to_ascii( filename_buffer, count+1 );
	
	jobject filename_obj = jvm->NewStringUTF(filename_buffer);
	jbyteArray array_obj = (jbyteArray) jvm->CallObjectMethod( android_core_obj, m_jniLoadGamestate, filename_obj );
  jvm->DeleteLocalRef( filename_obj );
	
  if (array_obj)
  {
    delete filename_buffer;

		JavaByteArray wrapper = JavaByteArray(array_obj);
		count = wrapper.count;
		
    SlagString* result = SlagString::create( count );
    SLAG_PUSH_REF( result );

    SlagChar* data = (SlagChar*) result->characters;
    --data;
    ++count;
    while (--count) *(++data) = (SlagChar) wrapper.data[wrapper.count - count];

    result->set_hash_code();
  }
  else
  {
    slag_throw_file_not_found_error( filename_buffer );
    delete filename_buffer;
    return;
  }
}

void ResourceManager__save_gamestate__String_String()
{
  // ResourceManager::save_gamestate(String,String)
  SlagString* content = (SlagString*) SLAG_POP_REF();
  SlagString* filename = (SlagString*) SLAG_POP_REF();
  SLAG_POP_REF();   // discard singleton

	char filename_buffer[128];
	filename->to_ascii( filename_buffer, 128 );
	
	jobject filename_obj = jvm->NewStringUTF(filename_buffer);
	
	int count = content->count;
	jobject content_obj = jvm->NewString((jchar*)(content->characters),count);
	
  jboolean success = jvm->CallBooleanMethod( android_core_obj, m_jniSaveGamestate, filename_obj, content_obj );
  jvm->DeleteLocalRef( filename_obj );
	jvm->DeleteLocalRef( content_obj );
	
  if (!success)
  {
    slag_throw_file_error( filename_buffer );
  }
}

void ResourceManager__delete_gamestate__String()
{
  SlagString* filename = (SlagString*) SLAG_POP_REF();
  SLAG_POP_REF();   // discard singleton

	char filename_buffer[128];
	filename->to_ascii( filename_buffer, 128 );
	
	jobject filename_obj = jvm->NewStringUTF(filename_buffer);
	
  jboolean success = jvm->CallBooleanMethod( android_core_obj, m_jniDeleteGamestate, filename_obj );
  jvm->DeleteLocalRef( filename_obj );
}

void System__device_id()
{
  SLAG_POP_REF();  // discard singleton
  jstring id_obj = (jstring) jvm->CallObjectMethod( android_core_obj, m_jniGetDeviceID );

  int count = jvm->GetStringLength(id_obj);
  const jchar* id_data = jvm->GetStringChars( id_obj, NULL );

  SlagString* result = SlagString::create( count );
  memcpy( result->characters, id_data, count*2 );
  SLAG_PUSH_REF( (SlagObject*) result );  // leave result on stack
  result->set_hash_code();

  jvm->ReleaseStringChars( id_obj, id_data );
}

void System__open_url__String()
{
  jstring url = to_jstring( SLAG_POP_REF() );
  SLAG_POP_REF();  // discard singleton

  jvm->CallVoidMethod( android_core_obj, m_jniOpenURL, url );
  jvm->DeleteLocalRef( url );
}

void System__country_name()
{
  SLAG_POP_REF();  // discard singleton
  jstring id_obj = (jstring) jvm->CallObjectMethod( android_core_obj, m_jniGetCountryName );

  int count = jvm->GetStringLength(id_obj);
  const jchar* id_data = jvm->GetStringChars( id_obj, NULL );

  SlagString* result = SlagString::create( count );
  memcpy( result->characters, id_data, count*2 );
  SLAG_PUSH_REF( (SlagObject*) result );  // leave result on stack
  result->set_hash_code();

  jvm->ReleaseStringChars( id_obj, id_data );
}

void System__exit_program__Int32_String()
{
  SlagString* error_mesg = (SlagString*) SLAG_POP_REF();
  if (error_mesg) 
  {
    LOG( "ERROR - EXITING PROGRAM" );
    LOG( error_mesg );
  }

  SLAG_POP_INT32();  // ignore error code
  SLAG_POP_REF();    // singleton

  jvm->CallVoidMethod( android_core_obj, m_jniExitProgram );
}

void Texture_init( SlagInt32* data, int w, int h, int format );

jmp_buf plasmacore_png_jmp_buffer;
int     plasmacore_png_width;
int     plasmacore_png_height;
int*    plasmacore_png_buffer = NULL;
int     plasmacore_png_buffer_size = 0;
int     plasmacore_png_texture_format = 0;

static void plasmacore_png_error_handler( png_structp png_ptr, png_const_charp msg )
{
  fprintf(stderr, "libpng error: %s\n", msg);
  fflush(stderr);

  longjmp( plasmacore_png_jmp_buffer, 1 );
}

static void plasmacore_png_info_callback( png_structp png_ptr, png_infop info_ptr )
{
  int         color_type, bit_depth;
  png_uint_32 width, height;
  double      gamma;

  png_get_IHDR( png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
      NULL, NULL, NULL );
  plasmacore_png_width  = width;
  plasmacore_png_height = height;

  Texture_init( NULL, width, height, plasmacore_png_texture_format );

  if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_expand( png_ptr );
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand( png_ptr );
  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_expand( png_ptr );
  if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
  {
    png_set_gray_to_rgb( png_ptr );
  }

  png_read_update_info( png_ptr, info_ptr );
}

static void plasmacore_png_row_callback(png_structp png_ptr, png_bytep new_row,
                                  png_uint_32 row_num, int pass)
{
    if ( !new_row ) return;

    if (plasmacore_png_buffer && plasmacore_png_buffer_size < plasmacore_png_width)
    {
      delete plasmacore_png_buffer;
      plasmacore_png_buffer = NULL;
    }

    if ( !plasmacore_png_buffer )
    {
      plasmacore_png_buffer = new int[ plasmacore_png_width ];
      plasmacore_png_buffer_size = plasmacore_png_width;
    }

    // The appropriate GL texture was prepped just before this PNG decoding
    if (plasmacore_png_texture_format == 1)
    {
      // premultiply the alpha
      int count = plasmacore_png_width + 1;
      unsigned char* data = new_row;
      while (--count)
      {
        int a = data[3];
        data[0] = (data[0] * a) / 255;
        data[1] = (data[1] * a) / 255;
        data[2] = (data[2] * a) / 255;
        data += 4;
      }
      glTexSubImage2D( GL_TEXTURE_2D, 0, 0, row_num, plasmacore_png_width, 1, GL_RGBA, GL_UNSIGNED_BYTE, new_row );
    }
    else
    {
      // premultiply the alpha and convert it to 16-bit
      int count = plasmacore_png_width + 1;
      unsigned char* src = new_row;
      unsigned short* dest = ((unsigned short*) new_row) - 1;
      while (--count)
      {
        int a = src[3];
        int r = ((src[0] * a) / 255) >> 4;
        int g = ((src[1] * a) / 255) >> 4;
        int b = ((src[2] * a) / 255) >> 4;
        a >>= 4;
        *(++dest) = (unsigned short) ((r<<12) | (g<<8) | (b<<4) | a);
        src  += 4;
      }
      glTexSubImage2D( GL_TEXTURE_2D, 0, 0, row_num, plasmacore_png_width, 1, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, new_row );
    }
//printf( "PNG read row #%d (%02x,%02x,%02x,%02x)\n",row_num,new_row[0],new_row[1],new_row[2],new_row[3]);

    //png_progressive_combine_row(png_ptr, mainprog_ptr->row_pointers[row_num],
      //new_row);
}

bool plasmacore_decode_png( char* data, int data_size, int* width_ptr, int* height_ptr )
{
  png_structp  png_ptr;
  png_infop    info_ptr;

  png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL,
      plasmacore_png_error_handler, NULL );
  if ( !png_ptr ) return false; // Out of memory

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    png_destroy_read_struct( &png_ptr, NULL, NULL );
    return false;  // Out of memory
  }

  if (setjmp(plasmacore_png_jmp_buffer))
  {
    png_destroy_read_struct( &png_ptr, &info_ptr, NULL );
    return false;
  }

#ifdef PNG_HANDLE_AS_UNKNOWN_SUPPORTED
  // Prepare the reader to ignore all recognized chunks whose data won't be
  // used, i.e., all chunks recognized by libpng except for IHDR, PLTE, IDAT,
  // IEND, tRNS, bKGD, gAMA, and sRGB (small performance improvement).
  {
    static png_byte chunks_to_ignore[] = 
    {
       99,  72,  82,  77, '\0',  // cHRM
      104,  73,  83,  84, '\0',  // hIST
      105,  67,  67,  80, '\0',  // iCCP
      105,  84,  88, 116, '\0',  // iTXt
      111,  70,  70, 115, '\0',  // oFFs
      112,  67,  65,  76, '\0',  // pCAL
      112,  72,  89, 115, '\0',  // pHYs
      115,  66,  73,  84, '\0',  // sBIT
      115,  67,  65,  76, '\0',  // sCAL
      115,  80,  76,  84, '\0',  // sPLT
      115,  84,  69,  82, '\0',  // sTER
      116,  69,  88, 116, '\0',  // tEXt
      116,  73,  77,  69, '\0',  // tIME
      122,  84,  88, 116, '\0'   // zTXt
    };

    png_set_keep_unknown_chunks( png_ptr, PNG_HANDLE_CHUNK_NEVER,
        chunks_to_ignore, sizeof(chunks_to_ignore)/5 );
  }
#endif // PNG_HANDLE_AS_UNKNOWN_SUPPORTED

  png_set_progressive_read_fn( png_ptr, NULL,
      plasmacore_png_info_callback, plasmacore_png_row_callback, NULL);

  png_process_data( png_ptr, info_ptr, (unsigned char*) data, (unsigned long) data_size );
  png_destroy_read_struct( &png_ptr, &info_ptr, NULL );

  if (plasmacore_png_buffer)
  {
    delete plasmacore_png_buffer;
    plasmacore_png_buffer = NULL;
  }

  *width_ptr = plasmacore_png_width;
  *height_ptr = plasmacore_png_height;

  return true;
}

void Texture__init__String_Int32()
{
  plasmacore_png_texture_format = SLAG_POP_INT32();
  SlagString* filename_obj = (SlagString*) SLAG_POP_REF();
  // Leave Texture obj on stack.

  int data_size;
  char* data = image_archive.load( filename_obj, &data_size );

  if ( !data ) 
  {
    SLAG_POP_REF();  // Texture obj
    char buffer[256];
    filename_obj->to_ascii( buffer, 256 );
    slag_throw_file_not_found_error( buffer );
    return;
  }

  int width=0, height=0;
  if (plasmacore_decode_png( data, data_size, &width, &height ))
  {
/*
    SlagInt32* pixels = new SlagInt32[ width*height ];

    // premultiply the alpha
    SlagInt32* dest = pixels - 1;
    for (int j=0; j<height; ++j)
    {
      SlagInt32* cur = ((SlagInt32*) img->tpixels[j]) - 1;
      int count = width + 1;
      while (--count)
      {
        SlagInt32 color = *(++cur);
        int a = gd_alpha_to_alpha_map[(color >> 24) & 255];
        int r = (color >> 16) & 255;
        int g = (color >> 8) & 255;
        int b = color & 255;

        r = (r * a) / 255;
        g = (g * a) / 255;
        b = (b * a) / 255;

        *(++dest) = (a<<24) | (r<<16) | (g<<8) | b;
      }
    }

    gdImageDestroy( img );
    Texture_init( pixels, width, height, plasmacore_png_texture_format );
    delete pixels;
*/
  }
  else
  {
    SLAG_POP_REF();
  }

  delete data;
}

void Texture__draw_tile__Corners_Vector2_Vector2_Int32()
{
  SlagInt32 render_flags = SLAG_POP_INT32();
  Vector2 size = SLAG_POP(Vector2);
  Vector2 pos  = SLAG_POP(Vector2);
  Vector2 uv_a = SLAG_POP(Vector2);
  Vector2 uv_b = SLAG_POP(Vector2);
  SlagObject* texture_obj = SLAG_POP_REF();

  GLTexture* texture = NativeLayer_get_native_texture_data(texture_obj);
  if ( !texture ) return;

  double texture_width  = texture->texture_width;
  double texture_height = texture->texture_height;

  pos.x = pos.x * plasmacore.scale_factor + plasmacore.border_x;
  pos.y = pos.y * plasmacore.scale_factor + plasmacore.border_y;
  size.x *= plasmacore.scale_factor;
  size.y *= plasmacore.scale_factor;

  GLint src_rect[4];

  if (size.x < 0)
  {
    // hflip
    size.x = -size.x;
    src_rect[0] = (GLint) (uv_b.x * texture_width);   // right
    src_rect[2] = (GLint) -((uv_b.x - uv_a.x) * texture_width);   // width
  }
  else
  {
    src_rect[0] = (GLint) (uv_a.x * texture_width);   // left
    src_rect[2] = (GLint) ((uv_b.x - uv_a.x) * texture_width);   // width
  }

  if (size.y < 0)
  {
    // vflip
    size.y = -size.y;
    src_rect[1] = (GLint) (uv_a.y * texture_height);  // top
    src_rect[3] = (GLint) ((uv_b.y - uv_a.y) * texture_height); // height
  }
  else
  {
    src_rect[1] = (GLint) (uv_b.y * texture_height);  // bottom
    src_rect[3] = (GLint) -((uv_b.y - uv_a.y) * texture_height); // height
  }


  draw_buffer.render();

  if (render_flags & RENDER_FLAG_TEXTURE_WRAP)
  {
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  }
  else
  {
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  }

  if (render_flags & RENDER_FLAG_POINT_FILTER)
  {
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
  else
  {
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  glClientActiveTexture(GL_TEXTURE1);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glClientActiveTexture(GL_TEXTURE0);

  glClientActiveTexture(GL_TEXTURE0);
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, texture->id );
  glEnable(GL_TEXTURE_2D);

  glTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, src_rect );

  double y_pos = (plasmacore.display_height - size.y) - pos.y;
  glDrawTexfOES( (GLfloat) pos.x, (GLfloat)y_pos, 0.0f, (GLfloat) size.x, (GLfloat) size.y );
  glDisable(GL_TEXTURE_2D);
}

//=============================================================================
//  File I/O
//=============================================================================

void slag_adjust_filename_for_os( char* filename, int len )
{
  // no action
}

int slag_is_directory( const char* filename )
{
  jobject filename_obj = jvm->NewStringUTF(filename);
  jboolean result = jvm->CallBooleanMethod( android_core_obj, m_jniIsDirectory, filename_obj );
  jvm->DeleteLocalRef( filename_obj );

  return result ? 1 : 0;
}

int slag_file_exists( const char* filename )
{
  jobject filename_obj = jvm->NewStringUTF(filename);
  jboolean result = jvm->CallBooleanMethod( android_core_obj, m_jniFileExists, filename_obj );
  jvm->DeleteLocalRef( filename_obj );

  return result ? 1 : 0;
}

bool slag_file_helper_retrieve_filepath( char* buffer, int count )
{
  SlagObject* file_obj = SLAG_POP_REF();
  SVM_NULL_CHECK( file_obj, return false );
  SlagString* filepath_obj = ((SlagFile*)file_obj)->property_filepath;
  filepath_obj->to_ascii( buffer, count );
  return true;
}


void File__is_directory()
{
  char filename[512];

  slag_file_helper_retrieve_filepath( filename, 512 );
  SLAG_PUSH_INT32( slag_is_directory(filename) );
}

void File__exists()
{
  char filename[512];

  slag_file_helper_retrieve_filepath( filename, 512 );
  SLAG_PUSH_INT32( slag_file_exists(filename) );
}

void File__directory_listing__ArrayList_of_String()
{
  SlagObject* list = SLAG_POP_REF();
  SlagLocalRef gc_guard(list);

  SVM_NULL_CHECK( list, return );

  char filename[PATH_MAX];
  SlagObject**    list_ptr;

  slag_file_helper_retrieve_filepath( filename, PATH_MAX );

  jobject filename_obj = jvm->NewStringUTF(filename);
  jobjectArray array = (jobjectArray) jvm->CallObjectMethod( android_core_obj, 
        m_jniDirectoryListing, filename_obj );
  jvm->DeleteLocalRef( filename_obj );

  if ( !array )
  {
    slag_throw_file_not_found_error( filename );
    return;
  }

  JavaStringArray filenames( array );

  for (int i=0; i<filenames.count; ++i)
  {
    SLAG_PUSH_REF( list );
    SLAG_PUSH_REF( (SlagObject*) filenames[i] );
    SLAG_CALL( list->type, "add(String)" );
    SLAG_POP_REF();
  }
}

void File__absolute_filepath()
{
  char filepath[PATH_MAX];

  slag_file_helper_retrieve_filepath( filepath, PATH_MAX );

  jobject filepath_obj = jvm->NewStringUTF(filepath);
  jstring jabspath = (jstring) jvm->CallObjectMethod( android_core_obj, m_jniAbsoluteFilePath,
    filepath_obj );
  jvm->DeleteLocalRef( filepath_obj );

  const char* jabspath_utf8 = jvm->GetStringUTFChars( jabspath, NULL );
  strcpy( filepath, jabspath_utf8 );
  jvm->ReleaseStringUTFChars( jabspath, jabspath_utf8 );

  SLAG_PUSH_REF( SlagString::create(filepath) );
}

void File__copy__String()
{
  // File::copy(String)
  SlagString* new_name_obj = (SlagString*) SLAG_POP_REF();
  SVM_NULL_CHECK( new_name_obj, return );

  char new_name[PATH_MAX];
  new_name_obj->to_ascii( new_name, 512 );

  char filepath[512];
  if ( !slag_file_helper_retrieve_filepath( filepath, PATH_MAX ) ) return;

  jobject j_filepath_obj = jvm->NewStringUTF(filepath);
  jobject j_new_name_obj = jvm->NewStringUTF(filepath);

  jvm->CallVoidMethod( android_core_obj, m_jniFileCopy, j_filepath_obj, j_new_name_obj );

  jvm->DeleteLocalRef( j_new_name_obj );
  jvm->DeleteLocalRef( j_filepath_obj );
}


void File__rename__String()
{
  // File::rename(String)
  SlagString* new_name_obj = (SlagString*) SLAG_POP_REF();
  SVM_NULL_CHECK( new_name_obj, return );

  char new_name[PATH_MAX];
  new_name_obj->to_ascii( new_name, 512 );

  char filepath[512];
  if ( !slag_file_helper_retrieve_filepath( filepath, PATH_MAX ) ) return;

  jobject j_filepath_obj = jvm->NewStringUTF(filepath);
  jobject j_new_name_obj = jvm->NewStringUTF(filepath);

  jvm->CallVoidMethod( android_core_obj, m_jniFileRename, j_filepath_obj, j_new_name_obj );

  jvm->DeleteLocalRef( j_new_name_obj );
  jvm->DeleteLocalRef( j_filepath_obj );
}

void File__delete()
{
  // File::delete()
  char filepath[PATH_MAX];
  if ( !slag_file_helper_retrieve_filepath( filepath, PATH_MAX ) ) return;

  jobject filepath_obj = jvm->NewStringUTF(filepath);
  jvm->CallVoidMethod( android_core_obj, m_jniFileDelete, filepath_obj );
  jvm->DeleteLocalRef( filepath_obj );
}

void File__timestamp()
{
  // File::timestamp().Int64
  char filepath[PATH_MAX];
  if ( !slag_file_helper_retrieve_filepath( filepath, PATH_MAX ) ) return;

  jobject filepath_obj = jvm->NewStringUTF(filepath);
  jlong timestamp = jvm->CallLongMethod( android_core_obj, m_jniFileTimestamp, filepath_obj );
  jvm->DeleteLocalRef( filepath_obj );
  SLAG_PUSH_INT64( timestamp );
}

void File__touch()
{
  // File::touch()
  char filepath[PATH_MAX];
  if ( !slag_file_helper_retrieve_filepath( filepath, PATH_MAX ) ) return;

  jobject filepath_obj = jvm->NewStringUTF(filepath);
  jvm->CallVoidMethod( android_core_obj, m_jniFileTouch, filepath_obj );
  jvm->DeleteLocalRef( filepath_obj );
}

void File__native_mkdir()
{
  char filepath[PATH_MAX];
  if ( !slag_file_helper_retrieve_filepath( filepath, PATH_MAX ) ) return;

  jobject filepath_obj = jvm->NewStringUTF(filepath);
  jvm->CallVoidMethod( android_core_obj, m_jniFileMkdir, filepath_obj );
  jvm->DeleteLocalRef( filepath_obj );
}

void File__change_dir()
{
  SLAG_POP_REF();
  // no action
}

//--------------------------------------------------------------------
//  FileReader
//--------------------------------------------------------------------
#define ANDROID_FILE_BUFFER_SIZE 2048
struct AndroidFileInfo : SlagResource
{
  int     fp;
  int     total_size; // in whole file
  int     num_read;   // total
  int     pos;        // within buffer
  bool    is_reader;
  bool    finished;
  unsigned char buffer[ANDROID_FILE_BUFFER_SIZE];

  ~AndroidFileInfo()
  {
    close();
  }

  bool open_infile( const char* filename )
  {
    jobject j_filename_obj = jvm->NewStringUTF(filename);
    fp = jvm->CallIntMethod( android_core_obj, m_jniFileReaderOpen, j_filename_obj );
    jvm->DeleteLocalRef( j_filename_obj );

    if ( !fp ) return false;

    total_size = jvm->CallIntMethod( android_core_obj, m_jniFileReaderAvailable, fp );
    num_read = 0;
    pos = 0;
    is_reader = true;
    finished = false;

    if (total_size == 0) 
    {
      close();
      return true;  // success but zero-length file
    }

    fill_buffer();

    if (finished) return false;
    return true;
  }

  bool open_outfile( const char* filename, jboolean append )
  {
    jobject j_filename_obj = jvm->NewStringUTF(filename);
    fp = jvm->CallIntMethod( android_core_obj, m_jniFileWriterOpen, j_filename_obj, append );
    jvm->DeleteLocalRef( j_filename_obj );

    if ( !fp ) return false;

    total_size = 0;  // note: inaccurate for append mode but no easy fix
    pos = 0;
    is_reader = false;
    finished = false;

    return true;
  }

  void close()
  {
    if (fp)
    {
      if (is_reader)
      {
        jvm->CallVoidMethod( android_core_obj, m_jniFileReaderClose, fp );
      }
      else
      {
        flush();
        jvm->CallVoidMethod( android_core_obj, m_jniFileWriterClose, fp );
      }
      finished = true;
      fp = 0;
    }
  }

  void fill_buffer()
  {
    jbyteArray bytes = (jbyteArray) jvm->CallObjectMethod( android_core_obj,
        m_jniFileReaderReadBytes, fp, ANDROID_FILE_BUFFER_SIZE );
    if (bytes == NULL)
    {
      finished = true; // error
      slag_throw_file_error();
      return;
    }

    JavaByteArray byte_data(bytes);
    memcpy( buffer, byte_data.data, ANDROID_FILE_BUFFER_SIZE );
    pos = 0;
  }

  int peek()
  {
    return buffer[pos];
  }

  int read()
  {
    int result = buffer[pos++];
    if (++num_read == total_size)
    {
      finished = true;
      jvm->CallVoidMethod( android_core_obj, m_jniFileReaderClose, fp );
    }
    else if (pos == ANDROID_FILE_BUFFER_SIZE)
    {
      fill_buffer();
    }
    return result;
  }

  int remaining() { return total_size - num_read; }

  void skip( int count )
  {
    if (count > remaining()) count = remaining();
    if (count <= 0) return;

    while (count > 0)
    {
      int num_buffered = ANDROID_FILE_BUFFER_SIZE - pos;
      if (count >= num_buffered)
      {
        num_read += num_buffered;
        count -= num_buffered;
        fill_buffer();
      }
      else
      {
        num_read += count;
        pos += count;
        count = 0;
      }
    }
  }

  int read_bytes( void* dest_ptr, int count )
  {
    unsigned char* dest = (unsigned char*) dest_ptr;
    if (count > remaining()) count = remaining();

    int num_read_before = num_read;

    while (count > 0)
    {
      int num_buffered = ANDROID_FILE_BUFFER_SIZE - pos;
      if (count >= num_buffered)
      {
        memcpy( dest, buffer+pos, num_buffered );
        dest += num_buffered;
        num_read += num_buffered;
        count -= num_buffered;
        fill_buffer();
      }
      else
      {
        memcpy( dest, buffer+pos, count );
        dest += count;
        num_read += count;
        pos += count;
        count = 0;
      }
    }

    if (num_read == total_size) close();

    return num_read - num_read_before;
  }

  int read_chars( void* dest_ptr, int count )
  {
    SlagChar* dest = (SlagChar*) dest_ptr;
    if (count > remaining()) count = remaining();

    int num_read_before = num_read;

    while (count > 0)
    {
      int num_buffered = ANDROID_FILE_BUFFER_SIZE - pos;
      if (count >= num_buffered)
      {
        num_read += num_buffered;
        count -= num_buffered;

        ++num_buffered;
        --dest;
        unsigned char* src = (buffer+pos) - 1;
        while (--num_buffered)
        {
          *(++dest) = *(++src);
        }

        fill_buffer();
      }
      else
      {
        num_read += count;

        ++count;
        --dest;
        unsigned char* src = (buffer+pos) - 1;
        while (--count)
        {
          *(++dest) = *(++src);
        }
      }
    }

    if (num_read == total_size) close();

    return num_read - num_read_before;
  }

  void write( int ch )
  {
    ++total_size;
    buffer[pos] = (unsigned char) ch;
    if (++pos == ANDROID_FILE_BUFFER_SIZE)
    {
      flush();
    }
  }

  void write_bytes( void* data_ptr, int count )
  {
    char* data = (char*) data_ptr;

    if (pos + count >= ANDROID_FILE_BUFFER_SIZE)
    {
      flush();
    }

    while (count >= ANDROID_FILE_BUFFER_SIZE)
    {
      memcpy( buffer+pos, data, ANDROID_FILE_BUFFER_SIZE );
      count -= ANDROID_FILE_BUFFER_SIZE;
      data  += ANDROID_FILE_BUFFER_SIZE;
      pos   += ANDROID_FILE_BUFFER_SIZE;
      total_size += ANDROID_FILE_BUFFER_SIZE;
      flush();
    }

    if (count > 0)
    {
      memcpy( buffer+pos, data, count );
      pos        += count;
      total_size += count;
      flush();
    }
  }

  void write_chars( void* data_ptr, int count )
  {
    SlagChar* data = (SlagChar*) data_ptr;

    ++count;
    --data;
    while (--count) write( *(++data) );
  }

  void flush()
  {
    jbyteArray jbytes = (jbyteArray) jvm->CallObjectMethod( android_core_obj, m_jniGetIOBuffer, 
        ANDROID_FILE_BUFFER_SIZE );

    JavaByteArray src_wrapper( jbytes );
    memcpy( src_wrapper.data, buffer, pos );
    src_wrapper.release();

    jvm->CallVoidMethod( android_core_obj, m_jniFileWriterWriteBytes, fp, jbytes, pos );
    pos = 0;
  }
};



void FileReader__init__String()
{
  // method init( String filename )
  SlagString* filename_string = (SlagString*) SLAG_POP_REF();
  SlagObject* reader          = SLAG_POP_REF();
  FILE*       fp;
  char filename[512];

  SVM_NULL_CHECK( filename_string && reader, return );

  filename_string->to_ascii( filename, 512 );
  slag_adjust_filename_for_os( filename, 512 );

  if (slag_is_directory(filename))
  {
    slag_throw_file_error( filename );
    return;
  }

  SlagLocalRef gc_guard(reader);
  AndroidFileInfo* info = new AndroidFileInfo();
  if ( !info->open_infile(filename) )
  {
    // no luck
    slag_throw_file_not_found_error( filename );
    return;
  }

  if (info->finished) return;  // zero-length file

  SlagNativeData* data_obj = SlagNativeData::create( info, SlagNativeDataDeleteResource );
  SLAG_SET_REF(reader,"native_data",data_obj);
}

static AndroidFileInfo* get_reader_file_info( SlagObject* reader )
{
  SLAG_GET( SlagNativeData*, native_data, reader, "native_data" );
  if ( !native_data ) return NULL;

  return (AndroidFileInfo*) native_data->data;
}

static void close_reader( SlagObject* reader )
{
  SLAG_GET( SlagNativeData*, native_data, reader, "native_data" );
  if ( !native_data ) return;
  SLAG_SET_REF( reader, "native_data", NULL );

  SLAG_PUSH_REF( (SlagObject*) native_data );
  NativeData__clean_up();
}


void FileReader__close()
{
  SlagObject*   reader = SLAG_POP_REF();
  SVM_NULL_CHECK( reader, return );

  close_reader(reader);
}

void FileReader__has_another()
{
  SlagObject*   reader = SLAG_POP_REF();
  SVM_NULL_CHECK( reader, return );

  // Slag files auto-close after the last character so if it's still open that
  // means there's another.
  AndroidFileInfo* info = get_reader_file_info(reader);
  if (info) SLAG_PUSH_INT32( 1 );
  else SLAG_PUSH_INT32( 0 );
}

void FileReader__peek()
{
  SlagObject* reader = SLAG_POP_REF();
  SVM_NULL_CHECK( reader, return );

  AndroidFileInfo* info = get_reader_file_info(reader);
  if (info)
  {
    if (info->fp)
    {
      SLAG_PUSH_INT32( info->peek() );
      if (info->finished) close_reader(reader);
    }
  }
  else
  {
    SLAG_PUSH_INT32( -1 );
  }
}

void FileReader__read()
{
  SlagObject* reader = SLAG_POP_REF();
  SVM_NULL_CHECK( reader, return );

  AndroidFileInfo* info = get_reader_file_info(reader);
  if (info)
  {
    if (info->fp)
    {
      SLAG_PUSH_INT32( info->read() );
      if (info->finished) close_reader(reader);
    }
  }
  else
  {
    SLAG_PUSH_INT32( -1 );
  }
}

void FileReader__read__Array_of_Byte_Int32_Int32()
{
  SlagInt32 count    = SLAG_POP_INT32();
  SlagInt32 i        = SLAG_POP_INT32();
  SlagArray* array   = (SlagArray*) SLAG_POP_REF();
  SlagObject* reader = SLAG_POP_REF();

  SVM_NULL_CHECK( reader && array, return );

#if defined(SLAG_VM)
  if (i < 0 || count < 0 || i+count > array->array_count)
  {
    svm.throw_exception( svm.type_out_of_bounds_error );
    return;
  }
#endif

  AndroidFileInfo* info = get_reader_file_info(reader);
  if (info)
  {
    if (count > info->remaining()) count = info->remaining();

    if (info->fp)
    {
      info->read_bytes( (char*) array->data + i, count );
    }

    SLAG_PUSH_INT32( count );
  }
  else
  {
    SLAG_PUSH_INT32( 0 );  // no data left in file
  }
}

void FileReader__read__Array_of_Char_Int32_Int32()
{
  SlagInt32 count    = SLAG_POP_INT32();
  SlagInt32 i        = SLAG_POP_INT32();
  SlagArray* array   = (SlagArray*) SLAG_POP_REF();
  SlagObject* reader = SLAG_POP_REF();

  SVM_NULL_CHECK( reader && array, return );

#if defined(SLAG_VM)
  if (i < 0 || count < 0 || i+count > array->array_count)
  {
    svm.throw_exception( svm.type_out_of_bounds_error );
    return;
  }
#endif

  AndroidFileInfo* info = get_reader_file_info(reader);
  if (info)
  {
    if (count > info->remaining()) count = info->remaining();

    if (info->fp)
    {
      info->read_chars( (char*) array->data + i, count );
    }

    SLAG_PUSH_INT32( count );
  }
  else
  {
    SLAG_PUSH_INT32( 0 );  // no data left in file
  }
}

void FileReader__remaining()
{
  SlagObject* reader = SLAG_POP_REF();
  SVM_NULL_CHECK( reader, return );

  AndroidFileInfo* info = get_reader_file_info(reader);
  if (info)
  {
    SLAG_PUSH_INT32( info->remaining() );
  }
  else
  {
    SLAG_PUSH_INT32( 0 );
  }
}

void FileReader__skip__Int32()
{
  SlagInt32   skip_bytes = SLAG_POP_INT32();
  SlagObject* reader = SLAG_POP_REF();
  SVM_NULL_CHECK( reader, return );

  AndroidFileInfo* info = get_reader_file_info(reader);
  if (info)
  {
    info->skip(skip_bytes);
  }
}

void FileReader__position()
{
  SlagObject* reader = SLAG_POP_REF();
  SVM_NULL_CHECK( reader, return );

  AndroidFileInfo* info = get_reader_file_info(reader);
  if (info)
  {
    SLAG_PUSH_INT32( info->num_read );
  }
  else
  {
    SLAG_PUSH_INT32( info->total_size );
  }
}

//--------------------------------------------------------------------
//  FileWriter
//--------------------------------------------------------------------
void FileWriter__init__String_Logical()
{
  // method init( String filename, Logical append )
  int append = SLAG_POP_INT32();
  SlagString* filename_string = (SlagString*) SLAG_POP_REF();
  SlagObject* writer          = SLAG_POP_REF();
  FILE*       fp;
  char filename[512];

  SVM_NULL_CHECK( filename_string && writer, return );

  filename_string->to_ascii( filename, 512 );

  AndroidFileInfo* info = new AndroidFileInfo();
  if ( !info->open_outfile(filename,(bool)append) )
  {
    slag_throw_file_error( filename );
    return;
  }

  SlagLocalRef gc_guard( writer );
  SlagNativeData* data_obj = SlagNativeData::create( info, SlagNativeDataDeleteResource );

  SLAG_SET_REF(writer,"native_data",data_obj);
}

void FileWriter__close()
{
  // method close()
  SlagObject*   writer = SLAG_POP_REF();
  SVM_NULL_CHECK( writer, return );

  SLAG_GET( SlagNativeData*, native_data, writer, "native_data" );
  if ( !native_data ) return;
  SLAG_SET_REF( writer, "native_data", NULL );

  SLAG_PUSH_REF( (SlagObject*) native_data );
  NativeData__clean_up();
}

static AndroidFileInfo* get_writer_file_info( SlagObject* writer )
{
  SLAG_GET( SlagNativeData*, native_data, writer, "native_data" );
  if ( !native_data ) return NULL;

  return (AndroidFileInfo*) native_data->data;
}

void FileWriter__write__Char()
{
  // method write( Char value )
  SlagInt32     ch = SLAG_POP_INT32();
  SlagObject*   writer = SLAG_POP_REF();

  SVM_NULL_CHECK( writer, return );

  AndroidFileInfo* info = get_writer_file_info(writer);
  if (info)
  {
    info->write(ch);
    return;
  }
  else
  {
    slag_throw_file_error();
  }
}

void FileWriter__position()
{
  // method position.Int32
  SlagObject*   writer = SLAG_POP_REF();
  SVM_NULL_CHECK( writer, return );

  AndroidFileInfo* info = get_writer_file_info(writer);
  if (info)
  {
    SLAG_PUSH_INT32( info->total_size );
  }
  else
  {
    slag_throw_file_error();
  }
}

void FileWriter__write__Array_of_Char_Int32_Int32()
{
  SlagInt32   count  = SLAG_POP_INT32();
  SlagInt32   index  = SLAG_POP_INT32();
  SlagArray*  array  = (SlagArray*) SLAG_POP_REF();
  SlagObject* writer = SLAG_POP_REF();

  SVM_NULL_CHECK( writer && array, return );

  AndroidFileInfo* info = get_writer_file_info(writer);
  if (info)
  {
    int limit = index + count;
    if (index < 0 || count < 0 || limit > array->array_count)
    {
      slag_throw_invalid_operand_error();
      return;
    }

    info->write_chars( ((SlagChar*)array->data)+index, count );
  }
  else
  {
    slag_throw_file_error();
  }
}

void FileWriter__write__Array_of_Byte_Int32_Int32()
{
  SlagInt32   count  = SLAG_POP_INT32();
  SlagInt32   index  = SLAG_POP_INT32();
  SlagArray*  array  = (SlagArray*) SLAG_POP_REF();
  SlagObject* writer = SLAG_POP_REF();

  SVM_NULL_CHECK( writer && array, return );

  AndroidFileInfo* info = get_writer_file_info(writer);
  if (info)
  {
    int limit = index + count;
    if (index < 0 || count < 0 || limit > array->array_count)
    {
      slag_throw_invalid_operand_error();
      return;
    }

    info->write_chars( ((char*)array->data)+index, count );
  }
  else
  {
    slag_throw_file_error();
  }
}

void FileWriter__write__String()
{
  SlagString* st     = (SlagString*) SLAG_POP_REF();
  SlagObject* writer = SLAG_POP_REF();

  SVM_NULL_CHECK( writer && st, return );

  AndroidFileInfo* info = get_writer_file_info(writer);
  if (info)
  {
    info->write_chars( st->characters, st->count );
  }
  else
  {
    slag_throw_file_error();
  }
}

//=============================================================================
//  VideoPlayer
//=============================================================================
struct AndroidVideoPlayerInfo : SlagResource
{
  int player_id;

  AndroidVideoPlayerInfo( const char* filename )
  {
    jobject j_filename_obj = jvm->NewStringUTF(filename);
    player_id = jvm->CallIntMethod( android_core_obj, m_jniVideoPlay, j_filename_obj );
    jvm->DeleteLocalRef( j_filename_obj );
  }

  ~AndroidVideoPlayerInfo()
  {
    if (player_id)
    {
      jvm->CallVoidMethod( android_core_obj, m_jniVideoStop, player_id );
      player_id = 0;
    }
  }

  bool update()
  {
    if (player_id == 0) return false;
    return jvm->CallBooleanMethod( android_core_obj, m_jniVideoUpdate, player_id );
  }
};

void VideoPlayer__play__String()
{
  SlagString* filepath    = (SlagString*) SLAG_POP_REF();
  SlagObject* context = SLAG_POP_REF();  // context

  char filename[512];

#if defined(SLAG_VM)
  if ( !filepath )
  {
    svm.throw_exception( svm.type_null_reference_error );
    return;
  }
#endif

  ((SlagString*)filepath)->to_ascii( filename, 512 );
  slag_adjust_filename_for_os( filename, 512 );

  AndroidVideoPlayerInfo* player = new AndroidVideoPlayerInfo( filename );
  SLAG_PUSH_REF( SlagNativeData::create( player, SlagNativeDataDeleteResource ) );
}

void VideoPlayer__update__NativeData()
{
  AndroidVideoPlayerInfo* player = NULL;
  SlagNativeData* native_data = (SlagNativeData*) SLAG_POP_REF();
  if (native_data) player = (AndroidVideoPlayerInfo*) native_data->data;

  SlagObject* context = SLAG_POP_REF();  // context

  if ( !player || !player->update() )
  {
    SLAG_PUSH_LOGICAL( false );
  }
  else
  {
    SLAG_PUSH_LOGICAL( true );
  }
}


//=============================================================================
//  WebView
//=============================================================================
static int get_webview_index( SlagObject* slag_webview )
{
  SLAG_GET_INT32( id, slag_webview, "id" );
  id = jvm->CallIntMethod( android_core_obj, m_jniWebViewGet, id );
  SLAG_SET_INT32( slag_webview, "id", id );
  return id;
}

void WebView__view__URL()
{
  SlagObject* url_obj = SLAG_POP_REF();
  int id = get_webview_index( SLAG_POP_REF() );

  if (url_obj == NULL) return;

  SLAG_GET_REF( url_string, url_obj, "value" );
  jstring url = to_jstring( url_string );

  jvm->CallVoidMethod( android_core_obj, m_jniWebViewURL, id, url );

  jvm->DeleteLocalRef( url );
}

void WebView__view__String()
{
  jstring html = to_jstring( SLAG_POP_REF() );
  int id = get_webview_index( SLAG_POP_REF() );

  jvm->CallVoidMethod( android_core_obj, m_jniWebViewHTML, id, html );

  jvm->DeleteLocalRef( html );
}

void WebView__close()
{
  SlagObject* slag_webview = SLAG_POP_REF();
  SLAG_GET_INT32( id, slag_webview, "id" );
  if (id)
  {
    jvm->CallVoidMethod( android_core_obj, m_jniWebViewClose, id );
    SLAG_SET_INT32( slag_webview, "id", 0 );
  }
}

void WebView__bounds__Box()
{
  Vector2 position = SLAG_POP(Vector2);
  Vector2 size = SLAG_POP(Vector2);
  int id = get_webview_index( SLAG_POP_REF() );

  position = plasmacore.point_to_screen(position);
  size     = plasmacore.size_to_screen(size);

  jvm->CallVoidMethod( android_core_obj, m_jniWebViewSetBounds, id, 
      (int)position.x, (int)position.y, (int) size.x, (int) size.y );
}


void WebView__visible__Logical()
{
  int setting = SLAG_POP_INT32();
  int id = get_webview_index( SLAG_POP_REF() );
  jboolean jsetting = (setting==1) ? true : false;
  jvm->CallVoidMethod( android_core_obj, m_jniWebViewSetVisible, id, jsetting );
}

void WebView__visible()
{
  int id = get_webview_index( SLAG_POP_REF() );
  jboolean setting = jvm->CallBooleanMethod( android_core_obj, m_jniWebViewGetVisible, id );
  SLAG_PUSH_INT32( setting ? 1 : 0 );
}

void WebView__loaded()
{
  int id = get_webview_index( SLAG_POP_REF() );
  jboolean setting = jvm->CallBooleanMethod( android_core_obj, m_jniWebViewGetLoaded, id );
  SLAG_PUSH_INT32( setting ? 1 : 0 );
}

void WebView__failed()
{
  int id = get_webview_index( SLAG_POP_REF() );
  jboolean setting = jvm->CallBooleanMethod( android_core_obj, m_jniWebViewGetFailed, id );
  SLAG_PUSH_INT32( setting ? 1 : 0 );
}

