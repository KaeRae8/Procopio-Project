
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


public class CulturalSite implements Parcelable
{
  String name;
  String url;
  double lat;
  double lon;
  int category;

  public CulturalSite(Parcel source)
  {
    this.name = source.readString();
    this.url  = source.readString();
    this.lat  = source.readDouble();
    this.lon  = source.readDouble();
    this.category = source.readInt();
  }

  public CulturalSite(String str1, double num1, double num2, int num3, String str2)
  {
    name = str1;
    lat = num1;
    lon = num2;
    category = num3;
    url = str2;
  }

  public int describeContents(){
    return this.hashCode();
  }

  public void writeToParcel(Parcel dest, int flags)
  {
    dest.writeString(name);
    dest.writeString(url);
    dest.writeDouble(lat);
    dest.writeDouble(lon);
    dest.writeInt(category);
  }

  public static final Parcelable.Creator CREATOR = new Parcelable.Creator() {
    public CulturalSite createFromParcel(Parcel in){
      return new CulturalSite(in);
    }

    public CulturalSite[] newArray(int size){
      return new CulturalSite[size];
    }
  };

}
