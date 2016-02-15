//=============================================================================
// plasmacore.cpp
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

#if defined(ANDROID)
void log(const char*);
#endif

#include <string.h>
#include "unzip.h"
#include "plasmacore.h"

void plasmacore_load_settings();
void plasmacore_hook_native_methods();

Plasmacore plasmacore;
bool mouse_visible = true;
bool log_drawing = false;

#ifdef ANDROID
extern char* android_etc_data;
extern int   android_etc_size;
#endif

//=============================================================================
//  Miscellaneous
//=============================================================================

bool is_id( char ch )
{
  if (ch >= 'A' && ch <= 'Z') return true;
  if (ch >= 'a' && ch <= 'z') return true;
  if (ch >= '0' && ch <= '9') return true;
  if (ch == '_' || ch == '.' || ch == '-') return true;
  return false;
}

int argb_to_rgba( int argb )
{
  unsigned char bytes[4];
  bytes[0] = (argb >> 16);
  bytes[1] = (argb >> 8);
  bytes[2] = argb;
  bytes[3] = (argb >> 24);
  return *((SlagInt32*)bytes);
}

void swap_red_and_blue( SlagInt32* data, int count )
{
  --data;
  ++count;
  while (--count)
  {
    int color = *(++data);
    int ag = color & 0xff00ff00;
    int rb = color & 0x00ff00ff;
    *data = (ag | (rb>>16) | (rb<<16));
  }
}

//=============================================================================
//  TouchManager
//=============================================================================
int TouchManager::begin_touch( double x, double y )
{
  if (num_active_touches < PLASMACORE_MAX_TOUCHES)
  {
    for (int i=0; i<PLASMACORE_MAX_TOUCHES; ++i)
    {
      if ( !touches[i].active )
      {
        touches[i].active = true;
        touches[i].position.x = x;
        touches[i].position.y = y;
        ++num_active_touches;
        return i+1;
      }
    }
  }
  return 0;
}

int TouchManager::update_touch( double x, double y )
{
  int best_i = -1;
  double dx,dy;
  double best_r2 = 0;
  for (int i=0; i<PLASMACORE_MAX_TOUCHES; i++)
  {
    if (touches[i].active)
    {
      dx = touches[i].position.x - x;
      dy = touches[i].position.y - y;
      double r2 = dx*dx + dy*dy;
      if (best_i == -1 || r2 < best_r2)
      {
        best_r2 = r2;
        best_i = i;
      }
    }
  }

  if (best_i == -1)
  {
    printf( "ERROR: update_touch with no active touches!\n" );
    return 0;
  }

  touches[best_i].position.x = x;
  touches[best_i].position.y = y;
  return best_i+1;
}

int TouchManager::end_touch( double x, double y )
{
  int i = update_touch(x,y);
  touches[i-1].active = 0;
  --num_active_touches;
  return i;
}


//=============================================================================
//  Archive
//=============================================================================
char* Archive::load( SlagString* filename, int* size )
{
  if (filename == NULL) return NULL;

  char buffer[PATH_MAX];
  filename->to_ascii( buffer, PATH_MAX );
  return load( buffer, size );
}

#ifndef ANDROID
char* Archive::load( const char* filename, int *size )
{
  if (strncmp(filename,"internal:",9) == 0)
  {
    filename += 9;
    if (0 == strcmp(filename,"font_system_17"))
    {
      *size = embedded_font_system_17_size;
      char* buffer = (char*) new char[ embedded_font_system_17_size ];
      memcpy( buffer, embedded_font_system_17, embedded_font_system_17_size );
      return buffer;
    }
  }

  unzFile zfp = unzOpen( archive_filename );
  if ( !zfp ) return NULL;

  int  filename_len = strlen(filename);
  char first_char = filename[0];

  int err = UNZ_OK;
  err = unzGoToFirstFile(zfp);
  while (UNZ_OK == err)
  {
    char current_filename[256+1];
    err = unzGetCurrentFileInfo( zfp, NULL, current_filename, 256, NULL, 0, NULL, 0 );
    if (UNZ_OK == err)
    {
      int cur_len = strlen(current_filename);
      if (filename_len <= cur_len)
      {
        // do a substring match
        int diff = cur_len - filename_len;
        for (int i=0; i<=diff; i++)
        {
          if (first_char == current_filename[i] 
              && 0 == strncmp(filename,current_filename+i,filename_len))
          {
            int prev_char;
            if (i > 0) prev_char = current_filename[i-1];
            else       prev_char = 0;
            int next_char = current_filename[i+filename_len];

            if ( !is_id(prev_char) && (next_char==0 || next_char=='.') )
            {
              // found our match
              if (UNZ_OK == unzOpenCurrentFile(zfp))
              {
                unz_file_info file_info;
                unzGetCurrentFileInfo( zfp, &file_info, 0, 0, 0, 0, 0, 0 );

                int data_size = file_info.uncompressed_size;
                if (size) *size = data_size;

                char* buffer = (char*) new char[ data_size ];
                unzReadCurrentFile( zfp, buffer, data_size );
                unzClose(zfp);

                return buffer;
              }
            }
          }
        }
      }

      err = unzGoToNextFile(zfp);
    }
  }
  unzClose(zfp);

  return NULL;
}

#endif

void plasmacore_init()
{
  // Set up the Slag VM
  LOG( "Plasmacore initializing" );
  atexit( plasmacore_shut_down );
  slag_init();
}

void plasmacore_shut_down()
{
  LOG( "Plasmacore shutting down" );
  plasmacore_queue_event( plasmacore.event_suspend );
  plasmacore_queue_event( plasmacore.event_shut_down );
  plasmacore_dispatch_pending_events();
  NativeLayer_shut_down();
  slag_shut_down();
}

void plasmacore_configure( int default_display_width, int default_display_height, 
    bool force_default_size, bool allow_new_orientation )
{
  char logmesg[80];
  sprintf( logmesg, "Plasmacore configuring with default display size %dx%d", 
           default_display_width, default_display_height );
  LOG( logmesg );

  plasmacore.set_defaults();

  plasmacore.display_width = 0;
  plasmacore.display_height = 0;

  plasmacore_hook_native_methods();

  plasmacore_load_settings();
  if (force_default_size) plasmacore.display_width = 0;

  plasmacore.original_orientation = plasmacore.orientation;
  if (allow_new_orientation)
  {
    if (plasmacore.orientation == 1 || plasmacore.orientation == 3)
    {
      int temp = default_display_width;
      default_display_width = default_display_height;
      default_display_height = temp;
    }
  }
  else
  {
    plasmacore.orientation = 0;
  }

  if ( !plasmacore.display_width )
  {
    plasmacore.display_width = default_display_width;
    plasmacore.display_height = default_display_height;
  }

  plasmacore.scale_factor = 1.0;
}

void log( const char* st );

void plasmacore_queue_event( SlagObject* type )
{
  SLAG_FIND_TYPE( type_SignalManager, "SignalManager" );
  SLAG_PUSH_REF( type_SignalManager->singleton() );
  SLAG_PUSH_REF( type );
  SLAG_PUSH_REF( NULL );
  SLAG_CALL( type_SignalManager, "queue_native(String,Object)" );
}

void plasmacore_queue_object_event( SlagObject* type, SlagObject* object, double x, double y )
{
  SLAG_FIND_TYPE( type_SignalManager, "SignalManager" );
  SLAG_PUSH_REF( type_SignalManager->singleton() );
  SLAG_PUSH_REF( type );

  {
    SLAG_FIND_TYPE( type_SignalObjectArg, "SignalObjectArg" );
    SLAG_PUSH_REF( type_SignalObjectArg->create() );
    SLAG_DUPLICATE_REF();

    SLAG_PUSH_REF( object );
    SLAG_PUSH_REAL64( x );
    SLAG_PUSH_REAL64( y );
    SLAG_CALL( type_SignalObjectArg, "init(Object,Real64,Real64)" );
  }

  SLAG_CALL( type_SignalManager, "queue_native(String,Object)" );
}

void plasmacore_queue_data_event( SlagObject* type, int id, int index, bool flag,
    double x, double y )
{
  SLAG_FIND_TYPE( type_SignalManager, "SignalManager" );
  SLAG_PUSH_REF( type_SignalManager->singleton() );
  SLAG_PUSH_REF( type );

  {
    SLAG_FIND_TYPE( type_SignalDataArg, "SignalDataArg" );
    SLAG_PUSH_REF( type_SignalDataArg->create() );
    SLAG_DUPLICATE_REF();

    SLAG_PUSH_INT32( id );
    SLAG_PUSH_INT32( index );
    SLAG_PUSH_LOGICAL( flag );
    SLAG_PUSH_REAL64( x );
    SLAG_PUSH_REAL64( y );
    SLAG_CALL( type_SignalDataArg, "init(Int32,Int32,Logical,Real64,Real64)" );
  }

  SLAG_CALL( type_SignalManager, "queue_native(String,Object)" );
}

void plasmacore_dispatch_pending_events()
{
  mm.check_gc();
  SLAG_FIND_TYPE( type_SignalManager, "SignalManager" );
  SLAG_PUSH_REF( type_SignalManager->singleton() );
  SLAG_CALL( type_SignalManager, "raise_pending()" );
  SLAG_POP_LOGICAL();
}


void plasmacore_launch()
{
#ifdef SLAG_VM
#ifdef ANDROID
  SlagLoader loader;
  loader.load( android_etc_data, android_etc_size );
#else
  char buffer[512];
  strcpy(buffer,"game.etc");
  slag_adjust_filename_for_os( buffer, 512 );

  SlagLoader loader;
  loader.load( buffer );
#endif //ANDROID
#endif

  slag_configure();

  plasmacore.event_launch = plasmacore_find_event_key( "launch" );
  plasmacore.event_update = plasmacore_find_event_key( "update" );
  plasmacore.event_draw   = plasmacore_find_event_key( "draw" );
  plasmacore.event_key    = plasmacore_find_event_key( "key" );
  plasmacore.event_mouse_move = plasmacore_find_event_key( "mouse_move" );
  plasmacore.event_mouse_button = plasmacore_find_event_key(  "mouse_button" );
  plasmacore.event_mouse_wheel = plasmacore_find_event_key( "mouse_wheel" );
  plasmacore.event_textures_lost = plasmacore_find_event_key( "textures_lost" );
  plasmacore.event_suspend = plasmacore_find_event_key( "suspend" );
  plasmacore.event_resume = plasmacore_find_event_key( "resume" );
  plasmacore.event_shut_down = plasmacore_find_event_key( "shut_down" );

  plasmacore_queue_object_event( plasmacore.event_launch, *slag_main_object, 
      plasmacore.display_width, plasmacore.display_height );
  plasmacore_dispatch_pending_events();
}

bool plasmacore_update()
{
  // Returns "true" if the screen should be redrawn
  if (plasmacore.updates_per_second == 0.0) return false;

  SlagInt64 time_ms = slag_get_time_ms();
  SlagInt64 elapsed_ms = (time_ms - plasmacore.last_update_time_ms);
  if (elapsed_ms > 1000 || elapsed_ms == 0) elapsed_ms = 0;
  plasmacore.last_update_time_ms = time_ms;

  plasmacore.time_debt += elapsed_ms / 1000.0;

  if (plasmacore.time_debt > 0.1)
  {
    plasmacore.time_debt = 1.0 / plasmacore.updates_per_second;
  }

  bool draw_allowed = true;
  bool first = true;

  while (first || plasmacore.time_debt >= 1.0 / plasmacore.updates_per_second)
  {
    first = false;
    plasmacore.time_debt -= 1.0 / plasmacore.updates_per_second;

    plasmacore_queue_event( plasmacore.event_update );
    plasmacore_dispatch_pending_events();
  }

  return draw_allowed;
}

void plasmacore_draw()
{
  if (log_drawing) LOG( "+draw()" );

  SlagInt64 start_ms = slag_get_time_ms();

  plasmacore_clear_transforms();
  NativeLayer_begin_draw();
  plasmacore_queue_event( plasmacore.event_draw );
  plasmacore_dispatch_pending_events();
  NativeLayer_end_draw();

  plasmacore.last_draw_time_ms = int(slag_get_time_ms() - start_ms);

  if (log_drawing) LOG( "-draw()" );
}

void plasmacore_clear_transforms()
{
  plasmacore.camera_transform_stack_count = 0;
  plasmacore.object_transform_stack_count = 0;
  plasmacore.camera_transform_stack_modified = true;
  plasmacore.object_transform_stack_modified = true;
}

bool plasmacore_set_transform()
{
  bool modified = false;

  int  camera_count = plasmacore.camera_transform_stack_count;
  if (plasmacore.camera_transform_stack_modified)
  {
    modified = true;
    plasmacore.camera_transform_stack_modified = false;

    if (camera_count > 0)
    {
      Matrix2x3* mptr = plasmacore.camera_transform_stack - 1;

      plasmacore.camera_transform = *(++mptr);

      while (--camera_count)
      {
        plasmacore.camera_transform = Matrix2x3_multiply( plasmacore.camera_transform, *(++mptr) );
      }
    }
    else
    {
      plasmacore.camera_transform = Matrix2x3(1,0,0,0,1,0);
    }
  }

  int  object_count = plasmacore.object_transform_stack_count;
  if (plasmacore.object_transform_stack_modified)
  {
    modified = true;
    plasmacore.object_transform_stack_modified = false;

    if (object_count > 0)
    {
      Matrix2x3* mptr = plasmacore.object_transform_stack + object_count;

      plasmacore.object_transform = *(--mptr);

      while (--object_count)
      {
        plasmacore.object_transform = Matrix2x3_multiply( plasmacore.object_transform, *(--mptr) );
      }
    }
    else
    {
      plasmacore.object_transform = Matrix2x3(1,0,0,0,1,0);
    }
  }

  if (modified)
  {
    plasmacore.transform = Matrix2x3_multiply( 
        plasmacore.camera_transform, plasmacore.object_transform );
  }

  return true;
}

void TransformManager__current()
{
  SLAG_POP_REF(); /* discard singleton ref */

  plasmacore_set_transform();
  SLAG_PUSH( Matrix2x3, plasmacore.transform );
}

void TransformManager__create_from__Vector2_Vector2_Radians_Vector2_Vector2_Logical_Logical()
{
  bool vflip = (SLAG_POP_INT32() != 0);
  bool hflip = (SLAG_POP_INT32() != 0);
  Vector2 pos   = SLAG_POP(Vector2);
  Vector2 scale = SLAG_POP(Vector2);
  SlagReal64 angle = SLAG_POP_REAL64();
  Vector2 handle = SLAG_POP(Vector2);
  Vector2 size   = SLAG_POP(Vector2);
  SLAG_POP_REF();

  if (hflip || vflip)
  {
    handle.x -= size.x / 2.0;
    handle.y -= size.y / 2.0;
  }

  double cost = cos(angle);
  double sint = sin(angle);

  double r1c1 = cost*scale.x;
  double r1c2 = -sint*scale.y;
  double r1c3 = pos.x - scale.x*handle.x*cost + sint*scale.y*handle.y;

  double r2c1 = sint*scale.x;
  double r2c2 = cost*scale.y;
  double r2c3 = pos.y - scale.x*handle.x*sint - cost*scale.y*handle.y;

  Matrix2x3 m(r1c1,r1c2,r1c3,r2c1,r2c2,r2c3);
  if (hflip || vflip)
  {
    if (hflip)
    {
      if (vflip)
      {
        m = Matrix2x3_multiply( m, Matrix2x3(-1,0,0,0,-1,0) );
      }
      else
      {
        m = Matrix2x3_multiply( m, Matrix2x3(-1,0,0,0,1,0) );
      }
    }
    else
    {
      m = Matrix2x3_multiply( m, Matrix2x3(1,0,0,0,-1,0) );
    }

    // translate by -size/2
    m = Matrix2x3_multiply( m, Matrix2x3(1,0,-size.x/2.0,0,1,-size.y/2.0) );
  }

  SLAG_PUSH( Matrix2x3, m );
}

void TransformManager__push_camera_transform__Transform()
{
  Matrix2x3 m = SLAG_POP(Matrix2x3);
  SLAG_POP_REF();

  if (plasmacore.camera_transform_stack_count < TRANSFORM_STACK_SIZE)
  {
    plasmacore.camera_transform_stack[plasmacore.camera_transform_stack_count++] = m;
    plasmacore.camera_transform_stack_modified = true;
  }
}

void TransformManager__pop_camera_transform()
{
  SLAG_POP_REF();
  if (--plasmacore.camera_transform_stack_count < 0) plasmacore.camera_transform_stack_count = 0;
  plasmacore.camera_transform_stack_modified = true;
}

void TransformManager__push_object_transform__Transform()
{
  Matrix2x3 m = SLAG_POP(Matrix2x3);
  SLAG_POP_REF();

  if (plasmacore.object_transform_stack_count < TRANSFORM_STACK_SIZE)
  {
    plasmacore.object_transform_stack[plasmacore.object_transform_stack_count++] = m;
    plasmacore.object_transform_stack_modified = true;
  }
}

void TransformManager__pop_object_transform()
{
  SLAG_POP_REF();
  if (--plasmacore.object_transform_stack_count < 0) plasmacore.object_transform_stack_count = 0;
  plasmacore.object_transform_stack_modified = true;
}


void plasmacore_on_exit_request()
{
  SLAG_FIND_TYPE( type_application, "Application" );
  SLAG_PUSH_REF( type_application->singleton() );
  SLAG_CALL( type_application, "on_exit_request()" );
}

SlagObject* plasmacore_find_event_key( const char* event_type )
{
  SLAG_FIND_TYPE( type_event_mgr, "SignalManager" );
  SLAG_PUSH_REF( type_event_mgr->singleton() );
  SLAG_PUSH_REF( SlagString::create(event_type) );
  SLAG_CALL( type_event_mgr, "find_signal_id(String)" );
  return SLAG_POP_REF();
}

void plasmacore_consume_ws( char*& cur_line, int& count )
{
  while (count && (*cur_line == ' ' || *cur_line == '\t'))
  {
    --count;
    ++cur_line;
  }
}

void plasmacore_consume_eol ( char*& cur_line, int& count )
{
  plasmacore_consume_ws( cur_line, count );
  while (count && *cur_line == '\n')
  {
    --count;
    ++cur_line;
  }
}

void plasmacore_discard_line( char*& cur_line, int& count )
{
  while (count && *cur_line != '\n')
  {
    --count;
    ++cur_line;
  }
  plasmacore_consume_eol(cur_line,count);
}

bool plasmacore_consume_id( const char* st, char*& cur_line, int& count )
{
  plasmacore_consume_ws( cur_line, count );
  int len = strlen(st);
  if (count < len) return false;
  for (int i=0; i<len; i++)
  {
    if (cur_line[i] != st[i]) return false;
  }
  count -= len;
  cur_line += len;
  return true;
}

int plasmacore_read_int( char*& cur_line, int& count )
{
  plasmacore_consume_ws( cur_line, count );
  int result = 0;
  while (count && *cur_line>='0' && *cur_line<='9')
  {
    result = (result * 10) + ((*cur_line) - '0');
    ++cur_line;
    --count;
  }
  return result;
}

void plasmacore_load_settings()
{
  char filename_buffer[256];
  strcpy( filename_buffer, "project.properties" );
  slag_adjust_filename_for_os( filename_buffer, 256 );

  FILE* fp = fopen(filename_buffer,"rb");
  if ( !fp ) return;

  fseek( fp, 0, SEEK_END );
  int size = (int) ftell(fp);
  fseek( fp, 0, SEEK_SET );

  char* buffer = new char[size+1];
  fread( buffer, 1, size, fp );
  buffer[size] = 0;
  fclose(fp);

  char* cur_line = buffer;
  int count = size;
  while (count)
  {
    if (plasmacore_consume_id("DISPLAY_SIZE:",cur_line,count))
    {
      plasmacore.display_width = plasmacore_read_int(cur_line,count);
      plasmacore.display_height = plasmacore_read_int(cur_line,count);
    }
    else if (plasmacore_consume_id("DISPLAY_ORIENTATION:",cur_line,count))
    {
      int orientation = 0;
      if (plasmacore_consume_id("up",cur_line,count)) orientation = 0;
      else if (plasmacore_consume_id("portrait",cur_line,count)) orientation = 0;
      else if (plasmacore_consume_id("0",cur_line,count)) orientation = 0;
      else if (plasmacore_consume_id("right",cur_line,count)) orientation = 1;
      else if (plasmacore_consume_id("landscape",cur_line,count)) orientation = 1;
      else if (plasmacore_consume_id("1",cur_line,count)) orientation = 1;
      else if (plasmacore_consume_id("down",cur_line,count)) orientation = 2;
      else if (plasmacore_consume_id("2",cur_line,count)) orientation = 2;
      else if (plasmacore_consume_id("left",cur_line,count)) orientation = 3;
      else if (plasmacore_consume_id("3",cur_line,count)) orientation = 3;
      plasmacore.orientation = orientation;
    }
    else plasmacore_discard_line(cur_line,count);
  }

  delete buffer;
}



#define MCOMBINE(j,i) result.r##j##c##i = m1.r##j##c1 * m2.r1c##i \
                                        + m1.r##j##c2 * m2.r2c##i

Matrix2x3 Matrix2x3_multiply( Matrix2x3 m1, Matrix2x3 m2 )
{
  /*
  a b c * g h i
  d e f   j k l
   =
  ag+bj ah+bk ai+bl+c
  dg+ej dh+ek di+el+f
  */
  Matrix2x3 result;

  MCOMBINE(1,1);
  MCOMBINE(1,2);
  MCOMBINE(1,3)+m1.r1c3;
  MCOMBINE(2,1);
  MCOMBINE(2,2);
  MCOMBINE(2,3)+m1.r2c3;

  return result;
}

double Matrix2x3_determinant( Matrix2x3 m )
{
  return m.r1c1*m.r2c2 - m.r1c2*m.r2c1;
}

Matrix2x3 Matrix2x3_invert( Matrix2x3 m )
{
  double invdet = 1.0 / (m.r1c1*m.r2c2 - m.r1c2*m.r2c1);
  Matrix2x3 result;

  result.r1c1 =  m.r2c2 * invdet;
  result.r1c2 = -m.r1c2 * invdet;
  result.r1c3 =  (m.r1c2*m.r2c3 - m.r1c3*m.r2c2) * invdet;

  result.r2c1 = -m.r2c1 * invdet;
  result.r2c2 =  m.r1c1 * invdet;
  result.r2c3 =  (m.r1c3*m.r2c1 - m.r1c1*m.r2c3) * invdet;

  return result;
}

Vector2 Matrix2x3_transform( Matrix2x3 m, Vector2 v )
{
  Vector2 result;
  result.x = v.x * m.r1c1 + v.y * m.r1c2 + m.r1c3;
  result.y = v.x * m.r2c1 + v.y * m.r2c2 + m.r2c3;
  return result;
}

#if !defined(ANDROID)
void Application__native_set_menu_options__ArrayList_of_String()
{
  SLAG_POP_REF(); 
  SLAG_POP_REF(); 
  // no action
}
#endif

extern Archive image_archive;
void NativeLayer_init_bitmap( SlagObject* bitmap_obj, char* raw_data, int data_size );

void Bitmap__init__ArrayList_of_Byte()
{
  // Bitmap::init(Byte[])
  SlagArrayList* list = (SlagArrayList*) SLAG_POP_REF();
  SlagObject* bitmap_obj = SLAG_POP_REF();

  NativeLayer_init_bitmap( bitmap_obj, (char*)(list->array->data), list->count );
}

void Bitmap__init__String()
{
  SlagString* filename_obj = (SlagString*) SLAG_POP_REF();
  SlagObject* bitmap_obj = SLAG_POP_REF();
  int data_size;
  char* data = image_archive.load( filename_obj, &data_size );

  if (data) 
  {
    NativeLayer_init_bitmap( bitmap_obj, data, data_size );
    delete data;
  }
  else
  {
    char buffer[256];
    filename_obj->to_ascii( buffer, 256 );
    slag_throw_file_not_found_error( buffer );
  }
}

void Bitmap__rotate_right()
{
  SlagBitmap* bitmap_obj = (SlagBitmap*) SLAG_POP_REF();

  SlagInt32 w = bitmap_obj->width;
  SlagInt32 h = bitmap_obj->height;

  SlagInt32* rotate_buffer = new SlagInt32[w*h];
  SlagInt32* dest_start = rotate_buffer + h - 1;
  int di = h;
  int dj = -1;

  SlagInt32* src = (SlagInt32*) (bitmap_obj->pixels->data);
  --src;  // prepare for preincrement ahead
  for (int j=h; j>0; --j)
  {
    SlagInt32* dest = dest_start;
    for (int i=w; i>0; --i)
    {
      *(dest) = *(++src);
      dest += di;
    }
    dest_start += dj;
  }

  memcpy( bitmap_obj->pixels->data, rotate_buffer, w*h*4 );
  delete rotate_buffer;

  bitmap_obj->width = h;
  bitmap_obj->height = w;
}

void Bitmap__rotate_left()
{
  SlagBitmap* bitmap_obj = (SlagBitmap*) SLAG_POP_REF();

  SlagInt32 w = bitmap_obj->width;
  SlagInt32 h = bitmap_obj->height;

  SlagInt32* rotate_buffer = new SlagInt32[w*h];
  SlagInt32* dest_start = rotate_buffer + w*h - h;
  int di = -h;
  int dj = 1;

  SlagInt32* src = (SlagInt32*) (bitmap_obj->pixels->data);
  --src;  // prepare for preincrement ahead
  for (int j=h; j>0; --j)
  {
    SlagInt32* dest = dest_start;
    for (int i=w; i>0; --i)
    {
      *(dest) = *(++src);
      dest += di;
    }
    dest_start += dj;
  }

  memcpy( bitmap_obj->pixels->data, rotate_buffer, w*h*4 );
  delete rotate_buffer;

  bitmap_obj->width = h;
  bitmap_obj->height = w;
}

void Bitmap__rotate_180()
{
  SlagBitmap* bitmap_obj = (SlagBitmap*) SLAG_POP_REF();
  SlagInt32 w = bitmap_obj->width;
  SlagInt32 h = bitmap_obj->height;
  SlagInt32* src  = (SlagInt32*) (bitmap_obj->pixels->data);
  SlagInt32* dest = src + w*h;
  --src;

  int count = (w*h/2) + 1;
  while (--count)
  {
    SlagInt32 c = *(++src);
    *src = *(--dest);
    *dest = c;
  }
}

void Bitmap__flip_horizontal()
{
  SlagBitmap* bitmap_obj = (SlagBitmap*) SLAG_POP_REF();

  SlagInt32 w = bitmap_obj->width;
  SlagInt32 h = bitmap_obj->height;

  SlagInt32* src_start = (SlagInt32*) (bitmap_obj->pixels->data);

  int j = h + 1;
  while (--j)
  {
    SlagInt32* src = src_start - 1;
    SlagInt32* dest = src_start + w;
    int count = (w>>1) + 1;
    while (--count)
    {
      SlagInt32 c = *(++src);
      *src = *(--dest);
      *dest = c;
    }
    src_start += w;
  }
}

void Bitmap__flip_vertical()
{
  SlagBitmap* bitmap_obj = (SlagBitmap*) SLAG_POP_REF();

  SlagInt32 w = bitmap_obj->width;
  SlagInt32 h = bitmap_obj->height;

  SlagInt32* src  = ((SlagInt32*) (bitmap_obj->pixels->data)) - 1;
  SlagInt32* dest = ((SlagInt32*) (bitmap_obj->pixels->data)) + w*h - w - 1;

  int j = (h>>1) + 1;
  while (--j)
  {
    int i = w + 1;
    while (--i)
    {
      SlagInt32 c = *(++src);
      *src = *(++dest);
      *dest = c;
    }
    dest -= (w + w);
  }
}

void Bitmap__resize_horizontal__Int32()
{
  SlagInt32 new_w = SLAG_POP_INT32();
  SlagBitmap* bitmap_obj = (SlagBitmap*) SLAG_POP_REF();

  SlagInt32 w = bitmap_obj->width;
  SlagInt32 h = bitmap_obj->height;

  if (w == new_w) return;
  if (new_w <= 0) return;

  SlagArray* original_data = bitmap_obj->pixels;
  while (new_w <= w/2 && (w&1)==0)
  {
    // average every two horizontal pixels to speed up the process
    int count = w*h / 2 + 1;
    SlagInt32* src  = ((SlagInt32*) (original_data->data)) - 1;
    SlagInt32* dest  = src;
    while (--count)
    {
      int c = *(++src);
      int a = (c >> 24) & 255;
      int r = (c >> 16) & 255;
      int g = (c >>  8) & 255;
      int b = c & 255;

      c = *(++src);
      a = (a + ((c >> 24) & 255)) >> 1;
      r = (r + ((c >> 16) & 255)) >> 1;
      g = (g + ((c >>  8) & 255)) >> 1;
      b = (b + (c & 255)) >> 1;

      *(++dest) = COMBINE_ARGB(a,r,g,b);
    }
    w /= 2;
    bitmap_obj->width = w;
  }

  SLAG_PUSH_REF( (SlagObject*) original_data );
  SLAG_PUSH_REF( (SlagObject*) bitmap_obj );  // for init() call
  SLAG_PUSH_INT32( new_w );
  SLAG_PUSH_INT32( h );
  SLAG_CALL( bitmap_obj->type, "init(Int32,Int32)" );
  original_data = (SlagArray*) SLAG_POP_REF();

  SlagArray* new_data = bitmap_obj->pixels;

  SlagInt32* src  = ((SlagInt32*) (original_data->data)) - 1;
  SlagInt32* dest = ((SlagInt32*) (new_data->data)) - 1;

  double sum_a, sum_r, sum_g, sum_b;
  sum_a = sum_r = sum_g = sum_b = 0.0;
  double cur_a, cur_r, cur_g, cur_b;
  cur_a = cur_r = cur_g = cur_b = 0.0;
  double sum_weight=0.0, cur_weight=0.0;
  double progress=0.0;

  double ratio = new_w / double(w);
  int lines = h + 1;
  while (--lines)
  {
    int columns = w;
    int dest_columns = new_w;
    while (columns--)
    {
      sum_a += cur_a;
      sum_r += cur_r;
      sum_g += cur_g;
      sum_b += cur_b;
      sum_weight += cur_weight;
      int c = *(++src);
      cur_a = (c >> 24) & 255;
      cur_r = (c >> 16) & 255;
      cur_g = (c >>  8) & 255;
      cur_b = c & 255;
      progress += ratio;

      cur_weight = 1.0;

      while (progress >= 2.0 && dest_columns > 1)
      {
        sum_weight += 1.0;
        *(++dest) = (int((sum_a+cur_a)/sum_weight)<<24) 
          | (int((sum_r+cur_r)/sum_weight)<<16) 
          | (int((sum_g+cur_g)/sum_weight)<<8)
          | int((sum_b+cur_b)/sum_weight);
        --dest_columns;
        sum_a = sum_r = sum_g = sum_b = sum_weight = 0.0;
        progress -= 1.0;
        cur_weight = 1.0 - (ratio - progress);
      }

      if (progress >= 1.0 || !columns)
      {
        double cur_part = ratio - (progress - 1.0);
        if (cur_part > 1.0) cur_part = 1.0;
        double a_part = cur_a * cur_part;
        double r_part = cur_r * cur_part;
        double g_part = cur_g * cur_part;
        double b_part = cur_b * cur_part;
        cur_a -= a_part;
        cur_r -= r_part;
        cur_g -= g_part;
        cur_b -= b_part;
        sum_a += a_part;
        sum_r += r_part;
        sum_g += g_part;
        sum_b += b_part;
        sum_weight += cur_part;
        cur_weight = 1.0 - cur_part;
        progress -= 1.0;

        *(++dest) = (int(sum_a/sum_weight)<<24) 
          | (int(sum_r/sum_weight)<<16) 
          | (int(sum_g/sum_weight)<<8)
          | int(sum_b/sum_weight);
        --dest_columns;

        sum_a = sum_r = sum_g = sum_b = sum_weight = 0.0;
      }
    }
  }
}

void Bitmap__resize_vertical__Int32()
{
  SlagInt32 new_h = SLAG_POP_INT32();
  SlagBitmap* bitmap_obj = (SlagBitmap*) SLAG_POP_REF();

  SlagInt32 w = bitmap_obj->width;
  SlagInt32 h = bitmap_obj->height;

  if (h == new_h) return;
  if (new_h <= 0) return;

  SlagArray* original_data = bitmap_obj->pixels;
  while (new_h <= h/2 && (h&1)==0)
  {
    // average every two vertical pixels to speed up the process
    SlagInt32* src  = ((SlagInt32*) (original_data->data)) - 1;
    SlagInt32* dest = ((SlagInt32*) (original_data->data)) - 1;
    int lines = h/2 + 1;
    while (--lines)
    {
      int count = w + 1;
      while (--count)
      {
        int c = *(++src);
        int a = (c >> 24) & 255;
        int r = (c >> 16) & 255;
        int g = (c >>  8) & 255;
        int b = c & 255;

        c = *(src+w);
        a = (a + ((c >> 24) & 255)) >> 1;
        r = (r + ((c >> 16) & 255)) >> 1;
        g = (g + ((c >>  8) & 255)) >> 1;
        b = (b + (c & 255)) >> 1;

        *(++dest) = COMBINE_ARGB(a,r,g,b);
      }
      src += w;
    }
    h /= 2;
    bitmap_obj->height = h;
  }

  SLAG_PUSH_REF( (SlagObject*) original_data );
  SLAG_PUSH_REF( (SlagObject*) bitmap_obj );  // for init() call
  SLAG_PUSH_INT32( w );
  SLAG_PUSH_INT32( new_h );
  SLAG_CALL( bitmap_obj->type, "init(Int32,Int32)" );
  original_data = (SlagArray*) SLAG_POP_REF();

  SlagArray* new_data = bitmap_obj->pixels;

  SlagInt32* src_start  = ((SlagInt32*) (original_data->data));
  SlagInt32* dest_start = ((SlagInt32*) (new_data->data));

  double sum_a, sum_r, sum_g, sum_b;
  sum_a = sum_r = sum_g = sum_b = 0.0;
  double cur_a, cur_r, cur_g, cur_b;
  cur_a = cur_r = cur_g = cur_b = 0.0;
  double sum_weight=0.0, cur_weight=0.0;
  double progress=0.0;

  double ratio = new_h / double(h);
  int columns = w + 1;
  while (--columns)
  {
    SlagInt32* src  = src_start;
    SlagInt32* dest = dest_start;
    int rows = h;
    int dest_rows = new_h;
    while (rows--)
    {
      sum_a += cur_a;
      sum_r += cur_r;
      sum_g += cur_g;
      sum_b += cur_b;
      sum_weight += cur_weight;
      int c = *src;
      src += w;
      cur_a = (c >> 24) & 255;
      cur_r = (c >> 16) & 255;
      cur_g = (c >>  8) & 255;
      cur_b = c & 255;
      progress += ratio;

      cur_weight = 1.0;

      while (progress >= 2.0 && dest_rows > 1)
      {
        sum_weight += 1.0;
        *dest = (int((sum_a+cur_a)/sum_weight)<<24) 
          | (int((sum_r+cur_r)/sum_weight)<<16) 
          | (int((sum_g+cur_g)/sum_weight)<<8)
          | int((sum_b+cur_b)/sum_weight);
        dest += w;
        --dest_rows;
        sum_a = sum_r = sum_g = sum_b = sum_weight = 0.0;
        progress -= 1.0;
        cur_weight = 1.0 - (ratio - progress);
      }

      if (progress >= 1.0 || !rows)
      {
        double cur_part = ratio - (progress - 1.0);
        if (cur_part > 1.0) cur_part = 1.0;
        double a_part = cur_a * cur_part;
        double r_part = cur_r * cur_part;
        double g_part = cur_g * cur_part;
        double b_part = cur_b * cur_part;
        cur_a -= a_part;
        cur_r -= r_part;
        cur_g -= g_part;
        cur_b -= b_part;
        sum_a += a_part;
        sum_r += r_part;
        sum_g += g_part;
        sum_b += b_part;
        sum_weight += cur_part;
        cur_weight = 1.0 - cur_part;
        progress -= 1.0;

        *dest = (int(sum_a/sum_weight)<<24) 
          | (int(sum_r/sum_weight)<<16) 
          | (int(sum_g/sum_weight)<<8)
          | int(sum_b/sum_weight);
        dest += w;
        --dest_rows;

        sum_a = sum_r = sum_g = sum_b = sum_weight = 0.0;
      }
    }
    ++src_start;
    ++dest_start;
  }
}


void Bitmap__copy_pixels_to__Int32_Int32_Int32_Int32_Bitmap_Int32_Int32_Logical()
{
  SlagInt32 blend_alpha = SLAG_POP_INT32();
  SlagInt32 dest_y = SLAG_POP_INT32();
  SlagInt32 dest_x = SLAG_POP_INT32();
  SlagObject* dest_obj = SLAG_POP_REF();
  SlagInt32 height = SLAG_POP_INT32();
  SlagInt32 width  = SLAG_POP_INT32();
  SlagInt32 src_y  = SLAG_POP_INT32();
  SlagInt32 src_x  = SLAG_POP_INT32();
  SlagObject* src_obj = SLAG_POP_REF();

  SVM_NULL_CHECK( src_obj, return );
  SVM_NULL_CHECK( dest_obj, return );

  SLAG_GET_INT32( src_width, src_obj,   "width" );
  SLAG_GET_INT32( dest_width, dest_obj, "width" );
  SLAG_GET_REF( src_array,  src_obj,  "data" );
  SLAG_GET_REF( dest_array, dest_obj, "data" );
  SlagInt32* src_data = (SlagInt32*) ((SlagArray*)src_array)->data;
  SlagInt32* dest_data = (SlagInt32*) ((SlagArray*)dest_array)->data;

  SlagInt32 dest_skip_width = dest_width - width;
  SlagInt32 src_skip_width  = src_width - width;

  src_data  += src_y * src_width + src_x - 1;
  dest_data += dest_y * dest_width + dest_x - 1;

  if (blend_alpha)
  {
    for (int j=height; j>0; --j)
    {
      for (int i=width; i>0; --i)
      {
        int bottom = *(++dest_data);
        int top    = *(++src_data);
        int tr = (top >> 16) & 255;
        int tg = (top >> 8) & 255;
        int tb = (top & 255);
        int r = (bottom >> 16) & 255;
        int g = (bottom >> 8) & 255;
        int b = (bottom & 255);
        int inv_alpha = 255 - ((top >> 24) & 255);

        // we assume that tr, tg, and tb are premultiplied
        tr += ((r * inv_alpha) / 255);
        tg += ((g * inv_alpha) / 255);
        tb += ((b * inv_alpha) / 255);
        *dest_data = (SlagInt32) (0xff000000 | (tr<<16) | (tg<<8) | tb);
      }
      dest_data += dest_skip_width;
      src_data  += src_skip_width;
    }
  }
  else
  {
    for (int j=height; j>0; --j)
    {
      for (int i=width; i>0; --i)
      {
        *(++dest_data) = *(++src_data);
      }
      dest_data += dest_skip_width;
      src_data  += src_skip_width;
    }
  }
}

void Display__last_draw_time_ms()
{
  SLAG_POP_REF();  // singleton
  SLAG_PUSH_INT32( plasmacore.last_draw_time_ms );
}

void Display__native_scale_to_fit__Int32_Int32()
{
  SlagInt32 height = SLAG_POP_INT32();
  SlagInt32 width  = SLAG_POP_INT32();
  SLAG_POP_REF();

  double scale_x = plasmacore.display_width / (double) width;
  double scale_y = plasmacore.display_height / (double) height;

  if (scale_x < scale_y) plasmacore.scale_factor = scale_x;
  else                   plasmacore.scale_factor = scale_y;

  int scaled_width = (int)(width * plasmacore.scale_factor);
  int scaled_height = (int)(height * plasmacore.scale_factor);
  plasmacore.border_x = (plasmacore.display_width - scaled_width) / 2;
  plasmacore.border_y = (plasmacore.display_height - scaled_height) / 2;

  SLAG_PUSH( Vector2, Vector2(scaled_width,scaled_height) );
}

#if !defined(ANDROID) && TARGET_OS_IPHONE == 0
void Input__keyboard_visible__Logical()
{
  SLAG_POP_INT32();
  SLAG_POP_REF();  // discard singleton
  // no action
}

void Input__keyboard_visible()
{
  SLAG_POP_REF();  // discard singleton
  SLAG_PUSH_INT32(0);
}
#endif


extern Archive data_archive;

void ResourceManager__load_data_file__String()
{
  // ResourceManager::load_data_file(String).String
  SlagString* filename_obj = (SlagString*) SLAG_POP_REF();
  SLAG_POP_REF();   // discard singleton

  SLAG_PUSH_REF( (SlagObject*) filename_obj );

  int count;
  char* archive_data = data_archive.load( filename_obj, &count );

  filename_obj = (SlagString*) SLAG_POP_REF();

  if (archive_data)
  {
    SLAG_PUSH_REF( SlagString::create( archive_data, count ) );
    delete archive_data;
  }
  else
  {
    char buffer[256];
    buffer[0]='!';
    filename_obj->to_ascii(buffer+1,255);
    slag_throw_file_not_found_error( buffer );
  }
}

#ifndef ANDROID
void ResourceManager__load_gamestate__String()
{
  // ResourceManager::load_gamestate(String).String
  SlagString* filename_obj = (SlagString*) SLAG_POP_REF();
  SLAG_POP_REF();   // discard singleton

  char buffer[128];
  strcpy( buffer, "save/" );
  filename_obj->to_ascii( buffer+5, 128-5 );

  FILE* fp = fopen( buffer, "rb" );
  if (fp)
  {
    fseek( fp, 0, SEEK_END );
    int count = ftell(fp);
    fseek( fp, 0, SEEK_SET );

    SlagString* content = SlagString::create( count );
    SLAG_PUSH_REF( content );

    SlagChar* data = content->characters;
    --data;
    ++count;
    while (--count) *(++data) = (SlagChar) getc(fp);

    fclose(fp);

    content->set_hash_code();
  }
  else
  {
    slag_throw_file_not_found_error( buffer );
  }
}


void ResourceManager__save_gamestate__String_String()
{
  // ResourceManager::save_gamestate(String,String)
  SlagString* content = (SlagString*) SLAG_POP_REF();
  SlagString* filename_obj = (SlagString*) SLAG_POP_REF();
  SLAG_POP_REF();   // discard singleton

  char buffer[128];
  strcpy( buffer, "save/" );
  filename_obj->to_ascii( buffer+5, 128-5 );

  FILE* fp = fopen( buffer, "wb" );
  if (fp)
  {
    SlagChar* data = content->characters - 1;
    int count = content->count + 1;
    while (--count) putc( *(++data), fp );
    fclose(fp);
  }
  else
  {
    slag_throw_file_not_found_error( buffer );
    return;
  }
}

void ResourceManager__delete_gamestate__String()
{
  SlagString* filename_obj = (SlagString*) SLAG_POP_REF();
  SLAG_POP_REF();   // discard singleton

  char filepath[128];
  strcpy( filepath, "save/" );
  filename_obj->to_ascii( filepath+5, 128-5 );

#if defined(_WIN32)

  DeleteFile(filepath) || RemoveDirectory(filepath);
 
#elif defined(UNIX)
  remove(filepath);
#else
# error ResourceManager__delete_gamestate__String for this OS.
#endif
}
#endif //ANDROID

#if TARGET_OS_IPHONE == 0 && !defined(ANDROID)
void System__device_id()
{
  SLAG_PEEK_REF() = NULL;
}

#endif // not TARGET_OS_IPHONE

#if TARGET_OS_IPHONE == 0 && !defined(ANDROID)
void System__open_url__String()
{
  SLAG_POP_REF();
  SLAG_POP_REF();
  // no action
}
#endif

void SystemMonitor__log_drawing__Logical()
{
  log_drawing = SLAG_POP_INT32() > 0;
  SLAG_POP_REF(); /* singleton context */
}

void Texture__native_release()
{
  // Texture::native_release()
  SlagObject* texture_obj = SLAG_POP_REF();
  SVM_NULL_CHECK( texture_obj, return );

  SLAG_GET_REF( native_data, texture_obj, "native_data" );
  if ( !native_data ) return;
  SLAG_SET_REF( texture_obj, "native_data", NULL );

  ((SlagNativeData*)native_data)->release();
}

void Texture__set__Bitmap()
{
  SLAG_PUSH_INT32( 0 );  // dummy value for pixel format suggestion
  Texture__init__Bitmap_Int32();
}

void TransformManager__inverse__Transform()
{
  // TransformManager::op*(Transform,Transform).Transform
  Matrix2x3 m = SLAG_POP(Matrix2x3);

  SLAG_POP_REF(); /* discard singleton ref */

  SLAG_PUSH( Matrix2x3, Matrix2x3_invert(m) );
}

void TransformManager__opMUL__Transform_Transform()
{
  // TransformManager::op*(Transform,Transform).Transform
  Matrix2x3 m2 = SLAG_POP(Matrix2x3);
  Matrix2x3 m1 = SLAG_POP(Matrix2x3);

  SLAG_POP_REF(); // discard singleton ref

  SLAG_PUSH( Matrix2x3, Matrix2x3_multiply(m1,m2) );
}


void plasmacore_hook_native_methods()
{
  slag_hook_native( "Application","log(String)", Application__log__String );
  slag_hook_native( "Application","title(String)",Application__title__String);

  slag_hook_native("Bitmap","init(ArrayList<<Byte>>)", Bitmap__init__ArrayList_of_Byte );
  slag_hook_native("Bitmap","init(String)", Bitmap__init__String );
  slag_hook_native("Bitmap","to_png_bytes()", Bitmap__to_png_bytes );
  slag_hook_native("Bitmap","to_jpg_bytes(Real64)", Bitmap__to_jpg_bytes__Real64 );

  slag_hook_native( "Bitmap","copy_pixels_to(Int32,Int32,Int32,Int32,Bitmap,Int32,Int32,Logical)",
      Bitmap__copy_pixels_to__Int32_Int32_Int32_Int32_Bitmap_Int32_Int32_Logical );

  slag_hook_native("Bitmap", "rotate_right()",      Bitmap__rotate_right );
  slag_hook_native("Bitmap", "rotate_left()",       Bitmap__rotate_left );
  slag_hook_native("Bitmap", "rotate_180()",        Bitmap__rotate_180 );
  slag_hook_native("Bitmap", "flip_horizontal()",   Bitmap__flip_horizontal );
  slag_hook_native("Bitmap", "flip_vertical()",     Bitmap__flip_vertical );
  slag_hook_native("Bitmap", "resize_horizontal(Int32)", Bitmap__resize_horizontal__Int32 );
  slag_hook_native("Bitmap", "resize_vertical(Int32)", Bitmap__resize_vertical__Int32 );

  slag_hook_native("Display","flush()", Display__flush );
  slag_hook_native("Display","fullscreen()",Display__fullscreen);
  slag_hook_native("Display","fullscreen(Logical)",Display__fullscreen__Logical);
  slag_hook_native("Display","last_draw_time_ms()", Display__last_draw_time_ms );
  slag_hook_native("Display","native_set_clipping_region(Box)",
    Display__native_set_clipping_region__Box );
  slag_hook_native("Display","screen_shot(Bitmap)",Display__screen_shot__Bitmap);

  slag_hook_native("Display","native_set_draw_target(OffscreenBuffer,Logical)",
      Display__native_set_draw_target__OffscreenBuffer_Logical);
  slag_hook_native( "Display","native_scale_to_fit(Int32,Int32)",
      Display__native_scale_to_fit__Int32_Int32 );



  slag_hook_native("Input","mouse_visible(Logical)",Input__mouse_visible__Logical);
  slag_hook_native("Input","keyboard_visible(Logical)",Input__keyboard_visible__Logical);
  slag_hook_native("Input","keyboard_visible()",Input__keyboard_visible);
  slag_hook_native("Input","input_capture(Logical)",Input__input_capture__Logical);

  slag_hook_native("LineManager","draw(Line,Color,Render)",LineManager__draw__Line_Color_Render);

  slag_hook_native( "NativeSound","init(String)", NativeSound__init__String );
  slag_hook_native("NativeSound","init(ArrayList<<Byte>>)", NativeSound__init__ArrayList_of_Byte);
  slag_hook_native("NativeSound","create_duplicate()",NativeSound__create_duplicate);
  slag_hook_native("NativeSound","play()",NativeSound__play);
  slag_hook_native("NativeSound","pause()",NativeSound__pause);
  slag_hook_native("NativeSound","is_playing()",NativeSound__is_playing);
  slag_hook_native("NativeSound","volume(Real64)",NativeSound__volume__Real64);
  slag_hook_native("NativeSound","pan(Real64)",NativeSound__pan__Real64);
  slag_hook_native("NativeSound","pitch(Real64)",NativeSound__pitch__Real64);
  slag_hook_native("NativeSound","repeats(Logical)",NativeSound__repeats__Logical);
  slag_hook_native("NativeSound","current_time()",NativeSound__current_time);
  slag_hook_native("NativeSound","current_time(Real64)",NativeSound__current_time__Real64);
  slag_hook_native("NativeSound","duration()",NativeSound__duration);

  slag_hook_native( "OffscreenBuffer","clear(Color)",OffscreenBuffer__clear__Color );

  slag_hook_native("QuadManager","fill(Quad,ColorGradient,Render)",
      QuadManager__fill__Quad_ColorGradient_Render );

  slag_hook_native("ResourceManager","load_data_file(String)", 
      ResourceManager__load_data_file__String );

  slag_hook_native("ResourceManager","load_gamestate(String)", 
      ResourceManager__load_gamestate__String );
  slag_hook_native("ResourceManager","save_gamestate(String,String)", 
      ResourceManager__save_gamestate__String_String );
  slag_hook_native("ResourceManager","delete_gamestate(String)", 
      ResourceManager__delete_gamestate__String );

  slag_hook_native( "System","device_id()",System__device_id);

  slag_hook_native( "System","max_texture_size()", System__max_texture_size );
  slag_hook_native( "System","open_url(String)", System__open_url__String );
  slag_hook_native( "System","country_name()", System__country_name );

  slag_hook_native( "SystemMonitor","log_drawing(Logical)", SystemMonitor__log_drawing__Logical );

  slag_hook_native("Texture","init(Bitmap,Int32)",Texture__init__Bitmap_Int32);
#if TARGET_OS_IPHONE || defined(ANDROID)
  slag_hook_native("Texture","init(String,Int32)",Texture__init__String_Int32);
#endif
  slag_hook_native("Texture","init(Vector2,Int32)",Texture__init__Vector2_Int32 );
  slag_hook_native("Texture","init(Vector2)",      Texture__init__Vector2 );
  slag_hook_native("Texture","native_release()",Texture__native_release);
  slag_hook_native("Texture","draw(Corners,Vector2,Color,Render,Blend)",Texture__draw__Corners_Vector2_Color_Render_Blend);
  slag_hook_native("Texture","draw(Corners,Vector2,Color,Render,Blend,Texture,Corners)",
      Texture__draw__Corners_Vector2_Color_Render_Blend_Texture_Corners );
  slag_hook_native("Texture","draw(Corners,Quad,ColorGradient,Render,Blend)",Texture__draw__Corners_Quad_ColorGradient_Render_Blend);
  slag_hook_native("Texture","draw(Vector2,Vector2,Vector2,Triangle,Color,Color,Color,Render,Blend)",
      Texture__draw__Vector2_Vector2_Vector2_Triangle_Color_Color_Color_Render_Blend );
  slag_hook_native("Texture","draw_tile(Corners,Vector2,Vector2,Int32)",
      Texture__draw_tile__Corners_Vector2_Vector2_Int32 );
  slag_hook_native("Texture","set(Bitmap)",Texture__set__Bitmap);
  slag_hook_native("Texture","set(Bitmap,Vector2)",Texture__set__Bitmap_Vector2);

  slag_hook_native( 
      "TransformManager","create_from(Vector2,Vector2,Radians,Vector2,Vector2,Logical,Logical)", 
      TransformManager__create_from__Vector2_Vector2_Radians_Vector2_Vector2_Logical_Logical );
  slag_hook_native("TransformManager","current()", TransformManager__current );
  slag_hook_native("TransformManager","inverse(Transform)",      
      TransformManager__inverse__Transform );
  slag_hook_native("TransformManager","op*(Transform,Transform)",      
      TransformManager__opMUL__Transform_Transform);

  slag_hook_native( "TransformManager","push_object_transform(Transform)", 
      TransformManager__push_object_transform__Transform );
  slag_hook_native( "TransformManager","pop_object_transform()", 
      TransformManager__pop_object_transform );
  slag_hook_native( "TransformManager","push_camera_transform(Transform)", 
      TransformManager__push_camera_transform__Transform );
  slag_hook_native( "TransformManager","pop_camera_transform()", 
      TransformManager__pop_camera_transform );

  slag_hook_native("TriangleManager","fill(Triangle,Color,Color,Color,Render)",
      TriangleManager__fill__Triangle_Color_Color_Color_Render );

  slag_hook_native("Vector2Manager","draw(Vector2,Color,Render)",Vector2Manager__draw__Vector2_Color_Render);
}

