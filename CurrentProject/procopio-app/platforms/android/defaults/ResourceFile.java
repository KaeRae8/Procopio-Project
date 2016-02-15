package com.$(DEVELOPER_PACKAGE_ID).$(PROJECT_PACKAGE_ID);

import java.io.*;
/**
 *
 * @author ty
 * @notes This class works in conjuction with ResourceDownloader
 * @notes It encaspulates a downloadable resource
 */
public class ResourceFile 
{
  //The Plasmacore API cares about this name.  This is the original name the developer gave the resource file.
  public String originalFileName=null;  

  //The Plasmacore API uses this File object to interface with the downloaded file (i.e. 'myInputStream = new FileInputStream(resource.fileHandle)')
  public File fileHandle=null;  //this is a File object that is referencing a downloaded file on the sdcard

  //ResourceFiles are constructed within the ResourceDownloader
  public ResourceFile( File _file, String _original_name, String _new_name, String _hash )
  {
    fileHandle=_file;
    originalFileName=_original_name;
    newFileName=_new_name;
    hash=_hash;
  }

  //INTERNAL STATE
  public String newFileName=null;  //this is the new name that includes the hash (e.g. actual name)
  public String hash=null;
}

