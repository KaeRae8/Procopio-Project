//
//  SlagMethods.m
//  plasmacore_ios
//
//  Created by Kenneth Shaw on 4/30/15.
//
//

#import "SlagMethods.h"

#include "plasmacore.h"
#include "ios_core.h"

@implementation SlagMethods

static SlagMethods *sharedInstance = nil;

+ (SlagMethods *)sharedManager {
    static dispatch_once_t onceQueue;
    
    dispatch_once(&onceQueue, ^{
        sharedInstance = [[SlagMethods alloc] init];
        
    });
    
    return sharedInstance;
}

-(void)backFromMaps{
    SLAG_FIND_TYPE(type, "TriggeredMethods");
    SLAG_PUSH_REF(type->singleton() );
    SLAG_CALL(type, "backFromMaps()" );
}

-(void)goToSitePage:(NSString *)title withCategory:(int)category{
    SLAG_FIND_TYPE(type, "TriggeredMethods");
    SLAG_PUSH_REF(type->singleton() );
    SLAG_PUSH_REF(to_slag_string(title));
    SLAG_PUSH_INT32(category);
    SLAG_CALL(type, "goToSitePage(String,Int32)" );
}

-(void)spinnerShowing{
    SLAG_FIND_TYPE(type, "TriggeredMethods");
    SLAG_PUSH_REF(type->singleton() );
    SLAG_CALL(type, "loaderDone()" );
}

@end
