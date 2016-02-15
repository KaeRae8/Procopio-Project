//
//  ProcopioViewController.swift
//  Indigenous San Diego
//
//  Created by Kenneth Shaw on 2/6/16.
//  Copyright © 2016 Procopio. All rights reserved.
//

import UIKit

class ProcopioViewController: UIViewController {
    @IBOutlet var contentView: UIView!
    @IBOutlet var logo: UIImageView!
    @IBOutlet var titleLabel: UILabel!
    @IBOutlet var label1: UILabel!
    @IBOutlet var label2: UILabel!
    var endedEarly = false

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
    }
    
    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(animated)
        self.logo.alpha = 0.0
        self.titleLabel.alpha = 0.0
        self.label1.alpha = 0.0
        self.label2.alpha = 0.0
    }
    
    override func viewDidAppear(animated: Bool) {
        super.viewDidAppear(animated)
        self.logo.alpha = 0.0
        self.titleLabel.alpha = 0.0
        self.label1.alpha = 0.0
        self.label2.alpha = 0.0
        UIView.animateWithDuration(1.5, delay: 0.0, options: [], animations: {
            self.logo.alpha = 1.0
            self.titleLabel.alpha = 1.0
            self.label1.alpha = 1.0
            self.label2.alpha = 1.0
            }, completion: {(finished) in
                UIView.animateWithDuration(1.5, delay: 0.5, options: [], animations: {
                    self.logo.alpha = 0.0
                    self.titleLabel.alpha = 0.0
                    self.label1.alpha = 0.0
                    self.label2.alpha = 0.0
                    }, completion: {(finished) in
                        if self.endedEarly { return }
                        let dispatchTime: dispatch_time_t = dispatch_time(DISPATCH_TIME_NOW, Int64(0.5 * Double(NSEC_PER_SEC)))
                        dispatch_after(dispatchTime, dispatch_get_main_queue(), {
                            self.performSegueWithIdentifier("viewMusicSplash", sender: nil)
                        })
                })
        })
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    

    @IBAction func tappedScreen(sender: AnyObject) {
        endedEarly = true
        self.performSegueWithIdentifier("viewMusicSplash", sender: nil)
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
