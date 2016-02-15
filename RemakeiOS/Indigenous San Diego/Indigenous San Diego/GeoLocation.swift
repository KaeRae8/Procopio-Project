//
//  GeoLocation.swift
//  plasmacore_ios
//
//  Created by Kenneth Shaw on 5/1/15.
//
//

import Foundation
import MapKit


struct GeoLocation {
    var latitude: Double
    var longitude: Double
}

extension GeoLocation {
    var coordinate: CLLocationCoordinate2D {
        return CLLocationCoordinate2DMake(self.latitude, self.longitude)
    }
    
    var mapPoint: MKMapPoint {
        return MKMapPointForCoordinate(self.coordinate)
    }
}