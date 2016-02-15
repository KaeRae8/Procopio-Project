//
//  SiteTable.swift
//  Indigenous San Diego
//
//  Created by Kenneth Shaw on 11/6/15.
//  Copyright © 2015 Procopio. All rights reserved.
//

import UIKit

class SiteTable: UIViewController {
    @IBOutlet var tableView: UITableView!
    @IBOutlet var pageTitle: UILabel!
    var oldOffset: CGPoint!
    var data: [CulturalSite]!
    var cellTapped = false
    var currentRow = -1
    var tempTitle: String!

    override func viewDidLoad() {
        super.viewDidLoad()
        tableView.registerNib(UINib(nibName: "SiteCell", bundle: nil), forCellReuseIdentifier: "cell")
        // Do any additional setup after loading the view.
        pageTitle.text = tempTitle
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
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


extension SiteTable: UITableViewDelegate{
    func tableView(tableView: UITableView, didSelectRowAtIndexPath indexPath: NSIndexPath) {
        /*var cell: SiteCell!
        if cellTapped{
            cell = tableView.cellForRowAtIndexPath(NSIndexPath(forRow: currentRow, inSection: 0)) as! SiteCell
            cell.hoursLabel.text = "Mon - Sat: 6am - 9pm"
            cell.milesLabel.text = "43 miles"
            UIView.animateWithDuration(0.5, animations: {
                cell.readMoreButton.alpha = 0.0
                cell.addressLabel.alpha = 0.0
                cell.descLabel.alpha = 0.0
                cell.sitePage.alpha = 0.0
            })
            tableView.setContentOffset(oldOffset, animated: true)
        }else{
            cell = tableView.cellForRowAtIndexPath(indexPath) as! SiteCell
            cell.hoursLabel.text = "Business Hours\nMon - Sat: 6am - 9pm"
            cell.milesLabel.text = "Travel Dist.\n43 miles"
            UIView.animateWithDuration(0.5, animations: {
                cell.readMoreButton.alpha = 1.0
                cell.addressLabel.alpha = 1.0
                cell.descLabel.alpha = 1.0
                cell.sitePage.alpha = 1.0
            })
            oldOffset = tableView.contentOffset
            tableView.scrollToRowAtIndexPath(indexPath, atScrollPosition:.Top, animated:true)
        }
        
        let selectedRowIndex = indexPath
        currentRow = selectedRowIndex.row
        tableView.beginUpdates()
        tableView.endUpdates()*/
        currentRow = indexPath.row
        self.performSegueWithIdentifier("sitePageTransition", sender: nil)
    }
    
    func tableView(tableView: UITableView, heightForRowAtIndexPath indexPath: NSIndexPath) -> CGFloat {
        /*if indexPath.row == currentRow {
            if cellTapped == false {
                cellTapped = true
                return tableView.frame.size.height
            } else {
                cellTapped = false
                return tableView.frame.size.height/5.0
            }
        }*/
        return tableView.frame.size.height/5.0
    }
    
    func scrollViewDidScroll(scrollView: UIScrollView) {
        if (scrollView == self.tableView) {
            for indexPath in self.tableView.indexPathsForVisibleRows!{
                self.setCellImageOffset(self.tableView.cellForRowAtIndexPath(indexPath) as! SiteCell, indexPath: indexPath)
            }
        }
    }
    
    func setCellImageOffset(cell: SiteCell, indexPath: NSIndexPath) {
        let cellFrame = self.tableView.rectForRowAtIndexPath(indexPath)
        let cellFrameInTable = self.tableView.convertRect(cellFrame, toView:self.tableView.superview)
        let cellOffset = cellFrameInTable.origin.y + cellFrameInTable.size.height
        let tableHeight = self.tableView.bounds.size.height + cellFrameInTable.size.height
        let cellOffsetFactor = cellOffset / tableHeight
        cell.setBackgroundOffset(cellOffsetFactor)
    }
    
    func tableView(tableView: UITableView, willDisplayCell cell: UITableViewCell, forRowAtIndexPath indexPath: NSIndexPath) {
        let siteCell = cell as! SiteCell
        self.setCellImageOffset(siteCell, indexPath: indexPath)
    }
    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        if segue.identifier == "sitePageTransition"{
            let vc = segue.destinationViewController as! SitePageViewController
            vc.data = data[currentRow] 
        }
    }
}

extension SiteTable: UITableViewDataSource{
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return data.count
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCellWithIdentifier("cell", forIndexPath: indexPath) as! SiteCell
        
        let info = data[indexPath.row]
        cell.title.text = info.name
        var fileName = info.name
        fileName = "\(fileName).jpeg"
        let imgView = UIImageView(frame: self.view.frame)
        imgView.contentMode = .ScaleAspectFit
        imgView.backgroundColor = UIColor.clearColor()
        if let img = UIImage(named:fileName){
            imgView.image = img
            cell.cellImg.image = img
        }else{
            let array: [String] = ["Misc3.jpeg", "Misc4.jpeg", "Misc5.jpeg"]
            let randomIndex = Int(arc4random_uniform(UInt32(array.count)))
            imgView.image = UIImage(named:array[randomIndex])
            cell.cellImg.image = UIImage(named:"Misc1.jpeg")
        }
        cell.img = imgView
        cell.img.hidden = true
        cell.img.center = CGPointMake(cell.bounds.midX, cell.bounds.midY)
        cell.img.autoresizingMask = [.None]
        cell.contentView.insertSubview(cell.img, belowSubview: cell.title)
        
        return cell
    }
}

extension UIView {
    @IBInspectable var cornerRadius: CGFloat {
        get {
            return layer.cornerRadius
        }
        set {
            layer.cornerRadius = newValue
            layer.masksToBounds = newValue > 0
        }
    }
}