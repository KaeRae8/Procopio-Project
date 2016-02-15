#ifndef PLASMACORE_H
#define PLASMACORE_H
//=============================================================================
// plasmacore.h
//
// 3.5.0 (2011.06.18)
//
// http://plasmaworks.com/plasmacore
// 
// Code common to all versions of Plasmacore.
//
// ----------------------------------------------------------------------------
//
// Copyright 2008-2011 Plasmaworks LLC
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
//
//=============================================================================

#include "slag.h"

//#if defined(SLAG_XC)
//#  include "game_xc.h"
//#endif

extern int embedded_font_system_17_size;
extern unsigned char embedded_font_system_17[];

void LOG( const char* mesg );

/*
template <typename DataType> 
struct ArrayList
{
  DataType* data;
  int count;
  int capacity;

  ArrayList( int capacity=0 )
  {
    this->capacity = capacity;
    if (capacity) data = new DataType[capacity];
    else data = 0;
    count = 0;
  }

  ~ArrayList()
  {
    if (data) 
    {
      delete data;
      data = 0;
    }
  }

  DataType remove_first()
  {
    DataType result = data[0];
    for (int i=1; i<count; ++i)
    {
      data[i-1] = data[i];
    }
    --count;
    return result;
  }

  int index_of( DataType value )
  {
    for (int i=0; i<count; ++i)
    {
      if (data[i] == value) return i;
    }
    return -1;
  }

  DataType remove( int i )
  {
    DataType result = data[i];
    ++i;
    for (; i<count; ++i)
    {
      data[i-1] = data[i];
    }
    --count;
    return result;
  }

  void remove_value( DataType value )
  {
    int i = index_of( value );
    if (i >= 0) remove(i);
  }

  DataType remove_last()
  {
    return data[--count];
  }

  void clear()
  {
    count = 0;
  }

  void add( DataType value )
  {
    if (count == capacity) ensure_capacity( capacity ? capacity*2 : 10 );
    data[count++] = value;
  }

  DataType operator[]( int index )
  {
    return data[index];
  }

  void ensure_capacity( int c )
  {
    if (capacity >= c) return;

    capacity = c;

    if ( !data )
    {
      data = new DataType[c];
    }
    else
    {
      DataType* new_data = new DataType[c];
      memcpy( new_data, data, sizeof(DataType)*count );
      delete data;
      data = new_data;
    }
  }
};
*/

#ifdef ANDROID
#define DATA_ARCHIVE   1
#define IMAGES_ARCHIVE 2
#define SOUNDS_ARCHIVE 3
struct Archive
{
  int archive_type;

  Archive( int type ); 

  char* load( SlagString* filename, int* size );
  char* load( const char* filename, int* size );
};
#else
struct Archive
{
  char* archive_filename;

  Archive( const char* archive_filename );
  ~Archive();

  void* open();
  char* load( SlagString* filename, int* size );
  char* load( const char* filename, int* size );
};
#endif

#define RENDER_FLAG_POINT_FILTER (1 << 0)
#define RENDER_FLAG_FIXED_COLOR  (1 << 2)
#define RENDER_FLAG_TEXTURE_WRAP (1 << 3)
#define RENDER_FLAG_OPAQUE       (1 << 4)

//--------
#define BLEND_ZERO                0
#define BLEND_ONE                 1
#define BLEND_SRC_ALPHA           2
#define BLEND_INVERSE_SRC_ALPHA   3
#define BLEND_DEST_ALPHA          4
#define BLEND_INVERSE_DEST_ALPHA  5
#define BLEND_DEST_COLOR          6
#define BLEND_INVERSE_DEST_COLOR  7
#define BLEND_OPAQUE              8
#define BLEND_SRC_COLOR           9
#define BLEND_INVERSE_SRC_COLOR  10
//--------

#define PIXEL_FORMAT_RGB32     1
#define PIXEL_FORMAT_RGB16     2
#define PIXEL_FORMAT_INDEXED   4

//--------

#define MAX_BUFFERED_VERTICES 512*3
#define DRAW_TEXTURED_TRIANGLES 1
#define DRAW_SOLID_TRIANGLES    2
#define DRAW_LINES              3
#define DRAW_POINTS             4


//--------

#define TRANSFORM_STACK_SIZE 32

#define SEPARATE_ARGB(c,a,r,g,b) a=(c>>24)&255; r=(c>>16)&255; g=(c>>8)&255; b=c&255
#define COMBINE_ARGB(a,r,g,b) ((a<<24)|(r<<16)|(g<<8)|b)

/*
struct DelegateMessage
{
  virtual void dispatch() { } 
};

extern ArrayList<DelegateMessage*> svm_delegate_messages;

void dispatch_delegate_messages();
*/

struct SlagBitmap : SlagObject
{
  SlagArray*    pixels;
  SlagInt32     width;
  SlagInt32     height;
};

struct Matrix2x3
{
  SlagReal64 r1c1, r1c2, r1c3;
  SlagReal64 r2c1, r2c2, r2c3;

  Matrix2x3() : r1c1(0), r1c2(0), r1c3(0), r2c1(0), r2c2(0), r2c3(0) { }
  Matrix2x3( SlagReal64 v1, SlagReal64 v2, SlagReal64 v3, SlagReal64 v4, SlagReal64 v5, SlagReal64 v6 ) 
    : r1c1(v1), r1c2(v2), r1c3(v3), r2c1(v4), r2c2(v5), r2c3(v6) { }
};

typedef struct Vector2
{
  SlagReal64 x, y;

	Vector2() { }
	Vector2( SlagReal64 _x, SlagReal64 _y ) : x(_x), y(_y) {}

} Vector2;

struct PCoreBox
{
  Vector2 top_left, size;

  PCoreBox() {}
  PCoreBox( Vector2 tl, Vector2 br ) : top_left(tl), size(br) {}
  PCoreBox( double x, double y, double w, double h ) : top_left(x,y), size(w,h) {}
};

extern bool mouse_visible;
extern bool log_drawing;
extern bool use_scissor;

struct Plasmacore
{
  int display_width;
  int display_height;

  double scale_factor;
  int border_x;
  int border_y;

  int fast_hardware;
  int orientation;  // 0..3 = up right down left
  int original_orientation;  // what's read in from plasmacore

  bool is_fullscreen;
  bool borderless_window;

  ArrayList<char*> command_line_args;

  double updates_per_second;  // default: 60.0
  int    target_fps;          // default: 60   - should always be <= updates_per_second
  double time_debt;           // in seconds

  SlagInt64  last_update_time_ms;  // timestamp
  SlagInt32  last_draw_time_ms;    // interval

  // These are references to permanent string objects and won't change during GC.
  SlagObject* event_launch;
  SlagObject* event_update;
  SlagObject* event_draw;
  SlagObject* event_key;
  SlagObject* event_mouse_move;
  SlagObject* event_mouse_button;
  SlagObject* event_mouse_wheel;
  SlagObject* event_textures_lost;
  SlagObject* event_suspend;
  SlagObject* event_resume;
  SlagObject* event_shut_down;

  Matrix2x3 transform;
  Matrix2x3 camera_transform;
  Matrix2x3 object_transform;

  Matrix2x3 camera_transform_stack[TRANSFORM_STACK_SIZE];
  Matrix2x3 object_transform_stack[TRANSFORM_STACK_SIZE];
  int  camera_transform_stack_count;
  int  object_transform_stack_count;
  bool camera_transform_stack_modified;
  bool object_transform_stack_modified;

  Plasmacore()
  {
    updates_per_second = 60;
//#if defined(ANDROID)
//target_fps = 30;
//#else
    target_fps = 60;
//#endif
    orientation = 0;
    set_defaults(); 
  }

  void set_defaults()
  {
    scale_factor = 1.0;
    border_x = 0;
    border_y = 0;
    is_fullscreen  = false;
    borderless_window = false;
    fast_hardware = true;

    time_debt = 0.0;
    last_update_time_ms = 0;
    last_draw_time_ms   = 0;

    event_launch = NULL;
    event_update = NULL;
    event_draw   = NULL;

    camera_transform_stack_count = 0;
    camera_transform_stack_modified = true;
    object_transform_stack_count = 0;
    object_transform_stack_modified = true;
  }

  Vector2 point_to_screen( Vector2 v )
  {
    return Vector2( v.x*scale_factor+border_x, v.y*scale_factor+border_y );
  }

  Vector2 size_to_screen( Vector2 v )
  {
    return Vector2( v.x*scale_factor, v.y*scale_factor );
  }
};

extern Plasmacore plasmacore;

//=============================================================================
//  UpdateCycleRegulator
//=============================================================================
struct UpdateCycleRegulator
{
  SlagInt64 next_frame_time;
  int       ms_error;

  UpdateCycleRegulator() : next_frame_time(0), ms_error(0)
  {
    reset();
  }

  void reset()
  {
    next_frame_time = slag_get_time_ms() + (1000/plasmacore.target_fps);
  }

  int update()
  {
    SlagInt64 cur_time = slag_get_time_ms();
    int kill_ms = int(next_frame_time - cur_time);

    if (kill_ms <= 0)
    {
      next_frame_time = cur_time;
      kill_ms = 1;
    }
    next_frame_time += 1000 / plasmacore.target_fps;
    ms_error += 1000 % plasmacore.target_fps;
    if (ms_error >= plasmacore.target_fps)
    {
      ms_error -= plasmacore.target_fps;
      ++next_frame_time;
    }
    return kill_ms;
  }
};

//=============================================================================
//  TouchInfo
//=============================================================================
#define PLASMACORE_MAX_TOUCHES 10

struct TouchInfo
{
  int     active;
  Vector2 position;

  TouchInfo() { active = 0; }
};

struct TouchManager
{
  TouchInfo touches[PLASMACORE_MAX_TOUCHES];
  int   num_active_touches;

  TouchManager() { num_active_touches = 0; }

  void reset()
  {
    num_active_touches = 0;
    for (int i=0; i<PLASMACORE_MAX_TOUCHES; ++i)
    {
      touches[i].active = false;
    }
  }

  // Each method returns the index (1+) that TouchManager
  // is using to track the event.
  int begin_touch( double x, double y );
  int update_touch( double x, double y );
  int end_touch( double x, double y );
};


#if !defined(_WIN32)
# define GL_PLASMACORE
# if defined(ANDROID)
#   include <GLES/gl.h>
#   include <GLES/glext.h>
#   define MAIN_BUFFER 0
# elif defined(MAC)
#   import <QuartzCore/QuartzCore.h>
#   include "SDL.h"
#   define glBindFramebufferOES glBindFramebufferEXT
#   define GL_FRAMEBUFFER_OES   GL_FRAMEBUFFER_EXT
#   define GL_FRAMEBUFFER_COMPLETE_OES   GL_FRAMEBUFFER_COMPLETE_EXT
#   define GL_COLOR_ATTACHMENT0_OES   GL_COLOR_ATTACHMENT0_EXT
#   define glFramebufferTexture2DOES   glFramebufferTexture2DEXT
#   define glCheckFramebufferStatusOES   glCheckFramebufferStatusEXT
#   define glDeleteFramebuffersOES glDeleteFramebuffersEXT
#   define MAIN_BUFFER 0
# elif TARGET_OS_IPHONE
#   include <OpenGLES/ES1/gl.h>
#   include <OpenGLES/ES1/glext.h>
#   define MAIN_BUFFER NativeLayer_get_frame_buffer()
    GLuint NativeLayer_get_frame_buffer();
# elif defined(UNIX)
#   define GL_GLEXT_PROTOTYPES 1
#   include <SDL/SDL.h>
#   include <SDL/SDL_opengl.h>
#   define glBindFramebufferOES glBindFramebufferEXT
#   define GL_FRAMEBUFFER_OES   GL_FRAMEBUFFER_EXT
#   define GL_FRAMEBUFFER_COMPLETE_OES   GL_FRAMEBUFFER_COMPLETE_EXT
#   define GL_COLOR_ATTACHMENT0_OES   GL_COLOR_ATTACHMENT0_EXT
#   define glFramebufferTexture2DOES   glFramebufferTexture2DEXT
#   define glCheckFramebufferStatusOES   glCheckFramebufferStatusEXT
#   define glDeleteFramebuffersOES glDeleteFramebuffersEXT
#   define MAIN_BUFFER 0
# endif

# include "gl_core.h"
#endif


int  argb_to_rgba( int argb );
void swap_red_and_blue( SlagInt32* data, int count );


Matrix2x3 Matrix2x3_multiply( Matrix2x3 m1, Matrix2x3 m2 );
double    Matrix2x3_determinant( Matrix2x3 m );
Matrix2x3 Matrix2x3_invert( Matrix2x3 m );
Vector2   Matrix2x3_transform( Matrix2x3 m, Vector2 v );

void plasmacore_init();
void plasmacore_shut_down();
void plasmacore_configure( int default_display_width, int default_display_height, bool force_default_size, bool allow_new_orientation );
void plasmacore_set_scale( int new_width, int new_height );
void plasmacore_queue_event( SlagObject* type );
void plasmacore_queue_object_event( SlagObject* type, SlagObject* object, double x=0.0, double y=0.0  );
void plasmacore_queue_data_event( SlagObject* type, int id, int index, bool flag=false,
    double x=0.0, double y=0.0 );
void plasmacore_dispatch_pending_events();
void plasmacore_launch();
bool plasmacore_update();
void plasmacore_draw();
void plasmacore_clear_transforms();
bool plasmacore_set_transform();
void plasmacore_on_exit_request();
SlagObject* plasmacore_find_event_key( const char* event_type );

void NativeLayer_sleep( int ms );

void NativeLayer_alert( const char* mesg );
void NativeLayer_begin_draw();
void NativeLayer_end_draw();
void NativeLayer_shut_down();

void Application__log__String();
void Application__title__String();

void Bitmap__copy_pixels_to__Int32_Int32_Int32_Int32_Bitmap_Int32_Int32_Logical();
void Bitmap__init__ArrayList_of_Byte();
void Bitmap__init__String();
void Bitmap__to_png_bytes();
void Bitmap__to_jpg_bytes__Real64();
void Bitmap__rotate_right();
void Bitmap__rotate_left();
void Bitmap__rotate_180();
void Bitmap__flip_horizontal();
void Bitmap__flip_vertical();
void Bitmap__resize_horizontal__Int32();
void Bitmap__resize_vertical__Int32();

void Display__flush();
void Display__fullscreen();
void Display__fullscreen__Logical();
void Display__last_draw_time_ms();
void Display__native_set_clipping_region__Box();
void Display__native_set_draw_target__OffscreenBuffer_Logical();
void Display__screen_shot__Bitmap();
void Display__native_scale_to_fit__Int32_Int32();

void Image__uv();
void Image__uv__Corners();

void Input__mouse_visible__Logical();
void Input__keyboard_visible__Logical();
void Input__keyboard_visible();
void Input__input_capture__Logical();

void LineManager__draw__Line_Color_Render();

void NativeSound__init__String();
void NativeSound__init__ArrayList_of_Byte();

void NativeSound__create_duplicate();
void NativeSound__play();
void NativeSound__pause();
void NativeSound__is_playing();
void NativeSound__volume__Real64();
void NativeSound__pan__Real64();
void NativeSound__pitch__Real64();
void NativeSound__repeats__Logical();
void NativeSound__current_time();
void NativeSound__current_time__Real64();
void NativeSound__duration();

void OffscreenBuffer__clear__Color();

void QuadManager__fill__Quad_ColorGradient_Render();

void ResourceManager__load_data_file__String();
void ResourceManager__load_gamestate__String();
void ResourceManager__save_gamestate__String_String();
void ResourceManager__delete_gamestate__String();

void System__device_id();
void System__max_texture_size();
void System__open_url__String();
void System__country_name();

void SystemMonitor__log_drawing__Logical();

void Texture__init__Bitmap_Int32();
void Texture__init__String_Int32();
void Texture__init__Vector2_Int32();
void Texture__init__Vector2();
void Texture__native_release();
void Texture__draw__Corners_Vector2_Color_Render_Blend();
void Texture__draw__Corners_Vector2_Color_Render_Blend_Texture_Corners();
void Texture__draw__Corners_Quad_ColorGradient_Render_Blend();
void Texture__draw__Vector2_Vector2_Vector2_Triangle_Color_Color_Color_Render_Blend();
void Texture__draw_tile__Corners_Vector2_Vector2_Int32();
void Texture__set__Bitmap();
void Texture__set__Bitmap_Vector2();

void TransformManager__create_from__Vector2_Vector2_Radians_Vector2_Vector2_Logical_Logical();
void TransformManager__current();
void TransformManager__inverse__Transform();
void TransformManager__opMUL__Transform_Transform();
void TransformManager__push_object_transform__Transform();
void TransformManager__pop_object_transform();
void TransformManager__push_camera_transform__Transform();
void TransformManager__pop_camera_transform();

void TriangleManager__fill__Triangle_Color_Color_Color_Render();

void Vector2Manager__draw__Vector2_Color_Render();

#endif // PLASMACORE_H

