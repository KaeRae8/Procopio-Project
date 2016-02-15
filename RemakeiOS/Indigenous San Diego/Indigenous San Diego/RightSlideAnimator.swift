//
//  RightSlideAnimator.swift
//  Discount Cab 2.0 Driver
//
//  Created by Kenneth Shaw on 1/19/16.
//  Copyright © 2016 Veyo Logistics. All rights reserved.
//

import UIKit

class RightSlideAnimator: NSObject, UIViewControllerAnimatedTransitioning {
    let duration    = 0.25
    var presenting  = true
    
    func transitionDuration(transitionContext: UIViewControllerContextTransitioning?) -> NSTimeInterval {
        return duration
    }
    
    func animateTransition(transitionContext: UIViewControllerContextTransitioning) {
        let containerView = transitionContext.containerView()!
        
        let toView = transitionContext.viewForKey(UITransitionContextToViewKey)!
        let fromView = transitionContext.viewForKey(UITransitionContextFromViewKey)!
        let viewSize = fromView.frame
        
        if presenting{
            toView.frame = CGRectMake(viewSize.size.width, viewSize.origin.y, viewSize.size.width, viewSize.size.height)
            fromView.frame = CGRectMake(0, viewSize.origin.y, viewSize.size.width, viewSize.size.height)
        }else{
            //toView.frame = CGRectMake(-viewSize.size.width, viewSize.origin.y, viewSize.size.width, viewSize.size.height)
            fromView.frame = CGRectMake(0, viewSize.origin.y, viewSize.size.width, viewSize.size.height)
        }
        containerView.addSubview(toView)
        
        UIView.animateWithDuration(duration,
            animations: {
                if self.presenting{
                    fromView.frame = CGRectMake(-viewSize.size.width, viewSize.origin.y, viewSize.size.width, viewSize.size.height)
                    toView.frame = CGRectMake(0, viewSize.origin.y, viewSize.size.width, viewSize.size.height)
                }else{
                    fromView.frame = CGRectMake(viewSize.size.width, viewSize.origin.y, viewSize.size.width, viewSize.size.height)
                    toView.frame = CGRectMake(0, viewSize.origin.y, viewSize.size.width, viewSize.size.height)
                }
            }, completion: { _ in
                transitionContext.completeTransition(true)
            })
    }

}
