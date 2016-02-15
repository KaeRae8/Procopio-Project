package com.plasmaworks.procopio;

import android.app.*;
import android.app.Dialog;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.view.Menu;
import android.widget.Toast;
import android.widget.Spinner;
import android.widget.*;
import android.view.View;
import android.view.ViewGroup;
import java.util.ArrayList;
import java.util.List;
import android.view.View.OnClickListener;  
import android.widget.ArrayAdapter;
import android.widget.AdapterView.OnItemSelectedListener;
import java.util.*;
import java.util.concurrent.*;
import android.content.*;
import android.net.Uri;
import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GooglePlayServicesUtil;
import com.google.android.gms.maps.CameraUpdateFactory;
import com.google.android.gms.maps.GoogleMap;
import com.google.android.gms.maps.OnMapReadyCallback;
import com.google.android.gms.maps.UiSettings;
import com.google.android.gms.maps.GoogleMap.OnMapClickListener;
import com.google.android.gms.maps.GoogleMap.OnMapLoadedCallback;
import com.google.android.gms.maps.GoogleMap.OnMapLongClickListener;
import com.google.android.gms.maps.GoogleMap.OnInfoWindowClickListener;
import com.google.android.gms.maps.SupportMapFragment;
import com.google.android.gms.maps.model.LatLng;
import com.google.android.gms.maps.model.LatLngBounds;
import com.google.android.gms.maps.model.Marker;
import com.google.android.gms.maps.model.MarkerOptions;
import com.google.android.gms.maps.CameraUpdate;
import com.google.android.gms.maps.CameraUpdateFactory;
import com.plasmaworks.procopio.CulturalSite;
import com.plasmaworks.procopio.MarkerAndCategory;
import com.plasmaworks.procopio.R;

public class MapActivity extends FragmentActivity {
	
	GoogleMap googleMap;
  ArrayList<CulturalSite> museums = new ArrayList<CulturalSite>();
  ArrayList<CulturalSite> tribalMuseums = new ArrayList<CulturalSite>();
  ArrayList<CulturalSite> indianLands = new ArrayList<CulturalSite>();
  ArrayList<CulturalSite> higherEd = new ArrayList<CulturalSite>();
  ArrayList<CulturalSite> preserves = new ArrayList<CulturalSite>();
  ArrayList<CulturalSite> businesses = new ArrayList<CulturalSite>();
  ArrayList<CulturalSite> missions = new ArrayList<CulturalSite>();
  ArrayList<CulturalSite> nBusinesses = new ArrayList<CulturalSite>();
  ArrayList<MarkerAndCategory> currMarkers = new ArrayList<MarkerAndCategory>();
	SharedPreferences sharedPreferences;	
	int locationCount = 0;
  ImageButton imageButton;
  private Spinner spinner;
  boolean firstSelect = false;
 	

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

    Intent i = getIntent();
    museums = i.getParcelableArrayListExtra("museums");
    tribalMuseums = i.getParcelableArrayListExtra("tribalMuseums");
    indianLands = i.getParcelableArrayListExtra("indianLands");
    higherEd = i.getParcelableArrayListExtra("higherEd");
    preserves = i.getParcelableArrayListExtra("preserves");
    businesses = i.getParcelableArrayListExtra("businesses");
    missions = i.getParcelableArrayListExtra("missions");
    nBusinesses = i.getParcelableArrayListExtra("nBusinesses");

    spinner = (Spinner) findViewById(R.id.spinner);
    spinner.setPrompt("Choose a Category");
    List<String> list = new ArrayList<String>(); 
    list.add("Tribal Sponsored Museums");
    list.add("Tribal Owned Businesses");
    list.add("Tribal Lands");
    list.add("Public Museums");
    list.add("Other Native Businesses");
    list.add("Natural or Cultural Preserves");
    list.add("Higher Education");
    list.add("Spanish Missions");

    ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>
                     (this, android.R.layout.simple_spinner_item,list);
                      
    dataAdapter.setDropDownViewResource
                 (android.R.layout.simple_spinner_dropdown_item);
                  
    spinner.setAdapter(dataAdapter);
     
    // Spinner item selection Listener  
    addListenerOnSpinnerItemSelection();

    imageButton = (ImageButton) findViewById(R.id.imageButton);
 
		imageButton.setOnClickListener(new OnClickListener() {
 
			@Override
			public void onClick(View arg0) {
 
			   setResult(Activity.RESULT_OK, new Intent().putExtra("choice", 0));
         finish();
 
			}
 
		});
		
		// Getting Google Play availability status
    int status = GooglePlayServicesUtil.isGooglePlayServicesAvailable(getBaseContext());

        // Showing status
    if(status!=ConnectionResult.SUCCESS){ // Google Play Services are not available

        int requestCode = 10;
        Dialog dialog = GooglePlayServicesUtil.getErrorDialog(status, this, requestCode);
        dialog.show();

    }else { // Google Play Services are available        	

        // Getting reference to the SupportMapFragment of activity_main.xml
        //SupportMapFragment fm = (SupportMapFragment) getSupportFragmentManager().findFragmentById(R.id.map);

        SupportMapFragment fm = SupportMapFragment.newInstance();
        getSupportFragmentManager().beginTransaction().replace(R.id.map_container,fm).commit();

        //this you should do anyway
        fm.getMapAsync(new OnMapReadyCallback() {
            @Override
            public void onMapReady(GoogleMap map) {
                //setup map - optional

                googleMap = map;
                googleMap.setMyLocationEnabled(true);

                googleMap.setOnMapClickListener(new OnMapClickListener() {

                  @Override
                  public void onMapClick(LatLng point) { 

                  }
                });
                    
                    
                googleMap.setOnMapLongClickListener(new OnMapLongClickListener() {
                  @Override
                  public void onMapLongClick(LatLng point) { 
                    
                  }
                });

                googleMap.setOnInfoWindowClickListener(new OnInfoWindowClickListener() {

                  @Override
                  public void onInfoWindowClick(Marker marker) {
                    for (MarkerAndCategory m : currMarkers) {
                      if(m.marker.getTitle().equals(marker.getTitle())){
                        showDialog(m.marker.getTitle(),m.marker.getSnippet(),m.marker.getPosition(),m.category);
                        break;
                      }
                    } 
                  }
                });
                googleMap.setOnMapLoadedCallback(new GoogleMap.OnMapLoadedCallback() {
                  @Override
                  public void onMapLoaded() {
                    if(currMarkers.size() > 0){
                      LatLngBounds.Builder builder = new LatLngBounds.Builder();
                      for (MarkerAndCategory marker : currMarkers) {
                        builder.include(marker.marker.getPosition());
                      }
                      LatLngBounds bounds = builder.build();
                      int padding = 0; // offset from edges of the map in pixels
                      CameraUpdate cu = CameraUpdateFactory.newLatLngBounds(bounds, padding);
                      googleMap.animateCamera(cu);
                    }
                  }
                });
                currMarkers.clear();
                for(int j = 0;j<tribalMuseums.size();j++){
                  currMarkers.add(new MarkerAndCategory(drawMarker(new LatLng(tribalMuseums.get(j).lat,tribalMuseums.get(j).lon),tribalMuseums.get(j).name,tribalMuseums.get(j).url),tribalMuseums.get(j).category)); 
                }
            }
        });
    }             
	}

  public void addListenerOnSpinnerItemSelection()
  {
    spinner.setOnItemSelectedListener(new OnItemSelectedListener(){
      @Override
      public void onItemSelected(AdapterView<?> parent, View view, int pos,long id) {
        if(!firstSelect){
          firstSelect = true;
        }else{
          googleMap.clear();
          ArrayList<CulturalSite> currSites = getSiteArray(parent.getItemAtPosition(pos).toString());
          currMarkers.clear();
          for(int j = 0;j<currSites.size();j++){
            currMarkers.add(new MarkerAndCategory(drawMarker(new LatLng(currSites.get(j).lat,currSites.get(j).lon),currSites.get(j).name,currSites.get(j).url),currSites.get(j).category));
          }
          LatLngBounds.Builder builder = new LatLngBounds.Builder();
          for (MarkerAndCategory marker : currMarkers) {
              builder.include(marker.marker.getPosition());
          }
          LatLngBounds bounds = builder.build();
          int padding = 0; // offset from edges of the map in pixels
          CameraUpdate cu = CameraUpdateFactory.newLatLngBounds(bounds, padding);
          googleMap.animateCamera(cu);
        }
      }

      @Override
      public void onNothingSelected(AdapterView<?> arg0) {
          // TODO Auto-generated method stub

      }
    });
  }

  public ArrayList<CulturalSite> getSiteArray(String category)
  {
    if(category.equals("Public Museums"))
    {
       return museums;
    }else if(category.equals("Tribal Sponsored Museums"))
    {
      return tribalMuseums;
    }else if(category.equals("Tribal Lands"))
    {
      return indianLands;
    }else if(category.equals("Higher Education"))
    {
      return higherEd;
    }else if(category.equals("Natural or Cultural Preserves"))
    {
      return preserves;
    }else if(category.equals("Tribal Owned Businesses"))
    {
      return businesses;
    }else if(category.equals("Spanish Missions"))
    {
      return missions;
    }else if(category.equals("Other Native Businesses"))
    {
      return nBusinesses;
    }else{
      return new ArrayList<CulturalSite>();  
    }
  }


  void showDialog(String name,String url,LatLng pos,int category)
  {
    final Activity activity = this;
    final String title = name;
    final String site = url;
    final Double lat = pos.latitude;
    final Double lon = pos.longitude;
    final int cat = category;
	  activity.runOnUiThread(new Runnable() {
		  @Override
		  public void run() {
			  showThisDialog(activity,title,site,lat,lon,cat);
		  }
	  });
  }

  private void showThisDialog(Context ctx,String name,String url,Double lat,Double lon,int cat)
  {
    final String site = url;
    final Double latitude = lat;
    final Double longitude = lon;
    final int category = cat;
    final String n = name;
    AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);

	  // set title
	  alertDialogBuilder.setTitle(name);

	  // set dialog message
	  alertDialogBuilder
		  .setMessage("Choose an Action")
		  .setCancelable(false)
		  .setPositiveButton("Navigate",new DialogInterface.OnClickListener() {
			  public void onClick(DialogInterface dialog,int id) {
				  // if this button is clicked, close
				  // current activity
				  dialog.cancel();
          String navigate = "http://maps.google.com/maps?daddr=" + latitude + "," + longitude;
          Intent intent = new Intent(android.content.Intent.ACTION_VIEW, 
          Uri.parse(navigate));
          startActivity(intent);
			  }
		  })
      .setNeutralButton("Web Site",new DialogInterface.OnClickListener() {
			  public void onClick(DialogInterface dialog,int id) {
				  // if this button is clicked, close
				  // current activity
				  dialog.cancel();
          startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(site)));
			  }
		  })
      .setNegativeButton("Dismiss",new DialogInterface.OnClickListener() {
			  public void onClick(DialogInterface dialog,int id) {
				  // if this button is clicked, close
				  // current activity
				  dialog.cancel();
          //setResult(Activity.RESULT_OK, new Intent().putExtra("choice", 1).putExtra("name",n).putExtra("category",category));
          //finish();
			  }
		  });

	  // create alert dialog
	  AlertDialog alertDialog = alertDialogBuilder.create();

	  // show it
	  alertDialog.show();
  }
     
  private MarkerOptions drawMarker(LatLng point,String name,String url){
    // Creating an instance of MarkerOptions
    MarkerOptions markerOptions = new MarkerOptions();					
      
    // Setting latitude and longitude for the marker
    markerOptions.position(point);
    markerOptions.title(name);
    markerOptions.snippet("" + url);
      
    // Adding marker on the Google Map
    googleMap.addMarker(markerOptions);  
    return markerOptions;
  }
        
        

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}
}
