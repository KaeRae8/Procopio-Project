//=============================================================================
// gl_core.cpp
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

#include "gl_core.h"

bool use_scissor = false;

DrawBuffer draw_buffer;
GLTexture* all_textures = NULL;

void GLTexture::resize( int w, int h )
{
  image_width  = w;
  image_height = h;
  texture_width = 1;
  while (texture_width < w) texture_width <<= 1;
  texture_height = 1;
  while (texture_height < h) texture_height <<= 1;
}

DrawBuffer::DrawBuffer()
{
  draw_mode = DRAW_TEXTURED_TRIANGLES;
  texture = NULL;
  draw_target = NULL;
  alpha_src = NULL;
  render_flags = 0;
  src_blend = BLEND_SRC_ALPHA;
  dest_blend = BLEND_INVERSE_SRC_ALPHA;
  constant_color = 0;

  reset();
}

void DrawBuffer::reset()
{
  count = 0;
  vertex_pos = vertices;
  uv_pos     = uv;
  alpha_uv_pos = alpha_uv;
  color_pos  = colors;
  use_color_multiply = false;
}

void DrawBuffer::set_render_flags( int flags, int src_blend, int dest_blend )
{
  if (render_flags != flags || this->src_blend != src_blend || this->dest_blend != dest_blend) render();
  render_flags = flags;
  this->src_blend = src_blend;
  this->dest_blend = dest_blend;
}

void DrawBuffer::set_textured_triangle_mode( GLTexture* texture, GLTexture* alpha_src )
{
  if (draw_mode != DRAW_TEXTURED_TRIANGLES || this->texture != texture || this->alpha_src != alpha_src) render();
  draw_mode = DRAW_TEXTURED_TRIANGLES;
  this->texture = texture;
  this->alpha_src = alpha_src;
}

void DrawBuffer::set_solid_triangle_mode()
{
  if (draw_mode != DRAW_SOLID_TRIANGLES) render();
  draw_mode = DRAW_SOLID_TRIANGLES;
}

void DrawBuffer::set_line_mode()
{
  if (draw_mode != DRAW_LINES) render();
  draw_mode = DRAW_LINES;
}


void DrawBuffer::set_point_mode()
{
  if (draw_mode != DRAW_POINTS) render();
  draw_mode = DRAW_POINTS;
}

void DrawBuffer::set_draw_target( GLTexture* target )
{
  if (draw_target == target) return;

  render();
  draw_target = target;

  if (draw_target) 
  {
    glBindFramebufferOES( GL_FRAMEBUFFER_OES, draw_target->frame_buffer );
    glBindTexture( GL_TEXTURE_2D, 0 );
    glViewport(0, 0, draw_target->texture_width, draw_target->texture_height );
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#if TARGET_OS_IPHONE || defined(ANDROID)
    glOrthox( 0, draw_target->texture_width<<16, 0, draw_target->texture_height<<16, -1<<16, 1<<16 );
#else
    glOrtho( 0, draw_target->texture_width, 0, draw_target->texture_height, -1, 1 );
#endif
    glMatrixMode(GL_MODELVIEW);
  }
  else 
  {
    glBindFramebufferOES( GL_FRAMEBUFFER_OES, MAIN_BUFFER );
    glViewport(0, 0, plasmacore.display_width, plasmacore.display_height );
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#if TARGET_OS_IPHONE || (ANDROID)
    glOrthox( 0, plasmacore.display_width<<16, plasmacore.display_height<<16, 0, -1<<16, 1<<16 );
#else
    glOrtho( 0, plasmacore.display_width, plasmacore.display_height, 0, -1, 1 );
#endif
    glMatrixMode(GL_MODELVIEW);
    glEnable( GL_BLEND );
  }

}

void DrawBuffer::add( GLVertex v1, GLVertex v2, GLVertex v3,
    SlagInt32 color1, SlagInt32 color2, SlagInt32 color3,
    GLVertex uv1, GLVertex uv2, GLVertex uv3 )
{
  if (count == MAX_BUFFERED_VERTICES) render();

  if ((render_flags & RENDER_FLAG_FIXED_COLOR) && constant_color != color1)
  {
    render();
    constant_color = color1;
  }

  vertex_pos[0] = v1;
  vertex_pos[1] = v2;
  vertex_pos[2] = v3;
  vertex_pos += 3;
  color_pos[0] = color1;
  color_pos[1] = color2;
  color_pos[2] = color3;
  color_pos += 3;
  if (color1 != -1 || color2 != -1 || color3 != -1) use_color_multiply = true;
  uv_pos[0] = uv1;
  uv_pos[1] = uv2;
  uv_pos[2] = uv3;
  uv_pos += 3;
  count += 3;
}

void DrawBuffer::add( GLVertex v1, GLVertex v2, GLVertex v3,
    SlagInt32 color1, SlagInt32 color2, SlagInt32 color3,
    GLVertex uv1, GLVertex uv2, GLVertex uv3,
    GLVertex alpha_uv1, GLVertex alpha_uv2, GLVertex alpha_uv3 )
{
  if (count == MAX_BUFFERED_VERTICES) render();

  if ((render_flags & RENDER_FLAG_FIXED_COLOR) && constant_color != color1)
  {
    render();
    constant_color = color1;
  }

  vertex_pos[0] = v1;
  vertex_pos[1] = v2;
  vertex_pos[2] = v3;
  vertex_pos += 3;
  color_pos[0] = color1;
  color_pos[1] = color2;
  color_pos[2] = color3;
  color_pos += 3;
  if (color1 != -1 || color2 != -1 || color3 != -1) use_color_multiply = true;
  uv_pos[0] = uv1;
  uv_pos[1] = uv2;
  uv_pos[2] = uv3;
  uv_pos += 3;
  alpha_uv_pos[0] = alpha_uv1;
  alpha_uv_pos[1] = alpha_uv2;
  alpha_uv_pos[2] = alpha_uv3;
  alpha_uv_pos += 3;
  count += 3;
}


void DrawBuffer::add( GLVertex v1, GLVertex v2, GLVertex v3,
    SlagInt32 color1, SlagInt32 color2, SlagInt32 color3 )
{
  if (count == MAX_BUFFERED_VERTICES) render();
  vertex_pos[0] = v1;
  vertex_pos[1] = v2;
  vertex_pos[2] = v3;
  vertex_pos += 3;
  color_pos[0] = color1;
  color_pos[1] = color2;
  color_pos[2] = color3;
  color_pos += 3;
  if (color1 != -1 || color2 != -1 || color3 != -1) use_color_multiply = true;
  count += 3;
}

void DrawBuffer::add( GLVertex v1, GLVertex v2, SlagInt32 color )
{
  if (count == MAX_BUFFERED_VERTICES) render();
  vertex_pos[0] = v1;
  vertex_pos[1] = v2;
  vertex_pos += 2;
  color_pos[0] = color;
  color_pos[1] = color;
  color_pos += 2;
  if (color != -1) use_color_multiply = true;
  count += 2;
}

void DrawBuffer::add( GLVertex v, SlagInt32 color )
{
  if (count == MAX_BUFFERED_VERTICES) render();
  *(vertex_pos++) = v;
  *(color_pos++) = color;
  if (color != -1) use_color_multiply = true;
  ++count;
}

void DrawBuffer::add_box( double x1, double y1, double width, double height, SlagInt32 color )
{
  set_draw_target( NULL );
  set_render_flags( 0, BLEND_ONE, BLEND_ZERO );
  set_solid_triangle_mode();
  double x2 = x1 + width;
  double y2 = y1 + height;
  GLVertex v1( x1, y1 );
  GLVertex v2( x2, y1 );
  GLVertex v3( x2, y2 );
  GLVertex v4( x1, y2 );
  add( v1, v2, v4, color, color, color );
  add( v4, v2, v3, color, color, color );
}

void DrawBuffer::render()
{
  if ( !count ) return;

  glEnableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);

  GLenum src_factor, dest_factor;
  switch (src_blend)
  {
    case BLEND_ZERO:
      src_factor = GL_ZERO;
      break;
    case BLEND_ONE:
      src_factor = GL_ONE;
      break;
    case BLEND_SRC_ALPHA:
      src_factor = GL_SRC_ALPHA;
      break;
    case BLEND_INVERSE_SRC_ALPHA:
      src_factor = GL_ONE_MINUS_SRC_ALPHA;
      break;
    case BLEND_DEST_ALPHA:
      src_factor = GL_DST_ALPHA;
      break;
    case BLEND_INVERSE_DEST_ALPHA:
      src_factor = GL_ONE_MINUS_DST_ALPHA;
      break;
    case BLEND_DEST_COLOR:
      src_factor = GL_DST_COLOR;
      break;
    case BLEND_INVERSE_DEST_COLOR:
      src_factor = GL_ONE_MINUS_DST_COLOR;
      break;
    case BLEND_OPAQUE:
      src_factor = GL_SRC_ALPHA_SATURATE;
      break;
    default:
      src_factor = GL_SRC_ALPHA;
  }
  switch (dest_blend)
  {
    case BLEND_ZERO:
      dest_factor = GL_ZERO;
      break;
    case BLEND_ONE:
      dest_factor = GL_ONE;
      break;
    case BLEND_SRC_ALPHA:
      dest_factor = GL_SRC_ALPHA;
      break;
    case BLEND_INVERSE_SRC_ALPHA:
      dest_factor = GL_ONE_MINUS_SRC_ALPHA;
      break;
    case BLEND_DEST_ALPHA:
      dest_factor = GL_DST_ALPHA;
      break;
    case BLEND_INVERSE_DEST_ALPHA:
      dest_factor = GL_ONE_MINUS_DST_ALPHA;
      break;
    case BLEND_SRC_COLOR:
      dest_factor = GL_SRC_COLOR;
      break;
    case BLEND_INVERSE_SRC_COLOR:
      dest_factor = GL_ONE_MINUS_SRC_COLOR;
      break;
    default:
      dest_factor = GL_ONE_MINUS_SRC_ALPHA;
  }

  glBlendFunc( src_factor, dest_factor );

  if ((render_flags & RENDER_FLAG_OPAQUE)) 
  {
    glBlendFunc( GL_ONE, GL_ZERO );
    glDisableClientState(GL_COLOR_ARRAY);
  }

  switch (draw_mode)
  {
    case DRAW_TEXTURED_TRIANGLES:
      // set colorization flags
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);

      glActiveTexture( GL_TEXTURE0 );
      glEnable(GL_TEXTURE_2D);
      glBindTexture( GL_TEXTURE_2D, texture->id );

      if (render_flags & RENDER_FLAG_FIXED_COLOR)
      {
        GLfloat env_color[4] = {
          ((constant_color) & 255)/255.0f, 
          ((constant_color >> 8) & 255)/255.0f, 
          ((constant_color >> 16) & 255)/255.0f,
          ((constant_color >> 24) & 255)/255.0f };
        glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, env_color );
        glTexEnvf( GL_TEXTURE_ENV, GL_SRC0_RGB, GL_CONSTANT); 
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
      }
      else
      {
        glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
      }

      // set texture addressing to WRAP or CLAMP
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

      if (alpha_src)
      {
        glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
        glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);

        glActiveTexture( GL_TEXTURE1 );
        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, alpha_src->id );
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // grab color from first stage
        glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE );
        glTexEnvf( GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS );

        // use the alpha from second stage
        glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE );
        glTexEnvf( GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE );

        glActiveTexture( GL_TEXTURE0 );

      }
      else
      {
        glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE );
        glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE );
        glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE );
      }

      if ((render_flags & RENDER_FLAG_OPAQUE)) 
      {
        glDisable( GL_BLEND );
        glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE );
        glTexEnvf( GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE );
        glTexEnvf( GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE );
        glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE );
      }
      else if ( !use_color_multiply )
      {
        glTexEnvf( GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE );
        glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE );
      }

      glDrawArrays(GL_TRIANGLES, 0, count);
      glDisable(GL_TEXTURE_2D);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);

      if (alpha_src)
      {
        glActiveTexture( GL_TEXTURE1 );
        glDisable( GL_TEXTURE_2D );
        glActiveTexture( GL_TEXTURE0 );
      }

      glEnable( GL_BLEND );

      break;

    case DRAW_SOLID_TRIANGLES:
      glDrawArrays(GL_TRIANGLES, 0, count);
      break;

    case DRAW_LINES:
      glDrawArrays( GL_LINES, 0, count );
      break;

    case DRAW_POINTS:
      glDrawArrays( GL_POINTS, 0, count );
      break;
  }
  glDisableClientState(GL_COLOR_ARRAY);

  if (draw_target) 
  {
    // flushes offscreen buffer drawing queue
    glBindFramebufferOES( GL_FRAMEBUFFER_OES, MAIN_BUFFER ); 
    glBindFramebufferOES( GL_FRAMEBUFFER_OES, draw_target->frame_buffer );
  }

  reset();
}

void Display__screen_shot__Bitmap()
{
  // Application::screen_shot(Bitmap=null).Bitmap
  SlagBitmap* bitmap_obj = (SlagBitmap*) SLAG_POP_REF();
  SLAG_POP_REF();

  draw_buffer.render();

  SLAG_FIND_TYPE( type_bitmap, "Bitmap" );
  if ( !bitmap_obj )
  {
    bitmap_obj = (SlagBitmap*) type_bitmap->create();
  }

  int w = plasmacore.display_width;
  int h = plasmacore.display_height;

  SLAG_PUSH_REF( (SlagObject*) bitmap_obj );  // method result
  SLAG_PUSH_REF( (SlagObject*) bitmap_obj );  // for init(Int32,Int32)
  SLAG_PUSH_INT32( w );
  SLAG_PUSH_INT32( h );
  SLAG_CALL( type_bitmap, "init(Int32,Int32)" );  // only reallocates if wrong size

  bitmap_obj = (SlagBitmap*) SLAG_PEEK_REF();  // reset ref in case a gc happened

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  glReadPixels( 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, bitmap_obj->pixels->data );

  // reverse row order and maybe Red/Blue values, premultiply alpha
  SlagInt32* src_start = ((SlagInt32*) bitmap_obj->pixels->data);
  SlagInt32* dest_start = src_start + (w*(h-1));
  while (src_start <= dest_start)
  {
    SlagInt32* src  = src_start - 1;
    SlagInt32* dest = dest_start - 1;
    int c = w + 1;
    while (--c)
    {
      SlagInt32 color1 = *(++src);
      int r1 = (color1 >> 16) & 255;
      int g1 = (color1 >> 8) & 255;
      int b1 = color1 & 255;

      SlagInt32 color2 = *(++dest);
      int r2 = (color2 >> 16) & 255;
      int g2 = (color2 >> 8) & 255;
      int b2 = color2 & 255;

#if defined(ANDROID) || TARGET_OS_IPHONE
      *src  = (0xff000000) | (b2 << 16) | (g2 << 8) | r2;
      *dest = (0xff000000) | (b1 << 16) | (g1 << 8) | r1;
#else
      *src  = (0xff000000) | (r2 << 16) | (g2 << 8) | b2;
      *dest = (0xff000000) | (r1 << 16) | (g1 << 8) | b1;
#endif
    }
    src_start  += w;
    dest_start -= w;
  }

  if (plasmacore.border_x > 0 || plasmacore.border_y > 0)
  {
    // repack data to eliminate borders
    SlagInt32* dest = ((SlagInt32*) bitmap_obj->pixels->data) - 1;
    SlagInt32* src  = dest + plasmacore.border_y*w + plasmacore.border_x;
    int skip_cols = plasmacore.border_x * 2;
    int keep_cols = plasmacore.display_width - skip_cols;
    bitmap_obj->width = keep_cols;
    bitmap_obj->height = (plasmacore.display_height - (plasmacore.border_y*2));
    int rows = bitmap_obj->height + 1;
    while (--rows)
    {
      int cols = keep_cols + 1;
      while (--cols)
      {
        *(++dest) = *(++src);
      }
      src += skip_cols;
    }
  }

  if (plasmacore.scale_factor != 1.0)
  {
    SLAG_DUPLICATE_REF();
    SLAG_PUSH_INT32( plasmacore.display_width/plasmacore.scale_factor );
    Bitmap__resize_horizontal__Int32();

    SLAG_DUPLICATE_REF();
    SLAG_PUSH_INT32( plasmacore.display_height );
    Bitmap__resize_vertical__Int32();
  }
}

void Display__native_set_clipping_region__Box()
{
  // Application::native_set_clipping_region(Box)
  Vector2 top_left = SLAG_POP(Vector2);
  Vector2 size = SLAG_POP(Vector2);
  SLAG_POP_REF();  // discard singleton

  Vector2 bottom_right( top_left.x+size.x, top_left.y+size.y );

  if (plasmacore_set_transform())
  {
    top_left = Matrix2x3_transform( plasmacore.transform, top_left );
    bottom_right = Matrix2x3_transform( plasmacore.transform, bottom_right );
  }

  double x1 = top_left.x;
  double y1 = top_left.y;
  double x2 = bottom_right.x;
  double y2 = bottom_right.y;

  draw_buffer.render();

  GLint ix1 = (GLint)((x1*plasmacore.scale_factor+plasmacore.border_x)+0.5);
  GLint iy1 = (GLint)((y1*plasmacore.scale_factor+plasmacore.border_y)+0.5);
  GLint ix2 = (GLint)((x2*plasmacore.scale_factor+plasmacore.border_x)+0.5);
  GLint iy2 = (GLint)((y2*plasmacore.scale_factor+plasmacore.border_y)+0.5);

  if (plasmacore.orientation == 1)
  {
    int original_ix1 = ix1;
    int original_ix2 = ix2;
    ix2 = plasmacore.display_height - (iy1+1);
    iy1 = original_ix1;

    ix1 = plasmacore.display_height - (iy2+1);
    iy2 = original_ix2;
  }


  if (ix1 < 0) ix1 = 0;
  if (iy1 < 0) iy1 = 0;

  if (plasmacore.orientation == 1)
  {
    if (ix2 > plasmacore.display_height) ix2 = plasmacore.display_height;
    if (iy2 > plasmacore.display_width)  iy2 = plasmacore.display_width;

    glScissor( ix1, plasmacore.display_width-iy2, ix2-ix1, iy2-iy1 );
  }
  else
  {
    if (ix2 > plasmacore.display_width)  ix2 = plasmacore.display_width;
    if (iy2 > plasmacore.display_height) iy2 = plasmacore.display_height;

    glScissor( ix1, plasmacore.display_height-iy2, ix2-ix1, iy2-iy1 );
  }
  use_scissor = true;
  glEnable( GL_SCISSOR_TEST );
}

void Display__flush()
{
  SLAG_POP_REF();  // singleton
  draw_buffer.render();
}

GLTexture* NativeLayer_get_native_texture_data( SlagObject* texture_obj )
{
  SLAG_GET_REF( native_data, texture_obj, "native_data" );
  if ( !native_data ) return NULL;

  return (GLTexture*) (((SlagNativeData*)native_data)->data);
}

void Display__native_set_draw_target__OffscreenBuffer_Logical()
{
  // Application::native_set_draw_target(OffscreenBuffer,Logical)
  SlagInt32 blend = SLAG_POP_INT32();
  SlagObject* image_obj = SLAG_POP_REF();
  SLAG_POP_REF();  // discard singleton

  draw_buffer.render();

  if ( !image_obj )
  {
    draw_buffer.set_draw_target( NULL );
    draw_buffer.render();
    return;
  }

  SLAG_GET_REF( texture_obj, image_obj, "texture" );
  SVM_NULL_CHECK( texture_obj, return );

  GLTexture* target = NativeLayer_get_native_texture_data(texture_obj);
  draw_buffer.set_draw_target( target );

  if (blend) glEnable( GL_BLEND );
  else       glDisable(GL_BLEND);

  use_scissor = false;
  glDisable( GL_SCISSOR_TEST );
}

void LineManager__draw__Line_Color_Render()
{
  SlagInt32 render_flags = SLAG_POP_INT32();
  SlagInt32 color  = SLAG_POP_INT32();
  Vector2 pt1 = SLAG_POP(Vector2); 
  Vector2 pt2 = SLAG_POP(Vector2); 

  SLAG_POP_REF(); // discard singleton ref 

  if (plasmacore_set_transform())
  {
    pt1 = Matrix2x3_transform( plasmacore.transform, pt1 );
    pt2 = Matrix2x3_transform( plasmacore.transform, pt2 );
  }

  color = argb_to_rgba( color );
  draw_buffer.set_render_flags( render_flags, BLEND_SRC_ALPHA, BLEND_INVERSE_SRC_ALPHA );
  draw_buffer.set_line_mode();

  GLVertex v1(pt1.x, pt1.y);
  GLVertex v2(pt2.x, pt2.y);
  v1.transform();
  v2.transform();
  draw_buffer.add( v1, v2, color );
}

void QuadManager__fill__Quad_ColorGradient_Render()
{
  SlagInt32 render_flags = SLAG_POP_INT32();
  SlagInt32 color1 = SLAG_POP_INT32();
  SlagInt32 color2 = SLAG_POP_INT32();
  SlagInt32 color3 = SLAG_POP_INT32();
  SlagInt32 color4 = SLAG_POP_INT32();
  Vector2 pt1 = SLAG_POP(Vector2); 
  Vector2 pt2 = SLAG_POP(Vector2); 
  Vector2 pt3 = SLAG_POP(Vector2); 
  Vector2 pt4 = SLAG_POP(Vector2); 

  SLAG_POP_REF(); // discard singleton ref 

  if (plasmacore_set_transform())
  {
    pt1 = Matrix2x3_transform( plasmacore.transform, pt1 );
    pt2 = Matrix2x3_transform( plasmacore.transform, pt2 );
    pt3 = Matrix2x3_transform( plasmacore.transform, pt3 );
    pt4 = Matrix2x3_transform( plasmacore.transform, pt4 );
  }

  color1 = argb_to_rgba( color1 );
  color2 = argb_to_rgba( color2 );
  color3 = argb_to_rgba( color3 );
  color4 = argb_to_rgba( color4 );

  draw_buffer.set_render_flags( render_flags, BLEND_SRC_ALPHA, BLEND_INVERSE_SRC_ALPHA );
  draw_buffer.set_solid_triangle_mode();

  GLVertex v1( pt1.x, pt1.y );
  GLVertex v2( pt2.x, pt2.y );
  GLVertex v3( pt3.x, pt3.y );
  GLVertex v4( pt4.x, pt4.y );

  v1.transform();
  v2.transform();
  v3.transform();
  v4.transform();

  draw_buffer.add( v1, v2, v4, color1, color2, color4 );
  draw_buffer.add( v4, v2, v3, color4, color2, color3 );
}

void System__max_texture_size()
{
  // System.max_texture_size().Vector2
  SLAG_POP_REF();  // System singleton

  GLint tex_size; 
  glGetIntegerv( GL_MAX_TEXTURE_SIZE, &tex_size );
  SLAG_PUSH_REAL64( tex_size );
  SLAG_PUSH_REAL64( tex_size );

  // android is 1024x1024
}


void TriangleManager__fill__Triangle_Color_Color_Color_Render()
{
  SlagInt32 render_flags = SLAG_POP_INT32();
  SlagInt32 color3 = SLAG_POP_INT32();
  SlagInt32 color2 = SLAG_POP_INT32();
  SlagInt32 color1 = SLAG_POP_INT32();
  Vector2 pt1 = SLAG_POP(Vector2); 
  Vector2 pt2 = SLAG_POP(Vector2); 
  Vector2 pt3 = SLAG_POP(Vector2); 

  SLAG_POP_REF(); // discard singleton ref 

  if (plasmacore_set_transform())
  {
    pt1 = Matrix2x3_transform( plasmacore.transform, pt1 );
    pt2 = Matrix2x3_transform( plasmacore.transform, pt2 );
    pt3 = Matrix2x3_transform( plasmacore.transform, pt3 );
  }

  color1 = argb_to_rgba( color1 );
  color2 = argb_to_rgba( color2 );
  color3 = argb_to_rgba( color3 );

  draw_buffer.set_render_flags( render_flags, BLEND_SRC_ALPHA, BLEND_INVERSE_SRC_ALPHA );
  draw_buffer.set_solid_triangle_mode();

  GLVertex v1( pt1.x, pt1.y );
  GLVertex v2( pt2.x, pt2.y );
  GLVertex v3( pt3.x, pt3.y );

  v1.transform();
  v2.transform();
  v3.transform();

  draw_buffer.add( v1, v2, v3, color1, color2, color3 );
}

void Texture_init( SlagInt32* data, int w, int h, int format )
{
  // Helper fn
  SlagObject* texture_obj = SLAG_POP_REF();

  if (w==0 || h==0) return;

  static int prior_format = 0;

  if (format != prior_format)
  {
    prior_format = format;
    if (format == 1)
    {
      LOG( "Loading 32-bit textures." );
    }
    else
    {
      LOG( "Loading 16-bit textures." );
    }
  }

  if (format == 1)
  {
    // 32-bit

    int wp2 = 1;
    int hp2 = 1;
    int made_copy = 0;
    while (wp2 < w) wp2 <<= 1;
    while (hp2 < h) hp2 <<= 1;

    SlagInt32* processed_data;
    if ((wp2 > w || hp2 > h) && data)
    {
      made_copy = 1;
      processed_data = (SlagInt32*) new char[ wp2 * hp2 * 4 ];
      int i, j;
      int src_index = 0;
      int dest_index = 0;
      for (j=0; j<h; ++j)
      {
        int color = 0;
        for (i=0; i<w; ++i) 
        {
          processed_data[dest_index++] = color = data[src_index++];
        }
        for (i=w; i<wp2; ++i) processed_data[dest_index++] = color;
      }
      src_index = (h-1)*wp2;
      for (j=h; j<hp2; ++j)
      {
        for (i=0; i<wp2; ++i) processed_data[dest_index++] = processed_data[src_index++];
        src_index -= wp2;
      }

      data = processed_data;
    }

    // this method is also used to assign new data to an existing texture
    GLTexture* texture = NativeLayer_get_native_texture_data( texture_obj );
    if ( !texture )
    {
      texture = new GLTexture(w,h,false);

      //SlagLocalRef gc_guard(texture_obj);
      SlagObject* data_obj = SlagNativeData::create( texture, SlagNativeDataDeleteResource );

      SLAG_SET_REF( texture_obj, "native_data", data_obj );

      SLAG_SET( Vector2, texture_obj, "texture_size", Vector2(wp2,hp2) );
      SLAG_SET( Vector2, texture_obj, "image_size", Vector2(w,h) );
    }

    glBindTexture( GL_TEXTURE_2D, texture->id );
    glActiveTexture( GL_TEXTURE0 );

#ifdef SWAP_RED_AND_BLUE
    if (data) swap_red_and_blue( data, wp2*hp2 );
#endif

    // Define the OpenGL texture
    texture->resize( w, h );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, wp2, hp2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );

    if (made_copy)
    {
      delete processed_data;
    }
#ifdef SWAP_RED_AND_BLUE
    else
    {
      if (data) swap_red_and_blue( data, wp2*hp2 );
    }
#endif
  }
  else
  {
    // 16-bit
    int wp2 = 1;
    int hp2 = 1;
    while (wp2 < w) wp2 <<= 1;
    while (hp2 < h) hp2 <<= 1;

    SlagChar* processed_data = NULL;
    if (data)
    {
      processed_data = (SlagChar*) new char[ wp2 * hp2 * 2 ];
      int i, j;
      int src_index = 0;
      int dest_index = 0;
      for (j=0; j<h; ++j)
      {
        int color = 0;
        for (i=0; i<w; ++i) 
        {
          color = data[src_index++];
          int a = (color >> 24) & 255;
          int r = (color >> 16) & 255;
          int g = (color >> 8) & 255;
          int b = (color) & 255;
          r >>= 4;
          g >>= 4;
          b >>= 4;
          a >>= 4;
          color = (r<<12) | (g<<8) | (b<<4) | (a);
          processed_data[dest_index++] = color;
        }
        for (i=w; i<wp2; ++i) processed_data[dest_index++] = color;
      }
      src_index = (h-1)*wp2;
      for (j=h; j<hp2; ++j)
      {
        for (i=0; i<wp2; ++i) processed_data[dest_index++] = processed_data[src_index++];
        src_index -= wp2;
      }
    }

    // this method is also used to assign new data to an existing texture
    GLTexture* texture = NativeLayer_get_native_texture_data( texture_obj );
    if ( !texture )
    {
      texture = new GLTexture(w,h,false);

      //BardLocalRef gc_guard(texture_obj);
      SlagObject* data_obj = SlagNativeData::create( texture, SlagNativeDataDeleteResource );

      SLAG_SET_REF( texture_obj, "native_data", data_obj );

      SLAG_SET( Vector2, texture_obj, "texture_size", Vector2(wp2,hp2) );
      SLAG_SET( Vector2, texture_obj, "image_size", Vector2(w,h) );
    }

    glBindTexture( GL_TEXTURE_2D, texture->id );
    glActiveTexture( GL_TEXTURE0 );

    // Define the OpenGL texture
    texture->resize( w, h );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, wp2, hp2, 0, 
        GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4,  processed_data );

    delete processed_data;
  }
}

void Texture__init__Bitmap_Int32()
{
  // Texture::init(Bitmap,Int32)
  static int prior_format = 0;

  int pixel_format = SLAG_POP_INT32();
  if (pixel_format != prior_format)
  {
    prior_format = pixel_format;
    if (pixel_format == 1)
    {
      LOG( "Loading 32-bit textures." );
    }
    else
    {
      LOG( "Loading 16-bit textures." );
    }
  }

  if (pixel_format == 1)
  {
    // 32-bit
    SlagObject* bitmap_obj = SLAG_POP_REF();
    SlagObject* texture_obj = SLAG_POP_REF();

    SLAG_GET_REF( array, bitmap_obj, "data" );
    SlagInt32* data = (SlagInt32*) (((SlagArray*)array)->data);

    SLAG_GET_INT32( w, bitmap_obj, "width" );
    SLAG_GET_INT32( h, bitmap_obj, "height" );
    if (w==0 || h==0) return;
    int wp2 = 1;
    int hp2 = 1;
    int made_copy = 0;
    while (wp2 < w) wp2 <<= 1;
    while (hp2 < h) hp2 <<= 1;

    SlagInt32* processed_data;
    if (wp2 > w || hp2 > h)
    {
      made_copy = 1;
      processed_data = (SlagInt32*) new char[ wp2 * hp2 * 4 ];
      int i, j;
      int src_index = 0;
      int dest_index = 0;
      for (j=0; j<h; ++j)
      {
        int color = 0;
        for (i=0; i<w; ++i) 
        {
          processed_data[dest_index++] = color = data[src_index++];
        }
        for (i=w; i<wp2; ++i) processed_data[dest_index++] = color;
      }
      src_index = (h-1)*wp2;
      for (j=h; j<hp2; ++j)
      {
        for (i=0; i<wp2; ++i) processed_data[dest_index++] = processed_data[src_index++];
        src_index -= wp2;
      }

      data = processed_data;
    }

    // this method is also used to assign new data to an existing texture
    GLTexture* texture = NativeLayer_get_native_texture_data( texture_obj );
    if ( !texture )
    {
      texture = new GLTexture(w,h,false);

      SlagLocalRef gc_guard(texture_obj);
      SlagObject* data_obj = SlagNativeData::create( texture, SlagNativeDataDeleteResource );

      SLAG_SET_REF( texture_obj, "native_data", data_obj );

      SLAG_SET( Vector2, texture_obj, "texture_size", Vector2(wp2,hp2) );
      SLAG_SET( Vector2, texture_obj, "image_size", Vector2(w,h) );
    }

    glBindTexture( GL_TEXTURE_2D, texture->id );
    glActiveTexture( GL_TEXTURE0 );

#ifdef SWAP_RED_AND_BLUE
    swap_red_and_blue( data, wp2*hp2 );
#endif

    // Define the OpenGL texture
    texture->resize( w, h );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, wp2, hp2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );

    if (made_copy)
    {
      delete processed_data;
    }
#ifdef SWAP_RED_AND_BLUE
    else
    {
    swap_red_and_blue( data, wp2*hp2 );
    }
#endif
  }
  else
  {
    // 16-bit
    SlagObject* bitmap_obj = SLAG_POP_REF();
    SlagObject* texture_obj = SLAG_POP_REF();

    SLAG_GET_REF( array, bitmap_obj, "data" );
    SlagInt32* data = (SlagInt32*) (((SlagArray*)array)->data);

    SLAG_GET_INT32( w, bitmap_obj, "width" );
    SLAG_GET_INT32( h, bitmap_obj, "height" );
    if (w==0 || h==0) return;
    int wp2 = 1;
    int hp2 = 1;
    while (wp2 < w) wp2 <<= 1;
    while (hp2 < h) hp2 <<= 1;

    SlagInt16* processed_data;
    {
      processed_data = (SlagInt16*) new char[ wp2 * hp2 * 2 ];
      int i, j;
      int src_index = 0;
      int dest_index = 0;
      for (j=0; j<h; ++j)
      {
        int color = 0;
        for (i=0; i<w; ++i) 
        {
          color = data[src_index++];
          int a = (color >> 24) & 255;
          int r = (color >> 16) & 255;
          int g = (color >> 8) & 255;
          int b = (color) & 255;
          r >>= 4;
          g >>= 4;
          b >>= 4;
          a >>= 4;
          color = (r<<12) | (g<<8) | (b<<4) | (a);
          processed_data[dest_index++] = color;
        }
        for (i=w; i<wp2; ++i) processed_data[dest_index++] = color;
      }
      src_index = (h-1)*wp2;
      for (j=h; j<hp2; ++j)
      {
        for (i=0; i<wp2; ++i) processed_data[dest_index++] = processed_data[src_index++];
        src_index -= wp2;
      }
    }

    // this method is also used to assign new data to an existing texture
    GLTexture* texture = NativeLayer_get_native_texture_data( texture_obj );
    if ( !texture )
    {
      texture = new GLTexture(w,h,false);

      SlagLocalRef gc_guard(texture_obj);
      SlagObject* data_obj = SlagNativeData::create( texture, SlagNativeDataDeleteResource );

      SLAG_SET_REF( texture_obj, "native_data", data_obj );

      SLAG_SET( Vector2, texture_obj, "texture_size", Vector2(wp2,hp2) );
      SLAG_SET( Vector2, texture_obj, "image_size", Vector2(w,h) );
    }

    glBindTexture( GL_TEXTURE_2D, texture->id );
    glActiveTexture( GL_TEXTURE0 );

    // Define the OpenGL texture
    texture->resize( w, h );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, wp2, hp2, 0, 
        GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4,  processed_data );

    delete processed_data;
  }
}

void Texture__init__Vector2_Int32()
{
  // Texture::init(Vector2,Int32)
  int pixel_format = SLAG_POP_INT32();
  int w = (int) SLAG_POP_REAL64();
  int h = (int) SLAG_POP_REAL64();
  SlagObject* texture_obj = SLAG_POP_REF();

  if (w==0 || h==0) return;
  int wp2 = 1;
  int hp2 = 1;
  while (wp2 < w) wp2 <<= 1;
  while (hp2 < h) hp2 <<= 1;

  if (pixel_format == 1)
  {
    // 32-bit
    GLTexture* texture = new GLTexture(w,h,false);

    SlagLocalRef gc_guard(texture_obj);
    SlagObject* data_obj = SlagNativeData::create( texture, SlagNativeDataDeleteResource );

    SLAG_SET_REF( texture_obj, "native_data", data_obj );

    SLAG_SET( Vector2, texture_obj, "texture_size", Vector2(wp2,hp2) );
    SLAG_SET( Vector2, texture_obj, "image_size", Vector2(w,h) );

    glBindTexture( GL_TEXTURE_2D, texture->id );
    glActiveTexture( GL_TEXTURE0 );

    // Define the OpenGL texture
    texture->resize( w, h );

    char* buffer = new char[ wp2 * hp2 * 4 ];
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, wp2, hp2, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
    delete buffer;
  }
  else
  {
    // 16-bit
    GLTexture* texture = new GLTexture(w,h,false);

    SlagLocalRef gc_guard(texture_obj);
    SlagObject* data_obj = SlagNativeData::create( texture, SlagNativeDataDeleteResource );

    SLAG_SET_REF( texture_obj, "native_data", data_obj );

    SLAG_SET( Vector2, texture_obj, "texture_size", Vector2(wp2,hp2) );
    SLAG_SET( Vector2, texture_obj, "image_size", Vector2(w,h) );

    glBindTexture( GL_TEXTURE_2D, texture->id );
    glActiveTexture( GL_TEXTURE0 );

    // Define the OpenGL texture
    texture->resize( w, h );

    char* buffer = new char[ wp2 * hp2 * 2 ];
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, wp2, hp2, 0, 
        GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4,  buffer );
    delete buffer;
  }
}

void Texture__init__Vector2()
{
  // Texture::init(Vector2)
  Vector2 size = SLAG_POP(Vector2);
  SlagObject* texture_obj = SLAG_POP_REF();

  SVM_NULL_CHECK( texture_obj, return );

  int w = (int) size.x;
  int h = (int) size.y;
	if (w==0 || h==0) return;

  GLTexture* texture = new GLTexture(w,h,true);

  SlagLocalRef gc_guard(texture_obj);
  SlagObject* data_obj = SlagNativeData::create( texture, SlagNativeDataDeleteResource );

  SLAG_SET_REF( texture_obj, "native_data", data_obj );

  int wp2 = texture->texture_width;
  int hp2 = texture->texture_height;

  SLAG_SET( Vector2, texture_obj, "texture_size", Vector2(wp2,hp2) );
  SLAG_SET( Vector2, texture_obj, "image_size", Vector2(w,h) );

  glBindTexture( GL_TEXTURE_2D, texture->id );

  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, wp2, hp2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
  glFramebufferTexture2DOES( GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, texture->id, 0 );
  glEnable( GL_BLEND );

  glDisable( GL_SCISSOR_TEST );
  glClearColor( 0, 0, 0, 0 );
  glClear(GL_COLOR_BUFFER_BIT);
  if (use_scissor) glEnable( GL_SCISSOR_TEST );

  if (GL_FRAMEBUFFER_COMPLETE_OES != glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES))
  {
    printf( "Failed to create offscreen buffer.\n" );
    glDeleteFramebuffersOES( 1, &texture->frame_buffer );
    texture->frame_buffer = 0;
  }
}


void Texture__draw__Corners_Vector2_Color_Render_Blend()
{
  // Texture::draw(Corners,Vector2,Color,Render,Blend)
  SlagInt32 src_blend  = SLAG_POP_INT32();
  SlagInt32 dest_blend = SLAG_POP_INT32();
  SlagInt32 render_flags = SLAG_POP_INT32();
  SlagInt32    color = SLAG_POP_INT32();
  Vector2 size  = SLAG_POP(Vector2);
  Vector2 uv_a   = SLAG_POP(Vector2);
  Vector2 uv_b   = SLAG_POP(Vector2);
  SlagObject* texture_obj = SLAG_POP_REF();

  GLTexture* texture = NativeLayer_get_native_texture_data(texture_obj);
  if ( !texture ) return;

  draw_buffer.set_render_flags( render_flags, src_blend, dest_blend );
  draw_buffer.set_textured_triangle_mode( texture, NULL );

  Vector2 pt1( 0, 0 );
  Vector2 pt2( size.x, 0 );
  Vector2 pt3( size.x, size.y );
  Vector2 pt4( 0, size.y );

  if (plasmacore_set_transform())
  {
    pt1 = Matrix2x3_transform( plasmacore.transform, pt1 );
    pt2 = Matrix2x3_transform( plasmacore.transform, pt2 );
    pt3 = Matrix2x3_transform( plasmacore.transform, pt3 );
    pt4 = Matrix2x3_transform( plasmacore.transform, pt4 );
  }

  color = argb_to_rgba( color );

  GLVertex v1( pt1.x, pt1.y );
  GLVertex v2( pt2.x, pt2.y );
  GLVertex v3( pt3.x, pt3.y );
  GLVertex v4( pt4.x, pt4.y );

  v1.transform();
  v2.transform();
  v3.transform();
  v4.transform();

  GLVertex uv1( uv_a.x, uv_a.y );
  GLVertex uv2( uv_b.x, uv_a.y );
  GLVertex uv3( uv_b.x, uv_b.y );
  GLVertex uv4( uv_a.x, uv_b.y );

  draw_buffer.add( v1, v2, v4, color, color, color, uv1, uv2, uv4 );
  draw_buffer.add( v4, v2, v3, color, color, color, uv4, uv2, uv3 );
}

void Texture__draw__Corners_Vector2_Color_Render_Blend_Texture_Corners()
{
  // Texture::draw(Corners,Vector2,Color,Render,Blend,Texture,Corners)
  Vector2 alpha_uv_a = SLAG_POP(Vector2);
  Vector2 alpha_uv_b = SLAG_POP(Vector2);
  SlagObject*  alpha_texture_obj = SLAG_POP_REF();
  SlagInt32 src_blend  = SLAG_POP_INT32();
  SlagInt32 dest_blend = SLAG_POP_INT32();
  SlagInt32 render_flags = SLAG_POP_INT32();
  SlagInt32    color = SLAG_POP_INT32();
  Vector2 size  = SLAG_POP(Vector2);
  Vector2 uv_a   = SLAG_POP(Vector2);
  Vector2 uv_b   = SLAG_POP(Vector2);
  SlagObject* texture_obj = SLAG_POP_REF();

  GLTexture* texture = NativeLayer_get_native_texture_data(texture_obj);
  if ( !texture ) return;

  GLTexture* alpha_texture = NativeLayer_get_native_texture_data( alpha_texture_obj );
  if ( !alpha_texture ) return;

  draw_buffer.set_render_flags( render_flags, src_blend, dest_blend );
  draw_buffer.set_textured_triangle_mode( texture, alpha_texture );

  Vector2 pt1( 0, 0 );
  Vector2 pt2( size.x, 0 );
  Vector2 pt3( size.x, size.y );
  Vector2 pt4( 0, size.y );

  if (plasmacore_set_transform())
  {
    pt1 = Matrix2x3_transform( plasmacore.transform, pt1 );
    pt2 = Matrix2x3_transform( plasmacore.transform, pt2 );
    pt3 = Matrix2x3_transform( plasmacore.transform, pt3 );
    pt4 = Matrix2x3_transform( plasmacore.transform, pt4 );
  }

  color = argb_to_rgba( color );

  GLVertex v1( pt1.x, pt1.y );
  GLVertex v2( pt2.x, pt2.y );
  GLVertex v3( pt3.x, pt3.y );
  GLVertex v4( pt4.x, pt4.y );

  v1.transform();
  v2.transform();
  v3.transform();
  v4.transform();

  GLVertex uv1( uv_a.x, uv_a.y );
  GLVertex uv2( uv_b.x, uv_a.y );
  GLVertex uv3( uv_b.x, uv_b.y );
  GLVertex uv4( uv_a.x, uv_b.y );

  GLVertex alpha_uv1( alpha_uv_a.x, alpha_uv_a.y );
  GLVertex alpha_uv2( alpha_uv_b.x, alpha_uv_a.y );
  GLVertex alpha_uv3( alpha_uv_b.x, alpha_uv_b.y );
  GLVertex alpha_uv4( alpha_uv_a.x, alpha_uv_b.y );

  draw_buffer.add( v1, v2, v4, color, color, color, uv1, uv2, uv4, alpha_uv1, alpha_uv2, alpha_uv4 );
  draw_buffer.add( v4, v2, v3, color, color, color, uv4, uv2, uv3, alpha_uv4, alpha_uv2, alpha_uv3 );
}

void Texture__draw__Corners_Quad_ColorGradient_Render_Blend()
{
  // Texture::draw(Corners,Quad,ColorGradient,Render,Blend)
  SlagInt32 src_blend  = SLAG_POP_INT32();
  SlagInt32 dest_blend = SLAG_POP_INT32();
  SlagInt32 render_flags = SLAG_POP_INT32();
  SlagInt32    color1 = SLAG_POP_INT32();
  SlagInt32    color2 = SLAG_POP_INT32();
  SlagInt32    color3 = SLAG_POP_INT32();
  SlagInt32    color4 = SLAG_POP_INT32();
  Vector2 pt1    = SLAG_POP(Vector2);
  Vector2 pt2    = SLAG_POP(Vector2);
  Vector2 pt3    = SLAG_POP(Vector2);
  Vector2 pt4    = SLAG_POP(Vector2);
  Vector2 uv_a   = SLAG_POP(Vector2);
  Vector2 uv_b   = SLAG_POP(Vector2);
  SlagObject* texture_obj = SLAG_POP_REF();

  GLTexture* texture = NativeLayer_get_native_texture_data( texture_obj );
  if ( !texture ) return;

  draw_buffer.set_render_flags( render_flags, src_blend, dest_blend );
  draw_buffer.set_textured_triangle_mode( texture, NULL );

  if (plasmacore_set_transform())
  {
    pt1 = Matrix2x3_transform( plasmacore.transform, pt1 );
    pt2 = Matrix2x3_transform( plasmacore.transform, pt2 );
    pt3 = Matrix2x3_transform( plasmacore.transform, pt3 );
    pt4 = Matrix2x3_transform( plasmacore.transform, pt4 );
  }

  color1 = argb_to_rgba( color1 );
  color2 = argb_to_rgba( color2 );
  color3 = argb_to_rgba( color3 );
  color4 = argb_to_rgba( color4 );

  GLVertex v1( pt1.x, pt1.y );
  GLVertex v2( pt2.x, pt2.y );
  GLVertex v3( pt3.x, pt3.y );
  GLVertex v4( pt4.x, pt4.y );

  v1.transform();
  v2.transform();
  v3.transform();
  v4.transform();

  GLVertex uv1( uv_a.x, uv_a.y );
  GLVertex uv2( uv_b.x, uv_a.y );
  GLVertex uv3( uv_b.x, uv_b.y );
  GLVertex uv4( uv_a.x, uv_b.y );

  draw_buffer.add( v1, v2, v4, color1, color2, color4, uv1, uv2, uv4 );
  draw_buffer.add( v4, v2, v3, color4, color2, color3, uv4, uv2, uv3 );
}

void Texture__draw__Vector2_Vector2_Vector2_Triangle_Color_Color_Color_Render_Blend()
{
  // Texture::draw(Vector2,Vector2,Vector2,Triangle,Color,Color,Color,Render,Blend)
  SlagInt32 src_blend  = SLAG_POP_INT32();
  SlagInt32 dest_blend = SLAG_POP_INT32();
  SlagInt32 render_flags = SLAG_POP_INT32();
  SlagInt32    color3 = SLAG_POP_INT32();
  SlagInt32    color2 = SLAG_POP_INT32();
  SlagInt32    color1 = SLAG_POP_INT32();
  Vector2 pt1    = SLAG_POP(Vector2);
  Vector2 pt2    = SLAG_POP(Vector2);
  Vector2 pt3    = SLAG_POP(Vector2);
  Vector2 uv_c   = SLAG_POP(Vector2);
  Vector2 uv_b   = SLAG_POP(Vector2);
  Vector2 uv_a   = SLAG_POP(Vector2);
  SlagObject* texture_obj = SLAG_POP_REF();

  GLTexture* texture = NativeLayer_get_native_texture_data(texture_obj);
  if ( !texture ) return;

  draw_buffer.set_render_flags( render_flags, src_blend, dest_blend );
  draw_buffer.set_textured_triangle_mode( texture, NULL );

  if (plasmacore_set_transform())
  {
    pt1 = Matrix2x3_transform( plasmacore.transform, pt1 );
    pt2 = Matrix2x3_transform( plasmacore.transform, pt2 );
    pt3 = Matrix2x3_transform( plasmacore.transform, pt3 );
  }

  color1 = argb_to_rgba( color1 );
  color2 = argb_to_rgba( color2 );
  color3 = argb_to_rgba( color3 );

  GLVertex v1( pt1.x, pt1.y );
  GLVertex v2( pt2.x, pt2.y );
  GLVertex v3( pt3.x, pt3.y );

  v1.transform();
  v2.transform();
  v3.transform();

  GLVertex uv1( uv_a.x, uv_a.y );
  GLVertex uv2( uv_b.x, uv_b.y );
  GLVertex uv3( uv_c.x, uv_c.y );

  draw_buffer.add( v1, v2, v3, color1, color2, color3, uv1, uv2, uv3 );
}

void Texture__set__Bitmap_Vector2()
{
  Vector2 pos  = SLAG_POP(Vector2);
  SlagBitmap* bitmap_obj = (SlagBitmap*) SLAG_POP_REF();
  SVM_NULL_CHECK( bitmap_obj, return );
  SlagObject* texture_obj = SLAG_POP_REF();

  GLTexture* texture = NativeLayer_get_native_texture_data(texture_obj);
  if ( !texture ) return;

  glBindTexture( GL_TEXTURE_2D, texture->id );

  int x = (int) pos.x;
  int y = (int) pos.y;
  int w  = bitmap_obj->width;
  int h  = bitmap_obj->height;

  if (x<0 || y<0 || x+w > texture->texture_width || y+h > texture->texture_height)
  {
    return;
  }

#ifdef SWAP_RED_AND_BLUE
  swap_red_and_blue( (SlagInt32*) bitmap_obj->pixels->data, w*h );
#endif
  glTexSubImage2D( GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, bitmap_obj->pixels->data );
#ifdef SWAP_RED_AND_BLUE
  swap_red_and_blue( (SlagInt32*) bitmap_obj->pixels->data, w*h );
#endif
}

void Vector2Manager__draw__Vector2_Color_Render()
{
  SlagInt32 render_flags = SLAG_POP_INT32();
  SlagInt32 color  = SLAG_POP_INT32();
  Vector2 pt  = SLAG_POP(Vector2); 

  SLAG_POP_REF(); // discard singleton ref 

  if (plasmacore_set_transform())
  {
    pt = Matrix2x3_transform( plasmacore.transform, pt );
  }
  color = argb_to_rgba(color);

  draw_buffer.set_render_flags( render_flags, BLEND_SRC_ALPHA, BLEND_INVERSE_SRC_ALPHA );
  draw_buffer.set_point_mode();

  GLVertex v(pt.x+1.0/plasmacore.display_width,pt.y-1.0/plasmacore.display_height);
  v.transform();
  draw_buffer.add( v, color );
}

