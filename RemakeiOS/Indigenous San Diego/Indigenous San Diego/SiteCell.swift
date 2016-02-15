//
//  SiteCell.swift
//  Indigenous San Diego
//
//  Created by Kenneth Shaw on 11/6/15.
//  Copyright © 2015 Procopio. All rights reserved.
//

import UIKit

class SiteCell: UITableViewCell {
    @IBOutlet var title: UILabel!
    @IBOutlet var cellImg: UIImageView!
    @IBOutlet var bottomConstraint: NSLayoutConstraint!
    @IBOutlet var topConstraint: NSLayoutConstraint!
    let imageParallaxFactor: CGFloat = 80
    var imgTopInitial: CGFloat!
    var imgBottomInitial: CGFloat!
    var img: UIImageView!
    
    
    override func awakeFromNib() {
        super.awakeFromNib()
        self.clipsToBounds = true
        self.bottomConstraint.constant -= 2 * imageParallaxFactor
        self.imgTopInitial = self.topConstraint.constant
        self.imgBottomInitial = self.bottomConstraint.constant
    }

    override func setSelected(selected: Bool, animated: Bool) {
        super.setSelected(selected, animated: animated)

        // Configure the view for the selected state
    }
    
    func setBackgroundOffset(offset:CGFloat) {
        let boundOffset = max(0, min(1, offset))
        self.img.center.y = min(self.frame.size.height*0.76104651162791,max(-self.frame.size.height*0.3031007751938, 64.5 - (self.img.frame.size.height*(1.0-boundOffset))/2.0 + self.img.frame.size.height*0.15))
        let pixelOffset = (1-boundOffset)*2*imageParallaxFactor
        self.topConstraint.constant = self.imgTopInitial - pixelOffset
        self.bottomConstraint.constant = self.imgBottomInitial + pixelOffset
    }
}
