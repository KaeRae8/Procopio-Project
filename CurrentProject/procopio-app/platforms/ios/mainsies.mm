//
//  main.m
//  plasmacore
//
//  Created by Abe Pralle on 2/20/10.
//  Copyright Apple Inc 2010. All rights reserved.
//

/*#import <UIKit/UIKit.h>

#import "PlasmacoreAppDelegate.h"
#import "PlasmacoreView.h"
#import "CECardsAnimationController.h"
#import "MapController.h"
#include "plasmacore.h"
#include "ios_core.h"

UIApplication*            plasmacore_app  = nil;
PlasmacoreAppDelegate*    plasmacore_app_delegate  = nil;
PlasmacoreView*           plasmacore_view = nil;
PlasmacoreViewController* plasmacore_view_controller = nil;
UIWindow*                 plasmacore_window = nil;

@interface CustomPlasmacoreAppDelegate : PlasmacoreAppDelegate <UIAlertViewDelegate,UIViewControllerTransitioningDelegate>

@property (nonatomic, strong) CECardsAnimationController* transition;

@end

@implementation CustomPlasmacoreAppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions{
    
    if(![super application:application didFinishLaunchingWithOptions:launchOptions]) return false;
    
    self.transition = [[CECardsAnimationController alloc] init];
    
    return true;
}

-(void)showDialog{
    MapController *vc = [[MapController alloc] init];
    [plasmacore_view_controller presentViewController:vc animated:YES completion:nil];
}

- (id<UIViewControllerAnimatedTransitioning>)animationControllerForPresentedController:(UIViewController *)presented presentingController:(UIViewController *)presenting sourceController:(UIViewController *)source {
    
    self.transition.reverse = NO;
    return self.transition;
}

- (id<UIViewControllerAnimatedTransitioning>)animationControllerForDismissedController:(UIViewController *)dismissed {
    self.transition.reverse = YES;
    return self.transition;
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

@end

int main(int argc, char *argv[])
{
    @autoreleasepool {
        int retVal = UIApplicationMain(argc, argv, nil, @"CustomPlasmacoreAppDelegate");
        return retVal;
    }
}*/


