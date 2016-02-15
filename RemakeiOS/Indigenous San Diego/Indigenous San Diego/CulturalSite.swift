//
//  CulturalSite.swift
//  plasmacore_ios
//
//  Created by Kenneth Shaw on 5/1/15.
//
//

import Foundation
import MapKit

@objc protocol Alertable{
    @available(iOS 8.0, *)
    func alert() -> UIAlertController
}

class CulturalSite: NSObject {
    let name: String
    let location: GeoLocation
    let category: Int
    let url: String
    let generalInfo: String
    let hours: String
    let cost: String
    
    init(name: String, location: GeoLocation, category: Int, url: String, generalInfo: String, hours: String, cost: String) {
        self.name = name
        self.location = location
        self.category = category
        self.url = url
        self.generalInfo = generalInfo
        self.hours = hours
        self.cost = cost
    }
    
    convenience init(name: String, latitude: Double, longitude: Double, category: Int, url: String, generalInfo: String, hours: String, cost: String) {
        let location = GeoLocation(latitude: latitude, longitude: longitude)
        self.init(name: name, location: location, category: category, url:url, generalInfo:generalInfo, hours:hours, cost:cost)
    }
}

extension CulturalSite: MKAnnotation {
    var coordinate: CLLocationCoordinate2D {
        return self.location.coordinate
    }
    
    var title: String? {
        return self.name
    }
    
    func pinColor() -> MKPinAnnotationColor {
        switch(self.category){
        case 1:
            return MKPinAnnotationColor.Red
            break;
        case 2:
            return MKPinAnnotationColor.Green
            break;
        case 3:
            return MKPinAnnotationColor.Purple
            break;
        case 4:
            return MKPinAnnotationColor.Red
            break;
        case 5:
            return MKPinAnnotationColor.Green
            break;
        case 6:
            return MKPinAnnotationColor.Purple
            break;
        default:
            return MKPinAnnotationColor.Red
            break;
        }
    }
}

extension CulturalSite: Alertable {
    func alert() -> UIAlertController {
        let alert = UIAlertController(title: "Museum", message: "\(self.name)", preferredStyle: .Alert)
        return alert
    }
}