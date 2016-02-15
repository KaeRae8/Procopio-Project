//
//  MapViewController.swift
//  plasmacore_ios
//
//  Created by Kenneth Shaw on 4/30/15.
//
//

import UIKit
import MapKit

protocol MapViewControllerDelegate{
    func didFinishMapOperations()
}

class MapViewController: UIViewController {

    @IBOutlet var mapView: MKMapView!
    @IBOutlet var backButton: UIButton!
    @IBOutlet var dimView: UIView!
    @IBOutlet var sitePicker: UIPickerView!
    @IBOutlet var rotatingLogo: UIButton!
    @IBOutlet var backArrow: UIButton!
    @IBOutlet var siteLabel: UILabel!
    
    
    var delegate: MapViewControllerDelegate?
    var transition: CECrossfadeAnimationController = CECrossfadeAnimationController()
    var manager: CLLocationManager = CLLocationManager()
    var siteCollection: NSMutableArray = NSMutableArray()
    var siteIndex: Int = 0
    
    var timeForProcess: CFTimeInterval = 3
    
    override init(nibName nibNameOrNil: String!, bundle nibBundleOrNil: NSBundle!) {
        super.init(nibName: nibNameOrNil, bundle: nibBundleOrNil)
    }
    
    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.rotatingLogo.addTarget(self, action: Selector("goBack"), forControlEvents: UIControlEvents.TouchUpInside)
        self.rotatingLogo.userInteractionEnabled = true
        self.backArrow.addTarget(self, action: Selector("goBack"), forControlEvents: UIControlEvents.TouchUpInside)
        self.backArrow.userInteractionEnabled = true
        
        /*self.sites = [
            CulturalSite(name: "Mingei Intl Museum", latitude: 32.730987, longitude: -117.151051),
            CulturalSite(name: "Junipero Serra Museum", latitude: 32.759702, longitude: -117.193723),
            CulturalSite(name: "San Diego Museum of Man", latitude: 32.731430, longitude: -117.152495),
            CulturalSite(name: "San Diego Natural Hist Museum", latitude: 32.732102, longitude: -117.147564),
            CulturalSite(name: "The San Diego Museum of Art", latitude: 32.732404, longitude: -117.150425),
            CulturalSite(name: "Veterans Museum and Memorial Center", latitude: 32.725812, longitude: -117.148632),
            CulturalSite(name: "Worldbeat Cultural Center", latitude: 32.726996, longitude: -117.149577),
            CulturalSite(name: "San Diego Archaeological Center", latitude: 33.089071, longitude: -116.980918),
            CulturalSite(name: "Bonita Museum and Cultural Center", latitude: 32.660887, longitude: -117.034745),
            CulturalSite(name: "California Center for the Arts", latitude: 33.122659, longitude: -117.085800),
            CulturalSite(name: "Santa Ysabel Cultural Center", latitude: 33.109109, longitude: -116.673908),
            CulturalSite(name: "San Diego Museum of Hist", latitude: 32.732438, longitude: -117.147380),
            CulturalSite(name: "Kumeyaay Ipai Interpretive Center", latitude: 32.954217, longitude: -117.055684)
        ]*/
        
        self.sitePicker.hidden = false
        self.sitePicker.delegate = self
        self.sitePicker.dataSource = self
        self.sitePicker.delegate = self
        
        
        self.siteCollection.addObject("Tribal Sponsored Museums")
        self.siteCollection.addObject("Tribal Owned Businesses")
        self.siteCollection.addObject("Tribal Lands")
        self.siteCollection.addObject("Public Museums")
        self.siteCollection.addObject("Other Native Businesses")
        self.siteCollection.addObject("Cultural Trails & Landmarks")
        self.siteCollection.addObject("Higher Education")
        self.siteCollection.addObject("Spanish Missions")
        
        mapView.showsUserLocation = true
        mapView.delegate = self
        
        self.manager.delegate = self
        self.manager.distanceFilter = kCLDistanceFilterNone
        self.manager.desiredAccuracy = kCLLocationAccuracyBest
        
        self.manager.startUpdatingLocation()
        self.manager.requestWhenInUseAuthorization()
        
        self.displaySites(self.getSiteArray("Tribal Sponsored Museums"))
        // Do any additional setup after loading the view.
        
        let tracker = GAI.sharedInstance().defaultTracker
        tracker.set(kGAIScreenName, value: "Map Screen")
        
        let builder = GAIDictionaryBuilder.createScreenView()
        tracker.send(builder.build() as [NSObject : AnyObject])
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    override func viewWillAppear(animated: Bool) {
        //self.rotatingLogo.rotate360Degrees()
    }
    
    func getSiteArray(siteName: String) -> [CulturalSite]{
        var sites : [CulturalSite] = [CulturalSite]()
        switch(siteName){
        case "Public Museums":
            sites = SiteManager.sharedInstance.museums
            break;
        case "Tribal Sponsored Museums":
            sites = SiteManager.sharedInstance.tribalMuseums
            break;
        case "Tribal Lands":
            sites = SiteManager.sharedInstance.indianLands
            break;
        case "Higher Education":
            sites = SiteManager.sharedInstance.higherEd
            break;
        case "Cultural Trails & Landmarks":
            sites = SiteManager.sharedInstance.preserves
            break;
        case "Tribal Owned Businesses":
            sites = SiteManager.sharedInstance.businesses
            break;
        case "Spanish Missions":
            sites = SiteManager.sharedInstance.missions
            break;
        case "Other Native Businesses":
            sites = SiteManager.sharedInstance.nBusinesses
            break;
        default:
            break
        }
        return sites
    }
    
    func displaySites(sites: [CulturalSite]){
        mapView.removeAnnotations(self.mapView.annotations)
        mapView.addAnnotations(sites)
        let rectToDisplay = sites.reduce(MKMapRectNull) {
            (mapRect: MKMapRect, site: CulturalSite) -> MKMapRect in
            let sitePointRect = MKMapRect(origin: site.location.mapPoint, size: MKMapSize(width: 0, height: 0))
            return MKMapRectUnion(mapRect, sitePointRect)
        }
        
        self.mapView.setVisibleMapRect(rectToDisplay, edgePadding: UIEdgeInsetsMake(10, 10, 10, 10), animated: true)
    }
    

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        // Get the new view controller using segue.destinationViewController.
        // Pass the selected object to the new view controller.
    }
    */

    func goBack(){
        self.dismissViewControllerAnimated(true, completion: {
            self.delegate?.didFinishMapOperations()
        })
    }
    @IBAction func test(sender: AnyObject) {
        NSLog("YEAH")
    }
    
    @IBAction func choosePins(sender: AnyObject) {
        self.dimView.hidden = false
        self.sitePicker.hidden = false
    }
    @IBAction func closePins(sender: AnyObject) {
        self.sitePicker.hidden = true
        self.dimView.hidden = true
        self.displaySites(self.getSiteArray(self.siteLabel.text!))
        
        let tracker = GAI.sharedInstance().defaultTracker
        let builder = GAIDictionaryBuilder.createEventWithCategory("Site Map", action: "Changed Category", label: self.siteLabel.text!, value: 1)
        tracker.send(builder.build() as [NSObject : AnyObject])
    }
}

extension UIView {
    func rotate360Degrees(duration: CFTimeInterval = 18.0, completionDelegate: AnyObject? = nil) {
        let rotateAnimation = CABasicAnimation(keyPath: "transform.rotation")
        rotateAnimation.fromValue = 0.0
        rotateAnimation.toValue = CGFloat(M_PI * 2.0)
        rotateAnimation.duration = duration
        rotateAnimation.repeatCount = 5
        
        if let delegate: AnyObject = completionDelegate {
            rotateAnimation.delegate = delegate
        }
        self.layer.addAnimation(rotateAnimation, forKey: nil)
    }
}

extension MapViewController: UIPickerViewDataSource {
    func numberOfComponentsInPickerView(pickerView: UIPickerView) -> Int {
        return 1
    }
    
    func pickerView(pickerView: UIPickerView, numberOfRowsInComponent component: Int) -> Int {
        return self.siteCollection.count
    }
}

extension MapViewController: UIPickerViewDelegate {
    func pickerView(pickerView: UIPickerView, didSelectRow row: Int, inComponent component: Int) {
        NSLog("%@ selected", self.siteCollection[row] as! String)
        self.siteIndex = row
        self.siteLabel.text = self.siteCollection[row] as? String
    }
    
    func pickerView(pickerView: UIPickerView, attributedTitleForRow row: Int, forComponent component: Int) -> NSAttributedString? {
        let attrs = NSDictionary(objects: [UIFont(name: "HelveticaNeue-Bold", size: 15.0)!,UIColor.whiteColor()], forKeys: [NSFontAttributeName,NSForegroundColorAttributeName])
        return NSAttributedString(string: self.siteCollection[row] as! String, attributes: attrs as! [String : AnyObject])
    }
}

extension MapViewController: CLLocationManagerDelegate {
    
}

extension MapViewController: MKMapViewDelegate {
    func mapView(mapView: MKMapView, viewForAnnotation annotation: MKAnnotation) -> MKAnnotationView! {
        if let site = annotation as? CulturalSite {
            let view: MKPinAnnotationView
            if let dequeueView = mapView.dequeueReusableAnnotationViewWithIdentifier("pin") as? MKPinAnnotationView {
                dequeueView.annotation = annotation
                view = dequeueView
            }else{
                view = MKPinAnnotationView(annotation: annotation, reuseIdentifier: "pin")
                view.canShowCallout = true
                view.animatesDrop = true
                view.calloutOffset = CGPoint(x: -5, y: 5)
                view.rightCalloutAccessoryView = UIButton(type: .DetailDisclosure) as UIView
            }
            view.pinColor = site.pinColor()
            return view
        }
        return nil
    }
    
    func mapView(mapView: MKMapView, annotationView view: MKAnnotationView, calloutAccessoryControlTapped control: UIControl) {
        if let site = view.annotation as? CulturalSite{
            if let alertable = site as? Alertable {
                let alert = alertable.alert()
                alert.addAction(UIAlertAction(title: "View Site Page", style: .Default) {
                    action in
                        print("Go To Site Page Here \(site.name)")
                        self.dismissViewControllerAnimated(true, completion: {
                            self.goToSitePage(site)
                        })
                })
                alert.addAction(UIAlertAction(title: "Directions", style: .Default) {
                    action in
                    print("Directions Here")
                    var coordinate = site.coordinate
                    var placemark =  MKPlacemark(coordinate: coordinate, addressDictionary: nil)
                    var mapItem = MKMapItem(placemark: placemark)
                    mapItem.name = site.name
                    
                    // Set the directions mode to "Walking"
                    // Can use MKLaunchOptionsDirectionsModeDriving instead
                    var launchOptions = NSDictionary(object: MKLaunchOptionsDirectionsModeKey, forKey: MKLaunchOptionsDirectionsModeDriving)
                    // Get the "Current User Location" MKMapItem
                    var currentLocationMapItem = MKMapItem.mapItemForCurrentLocation()
                    // Pass the current location and destination map items to the Maps app
                    // Set the direction mode in the launchOptions dictionary
                    MKMapItem.openMapsWithItems([mapItem], launchOptions: launchOptions as! [String : AnyObject])
                })
                alert.addAction(UIAlertAction(title: "Ok", style: .Cancel, handler: nil))
                self.presentViewController(alert, animated: true, completion: nil)
            }
        }
    }
    
    func goToSitePage(site: CulturalSite){
        NSLog("Site page Category %d",site.category)
        switch(site.category){
        case 1:
            SlagMethods.sharedManager().goToSitePage(site.name, withCategory:0)
            break;
        case 2:
            SlagMethods.sharedManager().goToSitePage(site.name, withCategory:1)
            break;
        case 3:
            SlagMethods.sharedManager().goToSitePage(site.name, withCategory:2)
            break;
        case 4:
            SlagMethods.sharedManager().goToSitePage(site.name, withCategory:3)
            break;
        case 5:
            SlagMethods.sharedManager().goToSitePage(site.name, withCategory:4)
            break;
        case 6:
            SlagMethods.sharedManager().goToSitePage(site.name, withCategory:5)
            break;
        case 7:
            SlagMethods.sharedManager().goToSitePage(site.name, withCategory:6)
            break;
        case 8:
            SlagMethods.sharedManager().goToSitePage(site.name, withCategory:7)
            break;
        default:
            break;
        }
    }
}

extension MapViewController: UIViewControllerTransitioningDelegate {
    
    func animationControllerForPresentedController(presented: UIViewController, presentingController presenting: UIViewController, sourceController source: UIViewController) -> UIViewControllerAnimatedTransitioning? {
        transition.reverse = false
        return transition
    }
    
    func animationControllerForDismissedController(dismissed: UIViewController) -> UIViewControllerAnimatedTransitioning? {
        transition.reverse = true
        return transition
    }
}
