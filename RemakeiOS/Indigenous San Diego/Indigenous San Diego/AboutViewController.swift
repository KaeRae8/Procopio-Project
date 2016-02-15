//
//  AboutViewController.swift
//  Indigenous San Diego
//
//  Created by Kenneth Shaw on 2/6/16.
//  Copyright © 2016 Procopio. All rights reserved.
//

import UIKit

class AboutViewController: UIViewController {
    @IBOutlet var textView: UITextView!
    @IBOutlet var soundButton: UIButton!
    var aboutApp: NSMutableAttributedString!
    var aboutSCTCA: NSMutableAttributedString!
    var aboutNAKA: NSMutableAttributedString!
    var aboutProcopio: NSMutableAttributedString!

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        
        let about = "Indigenous San Diego highlights the Indigenous presence around San Diego. Indigenous San Diego was designed to raise public interest and awareness about San Diego's Indigenous museums, exhibits, businesses, lands and public sites of interest. Indigenous peoples are a significant part of Southern California's history and present day. \n\n" as NSString
        
        aboutApp = NSMutableAttributedString(string: about as String, attributes: [NSFontAttributeName:UIFont.systemFontOfSize(16.0),NSForegroundColorAttributeName: UIColor.whiteColor()])
        
        let boldFontAttribute = [NSFontAttributeName: UIFont.boldSystemFontOfSize(16.0),NSForegroundColorAttributeName: UIColor.whiteColor()]
        
        // Part of string to be bold
        aboutApp.addAttributes(boldFontAttribute, range: about.rangeOfString("Indigenous San Diego"))
        
        let sctca = "The Southern California Tribal Chairman's Association (SCTCA) is a consortium of 19 federally-recognized tribal governments in Southern California which serves the needs of its tribal members and descendants. SCTCA's Board is comprised of tribal chairpersons from each of its member Tribes. For more information: www.sctca.net \n\n" as NSString
        
        aboutSCTCA = NSMutableAttributedString(string: sctca as String, attributes: [NSFontAttributeName:UIFont.systemFontOfSize(16.0),NSForegroundColorAttributeName: UIColor.whiteColor()])
        
        let boldFontAttribute2 = [NSFontAttributeName: UIFont.boldSystemFontOfSize(16.0),NSForegroundColorAttributeName: UIColor.whiteColor()]
        
        // Part of string to be bold
        aboutSCTCA.addAttributes(boldFontAttribute2, range: sctca.rangeOfString("The Southern California Tribal Chairman's Association"))
        
        let naka = "Maataam Naka Shin was created by SCTCA to advance knowledge and understanding of area Indigenous cultures through history, art, traditions, and native science.  www.nakashin.org \n\n" as NSString
        
        aboutNAKA = NSMutableAttributedString(string: naka as String, attributes: [NSFontAttributeName:UIFont.systemFontOfSize(16.0),NSForegroundColorAttributeName: UIColor.whiteColor()])
        
        let boldFontAttribute3 = [NSFontAttributeName: UIFont.boldSystemFontOfSize(16.0),NSForegroundColorAttributeName: UIColor.whiteColor()]
        
        // Part of string to be bold
        aboutNAKA.addAttributes(boldFontAttribute3, range: naka.rangeOfString("Maataam Naka Shin"))
        
        let procopio = "Procopio's Native American Law Practice represents tribal governments and businesses, intertribal organizations and tribal-affiliated clients throughout the West. www.procopio.com. \n\nDeveloped by Kenny Shaw in association with Procopio\n\n" as NSString
        
        aboutProcopio = NSMutableAttributedString(string: procopio as String, attributes: [NSFontAttributeName:UIFont.systemFontOfSize(16.0),NSForegroundColorAttributeName: UIColor.whiteColor()])
        
        let boldFontAttribute4 = [NSFontAttributeName: UIFont.boldSystemFontOfSize(16.0),NSForegroundColorAttributeName: UIColor.whiteColor()]
        
        // Part of string to be bold
        aboutProcopio.addAttributes(boldFontAttribute4, range: procopio.rangeOfString("Procopio's"))
        
        aboutApp.appendAttributedString(aboutSCTCA)
        
        aboutApp.appendAttributedString(aboutNAKA)
        
        aboutApp.appendAttributedString(aboutProcopio)
        
        textView.attributedText = aboutApp
        textView.tintColor = UIColor.whiteColor()
        textView.linkTextAttributes = [NSUnderlineStyleAttributeName: NSUnderlineStyle.StyleSingle.rawValue]
        
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
    
    @IBAction func soundButtonPressed(sender: AnyObject) {
        if SiteData.sharedInstance.playMusic{
            soundButton.setImage(UIImage(named: "sound-off.png"), forState: .Normal)
            SiteData.sharedInstance.stopPlayback()
        }else{
            soundButton.setImage(UIImage(named: "sound-on.png"), forState: .Normal)
            SiteData.sharedInstance.playMyFile()
        }
    }
    
    @IBAction func backPressed(sender: AnyObject) {
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
