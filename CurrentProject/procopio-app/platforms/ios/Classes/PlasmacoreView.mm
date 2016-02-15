#include "ios_core.h"
#import <MediaPlayer/MPMoviePlayerController.h>

TouchManager touch_manager;
extern int plasmacore_input_scale;

//====================================================================
//  PlasmacoreViewController
//====================================================================
@implementation PlasmacoreViewController
@end


MPMoviePlayerController* active_movie_player = NULL;

@implementation PlasmacoreView

@synthesize animating;
@dynamic animation_frame_interval;

ES1Renderer* plasmacore_renderer;

// You must implement this method
+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

//The EAGL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
- (id)initWithFrame:(CGRect)bounds
{    
  if ((self = [super initWithFrame:bounds]))
  {
    // Get the layer
    CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

    eaglLayer.opaque = TRUE;
    eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
       [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, 
       kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];

    plasmacore_renderer = [[ES1Renderer alloc] init];
    renderer = plasmacore_renderer;

    if (!renderer)
    {
      return nil;
    }

    animating = FALSE;
    displayLinkSupported = FALSE;
    animation_frame_interval = 1;
    displayLink = nil;
    animationTimer = nil;

    // A system version of 3.1 or greater is required to use CADisplayLink. The NSTimer
    // class is used as fallback when it isn't available.
    NSString *reqSysVer = @"3.1";
    NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
    if ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending)
    {
      displayLinkSupported = TRUE;
    }
  }

  [self setMultipleTouchEnabled:YES];

  [[UIAccelerometer sharedAccelerometer] setUpdateInterval:(1.0/30.0)];
  [[UIAccelerometer sharedAccelerometer] setDelegate:self];

  // Link with a view controller
  plasmacore_view_controller = [[PlasmacoreViewController alloc] init];
  plasmacore_view_controller.view = self;

  return self;
}

- (void)drawView:(id)sender
{
  static bool locked = false;
  if (locked) return;
  locked = true;

  try
  {
    if (plasmacore_running) [renderer render];
  }
  catch (int error_code)
  {
    plasmacore_running = false;
    if (slag_error_message.value) [plasmacore_app_delegate alert:slag_error_message.value];
  }
  catch ( const char* mesg )
  {
    plasmacore_running = false;
    [self stopAnimation];
    [plasmacore_app_delegate alert:mesg];
  }

  locked = false;
}

- (void)layoutSubviews
{
    [renderer resizeFromLayer:(CAEAGLLayer*)self.layer];
    [self drawView:nil];
}

- (NSInteger)animation_frame_interval
{
    return animation_frame_interval;
}

- (void)setAnimationFrameInterval:(NSInteger)frameInterval
{
    // Frame interval defines how many display frames must pass between each time the
    // display link fires. The display link will only fire 30 times a second when the
    // frame internal is two on a display that refreshes 60 times a second. The default
    // frame interval setting of one will fire 60 times a second when the display refreshes
    // at 60 times a second. A frame interval setting of less than one results in undefined
    // behavior.
    if (frameInterval >= 1)
    {
        animation_frame_interval = frameInterval;

        if (animating)
        {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}

- (void)startAnimation
{
  if (!animating)
  {
    if (displayLinkSupported)
    {
      // CADisplayLink is API new to iPhone SDK 3.1. Compiling against earlier versions will result in a warning, but can be dismissed
      // if the system version runtime check for CADisplayLink exists in -initWithCoder:. The runtime check ensures this code will
      // not be called in system versions earlier than 3.1.

      displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView:)];
      [displayLink setFrameInterval:animation_frame_interval];
      [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    }
    else
    {
      animationTimer = [NSTimer 
          scheduledTimerWithTimeInterval:(NSTimeInterval)(animation_frame_interval/60.0) 
          target:self selector:@selector(drawView:) userInfo:nil repeats:TRUE];

    }
    animating = TRUE;
  }
}

- (void)stopAnimation
{
    if (animating)
    {
        if (displayLinkSupported)
        {
            [displayLink invalidate];
            displayLink = nil;
        }
        else
        {
            [animationTimer invalidate];
            animationTimer = nil;
        }

        animating = FALSE;
    }

    if (active_movie_player)
    {
      [active_movie_player stop];
    }
}

- (void)dealloc
{
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
  for (UITouch* touch in touches)
  {
    CGPoint os_pt = [touch locationInView:nil];
    double x = os_pt.x;
    double y = os_pt.y;
    if (plasmacore.orientation == 1)
    {
      double temp = y;
      y = plasmacore.display_height - (x+1);
      x = temp;
    }
    x = (x - plasmacore.border_x) / plasmacore.scale_factor;
    y = (y - plasmacore.border_y) / plasmacore.scale_factor;
    x *= plasmacore_input_scale;
    y *= plasmacore_input_scale;
    int index = touch_manager.begin_touch(x,y);
    if (index > 0)
    {
      plasmacore_queue_data_event( plasmacore.event_mouse_button,
        index, 1, true, x, y );
    }
  }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
  for (UITouch* touch in touches)
  {
    CGPoint os_pt = [touch locationInView:nil];
    double x = os_pt.x;
    double y = os_pt.y;
    if (plasmacore.orientation == 1)
    {
      double temp = y;
      y = plasmacore.display_height - (x+1);
      x = temp;
    }
    x = (x - plasmacore.border_x) / plasmacore.scale_factor;
    y = (y - plasmacore.border_y) / plasmacore.scale_factor;
    x *= plasmacore_input_scale;
    y *= plasmacore_input_scale;
    int index = touch_manager.update_touch(x,y);
    if (index > 0)
    {
      plasmacore_queue_data_event( plasmacore.event_mouse_move, 
        index, 0, true, x, y  );
    }
  }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
  for (UITouch* touch in touches)
  {
    CGPoint os_pt = [touch locationInView:nil];
    double x = os_pt.x;
    double y = os_pt.y;
    if (plasmacore.orientation == 1)
    {
      double temp = y;
      y = plasmacore.display_height - (x+1);
      x = temp;
    }
    x = (x - plasmacore.border_x) / plasmacore.scale_factor;
    y = (y - plasmacore.border_y) / plasmacore.scale_factor;
    x *= plasmacore_input_scale;
    y *= plasmacore_input_scale;

    int index = touch_manager.end_touch(x,y);
    if (index > 0)
    {
      plasmacore_queue_data_event( plasmacore.event_mouse_button,
        index, 1, false, x, y );
    }
  }
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
  [self touchesEnded:touches withEvent:event];
}

- (BOOL) textField:(UITextField*)field shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString*)string
{
  int ch;
  if (string.length > 0) ch = [string characterAtIndex:0];
  else ch = 8;
  plasmacore_queue_data_event( plasmacore.event_key, 1, ch, true );
  plasmacore_queue_data_event( plasmacore.event_key, 1, ch, false );
  field.text = @"     ";

  return YES;
}

- (void) textFieldDidEndEditing:(UITextField*)field
{
  keyboard_visible = false;
}

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration
{
  SlagReal64 x =  acceleration.x;
  SlagReal64 y = -acceleration.y;
  SlagReal64 z =  acceleration.z;

  SLAG_FIND_TYPE( type_input, "Input" );
  SlagObject* input_obj = type_input->singleton();
  SLAG_SET_REAL64( input_obj, "acceleration_x", x );
  SLAG_SET_REAL64( input_obj, "acceleration_y", y );
  SLAG_SET_REAL64( input_obj, "acceleration_z", z );
}


@end
