//
//  SiteManager.swift
//  plasmacore_ios
//
//  Created by Kenneth Shaw on 5/17/15.
//
//

import UIKit

private let _SiteManagerSharedInstance = SiteManager()

class SiteManager: NSObject {
    var museums: [CulturalSite] = [CulturalSite]()
    var tribalMuseums: [CulturalSite] = [CulturalSite]()
    var indianLands: [CulturalSite] = [CulturalSite]()
    var higherEd: [CulturalSite] = [CulturalSite]()
    var preserves: [CulturalSite] = [CulturalSite]()
    var businesses: [CulturalSite] = [CulturalSite]()
    var missions: [CulturalSite] = [CulturalSite]()
    var nBusinesses: [CulturalSite] = [CulturalSite]()
    
    static let sharedInstance = SiteManager()
}
