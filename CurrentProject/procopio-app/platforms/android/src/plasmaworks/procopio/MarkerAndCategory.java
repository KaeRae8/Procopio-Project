
package com.plasmaworks.procopio;

import java.io.*;
import java.util.*;
import java.security.MessageDigest;
import android.content.res.AssetManager;
import android.app.Activity;
import java.net.*;
import android.util.Log;
import android.os.*;
import android.os.Parcelable;
import android.os.Parcel;
import com.google.android.gms.maps.model.Marker;
import com.google.android.gms.maps.model.MarkerOptions;


public class MarkerAndCategory
{
  MarkerOptions marker;
  int category;

  public MarkerAndCategory(MarkerOptions marker,int category)
  {
    this.marker = marker;
    this.category  = category;
  }
  
}
