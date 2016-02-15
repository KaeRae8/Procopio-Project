//
//  SwiftPlasmacoreAppDelegate.swift
//  plasmacore_ios
//
//  Created by Kenneth Shaw on 4/22/15.
//
//
//
//  CustomPlasmacoreAppDel.swift
//  plasmacore_ios
//
//  Created by Kenneth Shaw on 4/22/15.
//
//

import UIKit
import MapKit
import Fabric
import Crashlytics


@available(iOS 8.0, *)
class SwiftPlasmacoreAppDelegate: PlasmacoreAppDelegate {
    
    var plasmacore_app: UIApplication?
    var plasmacore_app_delegate: PlasmacoreAppDelegate?
    var plasmacore_view: PlasmacoreView?
    var plasmacore_view_controller: PlasmacoreViewController?
    var plasmacore_window: UIWindow?
    var blurView: UIVisualEffectView?
    var transition: CECrossfadeAnimationController = CECrossfadeAnimationController()
    var activityIndicator: UIActivityIndicatorView = UIActivityIndicatorView(activityIndicatorStyle: .WhiteLarge)
    
    var tempCat = 0
    var tempIndx = 0
    
    override func application(application: UIApplication, didFinishLaunchingWithOptions launchOptions: [NSObject: AnyObject]?) -> Bool {
        // Override point for customization after application launch.
        if(!super.application(application, didFinishLaunchingWithOptions: launchOptions)){
            return false
        }
        
        plasmacore_app = self.getApp()
        plasmacore_app_delegate = self.getDelegate()
        plasmacore_view = self.getView()
        plasmacore_view_controller = self.getController()
        plasmacore_window = self.getWindow()
        
        Fabric.with([Crashlytics()])
        
        print("Tracker is \(GAI.sharedInstance().trackerWithTrackingId("UA-70873100-1"))")
        
        // Optional: configure GAI options.
        let gai = GAI.sharedInstance()
        gai.trackUncaughtExceptions = true  // report uncaught exceptions
        
        let tracker = GAI.sharedInstance().defaultTracker
        
        // Enable IDFA collection.
        tracker.allowIDFACollection = true
        
        return true
    }
    
    func subMenuView(name: String){
        print("Name \(name)")
        let tracker = GAI.sharedInstance().defaultTracker
        let builder = GAIDictionaryBuilder.createEventWithCategory("Submenu", action: "Viewed", label: name, value: 1)
        tracker.send(builder.build() as [NSObject : AnyObject])
    }
    
    func showDialog() {
        //let warningAlert: UIAlertView = UIAlertView(title: "Success", message: "You Genius", delegate: nil, cancelButtonTitle: "Cancel")
        //warningAlert.show()
        /*var vc: MapViewController = MapViewController(nibName: "MapViewController", bundle: nil)
        vc.transitioningDelegate = self
        vc.delegate = self
        plasmacore_view_controller!.presentViewController(vc, animated:true, completion:nil)*/
        let sb: UIStoryboard = UIStoryboard(name: "MainStoryboard", bundle: nil)
        let vc: MapViewController = sb.instantiateViewControllerWithIdentifier("MapViewController") as! MapViewController
        vc.transitioningDelegate = self
        vc.delegate = self
        plasmacore_view_controller!.presentViewController(vc, animated:true, completion:nil)
    }
    
    func openSCTCA(){
        let tracker = GAI.sharedInstance().defaultTracker
        let builder = GAIDictionaryBuilder.createEventWithCategory("Main Screen", action: "Open Website", label: "SCTCA", value: 1)
        tracker.send(builder.build() as [NSObject : AnyObject])
        if (objc_getClass("UIAlertController") == nil) {
            let alertView = UIAlertView(title: "Visit SCTCA Website?", message: "", delegate: self, cancelButtonTitle: "Cancel", otherButtonTitles:"Open")
            alertView.show()
        } else {
            let actionSheetController: UIAlertController = UIAlertController(title: "Visit SCTCA Website?", message: "", preferredStyle: .ActionSheet)
            
            let cancelAction: UIAlertAction = UIAlertAction(title: "Cancel", style: .Cancel) { action -> Void in
                
            }
            actionSheetController.addAction(cancelAction)
            let websiteAction: UIAlertAction = UIAlertAction(title: "Open", style: .Default) {
                action -> Void in
                UIApplication.sharedApplication().openURL(NSURL(string:"http://www.sctca.com")!)
            }
            actionSheetController.addAction(websiteAction)
            
            //Present the AlertController
            self.plasmacore_view_controller!.presentViewController(actionSheetController, animated: true, completion: nil)
        }
    }
    
    func openNAKA(){
        let tracker = GAI.sharedInstance().defaultTracker
        let builder = GAIDictionaryBuilder.createEventWithCategory("Main Screen", action: "Open Website", label: "Maataam Naka Shin", value: 1)
        tracker.send(builder.build() as [NSObject : AnyObject])
        if (objc_getClass("UIAlertController") == nil) {
            let alertView = UIAlertView(title: "Visit Maataam Naka Shin Website?", message: "", delegate: self, cancelButtonTitle: "Cancel", otherButtonTitles:"Open")
            alertView.show()
        } else {
            let actionSheetController: UIAlertController = UIAlertController(title: "Visit Maataam Naka Shin Website?", message: "", preferredStyle: .ActionSheet)
            
            let cancelAction: UIAlertAction = UIAlertAction(title: "Cancel", style: .Cancel) { action -> Void in
                
            }
            actionSheetController.addAction(cancelAction)
            let websiteAction: UIAlertAction = UIAlertAction(title: "Open", style: .Default) {
                action -> Void in
                UIApplication.sharedApplication().openURL(NSURL(string:"http://www.nakashin.org")!)
            }
            actionSheetController.addAction(websiteAction)
            
            //Present the AlertController
            self.plasmacore_view_controller!.presentViewController(actionSheetController, animated: true, completion: nil)
        }
    }
    
    func openPROCO(){
        let tracker = GAI.sharedInstance().defaultTracker
        let builder = GAIDictionaryBuilder.createEventWithCategory("Main Screen", action: "Open Website", label: "Procopio", value: 1)
        tracker.send(builder.build() as [NSObject : AnyObject])
        if (objc_getClass("UIAlertController") == nil) {
            let alertView = UIAlertView(title: "Visit Procopio Website?", message: "", delegate: self, cancelButtonTitle: "Cancel", otherButtonTitles:"Open")
            alertView.show()
        } else {
            let actionSheetController: UIAlertController = UIAlertController(title: "Visit Procopio Website?", message: "", preferredStyle: .ActionSheet)
            
            let cancelAction: UIAlertAction = UIAlertAction(title: "Cancel", style: .Cancel) { action -> Void in
                
            }
            actionSheetController.addAction(cancelAction)
            let websiteAction: UIAlertAction = UIAlertAction(title: "Open", style: .Default) {
                action -> Void in
                UIApplication.sharedApplication().openURL(NSURL(string:"http://www.procopio.com")!)
            }
            actionSheetController.addAction(websiteAction)
            
            //Present the AlertController
            self.plasmacore_view_controller!.presentViewController(actionSheetController, animated: true, completion: nil)
        }
    }
    
    func showSpinner(){
        activityIndicator.color = UIColor.whiteColor()
        activityIndicator.alpha = 1.0
        activityIndicator.center = plasmacore_view_controller!.view.center
        activityIndicator.hidesWhenStopped = false
        plasmacore_view_controller!.view.addSubview(activityIndicator)
        activityIndicator.startAnimating()
        SlagMethods.sharedManager().spinnerShowing()
    }
    
    func removeSpinner(){
        activityIndicator.removeFromSuperview()
        activityIndicator.stopAnimating()
    }
    
    func exploreAction(category: Int, index: Int){
        
        if (objc_getClass("UIAlertController") == nil) {
            tempCat = category
            tempIndx = index
            let alertView = UIAlertView(title: "Choose an action", message: "", delegate: self, cancelButtonTitle: "Cancel", otherButtonTitles:"Navigate","Open Website")
            alertView.show()
        } else {
            let actionSheetController: UIAlertController = UIAlertController(title: "Choose an action", message: "", preferredStyle: .ActionSheet)
            
            let cancelAction: UIAlertAction = UIAlertAction(title: "Cancel", style: .Cancel) { action -> Void in
                
            }
            actionSheetController.addAction(cancelAction)
            
            var site : CulturalSite?
            switch(category){
            case 0:
                site = SiteManager.sharedInstance.museums[index]
            case 1:
                site = SiteManager.sharedInstance.tribalMuseums[index]
            case 2:
                site = SiteManager.sharedInstance.indianLands[index]
            case 3:
                site = SiteManager.sharedInstance.higherEd[index]
            case 4:
                site = SiteManager.sharedInstance.preserves[index]
            case 5:
                site = SiteManager.sharedInstance.businesses[index]
            case 6:
                site = SiteManager.sharedInstance.missions[index]
            case 7:
                site = SiteManager.sharedInstance.nBusinesses[index]
            default:
                break
            }
            
            let navigationAction: UIAlertAction = UIAlertAction(title: "Navigate", style: .Default) {
                action -> Void in
                
                let tracker = GAI.sharedInstance().defaultTracker
                let builder = GAIDictionaryBuilder.createEventWithCategory("Explore Screen", action: "Navigate to Site", label: site!.name, value: 1)
                tracker.send(builder.build() as [NSObject : AnyObject])
                var coordinate = site!.coordinate
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
                let builder = GAIDictionaryBuilder.createEventWithCategory("Explore Screen", action: "Open Website", label: site!.url, value: 1)
                tracker.send(builder.build() as [NSObject : AnyObject])
                NSLog("Open Site: %@", site!.url)
                if site!.url.rangeOfString("http://") != nil{
                    UIApplication.sharedApplication().openURL(NSURL(string:site!.url)!)
                }else{
                    var newURL: String = NSString(format: "http://%@", site!.url) as String
                    UIApplication.sharedApplication().openURL(NSURL(string:newURL)!)
                }
            }
            actionSheetController.addAction(websiteAction)
            
            if(UIDevice.currentDevice().userInterfaceIdiom == .Pad){
                actionSheetController.popoverPresentationController!.sourceView = self.plasmacore_view!
                actionSheetController.popoverPresentationController!.sourceRect = CGRectMake(self.plasmacore_view!.bounds.size.width / 2.0, self.plasmacore_view!.bounds.size.height * 0.90, 1.0, 1.0)
            }
            // this is the center of the screen currently but it can be any point in the view
            
            self.plasmacore_view_controller!.presentViewController(actionSheetController, animated: true, completion: nil)
        }
    }
    
    func showBlurBox(heightOffset: Double){
        let blurEffect = UIBlurEffect(style: .Light)
        self.blurView = UIVisualEffectView(effect: blurEffect)
        self.blurView!.translatesAutoresizingMaskIntoConstraints = false
        self.blurView?.frame = CGRectMake(0.0, CGFloat(heightOffset/2.0), UIScreen.mainScreen().bounds.size.width, UIScreen.mainScreen().bounds.size.height/3.0)
        plasmacore_view!.insertSubview(self.blurView!, atIndex: 0)
    }
    
    func addSite(name: String, forCategory category: String, withLat lat: Double, Lon lon:Double, website site:String){
        switch(category){
        case "Museums":
            SiteManager.sharedInstance.museums.append(CulturalSite(name: name, latitude: lat, longitude: lon, category: 1, url: site))
            break;
        case "TribalMuseums":
            SiteManager.sharedInstance.tribalMuseums.append(CulturalSite(name: name, latitude: lat, longitude: lon, category: 2, url: site))
            break;
        case "Lands":
            SiteManager.sharedInstance.indianLands.append(CulturalSite(name: name, latitude: lat, longitude: lon, category: 3, url: site))
            break;
        case "Education":
            SiteManager.sharedInstance.higherEd.append(CulturalSite(name: name, latitude: lat, longitude: lon, category: 4, url: site))
            break;
        case "Preserves":
            SiteManager.sharedInstance.preserves.append(CulturalSite(name: name, latitude: lat, longitude: lon, category: 5, url: site))
            break
        case "Businesses":
            SiteManager.sharedInstance.businesses.append(CulturalSite(name: name, latitude: lat, longitude: lon, category: 6, url: site))
            break;
        case "Missions":
            SiteManager.sharedInstance.missions.append(CulturalSite(name: name, latitude: lat, longitude: lon, category: 7, url: site))
            break;
        case "NBusinesses":
            SiteManager.sharedInstance.nBusinesses.append(CulturalSite(name: name, latitude: lat, longitude: lon, category: 8, url: site))
            break;
        default:
            break;
        }
    }
}

@available(iOS 8.0, *)
extension SwiftPlasmacoreAppDelegate: UIAlertViewDelegate{
    func alertView(alertView: UIAlertView, clickedButtonAtIndex buttonIndex: Int){
        if(alertView.title == "Choose an action"){
            if(buttonIndex == 1){
                var site = self.getSite(tempCat, idx: tempIndx)
                var coordinate = site!.coordinate
                var placemark =  MKPlacemark(coordinate: coordinate, addressDictionary: nil)
                var mapItem = MKMapItem(placemark: placemark)
                mapItem.name = "Destination"
                
                var launchOptions = NSDictionary(object: MKLaunchOptionsDirectionsModeKey, forKey: MKLaunchOptionsDirectionsModeDriving)
                MKMapItem.openMapsWithItems([mapItem], launchOptions: launchOptions as! [String : AnyObject])
            }else if(buttonIndex == 2){
                var site = self.getSite(tempCat, idx: tempIndx)
                UIApplication.sharedApplication().openURL(NSURL(string:site!.url)!)
            }
        }else if(alertView.title == "Visit SCTCA Website?"){
            UIApplication.sharedApplication().openURL(NSURL(string:"http://www.sctca.com")!)
        }else if(alertView.title == "Visit Maataam Naka Shin Website?"){
            UIApplication.sharedApplication().openURL(NSURL(string:"http://www.nakashin.org")!)
        }else if(alertView.title == "Visit Procopio Website?"){
            UIApplication.sharedApplication().openURL(NSURL(string:"http://www.procopio.com")!)
        }
    }
    
    func getSite(cat: Int, idx: Int) -> CulturalSite?{
        var site : CulturalSite?
        switch(tempCat){
        case 0:
            site = SiteManager.sharedInstance.museums[tempIndx]
        case 1:
            site = SiteManager.sharedInstance.tribalMuseums[tempIndx]
        case 2:
            site = SiteManager.sharedInstance.indianLands[tempIndx]
        case 3:
            site = SiteManager.sharedInstance.higherEd[tempIndx]
        case 4:
            site = SiteManager.sharedInstance.preserves[tempIndx]
        case 5:
            site = SiteManager.sharedInstance.businesses[tempIndx]
        case 6:
            site = SiteManager.sharedInstance.missions[tempIndx]
        case 7:
            site = SiteManager.sharedInstance.nBusinesses[tempIndx]
        default:
            break
        }
        return site
    }
}

@available(iOS 8.0, *)
extension SwiftPlasmacoreAppDelegate: MapViewControllerDelegate{
    func didFinishMapOperations() {
        SlagMethods.sharedManager().backFromMaps()
    }
}

@available(iOS 8.0, *)
extension SwiftPlasmacoreAppDelegate: UIViewControllerTransitioningDelegate {
    
    func animationControllerForPresentedController(presented: UIViewController, presentingController presenting: UIViewController, sourceController source: UIViewController) -> UIViewControllerAnimatedTransitioning? {
        transition.reverse = false
        return transition
    }
    
    func animationControllerForDismissedController(dismissed: UIViewController) -> UIViewControllerAnimatedTransitioning? {
        transition.reverse = true
        return transition
    }
}

