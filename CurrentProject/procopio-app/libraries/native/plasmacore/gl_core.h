#ifndef GL_CORE_H
#define GL_CORE_H
//=============================================================================
// gl_core.h
//
// 3.5.0 (2011.06.18)
//
// http://plasmaworks.com/plasmacore
// 
// Code common to the OpenGL/OpenGL ES versions of Plasmacore.
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

#include "plasmacore.h"

#if defined(__APPLE__) || defined(ANDROID)
#define SWAP_RED_AND_BLUE 1
#endif

struct GLVertex
{
  GLfloat x,y;
  GLVertex() { }
  GLVertex( float _x, float _y ) : x(_x), y(_y) {}
  void transform()
  {
    x = (x * plasmacore.scale_factor + plasmacore.border_x);
    y = (y * plasmacore.scale_factor + plasmacore.border_y);
    if (plasmacore.orientation == 1)
    {
      int temp = x;
      x = plasmacore.display_height - (y+1);
      y = temp;
    }
  }
};

struct GLTexture : SlagResource
{
  static void on_textures_lost();

  GLuint id, frame_buffer;
  int image_width;
  int image_height;
  int texture_width;
  int texture_height;
  GLTexture* next_texture;

  GLTexture( int w, int h, bool offscreen_buffer );

  void resize( int w, int h );

  ~GLTexture() { destroy(); }

  void destroy();
};

extern GLTexture* all_textures;

struct DrawBuffer
{
  GLVertex vertices[MAX_BUFFERED_VERTICES];
  GLVertex uv[MAX_BUFFERED_VERTICES];
  GLVertex alpha_uv[MAX_BUFFERED_VERTICES];
  SlagInt32    colors[MAX_BUFFERED_VERTICES];

  int draw_mode;
  int count;
  int render_flags;
  GLVertex* vertex_pos;
  GLVertex* uv_pos;
  GLVertex* alpha_uv_pos;
  SlagInt32*    color_pos;
  int constant_color;
  int src_blend, dest_blend;

  bool use_color_multiply;

  GLTexture* texture;
  GLTexture* alpha_src;
  GLTexture* draw_target;

  DrawBuffer();
  void reset();
  void set_render_flags( int flags, int src_blend, int dest_blend );
  void set_textured_triangle_mode( GLTexture* texture, GLTexture* alpha_src );
  void set_solid_triangle_mode();
  void set_line_mode();
  void set_point_mode();
  void set_draw_target( GLTexture* target );
  void add( GLVertex v1, GLVertex v2, GLVertex v3,
      SlagInt32 color1, SlagInt32 color2, SlagInt32 color3,
      GLVertex uv1, GLVertex uv2, GLVertex uv3 );
  void add( GLVertex v1, GLVertex v2, GLVertex v3,
      SlagInt32 color1, SlagInt32 color2, SlagInt32 color3,
      GLVertex uv1, GLVertex uv2, GLVertex uv3,
      GLVertex alpha_uv1, GLVertex alpha_uv2, GLVertex alpha_uv3 );
  void add( GLVertex v1, GLVertex v2, GLVertex v3,
      SlagInt32 color1, SlagInt32 color2, SlagInt32 color3 );
  void add( GLVertex v1, GLVertex v2, SlagInt32 color );
  void add( GLVertex v, SlagInt32 color );
  void add_box( double x1, double y1, double width, double height, SlagInt32 color );
  void render();
};

extern DrawBuffer draw_buffer;

#endif // GL_CORE_H

