//
//  SiteTableViewController.swift
//  plasmacore_ios
//
//  Created by Kenneth Shaw on 5/1/15.
//
//

import UIKit

class SiteTableViewController: UIViewController {
    @IBOutlet var tableView: UITableView!
    var sites: [CulturalSite] = []
    
    override init(nibName nibNameOrNil: String!, bundle nibBundleOrNil: NSBundle!)  {
        // Initialize variables.
        
        super.init(nibName: nibNameOrNil, bundle: nibBundleOrNil)
    }

    required init?(coder aDecoder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.tableView.registerClass(UITableViewCell.self, forCellReuseIdentifier: "cell")
    }
}

extension SiteTableViewController: UITableViewDelegate,UITableViewDataSource {
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return sites.count
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        let cell:UITableViewCell = self.tableView.dequeueReusableCellWithIdentifier("cell")!
        
        cell.textLabel?.text = self.sites[indexPath.row].name
        
        return cell
    }
}
