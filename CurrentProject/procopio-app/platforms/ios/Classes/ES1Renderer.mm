//
//  ES1Renderer.m
//  plasmacore
//
//  Created by Abe Pralle on 2/20/10.
//  Copyright Apple Inc 2010. All rights reserved.
//

#import "ES1Renderer.h"
#import "ios_core.h"
#include "plasmacore.h"
#include "mac_audio.h"

ES1Renderer* renderer = 0;
int plasmacore_input_scale = 1;

extern bool plasmacore_allow_hires_iphone4;

@implementation ES1Renderer

// Create an OpenGL ES 1.1 context
- (id)init
{
    if (self = [super init])
    {
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        
        if (!context || ![EAGLContext setCurrentContext:context])
        {
            return nil;
        }
        
        // Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
        glGenFramebuffersOES(1, &frame_buffer);
        glGenRenderbuffersOES(1, &colorRenderbuffer);
        glBindFramebufferOES(GL_FRAMEBUFFER_OES, frame_buffer);
        glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, colorRenderbuffer);
    }
    
    return self;
}

/*
 -(void) destroyFrameBuffer
 {
 // Tear down GL
 if (frame_buffer)
 {
 glDeleteFramebuffersOES(1, &frame_buffer);
 frame_buffer = 0;
 }
 
 if (colorRenderbuffer)
 {
 glDeleteRenderbuffersOES(1, &colorRenderbuffer);
 colorRenderbuffer = 0;
 }
 }
 
 -(void) createFrameBuffer
 {
 // Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
 glGenFramebuffersOES(1, &frame_buffer);
 glBindFramebufferOES(GL_FRAMEBUFFER_OES, frame_buffer);
 
 glGenRenderbuffersOES(1, &colorRenderbuffer);
 glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
 
 glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, colorRenderbuffer);
 }
 */

- (void)render
{
    if ( !plasmacore_running ) return;
    
    update_sounds();
    
    renderer = self;
    if ( plasmacore_update() )
    {
        plasmacore_draw();
    }
    
    int updates_per_draw = int(plasmacore.updates_per_second / plasmacore.target_fps);
    if (updates_per_draw != plasmacore_view.animation_frame_interval)
    {
        [plasmacore_view setAnimation_frame_interval:updates_per_draw];
    }
}

// Kludge ala http://stackoverflow.com/questions/9120090/cocos2d-fails-on-ipad-yet-works-in-simulator
static BOOL resized_once = NO;

- (BOOL)resizeFromLayer:(CAEAGLLayer *)layer
{
    if (resized_once) return YES;
    resized_once = YES;
    
    //[self destroyFrameBuffer];
    //[self createFrameBuffer];
    
    if (plasmacore_allow_hires_iphone4
        && [[[UIDevice currentDevice] systemVersion] floatValue] >= 3.2f)
    {
        int w = (int) [UIScreen mainScreen].currentMode.size.width;
        int h = (int) [UIScreen mainScreen].currentMode.size.height;
        NSLog(@"Size is %d %d", w, h);
        if (w == 640 || w == 750)
        {
            plasmacore_view.contentScaleFactor = 2.0;
            plasmacore_view.layer.contentsScale = 2;
            plasmacore_input_scale = 2;
        }
        if (w == 1242 || w == 1125)
        {
            plasmacore_view.contentScaleFactor = 3.0;
            plasmacore_view.layer.contentsScale = 3;
            plasmacore_input_scale = 3;
        }
        if ((w == 2048 && h == 1536) || (w == 1536 && h == 2048))
        {
            plasmacore_view.contentScaleFactor = 2.0;
            plasmacore_view.layer.contentsScale = 2;
            plasmacore_input_scale = 2;
        }
    }
    
    // Allocate color buffer backing based on the current layer size
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:layer];
    
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES,
                                    &backingHeight);
    
    if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
    {
        NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return NO;
    }
    
    return YES;
}

- (void)dealloc
{
    // Tear down GL
    if (frame_buffer)
    {
        glDeleteFramebuffersOES(1, &frame_buffer);
        frame_buffer = 0;
    }
    
    if (colorRenderbuffer)
    {
        glDeleteRenderbuffersOES(1, &colorRenderbuffer);
        colorRenderbuffer = 0;
    }
    
    // Tear down context
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
    
    context = nil;
    
}

@end


void NativeLayer_begin_draw()
{
    [EAGLContext setCurrentContext:renderer->context];
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, renderer->frame_buffer);
    if (plasmacore.orientation == 1) glViewport(0, 0, plasmacore.display_height, plasmacore.display_width);
    else                             glViewport(0, 0, plasmacore.display_width, plasmacore.display_height);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (plasmacore.orientation == 1)
    {
        glOrthox( 0, plasmacore.display_height<<16, plasmacore.display_width<<16, 0, -1<<16, 1<<16 );
    }
    else
    {
        glOrthox( 0, plasmacore.display_width<<16, plasmacore.display_height<<16, 0, -1<<16, 1<<16 );
    }
    glMatrixMode(GL_MODELVIEW);
    
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
    
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
    glDisable( GL_SCISSOR_TEST );
    
    // Clear the screen to Display.background.color unless that color's alpha=0.
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
    
    // Prepare for drawing.
    glEnable( GL_BLEND );
    
    draw_buffer.set_draw_target( NULL );
}

void NativeLayer_end_draw()
{
    draw_buffer.render();
    
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, renderer->colorRenderbuffer);
    [renderer->context presentRenderbuffer:GL_RENDERBUFFER_OES];
}

