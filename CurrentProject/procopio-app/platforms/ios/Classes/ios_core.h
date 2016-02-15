//=============================================================================
// ios_core.slag
//
// $(PLASMACORE_VERSION) $(DATE)
//
// Contains iPhoneOS-specific Plasmacore functionality.
//
// ----------------------------------------------------------------------------
//
// $(COPYRIGHT)
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
#ifndef IOS_CORE_H
#define IOS_CORE_H

#import <UIKit/UIKit.h>
#import "PlasmacoreAppDelegate.h"
#import "PlasmacoreView.h"
#import "ES1Renderer.h"
#include "plasmacore.h"

NSString*   to_ns_string( SlagObject* _string_obj );
SlagObject* to_slag_string( NSString* str );
UIImage*    to_uiimage( SlagBitmap* bitmap_obj );
SlagObject* to_slag_bitmap( UIImage* img );
SlagObject* to_slag_byte_list( NSData* data );

extern UIApplication*         plasmacore_app;
extern PlasmacoreAppDelegate* plasmacore_app_delegate;
extern PlasmacoreView*        plasmacore_view;
extern UIWindow*              plasmacore_window;
extern ES1Renderer*           plasmacore_renderer;

extern bool plasmacore_running;
extern bool is_3gs_or_better;
extern bool keyboard_visible;
extern bool status_bar_visible;
extern int  screen_orientation;  // 0=portrait, 1=landscape

#endif // IOS_CORE_H
