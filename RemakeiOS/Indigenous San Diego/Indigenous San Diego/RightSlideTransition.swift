//
//  RightSlideTransition.swift
//  Discount Cab 2.0 Driver
//
//  Created by Kenneth Shaw on 1/19/16.
//  Copyright © 2016 Veyo Logistics. All rights reserved.
//

import UIKit

class RightSlideTransition: NSObject, UIViewControllerTransitioningDelegate{
    func animationControllerForPresentedController(presented: UIViewController, presentingController presenting: UIViewController, sourceController source: UIViewController) -> UIViewControllerAnimatedTransitioning? {
        let animator = RightSlideAnimator()
        animator.presenting = true
        return animator
    }
    
    func animationControllerForDismissedController(dismissed: UIViewController) -> UIViewControllerAnimatedTransitioning? {
        let animator = RightSlideAnimator()
        animator.presenting = false
        return animator
    }
}
