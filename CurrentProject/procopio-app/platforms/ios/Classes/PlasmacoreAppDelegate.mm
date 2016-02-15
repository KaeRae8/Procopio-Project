#import "PlasmacoreAppDelegate.h"
#import "PlasmacoreView.h"

#include "plasmacore.h"


extern UIApplication*         plasmacore_app;
extern PlasmacoreAppDelegate* plasmacore_app_delegate;
extern PlasmacoreView*        plasmacore_view;
extern UIWindow*              plasmacore_window;

@implementation PlasmacoreAppDelegate

@synthesize window;
@synthesize glView;

void NativeLayer_set_up();
void NativeLayer_configure();

bool plasmacore_running = false;

void perform_custom_setup();

- (void)alert:(const char*) mesg
{
  NSString* st = [NSString stringWithCString:mesg encoding:1];
  UIAlertView *alert_box = [[UIAlertView alloc] initWithTitle:@"Fatal Error" message:st  
	  delegate:nil cancelButtonTitle:@"Exit" otherButtonTitles:nil, nil];
  if (alert_box)
  {
    [alert_box show];
  }
}

- (void)initializePlasmacore: (UIApplication*) application
{
  try
  {
    NativeLayer_set_up();
    perform_custom_setup();

    NativeLayer_configure();

    plasmacore_launch();
    [glView startAnimation];

    plasmacore_running = true;
  }
  catch (int error_code)
  {
    if (slag_error_message.value) [self alert:slag_error_message.value];
  }
  catch ( const char* mesg )
  {
    [self alert:mesg];
  }
}

- (PlasmacoreView*) createPlasmacoreViewWithFrame:(CGRect) bounds
{
  return [[PlasmacoreView alloc] initWithFrame:bounds];
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions   
{
    CGRect bounds = [[UIScreen mainScreen] bounds];
    window = [[UIWindow alloc] initWithFrame:bounds];
    glView = [self createPlasmacoreViewWithFrame:bounds];
    
    // Assign globals
    plasmacore_app = application;
    plasmacore_app_delegate = self;
    plasmacore_window = window;
    plasmacore_view = glView;
    
    //window.rootViewController = plasmacore_view_controller;
    [window setRootViewController:plasmacore_view_controller];
    [window makeKeyAndVisible];
    

  [self performSelectorOnMainThread:@selector(initializePlasmacore:) withObject:application waitUntilDone:NO];
  return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
  [glView stopAnimation];
  plasmacore_queue_event( plasmacore.event_suspend );
  plasmacore_dispatch_pending_events();
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
  if (plasmacore_running)
  {
    [glView startAnimation];
    plasmacore_queue_event( plasmacore.event_resume );
    plasmacore_dispatch_pending_events();
  }
}

- (void)applicationWillTerminate:(UIApplication *)application
{
  [glView stopAnimation];
  plasmacore_on_exit_request();
  plasmacore_running = false;
}

-(BOOL)shouldAutorotate
{
    return NO;
}

-(NSUInteger)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskPortrait;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

-(UIInterfaceOrientation)preferredInterfaceOrientationForPresentation
{
    return UIInterfaceOrientationPortrait;
}

- (PlasmacoreView *)getView{
    return plasmacore_view;
}

- (PlasmacoreViewController *)getController{
    return  plasmacore_view_controller;
}

-(PlasmacoreAppDelegate *)getDelegate{
    return self;
}

-(UIWindow *)getWindow{
    return plasmacore_window;
}

-(UIApplication *)getApp{
    return plasmacore_app;
}

- (void)dealloc
{
  
}

@end
