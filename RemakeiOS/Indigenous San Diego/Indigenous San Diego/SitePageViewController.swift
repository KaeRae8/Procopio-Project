//
//  SitePageViewController.swift
//  Indigenous San Diego
//
//  Created by Kenneth Shaw on 2/6/16.
//  Copyright © 2016 Procopio. All rights reserved.
//

import UIKit
import MapKit

class SitePageViewController: UIViewController, UIAlertViewDelegate {
    @IBOutlet var blurView: FXBlurView!
    @IBOutlet var textView: UITextView!
    @IBOutlet var expandButton: UIButton!
    @IBOutlet var soundButton: UIButton!
    @IBOutlet var background: UIImageView!
    @IBOutlet var costLabel: UILabel!
    @IBOutlet var pageTitle: UILabel!
    @IBOutlet var hoursLabel: UILabel!
    @IBOutlet var directionsLabel: UILabel!
    @IBOutlet var blurHeightConstraint: NSLayoutConstraint!
    var expanded = false
    var data: CulturalSite!

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        blurView.blurRadius = 5.0
        blurView.tintColor = UIColor(red: 0.0/255, green: 0.0/255, blue: 0.0/255, alpha: 0.0)
        textView.text = data.generalInfo
        costLabel.text = "Cost: " + data.cost
        hoursLabel.text = "Hours: " + data.hours
        var fileName = data.name
        pageTitle.text = fileName
        fileName = "\(fileName).jpeg"
        background.image = UIImage(named: fileName)
        
        if SiteData.sharedInstance.playMusic{
            soundButton.setImage(UIImage(named: "sound-off.png"), forState: .Normal)
        }else{
            soundButton.setImage(UIImage(named: "sound-on.png"), forState: .Normal)
        }
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(animated)
        blurHeightConstraint.constant = self.view.frame.size.height*0.29401408450704
        self.view.layoutIfNeeded()
        textView.contentOffset = CGPoint(x: 0, y: 0)
    }
    
    func exploreAction(){
        
        if (objc_getClass("UIAlertController") == nil) {
            let alertView = UIAlertView(title: "Choose an action", message: "", delegate: self, cancelButtonTitle: "Cancel", otherButtonTitles:"Navigate","Open Website")
            alertView.show()
        } else {
            let actionSheetController: UIAlertController = UIAlertController(title: "Choose an action", message: "", preferredStyle: .ActionSheet)
            
            let cancelAction: UIAlertAction = UIAlertAction(title: "Cancel", style: .Cancel) { action -> Void in
                
            }
            actionSheetController.addAction(cancelAction)
            
            let navigationAction: UIAlertAction = UIAlertAction(title: "Navigate", style: .Default) {
                action -> Void in
                
                let tracker = GAI.sharedInstance().defaultTracker
                let builder = GAIDictionaryBuilder.createEventWithCategory("Explore Screen", action: "Navigate to Site", label: self.data.name, value: 1)
                tracker.send(builder.build() as [NSObject : AnyObject])
                var coordinate = self.data.coordinate
                var placemark =  MKPlacemark(coordinate: coordinate, addressDictionary: nil)
                var mapItem = MKMapItem(placemark: placemark)
                mapItem.name = "Destination"
                
                var launchOptions = NSDictionary(object: MKLaunchOptionsDirectionsModeKey, forKey: MKLaunchOptionsDirectionsModeDriving)
                MKMapItem.openMapsWithItems([mapItem], launchOptions: launchOptions as! [String : AnyObject])
            }
            actionSheetController.addAction(navigationAction)
            let websiteAction: UIAlertAction = UIAlertAction(title: "Open website", style: .Default) {
                action -> Void in
                let tracker = GAI.sharedInstance().defaultTracker
                let builder = GAIDictionaryBuilder.createEventWithCategory("Explore Screen", action: "Open Website", label: self.data.url, value: 1)
                tracker.send(builder.build() as [NSObject : AnyObject])
                let url = self.data.url
                NSLog("Open Site: %@", url)
                if url.rangeOfString("http://") != nil{
                    UIApplication.sharedApplication().openURL(NSURL(string:url)!)
                }else{
                    let newURL: String = NSString(format: "http://%@", url) as String
                    UIApplication.sharedApplication().openURL(NSURL(string:newURL)!)
                }
            }
            actionSheetController.addAction(websiteAction)
            
            if(UIDevice.currentDevice().userInterfaceIdiom == .Pad){
                actionSheetController.popoverPresentationController!.sourceView = self.view!
                actionSheetController.popoverPresentationController!.sourceRect = CGRectMake(self.view!.bounds.size.width / 2.0, self.view!.bounds.size.height * 0.90, 1.0, 1.0)
            }
            // this is the center of the screen currently but it can be any point in the view
            
            self.presentViewController(actionSheetController, animated: true, completion: nil)
        }
    }
    
    func alertView(alertView: UIAlertView, clickedButtonAtIndex buttonIndex: Int){
        if(alertView.title == "Choose an action"){
            if(buttonIndex == 1){
                var coordinate = self.data.coordinate
                var placemark =  MKPlacemark(coordinate: coordinate, addressDictionary: nil)
                var mapItem = MKMapItem(placemark: placemark)
                mapItem.name = "Destination"
                
                var launchOptions = NSDictionary(object: MKLaunchOptionsDirectionsModeKey, forKey: MKLaunchOptionsDirectionsModeDriving)
                MKMapItem.openMapsWithItems([mapItem], launchOptions: launchOptions as! [String : AnyObject])
            }else if(buttonIndex == 2){
                let url = self.data.url
                UIApplication.sharedApplication().openURL(NSURL(string:url)!)
            }
        }
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
    
    @IBAction func directionsAndWebsitePressed(sender: AnyObject) {
        exploreAction()
    }
    
    @IBAction func expandPressed(sender: AnyObject) {
        if expanded{
            blurHeightConstraint.constant = self.view.frame.size.height*0.29401408450704
            UIView.animateWithDuration(0.5, animations: {
                self.view.layoutIfNeeded()
            })
        }else{
            blurHeightConstraint.constant = self.view.frame.size.height*0.75352112676056
            UIView.animateWithDuration(0.5, animations: {
                self.blurView.blurRadius = 10.0
                self.view.layoutIfNeeded()
            })
        }
        expanded = !expanded
    }
    
    @IBAction func backButtonPressed(sender: AnyObject) {
        dismissViewControllerAnimated(true, completion: nil)
    }
    
    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        // Get the new view controller using segue.destinationViewController.
        // Pass the selected object to the new view controller.
    }
    */

}
