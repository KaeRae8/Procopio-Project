//
//  main.m
//  plasmacore
//
//  Created by Abe Pralle on 2/20/10.
//  Copyright Apple Inc 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "PlasmacoreAppDelegate.h"
#import "PlasmacoreView.h"

#include "plasmacore.h"

// These globals are set up during Plasmacore initialization and can be used
// in Slag native method implementations.  You'll need to include some of the
// headers above and redeclare any one that you use as "extern", e.g.: 
// "extern UIWindow* plasmacore_window;"
UIApplication*            plasmacore_app  = nil;
PlasmacoreAppDelegate*    plasmacore_app_delegate  = nil;
PlasmacoreView*           plasmacore_view = nil;
PlasmacoreViewController* plasmacore_view_controller = nil;
UIWindow*                 plasmacore_window = nil;

// Set this to "true" in perform_custom_setup() to allow high-res 480x960 displays 
// on iPhone 4.  You can then check Display.size == Vector2(480,960) to find out if 
// you're on an iPhone 4 and act accordingly.
bool plasmacore_allow_hires_iphone4 = false;

void perform_custom_setup()
{
  // See: http://plasmaworks.com/wiki/index.php/Custom_Native_Functionality
}


// Override any app delegate methods as desired.
@interface CustomPlasmacoreAppDelegate : PlasmacoreAppDelegate
{
}
// Commonly overridden
// - (PlasmacoreView*) createPlasmacoreViewWithFrame:(CGRect) bounds;
@end

@implementation CustomPlasmacoreAppDelegate
@end


int main(int argc, char *argv[])
{
  NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
  int retVal = UIApplicationMain(argc, argv, nil, @"CustomPlasmacoreAppDelegate");
  [pool release];
  return retVal;
}
