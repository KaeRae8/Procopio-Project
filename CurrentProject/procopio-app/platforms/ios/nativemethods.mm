//
//  nativemethods.m
//  plasmacore_ios
//
//  Created by Kenneth Shaw on 4/30/15.
//
//

#import <UIKit/UIKit.h>
#import <MapKit/MapKit.h>
#import "Indigenous_SD-swift.h"


#include "plasmacore.h"
#include "ios_core.h"

bool plasmacore_allow_hires_iphone4 = true;
UIApplication*            plasmacore_app  = nil;
PlasmacoreAppDelegate*    plasmacore_app_delegate  = nil;
PlasmacoreView*           plasmacore_view = nil;
PlasmacoreViewController* plasmacore_view_controller = nil;
UIWindow*                 plasmacore_window = nil;

void ObjC__showDialog();
void ObjC__showSpinner();
void ObjC__removeSpinner();
void ObjC__openSite__Int32();
void ObjC__showBlurBox__Real64();
void ObjC__addSiteForCategory__String_String_Real64_Real64_String();
void ObjC__exploreAction__Int32_Int32();
void ObjC__subMenuView__String();

void perform_custom_setup()
{
    // See: http://plasmaworks.com/wiki/index.php/Custom_Native_Functionality
}

void ObjC__addSiteForCategory__String_String_Real64_Real64_String()
{
    NSString *site = to_ns_string(SLAG_POP_REF());
    double lon = SLAG_POP_REAL64();
    double lat = SLAG_POP_REAL64();
    NSString *name = to_ns_string(SLAG_POP_REF());
    NSString *category = to_ns_string(SLAG_POP_REF());
    [((SwiftPlasmacoreAppDelegate *) plasmacore_app_delegate) addSite:name forCategory:category withLat:lat Lon:lon website:site];
    SLAG_POP_REF();
}

void ObjC__showDialog()
{
    SLAG_POP_REF();
    [((SwiftPlasmacoreAppDelegate *) plasmacore_app_delegate) showDialog];
}

void ObjC__subMenuView__String()
{
    NSString *name = to_ns_string(SLAG_POP_REF());
    NSLog(@"Here %@", name);
    [((SwiftPlasmacoreAppDelegate* ) plasmacore_app_delegate) subMenuView:name];
}

void ObjC__showSpinner()
{
    SLAG_POP_REF();
    [((SwiftPlasmacoreAppDelegate *) plasmacore_app_delegate) showSpinner];
}

void ObjC__openSite__Int32()
{
    int site = SLAG_POP_INT32();
    SLAG_POP_REF();
    if(site == 1){
        [((SwiftPlasmacoreAppDelegate *) plasmacore_app_delegate) openSCTCA];
    }else if (site == 2){
        [((SwiftPlasmacoreAppDelegate *) plasmacore_app_delegate) openNAKA];
    }else if (site == 3){
        [((SwiftPlasmacoreAppDelegate *) plasmacore_app_delegate) openPROCO];
    }
}

void ObjC__removeSpinner()
{
    SLAG_POP_REF();
    [((SwiftPlasmacoreAppDelegate *) plasmacore_app_delegate) removeSpinner];
}

void ObjC__showBlurBox__Real64()
{
    double heightOffset = SLAG_POP_REAL64();
    SLAG_POP_REF();
    [((SwiftPlasmacoreAppDelegate *) plasmacore_app_delegate) showBlurBox:heightOffset];
}

void ObjC__exploreAction__Int32_Int32()
{
    int index = SLAG_POP_INT32();
    int cat = SLAG_POP_INT32();
    SLAG_POP_REF();
    [((SwiftPlasmacoreAppDelegate *) plasmacore_app_delegate) exploreAction:cat index:index];
}