#import <UIKit/UIKit.h>

@class PlasmacoreView;
@class PlasmacoreViewController;

@interface PlasmacoreAppDelegate : NSObject <UIApplicationDelegate>
{
    UIWindow *window;
    PlasmacoreView *glView;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet PlasmacoreView *glView;

- (void)applicationWillTerminate:(UIApplication *)application;
- (void)applicationDidBecomeActive:(UIApplication *)application;
- (void)applicationWillResignActive:(UIApplication *)application;
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions;

- (void)alert:(const char*) mesg;

- (PlasmacoreView *)getView;
- (PlasmacoreViewController *)getController;
- (PlasmacoreAppDelegate *)getDelegate;
- (UIWindow *)getWindow;
- (UIApplication *)getApp;

@end

