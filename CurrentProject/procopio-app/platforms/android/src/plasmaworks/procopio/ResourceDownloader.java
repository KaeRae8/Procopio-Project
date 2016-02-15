//=============================================================================
// ResourceDownloader.java
//
// $(PLASMACORE_VERSION) (2015.05.10)
//
// http://plasmaworks.com/plasmacore
//
// Resource downloader for Android.
//
// ----------------------------------------------------------------------------
//
// $(COPYRIGHT)
//
// Licensed under the Apache License, Version 2.0 (the "License"); 
// you may not use this file except in compliance with the License. 
// You may obtain a copy of the License at 
//
//   http://www.apache.org/licenses/LICENSE-2.0 
//
// Unless required by applicable law or agreed to in writing, 
// software distributed under the License is distributed on an 
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific 
// language governing permissions and limitations under the License.
//
// ----------------------------------------------------------------------------
// History:
//   2009       / Ty Heath   - Original implementation.
//   2010.04.20 / Abe Pralle - Modified to be poll-driven.
//=============================================================================
package com.plasmaworks.procopio;

import java.io.*;
import java.util.*;
import java.security.MessageDigest;
import android.content.res.AssetManager;
import android.app.Activity;
import java.net.*;
import android.util.Log;
import android.os.*;

/**
 *
 * @author Ty Heath
 * @notes Instantiating a ResourceDownloader causes it to begin syncing resources
 * @notes All callback notifications will occur in the main thread
 *
 *
 */
public class ResourceDownloader
{
  //CONSTRUCTOR
  //@Parameters:
  //  _program_name = the directory name on the host that contains the program files we are going to download (i.e. 'armageddon_rider')
  //  _activity = The main activity, we need a reference to it here to do file i/o due to androids security model
  // 
  public ResourceDownloader( String _program_name, String key_filename, 
      Activity _activity)
  {
    Log.i("Plasmacore.ResourceDownloader", "ResourceDownloader instantiated, resource syncing will commence soon");

    program=_program_name;
    activity=_activity;

    readKeyFile(key_filename);
    if (error != null) return;

    resource_files = resources.iterator();

    // Make sure there's at least one file that needs loading
    for (ResourceFile f : resources)
    {
      if ( !f.fileHandle.exists() ) return;
    }

    // Resources already loaded
    progress = 1.0;
  }

  //gitrdone
  public void update()
  {
    // allow loading screen to be drawn first
    if (idle > 0)
    {
      --idle;
      return;
    }

    syncFile( resource_files.next() );
    progress = totalRead / (double)resourceTotalSize;
    if ( !resource_files.hasNext() || progress > 1.0 ) progress = 1.0;
  }

  //This method scans the key.txt file and generates a list of resources that need to be synced or checked
  private void readKeyFile( String key_filename )
  {
    ByteArrayOutputStream data=new ByteArrayOutputStream(16384);

    try
    {
      InputStream is = activity.getAssets().open(key_filename);

      //read the key file to extract information about downloadable resource files
      byte buf[]=new byte[512];
      int nread;
      while( (nread=is.read(buf)) != -1 )
      {
        if (nread > 0) 
        {
          data.write( buf, 0, nread );
        }
        else
        {
          try
          {
            Thread.sleep(5);
          }
          catch (InterruptedException err)
          {
          }
        }
      }
      is.close();
    }
    catch (IOException err)
    {
      error = "Error reading keyfile \"" + key_filename + "\".";
      return;
    }

    String sdata=new String(data.toByteArray());
    String rows[]=sdata.split("[\n]");
    base_url = rows[0].trim();
    Log.i("ResourceDownloader", "Base URL: "+base_url);

    try
    {
      resourceTotalSize=Integer.parseInt(rows[1].trim());
      Log.i("ResourceDownloader", "Total resource collection size = "+resourceTotalSize);
    }
    catch (RuntimeException err)
    {
      error = "Error parsing resource collection size from key file.";
      return;
    }

    //Do extensive tests on the state of the sd card
    if ( !checkSDCardState() ) return;  

    Log.i("ResourceDownloader", "The SDCard state is mounted and tests confirm it's ready");

    String sdcard_dir = Environment.getExternalStorageDirectory().getAbsolutePath() 
                        + File.separator;

    //make a data directory for this program
    File program_path=new File(sdcard_dir+"plasmacore"+File.separator+program+File.separator);
    if ( !program_path.exists() )
    {
      // Android Plasmacore originally used paths like 
      // "/sdcard/.plasmacore/.robot_football".  This has been changed to be
      // "/sdcard/plasmacore/robot_football".  If the old directory exists
      // just rename it to the new one.
      File old_path = new File(sdcard_dir+".plasmacore" + File.separator + "." + program);
      if (old_path.exists())
      {
        new File(sdcard_dir+"plasmacore").mkdirs();
        old_path.renameTo(program_path);
      }
      else
      {
        program_path.mkdirs();
      }
    }

    //The key file contains one file per row
    for(int i=2;i<rows.length;i++)
    {
      //this is a targetj
      String row = rows[i].trim();
      if (row.length()<4) continue;

      String parts[]=row.split("\\Q.\\E"); //split around '.'
      if(parts[parts.length-1].compareTo("key")!=0)  
      {
        error = "Corrupt resource key file"; //WTH
        return;
      }

      String hash=parts[parts.length-2];  //keep the hash for verification
      StringBuffer original_name=new StringBuffer(row.length());  //the original file name is at the start of the row
      //build the original file name from all parts minus the last 2
      for(int j=0;j<parts.length-2;j++){  
        original_name.append(parts[j]);
        if(j<parts.length-3)
          original_name.append(".");
      }

      //create our target
      ResourceFile resource_file=new ResourceFile(
          new File(program_path.getAbsolutePath()+File.separator+row),
          original_name.toString(),row,hash);
      //remember it
      resources.add(resource_file);
    }
  }

  //Check if the target exists, and if it doesn't download and do a 1 time hash verification
  private void syncFile(ResourceFile target)
  {
    if (target == null) return;

    InputStream is=null;
    FileOutputStream fos=null;
    if(target.fileHandle.exists())
    {
      totalRead+=(int)target.fileHandle.length();
      return;  //already done this bit.
    }
    try{
      File dir=new File(target.fileHandle.getParent());
      if(!dir.exists())
        dir.mkdirs();

      //try up to 2 times to download a file and verify the hash
      int max_tries = 3;
      for(int attempt=1;attempt<=max_tries;attempt++)
      {
        try
        {
          File tmpout=new File(target.fileHandle.getAbsolutePath()+".part");
          fos=new FileOutputStream(tmpout);

          URL url=new URL(base_url+"/"+target.newFileName);
          URLConnection urlConn = url.openConnection();
          is = urlConn.getInputStream();
          BufferedInputStream bis = new BufferedInputStream(is,16384);

          byte buf[]=new byte[16384];
          int nread;
          int this_file_size=0;
          SHA1 sha1=new SHA1();
          while( (nread=bis.read(buf)) != -1 ) 
          {
            if (nread > 0)
            {
              totalRead+=nread;
              this_file_size+=nread;
              fos.write(buf,0,nread);
              //optimization: hash the data as we receive it so we don't have to do a full file read operation later
              sha1.update(buf,0,nread);  
            }
            else
            {
              try
              {
                Thread.sleep(5);
              }
              catch (InterruptedException err)
              {
              }
            }
          }
          is.close();
          fos.close();
          is=null;
          fos=null;
          byte hash_bytes[]=sha1.digest();

          StringBuffer hash_out=new StringBuffer(41);

          //convert byte array into a base 16 string
          for(int i=0;i<hash_bytes.length;i++){
            int val=0x80 & hash_bytes[i];
            val+=(hash_bytes[i]&0x7f);
            String hex_val=Integer.toHexString(val);
            hash_out.append(hex_val);
          }

          //hash the temporary file to make sure it is not corrupt
          String hash=hash_out.toString();
          if(hash.compareTo(target.hash)!=0)
          {
            if (attempt < max_tries) continue;
            error = "Downloaded data was corrupt, restart and try again";
            return;
          }
          //Log.i("Plasmacore.ResourceDownloader","Downloaded file ("+this_file_size+" bytes): "+target.newFileName);
          //Log.i("Plasmacore.ResourceDownloader","Verified hashes ("+hash+" vs "+target.hash+"), the downloaded resource is good!");
          //only when it gets here is it suitable to be renamed thus making it a final copy
          tmpout.renameTo(target.fileHandle);
          break;
        }catch(Exception rutro){
          try{
            if(fos != null) fos.close();
            fos=null;
          }catch(Exception ignoramous){}
          try{
            if(is != null) is.close();
            is=null;
          }catch(Exception ignoramous){}
          if(attempt==max_tries) 
          {
            error = rutro.toString();
            return;
          }
        }
      }
    }catch(Exception e){
      error = "Could not download program data.  Check internet connection and try again.";
      return;
    }finally{
      if(is!=null){
        try{
          is.close();
        }catch(Exception ex){}
      }
      if(fos!=null){
        try{
          fos.close();
        }catch(Exception ex){}
      }
    }
  }

  //Do extensive tests on the sd card to verify it exists, is writable, and has space
  private boolean checkSDCardState() 
  {
    //Test the state of the device (mounted etc)
    String state=Environment.getExternalStorageState();
    if(state.compareTo(Environment.MEDIA_MOUNTED)!=0)
    {
      error = "The SD card is not mounted and writable ("+state+")";
      return false;
    }

    //test writability to confirm
    File sdcard=Environment.getExternalStorageDirectory();
    if(!sdcard.canWrite() )
    {
      error = "The SD card is mounted but tests report it is not writable";
      return false;
    }

    //Test the free space
    StatFs fs = new StatFs(sdcard.getAbsolutePath());
    long free_space=(long)fs.getAvailableBlocks()*(long)fs.getBlockSize();
    Log.i("Plasmacore.ResourceDownloader", "The SDCard has "+((double)free_space/(double)(1024*1024))+" MB of free space");

    if(free_space<resourceTotalSize)
    {
      error = "Insufficient space on SD card to download game files.";
      return false;
    }

    //if all these tests passed it should be good to go
    return true;
  }

  //internal class used to do SHA-1 hashing operations for file verification
  public class SHA1 
  {
    MessageDigest digester=null;
    byte[] result=null;
    public SHA1() throws Exception{
      digester = MessageDigest.getInstance("SHA-1");
    }

    public String convertToHex(byte[] data) throws Exception{
      StringBuffer buf = new StringBuffer();
      for (int i = 0; i < data.length; i++) {
        int halfbyte = (data[i] >>> 4) & 0x0F;
        int two_halfs = 0;
        do {
          if ((0 <= halfbyte) && (halfbyte <= 9))
            buf.append((char) ('0' + halfbyte));
          else
            buf.append((char) ('a' + (halfbyte - 10)));
          halfbyte = data[i] & 0x0F;
        } while(two_halfs++ < 1);
      }
      return buf.toString();
    }

    public void reset() throws Exception{
      result=null;
      digester = MessageDigest.getInstance("SHA-1");
    }

    //add data to be hashed
    public void update(byte []data){
      digester.update(data);
    }

    public void update(byte []data, int offset, int length){
      digester.update(data,offset,length);
    }

    //add data to be hashed
    public void updateFromText(String text) throws Exception{
      update(text.getBytes("iso-8859-1"));
    }

    //finalize the hash
    public byte[] digest() throws Exception{
      result=digester.digest();
      return result;
    }

    public String getResultHex() throws Exception{
      return convertToHex(result);
    }

    public byte[] getResult(){
      return result;
    }

    public byte[] digest(String text) throws Exception{
      updateFromText(text);
      return digest();
    }

    public byte[] digest(byte []data) throws Exception{
      update(data);
      return digest();
    }
  }

  //internal state members
  private String program;
  private String base_url;
  private Activity activity;
  private int resourceTotalSize=512;  //gets set during init to a realistic value, this is the total cummulative size of all resources
  private int totalRead=0;  //this is how many bytes have been processed so far with respect to downloaded resources
  private int idle = 2;  // remain idle for 2 updates at start
  private Iterator<ResourceFile> resource_files;

  public String  error;
  public double  progress;
  public ArrayList<ResourceFile> resources = new ArrayList<ResourceFile>();

}
