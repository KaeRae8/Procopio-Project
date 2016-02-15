//
//  SlagMethods.h
//  plasmacore_ios
//
//  Created by Kenneth Shaw on 4/30/15.
//
//

#import <Foundation/Foundation.h>

@interface SlagMethods : NSObject

+ (SlagMethods *)sharedManager;

-(void)backFromMaps;
-(void)goToSitePage:(NSString *)title withCategory:(int)category;
-(void)spinnerShowing;

@end
