//
//  MenuButton.swift
//  Indigenous San Diego
//
//  Created by Kenneth Shaw on 11/5/15.
//  Copyright © 2015 Procopio. All rights reserved.
//
import UIKit

protocol MenuButtonDelegate{
    func buttonWasTapped(button: MenuButton!)
}

@IBDesignable class MenuButton: UIView {
    
    @IBOutlet var button: UIButton!
    @IBOutlet var colorFill: UIView!
    @IBOutlet var label: UILabel!
    @IBOutlet weak private var contentView:UIView!
    var delegate: MenuButtonDelegate?
    
    func xibSetup() {
        contentView = loadViewFromNib()
        
        // use bounds not frame or it'll be offset
        contentView.frame = bounds
        
        // Make the view stretch with containing view
        contentView.autoresizingMask = [UIViewAutoresizing.FlexibleWidth, UIViewAutoresizing.FlexibleHeight]
        // Adding custom subview on top of our view (over any custom drawing > see note below)
        addSubview(contentView)
    }
    
    func loadViewFromNib() -> UIView {
        
        let bundle = NSBundle(forClass: self.dynamicType)
        let nib = UINib(nibName: "MenuButton", bundle: bundle)
        let view = nib.instantiateWithOwner(self, options: nil)[0] as! UIView
        
        return view
    }
    
    override init(frame: CGRect) {
        // 1. setup any properties here
        
        // 2. call super.init(frame:)
        super.init(frame: frame)
        
        // 3. Setup view from .xib file
        xibSetup()
    }
    
    required init?(coder aDecoder: NSCoder) {
        // 1. setup any properties here
        
        // 2. call super.init(coder:)
        super.init(coder: aDecoder)
        
        // 3. Setup view from .xib file
        xibSetup()
    }
    /*
    // Only override drawRect: if you perform custom drawing.
    // An empty implementation adversely affects performance during animation.
    override func drawRect(rect: CGRect) {
    // Drawing code
    }
    */
    @IBAction func buttonTapped(sender: AnyObject) {
        delegate?.buttonWasTapped(self)
    }
    
}
