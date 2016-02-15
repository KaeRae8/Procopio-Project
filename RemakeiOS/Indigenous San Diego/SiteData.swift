//
//  TribalMuseums.swift
//  Indigenous San Diego
//
//  Created by Kenneth Shaw on 2/6/16.
//  Copyright © 2016 Procopio. All rights reserved.
//

import UIKit
import AVFoundation

class SiteData: NSObject {
    //MARK: - Public Variables
    static let sharedInstance = SiteData()
    var tribalMuseums        : [CulturalSite] = [CulturalSite]()
    var culturalTrails       : [CulturalSite] = [CulturalSite]()
    var higherEducation      : [CulturalSite] = [CulturalSite]()
    var otherNativeBusinesses: [CulturalSite] = [CulturalSite]()
    var publicMuseums        : [CulturalSite] = [CulturalSite]()
    var spanishMuseums       : [CulturalSite] = [CulturalSite]()
    var tribalLands          : [CulturalSite] = [CulturalSite]()
    var tribalOwnedBusinesses: [CulturalSite] = [CulturalSite]()
    var player : AVAudioPlayer! = nil
    var playMusic = true
    
    
    //MARK: - Private functions
    private override init() {
        super.init()
    }
    
    func load(fileName: String){
        if let path = NSBundle.mainBundle().pathForResource(fileName, ofType: "plist"), array = NSArray(contentsOfFile: path) as? [AnyObject] {
            // use swift dictionary as normal
            switch(fileName){
            case "TribalMuseums":
                array as! [[NSObject: AnyObject]]
                for item in array as! [NSDictionary]{
                    self.tribalMuseums.append(CulturalSite(
                        name: item["Site"] as! String,
                        latitude: Double(item["Latitude"] as! String)!,
                        longitude: Double(item["Longitude"] as! String)!,
                        category: 2,
                        url: item["Website"] as! String,
                        generalInfo: item["General Info"] as! String,
                        hours: item["Hours"] as! String,
                        cost: item["Fees"] as! String))
                }
                //print("\(self.tribalMuseums)")
                break
            case "CulturalTrails":
                array as! [[NSObject: AnyObject]]
                for item in array as! [NSDictionary]{
                    self.culturalTrails.append(CulturalSite(
                        name: item["Site"] as! String,
                        latitude: Double(item["Latitude"] as! String)!,
                        longitude: Double(item["Longitude"] as! String)!,
                        category: 2,
                        url: item["Website"] as! String,
                        generalInfo: item["General Info"] as! String,
                        hours: item["Hours"] as! String,
                        cost: item["Fees"] as! String))
                }
                break
            case "HigherEducation":
                array as! [[NSObject: AnyObject]]
                for item in array as! [NSDictionary]{
                    self.higherEducation.append(CulturalSite(
                        name: item["Site"] as! String,
                        latitude: Double(item["Latitude"] as! String)!,
                        longitude: Double(item["Longitude"] as! String)!,
                        category: 2,
                        url: item["Website"] as! String,
                        generalInfo: item["General Info"] as! String,
                        hours: item["Hours"] as! String,
                        cost: item["Fees"] as! String))
                }
                break
            case "OtherNativeBusinesses":
                array as! [[NSObject: AnyObject]]
                for item in array as! [NSDictionary]{
                    self.otherNativeBusinesses.append(CulturalSite(
                        name: item["Site"] as! String,
                        latitude: Double(item["Latitude"] as! String)!,
                        longitude: Double(item["Longitude"] as! String)!,
                        category: 2,
                        url: item["Website"] as! String,
                        generalInfo: item["General Info"] as! String,
                        hours: item["Hours"] as! String,
                        cost: item["Fees"] as! String))
                }
                break
            case "PublicMuseums":
                array as! [[NSObject: AnyObject]]
                for item in array as! [NSDictionary]{
                    self.publicMuseums.append(CulturalSite(
                        name: item["Site"] as! String,
                        latitude: Double(item["Latitude"] as! String)!,
                        longitude: Double(item["Longitude"] as! String)!,
                        category: 2,
                        url: item["Website"] as! String,
                        generalInfo: item["General Info"] as! String,
                        hours: item["Hours"] as! String,
                        cost: item["Fees"] as! String))
                }
                break
            case "SpanishMuseums":
                array as! [[NSObject: AnyObject]]
                for item in array as! [NSDictionary]{
                    self.spanishMuseums.append(CulturalSite(
                        name: item["Site"] as! String,
                        latitude: Double(item["Latitude"] as! String)!,
                        longitude: Double(item["Longitude"] as! String)!,
                        category: 2,
                        url: item["Website"] as! String,
                        generalInfo: item["General Info"] as! String,
                        hours: item["Hours"] as! String,
                        cost: item["Fees"] as! String))
                }
                break
            case "TribalLands":
                array as! [[NSObject: AnyObject]]
                for item in array as! [NSDictionary]{
                    self.tribalLands.append(CulturalSite(
                        name: item["Site"] as! String,
                        latitude: Double(item["Latitude"] as! String)!,
                        longitude: Double(item["Longitude"] as! String)!,
                        category: 2,
                        url: item["Website"] as! String,
                        generalInfo: item["General Info"] as! String,
                        hours: item["Hours"] as! String,
                        cost: item["Fees"] as! String))
                }
                break
            case "TribalOwnedBusinesses":
                array as! [[NSObject: AnyObject]]
                for item in array as! [NSDictionary]{
                    self.tribalOwnedBusinesses.append(CulturalSite(
                        name: item["Site"] as! String,
                        latitude: Double(item["Latitude"] as! String)!,
                        longitude: Double(item["Longitude"] as! String)!,
                        category: 2,
                        url: item["Website"] as! String,
                        generalInfo: item["General Info"] as! String,
                        hours: item["Hours"] as! String,
                        cost: item["Fees"] as! String))
                }
                break
            default:
                break
            }
        }
    }
    
    func playMyFile() {
        let path = NSBundle.mainBundle().pathForResource("BirdSong", ofType:"m4a")
        let fileURL = NSURL(fileURLWithPath: path!)
        do {
            try self.player = AVAudioPlayer(contentsOfURL: fileURL)
            player.prepareToPlay()
            player.delegate = self
            player.numberOfLoops = -1
            player.play()
            playMusic = true
        } catch {
            print("Player not available")
        }
        
    }
    
    func stopPlayback(){
        playMusic = false
        player.stop()
    }
    
}

extension SiteData: AVAudioPlayerDelegate{
    func audioPlayerDidFinishPlaying(player: AVAudioPlayer, successfully flag: Bool) {
        
    }
}
