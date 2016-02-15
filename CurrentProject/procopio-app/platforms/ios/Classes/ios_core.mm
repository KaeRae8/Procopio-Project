//=============================================================================
// ios_core.slag
//
// 3.5.0 (2011.06.18)
//
// Contains iOS-specific (iPhone,iPad,iPod) Plasmacore functionality.
//
// ----------------------------------------------------------------------------
//
// Copyright 2008-2011 Plasmaworks LLC
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
//
// ----------------------------------------------------------------------------
//
// History:
//   May 2005 / Abe Pralle - Created for Plasmacore v2
//   Feb 2010 / Abe Pralle - Updated for Plasmacore v3
//
//=============================================================================

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import <MediaPlayer/MPMoviePlayerController.h>
#include <sys/sysctl.h>

#import  "PlasmacoreView.h"
#import  "PlasmacoreAppDelegate.h"
#import  "ES1Renderer.h"
#import  "ios_core.h"
#include "AudioToolbox/AudioServices.h"
#include "pthread.h"
#include "plasmacore.h"
#include "slag_stdlib.h"
#include "mac_audio.h"
#include "unzip.h"

#include <libkern/OSAtomic.h>

void IOS__is_ipad();
void IOS__enable_auto_dimming__Logical();
void IOS__orient__Int32();
void IOS__status_bar_visible__Logical();
void IOS__update_frequency_while_sleeping__Real64();
void IOS__vibrate();
void IOS__save_photo__Bitmap();

void IOS__supports_music_picker();

void VideoPlayer__play__String();
void VideoPlayer__update__NativeData();

void WebView__view__URL();
void WebView__view__String();
void WebView__bounds__Box();
void WebView__visible__Logical();
void WebView__visible();
void WebView__loaded();
void WebView__failed();

bool is_3gs_or_better = false;
bool keyboard_visible = false;
bool status_bar_visible = false;
int  screen_orientation = 0;  // 0=portrait, 1=landscape

double update_frequency_while_sleeping = 0;
extern bool plasmacore_allow_hires_iphone4;

UITextField* text_field = nil;

void LOG( const char* mesg )
{
    printf( "%s\n", mesg );
}

Archive data_archive( "data" );
Archive image_archive( "images" );


NSString* to_ns_string( SlagObject* _string_obj )
{
    if (_string_obj == NULL) return nil;
    SlagString* string_obj = (SlagString*) _string_obj;
    int count = string_obj->count;
    return [NSString stringWithCharacters:(const unichar*)string_obj->characters length:count];
}

SlagObject* to_slag_string( NSString* str )
{
    SlagString* result = (SlagString*) SlagString::create( [str length] );
    [str getCharacters:(unichar*)result->characters];
    SLAG_PUSH_REF( (SlagObject*) result );  // leave result on stack
    result->set_hash_code();
    return (SlagObject*) result;
}

UIImage* to_uiimage( SlagBitmap* bitmap_obj )
{
    char* data = (char*) bitmap_obj->pixels->data;
    int w = bitmap_obj->width;
    int h = bitmap_obj->height;
    int count = w * h;
    
    swap_red_and_blue( (SlagInt32*) data, count );
    CGContextRef cg_context = CGBitmapContextCreate( (GLubyte*)data, w, h, 8, w * 4,
                                                    CGColorSpaceCreateDeviceRGB(), kCGImageAlphaPremultipliedLast );
    CGImageRef cg_image = CGBitmapContextCreateImage( cg_context );
    UIImage* img = [UIImage imageWithCGImage:cg_image];
    CGImageRelease(cg_image);
    CGContextRelease(cg_context);
    swap_red_and_blue( (SlagInt32*) data, count );
    
    return img;
}

SlagObject* to_slag_bitmap( UIImage* img )
{
    SLAG_FIND_TYPE( type_bitmap, "Bitmap" );
    SlagObject* bitmap_obj = type_bitmap->create();
    
    int width  = (int) CGImageGetWidth(img.CGImage);
    int height = (int) CGImageGetHeight(img.CGImage);
    
    SLAG_PUSH_REF( bitmap_obj );  // method result
    SLAG_PUSH_REF(  bitmap_obj );  // for init(Int32,Int32)
    SLAG_PUSH_INT32( width );
    SLAG_PUSH_INT32( height );
    SLAG_CALL( type_bitmap, "init(Int32,Int32)" );
    bitmap_obj = SLAG_POP_REF();  // reset ref in case a gc happened
    
    SlagArray* array = ((SlagBitmap*) bitmap_obj)->pixels;
    
    // Uses the bitmap creation function provided by the Core Graphics framework.
    CGContextRef gc = CGBitmapContextCreate((GLubyte*)array->data, width, height, 8, width * 4, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaPremultipliedLast );
    CGContextClearRect( gc, CGRectMake(0.0, 0.0, (CGFloat)width, (CGFloat)height) );
    CGContextDrawImage( gc, CGRectMake(0.0, 0.0, (CGFloat)width, (CGFloat)height), img.CGImage );
    swap_red_and_blue( (SlagInt32*) array->data, width*height );
    CGContextRelease(gc);
    
    return bitmap_obj;
}

SlagObject* to_slag_byte_list( NSData* data )
{
    int count = [data length];
    
    SlagArrayList* list = slag_create_byte_list( NULL, count );
    
    [data getBytes:list->array->data length:count];
    return (SlagObject*) list;
}

GLTexture::GLTexture( int w, int h, bool offscreen_buffer )
{
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
    //mm.resource_bytes -= (texture_width * texture_height);
    glDeleteTextures( 1, &id );
    if (frame_buffer) glDeleteFramebuffersOES( 1, &frame_buffer );
}

GLuint NativeLayer_get_frame_buffer() { return plasmacore_renderer->frame_buffer; }

Archive::Archive( const char* archive_filename )
{
    
    @autoreleasepool {
    
    NSString* base_filename = [NSString stringWithCString:archive_filename encoding:1];
    NSString* path = [[NSBundle mainBundle] pathForResource:base_filename ofType:@"zip"];
    
    if (path) archive_filename = [path cStringUsingEncoding:1];
    
    int len = strlen(archive_filename) + 1;
    this->archive_filename = (char*) new char[len];
    if (!this->archive_filename) slag_throw_fatal_error( "Out of memory - application closing." );
    strcpy( this->archive_filename, archive_filename );
    
    }
}

Archive::~Archive()
{
    delete (archive_filename);
}

void* Archive::open()
{
    return unzOpen( archive_filename );
}

struct IOSSound : SlagResource
{
    SystemSoundID  system_sound_id;  // for sound effects
    SoundPlayer* audio_player;
    char* filename;
    bool system_sound_playing;
    bool system_sound_repeats;
    bool is_bg_music;
    int  data_size;
    
    IOSSound( char* filename, bool is_bg_music )
    {
        system_sound_id = 0;
        audio_player = NULL;
        system_sound_playing = false;
        system_sound_repeats = false;
        this->is_bg_music = is_bg_music;
        data_size = 0;
        
        int len = strlen( filename ) + 1;
        this->filename = new char[len];
        if (this->filename)
        {
            strcpy( this->filename, filename );
            create_from_filename();
        }
    }
    
    IOSSound( IOSSound* other )
    {
        system_sound_id = 0;
        audio_player = NULL;
        system_sound_playing = false;
        system_sound_repeats = false;
        data_size = other->data_size;
        is_bg_music = other->is_bg_music;
        
        int len = strlen( other->filename ) + 1;
        filename = new char[len];
        if (filename)
        {
            strcpy( filename, other->filename );
            create_from_filename();
        }
    }
    
    void create_from_filename()
    {
        if (is_bg_music)
        {
            NSData* data = [NSData dataWithContentsOfFile:[NSString stringWithCString:filename encoding:1]];
            if ( !data ) return;
            
            audio_player = new SoundPlayer( (char*) [data bytes], data.length );
            if ( !audio_player ) return;
            
            data_size = data.length;
        }
        else
        {
            CFURLRef url = (__bridge CFURLRef)[[NSURL alloc] initFileURLWithPath:[NSString stringWithCString:filename encoding:1]];
            if ( !url ) return;
            
            data_size = CFURLGetBytes( url, NULL, 0 );
            AudioServicesCreateSystemSoundID( url, &system_sound_id );
        }
        //mm.resource_bytes += data_size;
    }
    
    ~IOSSound()
    {
        if (filename)
        {
            delete filename;
            filename = 0;
        }
        
        //mm.resource_bytes -= data_size;
        
        if (audio_player)
        {
            delete audio_player;
            audio_player = NULL;
        }
        
        if (system_sound_id != 0)
        {
            AudioServicesDisposeSystemSoundID( system_sound_id );
        }
    }
    
    bool error() { return audio_player == NULL && system_sound_id == 0; }
    
    void play()
    {
        if (audio_player)
        {
            audio_player->play();
        }
        else
        {
            system_sound_playing = true;
            AudioServicesPlaySystemSound( system_sound_id );
        }
    }
    
    void pause()
    {
        if (audio_player) audio_player->pause();
        system_sound_playing = false;
    }
    
    bool is_playing()
    {
        if (audio_player)
        {
            return audio_player->is_playing();
        }
        return system_sound_playing;
    }
    
    void set_volume( SlagReal64 new_volume )
    {
        if (audio_player) audio_player->set_volume( new_volume );
    }
    
    void set_repeats( SlagInt32 setting )
    {
        if (audio_player)
        {
            if (audio_player) audio_player->repeats = setting;
        }
        else
        {
            system_sound_repeats = setting;
        }
    }
    
    double get_current_time()
    {
        if (audio_player) return audio_player->get_current_time();
        //if (audio_player) return audio_player.currentTime;
        return system_sound_playing ? 1.0 : 0.0;
    }
    
    void set_current_time( double new_time )
    {
        if (audio_player) audio_player->set_current_time( new_time );
        //if (audio_player) audio_player.currentTime = new_time;
        system_sound_playing = false;
    }
    
    double get_duration()
    {
        if (audio_player) return audio_player->duration;
        //if (audio_player) return audio_player.duration;
        return 1.0;
    }
    
    void set_callback()
    {
        if (system_sound_id)
        {
            AudioServicesAddSystemSoundCompletion( system_sound_id, NULL, NULL,
                                                  on_system_sound_complete, (void*) this );
        }
    }
    
    static void on_system_sound_complete( SystemSoundID id, void* user_data )
    {
        IOSSound* sound = (IOSSound*) user_data;
        if (sound)
        {
            if (sound->system_sound_repeats) sound->play();
            else sound->system_sound_playing = false;
        }
    }
};


void on_audio_interruption( void* user_data, UInt32 interruption_state )
{
    //if (interruption_state == kAudioSessionEndInterruption) NSLog(@"end audio interruption");
    //else                                                    NSLog(@"start audio interruption");
}


void NativeLayer_set_up()
{
    [[NSFileManager defaultManager] changeCurrentDirectoryPath:NSHomeDirectory()];
    [[NSFileManager defaultManager] changeCurrentDirectoryPath:@"Documents"];
    
    [[NSFileManager defaultManager] createDirectoryAtPath:@"save"
                              withIntermediateDirectories:NO
                                               attributes:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithInt:0755],
                                                           NSFilePosixPermissions,nil]
                                                    error:NULL];
    
    AudioSessionInitialize( NULL, NULL, on_audio_interruption, NULL );
    UInt32 audio_category = kAudioSessionCategory_UserInterfaceSoundEffects;
    AudioSessionSetProperty( kAudioSessionProperty_AudioCategory, sizeof(audio_category), &audio_category );
    AudioSessionSetActive( true );
    
    size_t size;
    sysctlbyname("hw.machine",NULL,&size,NULL,0);
    char* machine = (char*) malloc(size+1);  // prolly don't need the +1 on this one
    sysctlbyname("hw.machine",machine,&size,NULL,0);
    if ((0!=strcmp(machine,"iPhone1,1"))
        && (0!=strcmp(machine,"iPhone1,2"))
        && (0!=strcmp(machine,"iPod1,1"))
        && (0!=strcmp(machine,"iPod2,1")))
    {
        is_3gs_or_better = true;
        NSLog( @"Detected 3GS+" );
        //min_updates_per_draw = 1;
    }
    else
    {
        NSLog( @"Detected pre-3GS model" );
        //min_updates_per_draw = 2;
    }
    delete machine;
    
    plasmacore_init();
}

void update_status_bar_with_orientation( int orientation, bool visible )
{
    switch (orientation)
    {
        case 0:
            [plasmacore_app setStatusBarOrientation:UIInterfaceOrientationPortrait];
            break;
        case 1:
            [plasmacore_app setStatusBarOrientation:UIInterfaceOrientationLandscapeRight];
            break;
        case 2:
            [plasmacore_app setStatusBarOrientation:UIInterfaceOrientationPortraitUpsideDown];
            break;
        case 3:
            [plasmacore_app setStatusBarOrientation:UIInterfaceOrientationLandscapeLeft];
            break;
    }
    
    if (visible) [plasmacore_app setStatusBarHidden:NO  animated:NO];
    else         [plasmacore_app setStatusBarHidden:YES animated:NO];
}

void NativeLayer_configure()
{
    slag_hook_native( "IOS", "is_ipad()", IOS__is_ipad );
    slag_hook_native( "IOS", "enable_auto_dimming(Logical)", IOS__enable_auto_dimming__Logical );
    slag_hook_native( "IOS", "orient(Int32)", IOS__orient__Int32 );
    slag_hook_native( "IOS", "status_bar_visible(Logical)", IOS__status_bar_visible__Logical );
    //slag_hook_native( "IOS", "update_frequency_while_sleeping(Real64)", IOS__update_frequency_while_sleeping__Real64 );
    slag_hook_native( "IOS", "vibrate()", IOS__vibrate );
    slag_hook_native( "IOS", "save_photo(Bitmap)", IOS__save_photo__Bitmap );
    
    //slag_hook_native( "IOS", "supports_music_picker()", IOS__supports_music_picker );
    
    slag_hook_native( "VideoPlayer", "play(String)", VideoPlayer__play__String);
    slag_hook_native( "VideoPlayer", "update(NativeData)", VideoPlayer__update__NativeData );
    
    slag_hook_native( "WebView", "view(URL)",        WebView__view__URL );
    slag_hook_native( "WebView", "view(String)",     WebView__view__String );
    slag_hook_native( "WebView", "bounds(Box)",      WebView__bounds__Box );
    slag_hook_native( "WebView", "visible(Logical)", WebView__visible__Logical );
    slag_hook_native( "WebView", "visible()",        WebView__visible );
    slag_hook_native( "WebView", "loaded()",         WebView__loaded );
    slag_hook_native( "WebView", "failed()",         WebView__failed );
    
    CGRect bounds = [[UIScreen mainScreen] bounds];
    
    int w = (int) bounds.size.width;
    int h = (int) bounds.size.height;
    printf("Native Screen size is %d %d \n", w,h);
    if (plasmacore_allow_hires_iphone4)
    {
        if ([[[UIDevice currentDevice] systemVersion] floatValue] >= 3.2f)
        {
            int w2 = (int) [UIScreen mainScreen].currentMode.size.width;
            int h2 = (int) [UIScreen mainScreen].currentMode.size.height;
            printf("Native Screen size is %d %d \n", w2,h2);
            if (w2 == 640)
            {
                printf("In the iPhone 4 res \n");
                w = w2;
                h = h2;
            }
            else if((h2 == 1536 && w2 == 2048))
            {
                printf("In the iPad3 res \n");
                w = h2;
                h = w2;
            }else if((h2 == 2048 && w2 == 1536))
            {
                printf("In the iPad Air res \n");
                w = w2;
                h = h2;
            }else if ((w2 == 750 && h2 == 1334))
            {
                printf("In the iPhone 6 res \n");
                w = w2;
                h = h2;
            }else if ((w2 == 1242 && h2 == 2208))
            {
                printf("In the iPhone 6+ res \n");
                w = w2;
                h = h2;
            }else if ((w2 == 1125 && h2 == 2001))
            {
                printf("In the iPhone 6+ res (Zoomed) \n");
                w = w2;
                h = h2;
            }
        }
    }
    plasmacore_configure( w, h, true, true );
    update_status_bar_with_orientation( plasmacore.orientation, status_bar_visible );
}


void NativeLayer_init_bitmap( SlagObject* bitmap_obj, char* data, int data_size )
{
    SLAG_PUSH_REF( bitmap_obj );
    
    NSData* raw_data = [NSData dataWithBytesNoCopy:data length:data_size freeWhenDone:NO];
    CGImageRef bitmap_image = [UIImage imageWithData:raw_data].CGImage;
    
    if(bitmap_image)
    {
        // Get the width and height of the image
        int width = CGImageGetWidth(bitmap_image);
        int height = CGImageGetHeight(bitmap_image);
        
        SLAG_PUSH_REF( bitmap_obj );
        SLAG_PUSH_INT32( width );
        SLAG_PUSH_INT32( height );
        SLAG_CALL( bitmap_obj->type, "init(Int32,Int32)" );
        SLAG_GET( SlagArray*, array, bitmap_obj, "data" );
        
        // Uses the bitmap creation function provided by the Core Graphics framework.
        CGContextRef gc = CGBitmapContextCreate((GLubyte*)array->data, width, height, 8, width * 4, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaPremultipliedLast );
        CGContextClearRect( gc, CGRectMake(0.0, 0.0, (CGFloat)width, (CGFloat)height) );
        CGContextDrawImage(gc, CGRectMake(0.0, 0.0, (CGFloat)width, (CGFloat)height), bitmap_image);
        swap_red_and_blue( (SlagInt32*) array->data, width*height );
        CGContextRelease(gc);
    }
    
    
    SLAG_POP_REF();
}

void NativeLayer_shut_down()
{
    printf("Native shut down\n");
}

void Application__log__String()
{
    SlagString* mesg = (SlagString*) SLAG_POP_REF();  // ignore title string
    SLAG_POP_REF();  // discard singleton
    
    int count = mesg->count;
    if (count >= 512)
    {
        char* buffer = new char[count+1];
        if (buffer)
        {
            mesg->to_ascii( buffer, count+1 );
            LOG( buffer );
            delete buffer;
        }
    }
    else
    {
        char buffer[512];
        mesg->to_ascii( buffer, 512 );
        LOG( buffer );
    }
}


void Application__title__String()
{
    // Application::title(String)
    SLAG_POP_REF();  // ignore title string
    SLAG_POP_REF();  // discard singleton
}

void Bitmap__to_png_bytes()
{
    UIImage* img = to_uiimage( (SlagBitmap*) SLAG_POP_REF() );
    SLAG_PUSH_REF( to_slag_byte_list( UIImagePNGRepresentation(img) ) );
}

void Bitmap__to_jpg_bytes__Real64()
{
    SlagReal64 quality = SLAG_POP_REAL64();
    UIImage* img = to_uiimage( (SlagBitmap*) SLAG_POP_REF() );
    SLAG_PUSH_REF( to_slag_byte_list( UIImageJPEGRepresentation(img,quality) ) );
}

void Display__fullscreen()
{
    // Display::fullscreen().Logical
    SLAG_POP_REF();
    SLAG_PUSH_INT32( 1 );
}

void Display__fullscreen__Logical()
{
    // Display::fullscreen(Logical)
    SLAG_POP_INT32();  // ignore fullscreen setting
    SLAG_POP_REF();
}

void Input__keyboard_visible__Logical()
{
    SlagInt32 setting = SLAG_POP_INT32();
    SLAG_POP_REF();  // discard singleton
    
    if (keyboard_visible == setting) return;
    
    keyboard_visible = setting;
    
    if (setting)
    {
        if ( !text_field )
        {
            text_field = [[UITextField alloc] initWithFrame:CGRectMake(0,0,plasmacore.display_width,plasmacore.display_height)];
            text_field.hidden = YES;
            text_field.userInteractionEnabled = YES;
            text_field.autocorrectionType = UITextAutocorrectionTypeNo;
            text_field.autocapitalizationType = UITextAutocapitalizationTypeNone;
            [text_field setDelegate:plasmacore_view];
            [plasmacore_view addSubview:text_field];
        }
        text_field.text = @"     ";
        [text_field becomeFirstResponder];
    }
    else
    {
        [text_field resignFirstResponder];
    }
}

void Input__keyboard_visible()
{
    SLAG_POP_REF();
    SLAG_PUSH_INT32( keyboard_visible );
}

void Input__mouse_visible__Logical()
{
    SLAG_POP_INT32(); // ignore setting
    SLAG_POP_REF();      // discard singleton
}

void Input__input_capture__Logical()
{
    SLAG_POP_INT32(); // ignore setting
    SLAG_POP_REF();      // discard singleton
}


void IOS__is_ipad()
{
    SLAG_POP_REF();  // discard singleton
    
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
    {
        SLAG_PUSH_LOGICAL( true );
    }
    else
    {
        SLAG_PUSH_LOGICAL( false );
    }
}

void IOS__enable_auto_dimming__Logical()
{
    SlagInt32 setting = SLAG_POP_INT32();
    SLAG_POP_REF();  // discard singleton
    
    if (setting) plasmacore_app.idleTimerDisabled = NO;
    else         plasmacore_app.idleTimerDisabled = YES;
}

void IOS__orient__Int32()
{
    screen_orientation = SLAG_POP_INT32();
    SLAG_POP_REF();  // discard singleton
    
    update_status_bar_with_orientation( screen_orientation, status_bar_visible );
}

void IOS__status_bar_visible__Logical()
{
    status_bar_visible = SLAG_POP_INT32();
    SLAG_POP_REF();  // discard singleton
    update_status_bar_with_orientation( screen_orientation, status_bar_visible );
}

void IOS__update_frequency_while_sleeping__Real64()
{
    update_frequency_while_sleeping = SLAG_POP_REAL64();  // updates per second
    if (update_frequency_while_sleeping < 0.0) update_frequency_while_sleeping = 0.0;
    else if (update_frequency_while_sleeping > 60.0) update_frequency_while_sleeping = 60.0;
    
    SLAG_POP_REF();  // discard singleton
}


void IOS__vibrate()
{
    SLAG_POP_REF();  // discard singleton
    AudioServicesPlaySystemSound( kSystemSoundID_Vibrate );
}

void IOS__save_photo__Bitmap()
{
    UIImage* img = to_uiimage( (SlagBitmap*) SLAG_POP_REF() );
    SLAG_POP_REF(); // discard singleton
    UIImageWriteToSavedPhotosAlbum( img, nil, nil, nil );
}

void IOS__supports_music_picker()
{
    SLAG_POP_REF(); // discard singleton
    SLAG_PUSH_INT32(0);  // false for now
}

//-----------------------------------------------------------------------------
//  NativeSound
//-----------------------------------------------------------------------------

NSString* sound_extensions[] = { @"wav", @"WAV", @"mp3", @"MP3", @"aac", @"AAC",
    @"aif", @"AIF", @"aiff", @"AIFF", @"caf", @"CAF", @"m4a", @"M4A", @"mp4", @"MP4"  };

bool find_sound( char* filename, int buffer_size, bool* is_bg_music_ptr )
{
    // find possible start of extension
    int len = strlen(filename);
    int i = len - 1;
    while (i > 0 && filename[i] != '.') --i;
    
    NSString* path = nil;
    if (i > 0)
    {
        filename[i] = 0;
        NSString* base = [NSString stringWithCString:filename encoding:1];
        NSString* ext  = [NSString stringWithCString:filename+i+1 encoding:1];
        path = [[NSBundle mainBundle] pathForResource:base ofType:ext];
        if (path == nil) filename[i] = '.';
    }
    
    if (path == nil)
    {
        NSString* base = [NSString stringWithCString:filename encoding:1];
        int count = sizeof(sound_extensions) / sizeof(sound_extensions[0]);
        i = 0;
        while (path == nil && i < count)
        {
            path = [[NSBundle mainBundle] pathForResource:base ofType:sound_extensions[i++]];
        }
    }
    
    if (path != nil)
    {
        strcpy( filename, [path cStringUsingEncoding:1] );
        int i = strlen(filename) - 1;
        while (i > 0 && filename[i] != '.') --i;
        
        *is_bg_music_ptr = true;
        
        /*if (strcasecmp(filename+i,".wav")==0 || strcasecmp(filename+i,".caf")==0
         || strcasecmp(filename+i,".aif")==0 || strcasecmp(filename+i,".aiff")==0)
         {
         *is_bg_music_ptr = false;
         }
         else
         {
         *is_bg_music_ptr = true;
         }*/
        
        
        return true;
    }
    else
    {
        // try stripping off leading directory info
        int i = len-1;
        while (i >= 0)
        {
            if (filename[i]=='/' || filename[i]=='\\') break;
            --i;
        }
        if (i >= 0)
        {
            // alter filename to be substring after directory
            char* src_ptr  = filename + i + 1;
            char* dest_ptr = filename;
            while (*src_ptr)
            {
                *(dest_ptr++) = *(src_ptr++);
            }
            *dest_ptr = 0;
            return find_sound( filename, buffer_size, is_bg_music_ptr );
        }
        return false;
    }
}

void NativeSound__init__String()
{
    //printf("+init\n");
    // NativeSound::init(String)
    SlagString* filename_string = (SlagString*) SLAG_POP_REF();
    char filename[512];
    filename_string->to_ascii( filename, 507 );
    
    bool is_bg_music = false;
    if (find_sound(filename,512,&is_bg_music))
    {
        //is_bg_music = true;
        IOSSound* sound = new IOSSound( filename, is_bg_music );
        if (sound)
        {
            if ( !sound->error() )
            {
                SlagNativeData* data_obj = SlagNativeData::create(sound, SlagNativeDataDeleteResource);
                SlagObject* sound_obj = SLAG_POP_REF();
                SLAG_SET_REF( sound_obj, "native_data", data_obj );
                sound->set_callback();
            }
        }
    }
    else
    {
        slag_throw_file_error( filename );
    }
    //printf("-init\n");
}


void NativeSound__init__ArrayList_of_Byte()
{
    // NativeSound::init(Byte[])
    SlagArrayList* list = (SlagArrayList*) SLAG_POP_REF();
    IOSSound* sound = new IOSSound( (char*) list->array->data, list->count );
    
    if ( !sound || sound->error() )
    {
        return;
    }
    
    SlagNativeData* data_obj = SlagNativeData::create( sound, SlagNativeDataDeleteResource );
    SlagObject* sound_obj = SLAG_POP_REF();
    SLAG_SET_REF( sound_obj, "native_data", data_obj );
    sound->set_callback();
}

IOSSound* get_sound_from_stack()
{
    SlagObject* sound_obj = SLAG_POP_REF(); // sound object
    SLAG_GET( SlagNativeData*, native_data, sound_obj, "native_data" );
    if ( !native_data ) return NULL;
    
    return (IOSSound*) native_data->data;
}

void NativeSound__create_duplicate()
{
    // NativeSound::create_duplicate().Sound
    IOSSound* sound = get_sound_from_stack();
    if (sound)
    {
        sound = new IOSSound( sound );
        if ( sound )
        {
            SLAG_FIND_TYPE( type_sound, "NativeSound" );
            SlagObject* sound_obj = type_sound->create();
            SLAG_PUSH_REF( sound_obj ); // result
            
            SlagNativeData* data_obj = SlagNativeData::create( sound, SlagNativeDataDeleteResource );
            SLAG_SET_REF( sound_obj, "native_data", data_obj );
            
            sound->set_callback();
        }
        else
        {
            SLAG_PUSH_REF( NULL );
        }
    }
}

void NativeSound__play()
{
    //printf("+play\n");
    // NativeSound::play()
    IOSSound* sound = get_sound_from_stack();
    if (sound) sound->play();
    //printf("-play\n");
    
}

void NativeSound__pause()
{
    // NativeSound::pause()
    IOSSound* sound = get_sound_from_stack();
    if (sound) sound->pause();
}

void NativeSound__is_playing()
{
    //printf("+is_playing\n");
    
    // NativeSound::is_playing().Logical
    IOSSound* sound = get_sound_from_stack();
    if (sound) SLAG_PUSH_INT32( (sound && sound->is_playing()) );
    //printf("-is_playing\n");
    
}

void NativeSound__volume__Real64()
{
    // NativeSound::volume(Real64)
    double volume = SLAG_POP_REAL64();
    IOSSound* sound = get_sound_from_stack();
    if (sound) sound->set_volume(volume);
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
    IOSSound* sound = get_sound_from_stack();
    if (sound) sound->set_repeats(setting);
}

void NativeSound__current_time()
{
    // NativeSound::current_time().Real64
    IOSSound* sound = get_sound_from_stack();
    if (sound)
    {
        SLAG_PUSH_REAL64( (SlagReal64)(sound->get_current_time()) );
    }
}

void NativeSound__current_time__Real64()
{
    // NativeSound::current_time(Real64)
    SlagReal64 new_time = SLAG_POP_REAL64();
    IOSSound* sound = get_sound_from_stack();
    if (sound)
    {
        sound->set_current_time( new_time );
    }
}

void NativeSound__duration()
{
    // NativeSound::duration().Real64
    IOSSound* sound = get_sound_from_stack();
    if (sound)
    {
        SLAG_PUSH_REAL64( (SlagReal64) sound->get_duration() );
    }
}

void OffscreenBuffer__clear__Color()
{
    // OffscreenBuffer::clear(Color)
    draw_buffer.render();
    
    SlagInt32 color = SLAG_POP_INT32();
    SlagObject* buffer_obj = SLAG_POP_REF();
    
    SVM_NULL_CHECK( buffer_obj, return );
    
    SLAG_GET_REF( texture_obj, buffer_obj, "texture" );
    SVM_NULL_CHECK( texture_obj, return );
    
    SLAG_GET( SlagNativeData*, native_data, texture_obj, "native_data" );
    GLTexture* texture = (GLTexture*) native_data->data;
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
        glBindFramebufferOES( GL_FRAMEBUFFER_OES, NativeLayer_get_frame_buffer() );
    }
}


void slag_adjust_filename_for_os( char* filename, int buffer_size )
{
    int i = strlen(filename) - 1;
    while (i > 0 && filename[i] != '.') --i;
    if (i > 0)
    {
        filename[i] = 0;
        NSString* base = [NSString stringWithCString:filename encoding:1];
        NSString* ext  = [NSString stringWithCString:filename+i+1 encoding:1];
        NSString* path = [[NSBundle mainBundle] pathForResource:base ofType:ext];
        if (path == nil) filename[i] = '.';  // restore original
        else strcpy( filename, [path cStringUsingEncoding:1] );
    }
}


void SoundManager__audio_supported()
{
    SLAG_POP_REF();
    SLAG_PUSH_INT32( true );
}

void System__country_name()
{
    SLAG_POP_REF();  // singleton
    
    NSLocale *locale = [NSLocale currentLocale];
    NSString *country_code = [locale objectForKey:NSLocaleCountryCode];
    NSString *country_name = [locale displayNameForKey:NSLocaleCountryCode value: country_code];
    
    if ([country_name length] == 0)
    {
        SLAG_PUSH_REF( (SlagObject*) SlagString::create("unknown") );
    }
    else
    {
        SlagString* result = (SlagString*) SlagString::create( [country_name length] );
        [country_name getCharacters:(unichar*)result->characters];
        SLAG_PUSH_REF( (SlagObject*) result );  // leave result on stack
        result->set_hash_code();
    }
}

void System__device_id()
{
    SLAG_POP_REF();  // singleton context
    
    /*UIDevice *device = [UIDevice currentDevice];
     NSString *unique_identifier = [device uniqueIdentifier];
     if ( !unique_identifier )
     {
     SLAG_PUSH_REF( NULL );
     return;
     }
     
     SlagString* result = (SlagString*) SlagString::create( [unique_identifier length] );
     [unique_identifier getCharacters:(unichar*)result->characters];
     SLAG_PUSH_REF( (SlagObject*) result );  // leave result on stack
     result->set_hash_code();*/
    SLAG_PUSH_REF( NULL );
    return;
}

void System__open_url__String()
{
    SlagString* st = (SlagString*) SLAG_POP_REF();
    SLAG_POP_REF();
    
    SVM_NULL_CHECK( st, return );
    
    char buffer[512];
    st->to_ascii( buffer, 512 );
    NSURL* url = [NSURL URLWithString:[NSString stringWithCString:buffer encoding:1]];
    [[UIApplication sharedApplication] openURL:url];
}

void Texture_init( SlagInt32* data, int w, int h, int format );

void Texture__init__String_Int32()
{
    int format = SLAG_POP_INT32();
    SlagString* filename_obj = (SlagString*) SLAG_POP_REF();
    // Leave Texture obj on stack.
    
    int data_size;
    char* data = image_archive.load( filename_obj, &data_size );
    
    if ( !data )
    {
        SLAG_POP_REF();  // Texture obj
        char buffer[256];
        filename_obj->to_ascii( buffer, 256 );
        slag_throw_file_error( buffer );
        return;
    }
    
    NSData* raw_data = [NSData dataWithBytesNoCopy:data length:data_size freeWhenDone:NO];
    CGImageRef bitmap_image = [UIImage imageWithData:raw_data].CGImage;
    
    if(bitmap_image)
    {
        // Get the width and height of the image
        int width = CGImageGetWidth(bitmap_image);
        int height = CGImageGetHeight(bitmap_image);
        SlagInt32* pixels = new SlagInt32[ width * height ];
        
        // Uses the bitmap creation function provided by the Core Graphics framework.
        CGContextRef gc = CGBitmapContextCreate((GLubyte*)pixels, width, height, 8, width * 4,
                                                CGColorSpaceCreateDeviceRGB(), kCGImageAlphaPremultipliedLast );
        CGContextClearRect( gc, CGRectMake(0.0, 0.0, (CGFloat)width, (CGFloat)height) );
        CGContextDrawImage(gc, CGRectMake(0.0, 0.0, (CGFloat)width, (CGFloat)height), bitmap_image);
        CGContextRelease(gc);
        
        swap_red_and_blue( (SlagInt32*) pixels, width*height );
        Texture_init( pixels, width, height, format );
        
        delete pixels;
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
    
    SLAG_GET( SlagNativeData*, native_data, texture_obj, "native_data" );
    GLTexture* texture = (GLTexture*) native_data->data;
    if ( !texture ) return;
    
    draw_buffer.set_render_flags( render_flags, BLEND_ONE, BLEND_INVERSE_SRC_ALPHA );
    draw_buffer.set_textured_triangle_mode( texture, NULL );
    
    bool hflip;
    if (size.x < 0)
    {
        size.x = -size.x;
        hflip = true;
    }
    else
    {
        hflip = false;
    }
    
    bool vflip;
    if (size.y < 0)
    {
        size.y = -size.y;
        vflip = true;
    }
    else
    {
        vflip = false;
    }
    
    if (plasmacore.orientation == 1)
    {
        double temp = size.x;
        size.x = size.y;
        size.y = temp;
        
        temp = pos.y;
        pos.y = pos.x;
        pos.x = (plasmacore.display_height - temp) - size.x;
    }
    
    
    GLVertex v1( pos.x, pos.y );
    GLVertex v2( pos.x+size.x, pos.y );
    GLVertex v3( pos.x+size.x, pos.y+size.y );
    GLVertex v4( pos.x, pos.y+size.y );
    
    if (hflip)
    {
        GLVertex temp = v1;
        v1 = v2;
        v2 = temp;
        temp = v3;
        v3 = v4;
        v4 = temp;
    }
    
    if (vflip)
    {
        GLVertex temp = v1;
        v1 = v4;
        v4 = temp;
        temp = v2;
        v2 = v3;
        v3 = temp;
    }
    
    
    GLVertex uv1( uv_a.x, uv_a.y );
    GLVertex uv2( uv_b.x, uv_a.y );
    GLVertex uv3( uv_b.x, uv_b.y );
    GLVertex uv4( uv_a.x, uv_b.y );
    
    if (plasmacore.orientation == 1)
    {
        GLVertex temp = uv4;
        uv4 = uv3;
        uv3 = uv2;
        uv2 = uv1;
        uv1 = temp;
    }
    
    draw_buffer.add( v1, v2, v4, 0xffffffff, 0xffffffff, 0xffffffff, uv1, uv2, uv4 );
    draw_buffer.add( v4, v2, v3, 0xffffffff, 0xffffffff, 0xffffffff, uv4, uv2, uv3 );
    
    /*
     // GL extension drawtexture doesn't appear to work for iphone currently
     
     SlagInt32 render_flags = SLAG_POP_INT32();
     Vector2 size = SLAG_POP(Vector2);
     Vector2 pos  = SLAG_POP(Vector2);
     Vector2 uv_a = SLAG_POP(Vector2);
     Vector2 uv_b = SLAG_POP(Vector2);
     SlagObject* texture_obj = SLAG_POP_REF();
     
     SLAG_GET( SlagNativeData*, native_data, buffer_obj, "native_data" );
     GLTexture* texture = (GLTexture*) native_data->data;
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
     
     double y_pos = (plasmacore.screen_height - size.y) - pos.y;
     glDrawTexfOES( (GLfloat) pos.x, (GLfloat)y_pos, 0.0f, (GLfloat) size.x, (GLfloat) size.y );
     glDisable(GL_TEXTURE_2D);
     */
}

//=============================================================================
//  WebView
//=============================================================================
@interface PlasmacoreWebView : UIWebView<UIWebViewDelegate>
{
@public
    bool loaded;
    bool failed;
}
@end

@implementation PlasmacoreWebView
- (void) webViewDidFinishLoad:(UIWebView *)web_view
{
    self.hidden = false;
    self->loaded = true;
}

- (void) webView:(UIWebView*) web_view didFailLoadWithError:(NSError *)error
{
    self->failed = true;
}
@end


static void on_delete_webview( void* ptr )
{
    PlasmacoreWebView* view = (__bridge PlasmacoreWebView*) ptr;
    if (view)
    {
        view.delegate = nil;
        view.hidden = YES;
        [view removeFromSuperview];
    }
    
}

static PlasmacoreWebView* get_native_webview( SlagObject* slag_webview )
{
    SLAG_GET( SlagNativeData*, native_data, slag_webview, "data" );
    if (native_data == NULL)
    {
        SLAG_PUSH_REF( slag_webview );
        native_data = SlagNativeData::create( NULL, on_delete_webview );
        SLAG_SET_REF( slag_webview, "data", native_data );
        SLAG_POP_REF();
    }
    
    if (native_data->data == NULL)
    {
        CGRect bounds;
        bounds.origin.x = 0;
        bounds.origin.y = 0;
        bounds.size.width = 1;
        bounds.size.height = 1;
        PlasmacoreWebView* view = [[PlasmacoreWebView alloc] initWithFrame:bounds];
        view.autoresizingMask = UIViewAutoresizingFlexibleWidth;
        view.hidden = YES;
        view->loaded = false;
        view->failed  = false;
        [plasmacore_window addSubview:view];
        view.delegate = view;
        native_data->data = (__bridge void*)view;
    }
    
    return (__bridge PlasmacoreWebView*) native_data->data;
}

static void delete_native_webview( SlagObject* slag_webview )
{
    SLAG_GET( SlagNativeData*, native_data, slag_webview, "data" );
    if (native_data == NULL) return;
    if (native_data->data)
    {
        native_data->data = NULL;
    }
}

void WebView__view__URL()
{
    SlagObject* url_obj = SLAG_POP_REF();
    PlasmacoreWebView* view = get_native_webview( SLAG_POP_REF() );
    
    if (url_obj == NULL) return;
    
    SLAG_GET_REF( url_string, url_obj, "value" );
    NSString* url = to_ns_string( url_string );
    
    view->loaded = false;
    view->failed  = false;
    [view loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:url]]];
}

void WebView__view__String()
{
    NSString* html = to_ns_string( SLAG_POP_REF() );
    PlasmacoreWebView* view = get_native_webview( SLAG_POP_REF() );
    
    view->loaded = false;
    view->failed = false;
    [view loadHTMLString:html baseURL:nil];
}

void WebView__bounds__Box()
{
    Vector2 position = SLAG_POP(Vector2);
    Vector2 size = SLAG_POP(Vector2);
    PlasmacoreWebView* view = get_native_webview( SLAG_POP_REF() );
    
    position = plasmacore.point_to_screen(position);
    size     = plasmacore.size_to_screen(size);
    
    if (plasmacore.orientation == 1)
    {
        view.transform = CGAffineTransformIdentity;
        view.frame  = CGRectMake(0,0,size.x,size.y);
        
        double temp = size.x;
        size.x = size.y;
        size.y = temp;
        
        temp = position.y;
        position.y = position.x;
        position.x = (plasmacore.display_height - temp) - size.x;
        
        double pi = acos(-1);
        view.transform = CGAffineTransformMakeRotation(pi/2);
        view.center = CGPointMake(position.x + size.x/2, position.y + size.y/2);
    }
    else
    {
        view.transform = CGAffineTransformIdentity;
        view.frame = CGRectMake(position.x,position.y,size.x,size.y);
    }
}


void WebView__visible__Logical()
{
    int setting = SLAG_POP_INT32();
    PlasmacoreWebView* view = get_native_webview( SLAG_POP_REF() );
    if (setting) view.hidden = NO;
    else view.hidden = YES;
}

void WebView__visible()
{
    PlasmacoreWebView* view = get_native_webview( SLAG_POP_REF() );
    int visible = view.hidden ? 0 : 1;
    SLAG_PUSH_INT32(visible);
}

void WebView__loaded()
{
    PlasmacoreWebView* view = get_native_webview( SLAG_POP_REF() );
    SLAG_PUSH_INT32( view->loaded ? 1 : 0 );
}

void WebView__failed()
{
    PlasmacoreWebView* view = get_native_webview( SLAG_POP_REF() );
    SLAG_PUSH_INT32( view->failed ? 1 : 0 );
}


//=============================================================================
//  VideoPlayer
//=============================================================================
bool video_playback_finished = false;

@interface IOSVideoPlayerListener : NSObject
{
}
@end

@implementation IOSVideoPlayerListener
- (id) init
{
    video_playback_finished = false;
    return self;
}

- (void) on_playback_finished:(NSNotification*) notification
{
    video_playback_finished = true;
}

@end

extern MPMoviePlayerController* active_movie_player;

struct IOSVideoPlayerInfo : SlagResource
{
    MPMoviePlayerController* player;
    IOSVideoPlayerListener*  listener;
    
    IOSVideoPlayerInfo( const char* filename )
    {
        NSString* filepath = [NSString stringWithCString:filename encoding:1];
        
        NSURL* url = [NSURL fileURLWithPath:filepath];
        
        player = [[MPMoviePlayerController alloc] initWithContentURL:url];
        active_movie_player = player;
        
        if (plasmacore.orientation == 1)
        {
            [[player view] setTransform:CGAffineTransformMakeRotation(M_PI / 2)];
        }
        
        [[player view] setFrame: [plasmacore_view bounds]];
        [plasmacore_view addSubview: [player view]];
        
        listener = [[IOSVideoPlayerListener alloc] init];
        
        [[NSNotificationCenter defaultCenter] addObserver:listener
                                                 selector:@selector(on_playback_finished:)
                                                     name:MPMoviePlayerPlaybackDidFinishNotification
                                                   object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:listener
                                                 selector:@selector(on_playback_finished:)
                                                     name:MPMoviePlayerDidExitFullscreenNotification
                                                   object:nil];
        
        [player play];
    }
    
    ~IOSVideoPlayerInfo()
    {
        video_playback_finished = true;
        
        [[NSNotificationCenter defaultCenter] removeObserver:listener];
        
        [player stop];
        [[player view] removeFromSuperview];
        
        player = NULL;
        active_movie_player = NULL;
        
        listener = NULL;
    }
    
    bool update()
    {
        if (video_playback_finished)
        {
            return false;
        }
        else
        {
            return true;
        }
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
    
    strcpy( filename, "save/" );
    ((SlagString*)filepath)->to_ascii( filename+5, 500 );
    
    slag_adjust_filename_for_os( filename, 512 );
    
    FILE* file_check = fopen( filename, "rb" );
    if ( !file_check )
    {
        ((SlagString*)filepath)->to_ascii( filename, 512 );
        slag_adjust_filename_for_os( filename, 512 );
        
        file_check = fopen( filename, "rb" );
    }
    
    if ( file_check ) fclose( file_check );
    else return;
    
    IOSVideoPlayerInfo* player = new IOSVideoPlayerInfo( filename );
    SLAG_PUSH_REF( SlagNativeData::create(player, SlagNativeDataDeleteResource) );
}

void VideoPlayer__update__NativeData()
{
    IOSVideoPlayerInfo* player = NULL;
    SlagNativeData* native_data = (SlagNativeData*) SLAG_POP_REF();
    if (native_data) player = (IOSVideoPlayerInfo*) native_data->data;
    
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

