//
//  ViewController.swift
//  Indigenous San Diego
//
//  Created by Kenneth Shaw on 11/5/15.
//  Copyright © 2015 Procopio. All rights reserved.
//

import UIKit

class MainViewController: UIViewController {
    
    
    @IBOutlet var buttonCollection: [MenuButton]!
    @IBOutlet var soundButton: UIButton!
    @IBOutlet var backButton: UIButton!
    @IBOutlet var titleLabel: UILabel!
    @IBOutlet var buttonLeadingCollection: [NSLayoutConstraint]!
    @IBOutlet var buttonWidthCollection: [NSLayoutConstraint]!
    let transition = RightSlideAnimator()
    var tempTitle = ""
    var subTitle = ""
    var onMainMenu = true
    var loaded = false
    var viewingSite = false
    var viewingMap = false
    var viewingAbout = false
    var selectedIndex = 0
    var buttonWidth: CGFloat = 0
    var currentData: [CulturalSite]!
    
    let mainMenuItems = [" "," "," "," "," ","EXPLORE","MAP","ABOUT"]
    let subMenuItems = ["TRIBAL MUSEUMS","TRIBAL OWNED BUSINESSES","TRIBAL LANDS","PUBLIC MUSEUMS","OTHER NATIVE BUSINESSES","CULTURAL TRAILS & LANDMARKS","HIGHER EDUCATION","SPANISH MISSIONS"]

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        buttonWidth = buttonCollection.first!.frame.size.width
        for button in buttonCollection{
            button.delegate = self
        }
        backButton.alpha = 0.0
        tempTitle = "Indigenous San Diego"
        
        let range = 0...7
        moveButtonOffScreen(withRange: range,animated: false)
        SiteData.sharedInstance.load("TribalMuseums")
        SiteData.sharedInstance.load("CulturalTrails")
        SiteData.sharedInstance.load("HigherEducation")
        SiteData.sharedInstance.load("OtherNativeBusinesses")
        SiteData.sharedInstance.load("PublicMuseums")
        SiteData.sharedInstance.load("SpanishMuseums")
        SiteData.sharedInstance.load("TribalLands")
        SiteData.sharedInstance.load("TribalOwnedBusinesses")
    }
    
    override func viewDidAppear(animated: Bool) {
        if !loaded{
            loaded = true
            let range = 5...7
            moveButtonOnScreen(withRange: range,animated: true)
        }
        if viewingSite{
            viewingSite = false
            let range = 0...7
            moveButtonOnScreen(withRange: range,animated: true)
        }
        if viewingMap{
            viewingMap = false
            let range = 5...7
            moveButtonOnScreen(withRange: range,animated: true)
        }
        if viewingAbout{
            viewingAbout = false
            let range = 5...7
            moveButtonOnScreen(withRange: range,animated: true)
        }
    }
    
    override func viewWillDisappear(animated: Bool) {
        
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    @IBAction func soundPressed(sender: AnyObject) {
        if SiteData.sharedInstance.playMusic{
            soundButton.setImage(UIImage(named: "sound-off.png"), forState: .Normal)
            SiteData.sharedInstance.stopPlayback()
        }else{
            soundButton.setImage(UIImage(named: "sound-on.png"), forState: .Normal)
            SiteData.sharedInstance.playMyFile()
        }
    }

    @IBAction func backPressed(sender: AnyObject) {
        if !onMainMenu{
            self.onMainMenu = true
            let range = 0...7
            self.moveButtonOffScreen(withRange: range,animated: true)
            let dispatchTime: dispatch_time_t = dispatch_time(DISPATCH_TIME_NOW, Int64(1.225 * Double(NSEC_PER_SEC)))
            dispatch_after(dispatchTime, dispatch_get_main_queue(), {
                self.moveButtonOnScreen(withRange: 5...7, animated: true)
            })
        }
    }
    
    func setButtonTitles(){
        let menuTitles = onMainMenu ? mainMenuItems : subMenuItems
        var index = 0
        for button in buttonCollection{
            button.label.text = menuTitles[index]
            index++
        }
    }

    func moveButtonOffScreen(withRange range: Range<Int>, animated: Bool){
        var index = 0
        for constraint in buttonLeadingCollection{
            if range.contains(index){
                constraint.constant = -buttonWidth
            }
            if animated{
                UIView.animateWithDuration(0.35, delay: (0.0 + 0.0625*Double(index)), usingSpringWithDamping: 0.65, initialSpringVelocity: 0, options: .CurveEaseInOut, animations: {
                    self.buttonCollection[index].alpha = 0.0
                    self.buttonCollection[index].button.alpha = 0.69;
                    self.buttonCollection[index].colorFill.alpha = 0.69;
                    if self.onMainMenu { self.backButton.alpha = 0.0 }
                    self.view.layoutIfNeeded()
                    }, completion: nil)
            }else{
                self.buttonCollection[index].alpha = 0.0
            }
            index++
        }
        
        if !onMainMenu && !viewingSite{
            let dispatchTime: dispatch_time_t = dispatch_time(DISPATCH_TIME_NOW, Int64(1.225 * Double(NSEC_PER_SEC)))
            dispatch_after(dispatchTime, dispatch_get_main_queue(), {
                self.moveButtonOnScreen(withRange: 0...7, animated: true)
            })
        }else if selectedIndex == 6{
            viewingMap = true
            self.performSegueWithIdentifier("viewMapView", sender: nil)
        }else if selectedIndex == 7{
            viewingAbout = true
            self.performSegueWithIdentifier("viewAboutView", sender: nil)
        }
    }
    
    func moveButtonOnScreen(withRange range: Range<Int>, animated: Bool){
        setButtonTitles()
        var index = 0
        for constraint in buttonLeadingCollection{
            if range.contains(index){
                constraint.constant = 0
            }
            if animated{
                UIView.animateWithDuration(0.35, delay: (0.0 + 0.0625*(7.0-Double(index))), usingSpringWithDamping: 0.65, initialSpringVelocity: 0, options: .CurveEaseOut, animations: {
                    if range.contains(index) {self.buttonCollection[index].alpha = 1.0}
                    if !self.onMainMenu { self.backButton.alpha = 1.0 }
                    self.titleLabel.text = self.tempTitle
                    self.view.layoutIfNeeded()
                    }, completion: nil)
            }
            index++
        }
    }
    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        if segue.identifier == "viewSiteTable"{
            let vc = segue.destinationViewController as! SiteTable
            vc.data = currentData
            vc.tempTitle = self.subTitle
        }
    }
}

extension MainViewController: MenuButtonDelegate{
    func buttonWasTapped(button: MenuButton!) {
        tempTitle = button.label.text!.capitalizedString
        let index = button.tag - 1
        selectedIndex = index
        self.buttonLeadingCollection[index].constant = buttonWidth*0.1
        UIView.animateWithDuration(0.125, delay: 0, usingSpringWithDamping: 0.35, initialSpringVelocity: 0, options: .CurveEaseIn, animations: {
            button.button.alpha = 1.0
            button.colorFill.alpha = 1.0
            self.view.layoutIfNeeded()
            }, completion: {_ in
                if !self.onMainMenu{
                    self.subTitle = button.label.text!.capitalizedString
                    let tracker = GAI.sharedInstance().defaultTracker
                    let builder = GAIDictionaryBuilder.createEventWithCategory("Submenu", action: "Viewed", label: self.subTitle, value: 1)
                    tracker.send(builder.build() as [NSObject : AnyObject])
                    switch(index){
                    case 0:
                        self.currentData = SiteData.sharedInstance.tribalMuseums
                        break
                    case 1:
                        self.currentData = SiteData.sharedInstance.tribalOwnedBusinesses
                        break
                    case 2:
                        self.currentData = SiteData.sharedInstance.tribalLands
                        break
                    case 3:
                        self.currentData = SiteData.sharedInstance.publicMuseums
                        break
                    case 4:
                        self.currentData = SiteData.sharedInstance.otherNativeBusinesses
                        break
                    case 5:
                        self.currentData = SiteData.sharedInstance.culturalTrails
                        break
                    case 6:
                        self.currentData = SiteData.sharedInstance.higherEducation
                        break
                    case 7:
                        self.currentData = SiteData.sharedInstance.spanishMuseums
                        break
                    default:
                        break
                    }
                    self.viewingSite = true
                    let range = 0...7
                    self.moveButtonOffScreen(withRange: range,animated: true)
                    let dispatchTime: dispatch_time_t = dispatch_time(DISPATCH_TIME_NOW, Int64(0.8 * Double(NSEC_PER_SEC)))
                    dispatch_after(dispatchTime, dispatch_get_main_queue(), {
                        self.performSegueWithIdentifier("viewSiteTable", sender: button)
                        self.buttonLeadingCollection[index].constant = 0
                        button.button.alpha = 0.69
                        button.colorFill.alpha = 0.69
                        self.view.layoutIfNeeded()
                    })
                }else{
                    if index == 5 { self.onMainMenu = false }
                    let range = 0...7
                    self.moveButtonOffScreen(withRange: range,animated: true)
                }
        })
        
    }
}

extension MainViewController: UIViewControllerTransitioningDelegate{
    func animationControllerForPresentedController(presented: UIViewController, presentingController presenting: UIViewController, sourceController source: UIViewController) -> UIViewControllerAnimatedTransitioning? {
        transition.presenting = true
        return transition
    }
    
    func animationControllerForDismissedController(dismissed: UIViewController) -> UIViewControllerAnimatedTransitioning? {
        transition.presenting = false
        return transition
    }
}

