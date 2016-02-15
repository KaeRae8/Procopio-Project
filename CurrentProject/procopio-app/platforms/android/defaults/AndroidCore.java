//=============================================================================
// AndroidCore.java
//
// $(PLASMACORE_VERSION) $(DATE)
//
// Primary Java-side Plasmacore implementation.
//
// ----------------------------------------------------------------------------
//
// $(COPYRIGHT)
//
//   http://plasmaworks.com/plasmacore
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
//
// Adapted from the following Google sample code:
//
//   http://code.google.com/p/apps-for-android/source/browse/trunk/
//     Samples/OpenGLES/Triangle/src/com/google/android/opengles/triangle/
//     GLView.java?r=25
//
//=============================================================================
package com.$(DEVELOPER_PACKAGE_ID).$(PROJECT_PACKAGE_ID);

import java.io.*;
import java.lang.reflect.*;
import java.util.*;
import java.util.concurrent.*;
import android.app.*;
import android.content.*;
import android.content.res.*;
import android.graphics.*;
import android.hardware.*;
import android.media.*;
import android.net.*;
import android.os.*;
import android.os.PowerManager.*;
import android.opengl.*;
import android.telephony.*;
import android.util.*;
import android.view.*;
import android.view.inputmethod.*;
import android.widget.*;
import android.webkit.*;
import javax.microedition.khronos.egl.*;
import javax.microedition.khronos.opengles.*;

public class AndroidCore extends Activity implements SensorEventListener
{
  static
  {
    System.loadLibrary( "$(PROJECT_PACKAGE_ID)" );
  }

  final static public String LOGTAG = "Plasmacore";
  static AndroidCore instance;

  final static public int res_general = 0;
  final static public int res_data    = 1;
  final static public int res_image   = 2;
  final static public int res_sound   = 3;

  // PROPERTIES
  String project_id, key_filename;
  ArrayList<String> assets = new ArrayList<String>();
  ArrayList<ResourceFile> resources;
  ResourceDownloader downloader;

  PowerManager.WakeLock wake_lock;
  java.util.Timer       release_wake_timer;

  volatile boolean pausing;
  volatile boolean paused;

  AbsoluteLayout layout;
  GLSurfaceView view;

  TouchManager touch_manager = new TouchManager();
  boolean showing_keyboard=false;
  boolean hard_keyboard_hidden=true;
  double acceleration_x;
  double acceleration_y;
  double acceleration_z;

  byte[] io_buffer;
  ResourceBank<FileInputStream>  infile_bank  = new ResourceBank<FileInputStream>();
  ResourceBank<FileOutputStream> outfile_bank = new ResourceBank<FileOutputStream>();
  ResourceBank<AndroidSound>     sound_bank = new ResourceBank<AndroidSound>();
  ResourceBank<PCWebView>        web_view_bank = new ResourceBank<PCWebView>();

  // introspection
  Method m_getX;
  Method m_getY;
  Method m_getPointerCount;
  Method m_getPointerId;

  Handler alert_handler = new Handler()
  {
    public void handleMessage( Message m ) { alert( (String) m.obj ); }
  };

  Handler new_web_view_handler = new Handler()
  {
    public void handleMessage( Message m ) 
    {
      synchronized (web_view_bank)
      {
        PCWebView view = new PCWebView((AndroidCore)m.obj);
        view.getSettings().setJavaScriptEnabled(true);
        web_view_bank.set( m.what, view );

        view.setWebViewClient(
          new WebViewClient()
          {
            public void onPageFinished( WebView view, String url )
            {
              PCWebView web_view = (PCWebView) view;
              web_view.loaded = true;
              web_view.set_visible( true );
            }

            public void onReceivedError( WebView view, int error_code, String descr, String url )
            {
              ((PCWebView) view).failed = true;
            }

            public boolean shouldOverrideUrlLoading( WebView view, String url )
            {
              if (url.startsWith("market:"))
              {
                Intent intent = new Intent( Intent.ACTION_VIEW, Uri.parse(url) );
                startActivity(intent); 
                return true;
              }
              return false;
            }
          } );
      }
    }
  };

  Handler web_view_url_handler = new Handler()
  {
    public void handleMessage( Message m ) 
    {
      synchronized (web_view_bank)
      {
        PCWebView view = web_view_bank.get(m.what);
        if (view == null) return;
        view.loadUrl( (String) m.obj );
      }
    }
  };

  Handler web_view_html_handler = new Handler()
  {
    public void handleMessage( Message m ) 
    {
      synchronized (web_view_bank)
      {
        PCWebView view = web_view_bank.get(m.what);
        if (view == null) return;
        view.loadData( (String) m.obj , "text/html", "utf-8" );
      }
    }
  };

  Handler web_view_close_handler = new Handler()
  {
    public void handleMessage( Message m ) 
    {
      synchronized (web_view_bank)
      {
        PCWebView view = web_view_bank.get(m.what);
        if (view == null) return;
        view.set_visible(false);
        view.closed = true;
        web_view_bank.release(m.what);
      }
    }
  };

  Handler web_view_set_bounds_handler = new Handler()
  {
    public void handleMessage( Message m ) 
    {
      synchronized (web_view_bank)
      {
        PCWebView view = web_view_bank.get(m.what);
        if (view == null) return;
        view.x = (m.arg1 >>> 16);
        view.y = (m.arg1 & 0xffff);
        view.width  = (m.arg2 >>> 16);
        view.height = (m.arg2 & 0xffff);
      }
    }
  };

  Handler web_view_set_visible_handler = new Handler()
  {
    public void handleMessage( Message m ) 
    {
      synchronized (web_view_bank)
      {
        PCWebView view = web_view_bank.get(m.what);
        if (view == null) return;
        view.set_visible( m.arg1 != 0 );
      }
    }
  };

  Handler web_view_resume_handler = new Handler()
  {
    public void handleMessage( Message m ) 
    {
      ((PCWebView) m.obj).set_visible( true );
    }
  };

  // METHODS

  public void onCreate(Bundle savedInstanceState)
  {
    System.out.println( "onCreate()" );
    System.gc();

    super.onCreate(savedInstanceState);


    instance = this;

    requestWindowFeature( Window.FEATURE_NO_TITLE );
    getWindow().setFlags( WindowManager.LayoutParams.FLAG_FULLSCREEN, 
        WindowManager.LayoutParams.FLAG_FULLSCREEN );

    AndroidSound.sound_pool = new SoundPool(15, AudioManager.STREAM_MUSIC, 0);

    // collect asset list
    collectAssets("");
    collectAssets("images");
    collectAssets("data");
    collectAssets("sounds");

    for (int i=0; i<pending_events.length; ++i)
    {
      pending_events[i] = new PendingEvent();
    }

    // Use introspection to support multitouch if enabled
    try
    {
      Class classMotionEvent = Class.forName("android.view.MotionEvent");
      m_getPointerCount = classMotionEvent.getDeclaredMethod( "getPointerCount", (Class[]) null );
      m_getPointerId = classMotionEvent.getDeclaredMethod( "getPointerId", int.class );
      m_getX = classMotionEvent.getDeclaredMethod( "getX", int.class );
      m_getY = classMotionEvent.getDeclaredMethod( "getY", int.class );
    }
    catch (Exception err)
    {
      // API 1.6 or earlier
    }

    this.setVolumeControlStream(AudioManager.STREAM_MUSIC); //let volume control affect the music and sound

    // set up accelerometer listener
    SensorManager manager = (SensorManager)getSystemService(SENSOR_SERVICE);
    if(manager != null) 
    {
      Sensor accelerometer = manager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

      if(accelerometer != null)
      {
        manager.registerListener(this, accelerometer, SensorManager.SENSOR_DELAY_GAME);
      }
    }

    view = new GLSurfaceView(this);
    view.setRenderer( new PlasmacoreRenderer() );

    layout = new AbsoluteLayout(this);
    layout.addView(view);

    setContentView( layout );
  }

  protected void collectAssets( String path )
  {
    // Getting an assets list takes a while so avoid it if we can determine that
    // we've got a single file.
    if (path.endsWith(".png") || path.endsWith(".jpg") || path.endsWith(".wav")
        || path.endsWith(".ogg") || path.endsWith(".mp3") || path.endsWith(".m4a"))
    {
      assets.add(path);
      return;
    }

    try
    {
      String[] list = getAssets().list(path);
      if (list.length > 0)
      {
        for (String asset : getAssets().list(path))
        {
          // don't recurse in root "" since it takes a while
          if (path.length() > 0) collectAssets( path + "/" + asset );
          else assets.add(asset);
        }
      }
      else
      {
        assets.add( path );
      }
    }
    catch (Exception err)
    {
    }
  }

  public void onStart()
  {
    System.out.println( "onStart()" );
    super.onStart();
  }

  public void onRestart()
  {
    System.out.println( "onRestart()" );
    super.onRestart();
  }

  public void onResume()
  {
    pausing = false;
    paused = false;

    System.gc();

    super.onResume();
    view.onResume();
System.out.println( "onResume()" );

    for (int i=0; i<sound_bank.list.size(); ++i)
    {
      AndroidSound sound = sound_bank.list.get(i);
      if (sound != null) sound.system_resume();
    }

    // web views corrupt screen on resume if not hidden
    for (int i=0; i<web_view_bank.list.size(); ++i)
    {
      PCWebView view = web_view_bank.list.get(i);
      if (view != null) view.system_resume();
    }

    //downloader control code is now designed so the resources will attempt to download again if the user navigates back to the 
    //app after a download failure (i.e. they went to turn on their internet in settings) //Ty
    if(resources==null && downloader == null && key_filename != null)
    {
      startResourceDownloader();
    }
  }

  public void startResourceDownloader()
  {
    downloader = new ResourceDownloader(project_id,key_filename,this);
    if (downloader.progress == 1.0)
    {
      onResourceDownloaderDone();
      return;
    }
    else
    {
      addResourceDownloaderProgressEvent( -1.0 );
    }

    try
    {
      //Prevent the phone from sleeping until resources are finished downloading.
      PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
      wake_lock = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK, "My Tag");
      wake_lock.acquire();
      Log.i("PlasmaCore.Activity.onCreate", "Successfully acquired wake lock!");
    }
    catch(Exception err)
    {
      Log.e("PlasmaCore.Activity.onCreate", "Error creating wake lock: "+err);
    }
  }

  public void onPause()
  {
    System.out.println( "onPause()" );

    for (int i=0; i<sound_bank.list.size(); ++i)
    {
      AndroidSound sound = sound_bank.list.get(i);
      if (sound != null) sound.system_pause();
    }

    // web views corrupt screen on resume if not hidden
    for (int i=0; i<web_view_bank.list.size(); ++i)
    {
      PCWebView view = web_view_bank.list.get(i);
      if (view != null) view.system_pause();
    }

    int timeout = 1000;
    pausing = true;
    while ( !paused )
    {
      timeout -= 5;
      if (timeout <= 0) break;

      try
      {
        Thread.sleep(5);
      }
      catch (InterruptedException err)
      {
      }
    }

    super.onPause();
    view.onPause();
  }

  public void onStop()
  {
    System.out.println( "onStop()" );
    releaseWakeLock();
    super.onStop();
  }

  public void onDestroy()
  {
    System.out.println( "onDestroy()" );

    slagOnShutDown();

    AndroidSound.sound_pool.release();
    AndroidSound.sound_pool = null;

    releaseWakeLock();

    super.onDestroy();
  }

  public void releaseWakeLock()
  {
    try
    {
      if(wake_lock!=null) wake_lock.release();
      wake_lock=null;
    }catch(Exception ignore){}
  }

  public void onConfigurationChanged (Configuration newConfig)
  {
    super.onConfigurationChanged(newConfig);
    if ((newConfig.hardKeyboardHidden & Configuration.HARDKEYBOARDHIDDEN_YES)>0) hard_keyboard_hidden=true;
    else if ((newConfig.hardKeyboardHidden & Configuration.HARDKEYBOARDHIDDEN_NO)>0)  hard_keyboard_hidden=false;

    if (showing_keyboard)
    {
      InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
      imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, InputMethodManager.HIDE_IMPLICIT_ONLY);
    }
  }

  public void onResourceDownloaderError()
  {
    releaseWakeLock();
    Message mesg = alert_handler.obtainMessage();
    mesg.obj = downloader.error;
    alert_handler.sendMessage( mesg );
  }

  public void onResourceDownloaderDone()
  {
    Log.i("ResourceDownloader", "Resource downloader done");

    addResourceDownloaderProgressEvent( 1.0 );
    resources = downloader.resources;
    downloader=null; //free the reference

    //lazy fix for sleeping the instant it finishes downloading.  
    java.util.TimerTask wake_lock_task = new java.util.TimerTask()
    {
      public void run()
      {
        releaseWakeLock();
        release_wake_timer.cancel();
      }
    };
    release_wake_timer = new java.util.Timer();
    release_wake_timer.schedule(wake_lock_task,1000L*240L);  //let the phone go to sleep in 3 minutes if it wants.
  }

  public boolean onKeyDown(int keyCode, final KeyEvent event) 
  {
    switch(keyCode)
    {
      case KeyEvent.KEYCODE_BACK: 
        addPendingKeyEvent( true, 27, true ); // ESC
        return true;
      case KeyEvent.KEYCODE_MENU:
        addPendingKeyEvent( true, 282, false ); // F1
        return true;
      case KeyEvent.KEYCODE_VOLUME_UP:
      case KeyEvent.KEYCODE_VOLUME_DOWN:
        return super.onKeyDown(keyCode, event);
    }

    int keycode = event.getKeyCode();
    int unicode = 0;
    switch (keycode)
    {
      case 66: keycode = 0; unicode = 13; break;
      case 67: keycode = 0; unicode =  8; break;
      case 59: keycode = 304; break; // left shift
      case 60: keycode = 305; break; // right shift
      default: 
       keycode = 0; 
       unicode = event.getUnicodeChar(event.getMetaState());
    }

    if (keycode != 0)
    {
      addPendingKeyEvent( true, keycode, false );
    }
    else
    {
      addPendingKeyEvent( true, unicode, true );
    }
    return true;
  }

  public boolean onKeyUp(int keyCode, final KeyEvent event) 
  {
    switch(keyCode)
    {
      case KeyEvent.KEYCODE_BACK:
        addPendingKeyEvent( false, 27, true ); // ESC
        return true; 
      case KeyEvent.KEYCODE_MENU:
        addPendingKeyEvent( false, 282, false ); // F1
        return true; 
      case KeyEvent.KEYCODE_VOLUME_UP:
      case KeyEvent.KEYCODE_VOLUME_DOWN:
        return super.onKeyUp(keyCode, event);
    }

    int keycode = event.getKeyCode();
    int unicode = 0;
    switch (keycode)
    {
      case 66: keycode = 0; unicode = 13; break;
      case 67: keycode = 0; unicode =  8; break;
      case 59: keycode = 304; break; // left shift
      case 60: keycode = 305; break; // right shift
      default: 
        keycode = 0;
        unicode = event.getUnicodeChar(event.getMetaState());
    }

    if (keycode != 0)
    {
      addPendingKeyEvent( false, keycode, false );
    }
    else
    {
      addPendingKeyEvent( false, unicode, true );
    }
    return true;
  }

  public boolean onTouchEvent( final MotionEvent event ) 
  {
    try
    {
      switch(event.getAction()) 
      {
        case MotionEvent.ACTION_DOWN:
        case 5:  //MotionEvent.ACTION_POINTER_1_DOWN:
          begin_touch( event.getX(), event.getY() );
          break;

        case MotionEvent.ACTION_UP:
        case 6:  //MotionEvent.ACTION_POINTER_1_UP:
          end_touch( event.getX(), event.getY() );
          break;

        case 261: //MotionEvent.ACTION_POINTER_2_DOWN:
          if (m_getX != null)
          {
            double x = (Float) m_getX.invoke( event, 1 );
            double y = (Float) m_getY.invoke( event, 1 );
            begin_touch( x, y );
          }
          break;

        case 262: //MotionEvent.ACTION_POINTER_2_UP:
          if (m_getX != null)
          {
            double x = (Float) m_getX.invoke( event, 1 );
            double y = (Float) m_getY.invoke( event, 1 );
            end_touch( x, y );
          }
          break;


        case 517: //MotionEvent.ACTION_POINTER_3_DOWN:
          if (m_getX != null)
          {
            double x = (Float) m_getX.invoke( event, 2 );
            double y = (Float) m_getY.invoke( event, 2 );
            begin_touch( x, y );
          }
          break;

        case 518: //MotionEvent.ACTION_POINTER_3_UP:
          if (m_getX != null)
          {
            double x = (Float) m_getX.invoke( event, 2 );
            double y = (Float) m_getY.invoke( event, 2 );
            end_touch( x, y );
          }
          break;

        case MotionEvent.ACTION_MOVE:
          if (m_getPointerCount == null)
          {
            update_touch( event.getX(), event.getY() );
          }
          else
          {
            int count = (Integer) m_getPointerCount.invoke( event );
            for(int i = 0; i < count; ++i) 
            {
              int id = (Integer) m_getPointerId.invoke( event, i );
              double x = (Float) m_getX.invoke( event, id );
              double y = (Float) m_getY.invoke( event, id );
              update_touch( x, y );
            }
          }
          break;
      }
    }
    catch (Exception err)
    {
      // never gonna happen
    }

    return true;
  }

  void begin_touch( double x, double y )
  {
    addPendingTouchEvent( 0, touch_manager.begin_touch(x,y), x, y );
  }

  void update_touch( double x, double y )
  {
    addPendingTouchEvent( 1, touch_manager.update_touch(x,y), x, y );
  }

  void end_touch( double x, double y )
  {
    addPendingTouchEvent( 2, touch_manager.end_touch(x,y), x, y );
  }

  public void onSensorChanged(SensorEvent event)
  {
    if(event.sensor.getType() == Sensor.TYPE_ACCELEROMETER)
    {
      acceleration_x = event.values[0] / SensorManager.STANDARD_GRAVITY;
      acceleration_y = event.values[1] / SensorManager.STANDARD_GRAVITY;
      acceleration_z = event.values[2] / SensorManager.STANDARD_GRAVITY;
    }
  }

  public void onAccuracyChanged(Sensor sensor, int accuracy) {}

  //---------------------------------------------------------------------------

  public class PendingEvent
  {
    final static public int KEY_EVENT = 0;
    final static public int TOUCH_EVENT = 1;
    final static public int PROGRESS_EVENT = 2;
    final static public int CUSTOM_EVENT = 3;

    int type;

    boolean is_press;
    boolean is_unicode;
    int     code;
    int     id;
    double  x,y,z;
    String  custom_id;
    String  message;

    public void KeyEvent( boolean is_press, int code, boolean is_unicode )
    {
      type = KEY_EVENT;
      this.is_press = is_press;
      this.code = code;
      this.is_unicode = is_unicode;
    }

    public void TouchEvent( int stage, int id, double x, double y )
    {
      type = TOUCH_EVENT;
      this.code = stage;
      this.id = id;
      this.x = x;
      this.y = y;
    }

    public void ProgressEvent( double progress )
    {
      type = PROGRESS_EVENT;
      x = progress;
    }

    public void CustomEvent( String custom_id, double value, String message )
    {
      type = CUSTOM_EVENT;
      this.custom_id = custom_id;
      x = value;
      this.message = message;
    }

    public void dispatch()
    {
      switch (type)
      {
        case KEY_EVENT:
          slagKeyEvent( is_press, code, is_unicode );
          break;

        case TOUCH_EVENT:
          slagTouchEvent( code, id, x, y );
          break;

        case PROGRESS_EVENT:
          slagOnResourceDownloaderProgress( x );
          break;

        case CUSTOM_EVENT:
          slagCustomEvent( custom_id, x, message );
          break;
      }
    }
  }

  public PendingEvent[] pending_events = new PendingEvent[50];
  public int num_pending_events = 0;

  public PendingEvent addPendingEvent()
  {
    if (++num_pending_events == pending_events.length) --num_pending_events;
    return pending_events[num_pending_events-1];
  }

  public void addPendingKeyEvent( boolean is_press, int code, boolean is_unicode )
  {
    synchronized (pending_events)
    {
      addPendingEvent().KeyEvent( is_press, code, is_unicode );
    }
  }

  public void addPendingTouchEvent( int stage, int id, double x, double y )
  {
    synchronized (pending_events)
    {
      addPendingEvent().TouchEvent( stage, id, x, y );
    }
  }

  public void addResourceDownloaderProgressEvent( double progress )
  {
    synchronized (pending_events)
    {
      addPendingEvent().ProgressEvent( progress );
    }
  }

  public void addPendingCustomEvent( String custom_id )
  {
    synchronized (pending_events)
    {
      addPendingEvent().CustomEvent( custom_id, 0, null );
    }
  }

  public void addPendingCustomEvent( String custom_id, double value )
  {
    synchronized (pending_events)
    {
      addPendingEvent().CustomEvent( custom_id, value, null );
    }
  }

  public void addPendingCustomEvent( String custom_id, String message )
  {
    synchronized (pending_events)
    {
      addPendingEvent().CustomEvent( custom_id, 0, message );
    }
  }

  public void addPendingCustomEvent( String custom_id, double value, String message )
  {
    synchronized (pending_events)
    {
      addPendingEvent().CustomEvent( custom_id, value, message );
    }
  }

  public void trace( String message )
  {
    addPendingCustomEvent( "trace", message );
  }

  //---------------------------------------------------------------------------

  //public class PlasmacoreRenderer implements PlasmacoreView.Renderer
  public class PlasmacoreRenderer implements GLSurfaceView.Renderer
  {
    boolean created_vm = false;
    int     ms_error = 0;
    long    next_time;

    //public void init()
    //{
    //}

    //public int[] getConfigSpec()
    //{
      //int[] configSpec = { EGL10.EGL_DEPTH_SIZE, 16, EGL10.EGL_NONE };
      //return configSpec;
    //}

    public void onSurfaceChanged( GL10 gl, int width, int height )
    {
      if (created_vm) return;
      created_vm = true;

      byte[] etc_bytes = jniLoadResource(0,"game.etc");
      if ( !slagCreate( width, height, etc_bytes ) )
      {
        finish();
      }
    }

    public void onSurfaceCreated( GL10 gl, EGLConfig config )
    {
      if (created_vm) slagTexturesLostEvent();
    }

    public void onDrawFrame( GL10 gl )
    {
      if (pausing)
      {
        if ( !paused )
        {
          slagOnPause();
          paused = true;
        }
        return;
      }

      if (downloader != null)
      {
        downloader.update();
        if (downloader.error != null) 
        {
          onResourceDownloaderError();
          return;
        }
        else if (downloader.progress == 1.0) onResourceDownloaderDone();
        else addResourceDownloaderProgressEvent( downloader.progress );
      }

      synchronized (pending_events)
      {
        for (int i=0; i<num_pending_events; ++i)
        {
          pending_events[i].dispatch();
        }
        num_pending_events = 0;
      }

      // dispatch current acceleration
      slagAccelerationEvent( -acceleration_x, acceleration_y, -acceleration_z );

      // synchronized to avoid conflict with event dispatches
      int target_fps = slagUpdateDrawEvent();
      if (target_fps == 0) target_fps = 1;

      long cur_time = System.currentTimeMillis();
      long kill_ms = next_time - cur_time;
      if (kill_ms < 0)
      {
        next_time = cur_time;
        kill_ms = 1;
      }
      next_time += 1000 / target_fps;
      ms_error  += 1000 % target_fps;
      if (ms_error >= target_fps)
      {
        ++next_time;
      }

      try
      {
        Thread.sleep(kill_ms);
      }
      catch (InterruptedException err)
      {
      }
    }
  }

  //---------------------------------------------------------------------------

  public void alert( String mesg )
  {
    if (view != null) view.onPause();

    new AlertDialog.Builder(this)
      .setTitle("Fatal Error")
      .setMessage(mesg)
      .setCancelable(false)
      .setPositiveButton( "Okay", 
          new DialogInterface.OnClickListener()
          {
            public void onClick( DialogInterface dialog, int which )
            {
              jniExitProgram();
            }
          }
        )
    .show();
  }

  public boolean isID( int ch )
  {
    if (ch >= 'A' && ch <= 'Z') return true;
    if (ch >= 'a' && ch <= 'z') return true;
    if (ch >= '0' && ch <= '9') return true;
    if (ch == '_' || ch == '.' || ch == '-') return true;
    return false;
  }

  public boolean equalsSubstring( String st1, String st2, int st2_i, int compare_count )
  {
    if (st1.length() != compare_count) return false;
    if (st2_i + compare_count > st2.length()) return false;

    int st1_i = -1;
    --st2_i;
    ++compare_count;
    while (--compare_count != 0)
    {
      if (st1.charAt(++st1_i) != st2.charAt(++st2_i)) return false;
    }

    return true;
  }

  public boolean partialNameMatch( String partial, String full )
  {
    int full_len = full.length();
    int partial_len = partial.length();
    int partial_ch0 = partial.charAt(0);

    if (partial_len > full_len) return false;

    // look for a substring match
    int diff = full_len - partial_len;
    for (int i=0; i<=diff; i++)
    {
      if (partial_ch0 == full.charAt(i) && equalsSubstring(partial,full,i,partial_len))
      {
        int prev_char;
        if (i > 0) prev_char = full.charAt(i-1);
        else prev_char = 0;

        int next_char;
        if(i+partial_len < full.length()) next_char = full.charAt(i+partial_len);
        else next_char = 0; 
        if ( !isID(prev_char) && (next_char==0 || next_char=='.'))
        {
          return true;
        }
      }
    }

    return false;
  }

  //---------------------------------------------------------------------------

  native public boolean slagCreate( int width, int height, byte[] etc_bytes );

  native public int  slagUpdateDrawEvent();
  native public void slagTexturesLostEvent();
  native public void slagKeyEvent( boolean press, int code, boolean is_unicode );
  native public void slagTouchEvent( int stage, int id, double x, double y );
  native public void slagCustomEvent( String custom_id, double value, String message );
  native public void slagAccelerationEvent( double x, double y, double z );
  native public void slagOnResourceDownloaderProgress( double progress );
  native public void slagOnPause();
  native public void slagOnShutDown();

  //---------------------------------------------------------------------------

  public void jniLog( String mesg )
  {
    System.out.println( mesg );
  }

  public int jniAndroidIsTablet()
  {
    try 
    {
      // Compute screen size
      DisplayMetrics dm = getResources().getDisplayMetrics();
      float screenWidth  = dm.widthPixels / dm.xdpi;
      float screenHeight = dm.heightPixels / dm.ydpi;
      double size = Math.sqrt(Math.pow(screenWidth, 2) +
                              Math.pow(screenHeight, 2));
      // Tablet devices should have a screen size greater than 6 inches
      if (size >= 6.0) return 1;
      else             return 0;
    } 
    catch( Throwable t )
    {
      Log.e( LOGTAG, "Failed to compute screen size", t );
      return 0;
    }
  }

  public int jniAndroidMemoryClass()
  {
    try
    {
      // 1.6 devices don't define getMemoryClass() so we have to check and
      // see if the method exists.
      Class classActivityManager = Class.forName( "android.app.ActivityManager" );
      ActivityManager mgr = (ActivityManager) getSystemService("activity");
      Method m_getMemoryClass = classActivityManager.getDeclaredMethod( 
          "getMemoryClass", (Class[]) null );
      int mb = (Integer) m_getMemoryClass.invoke( mgr );

      return mb;
    }
    catch (Exception err)
    {
      return 16;
    }
  }

  public void collectResources( String base_path, String rel_path )
  {
    File file = new File(base_path+rel_path);
    if (file.isDirectory())
    {
      for (String filename : file.list())
      {
        if (rel_path.length() > 0)
        {
          collectResources( base_path, rel_path + File.separator + filename );
        }
        else
        {
          collectResources( base_path, filename );
        }
      }
    }
    else if (file.exists())
    {
      // Protect against SHA-1 hashed filenames getting into the list
      int last_dot = rel_path.lastIndexOf('.');
      if (last_dot >= 0 && (rel_path.length() - last_dot) <= 8)
      {
        resources.add( new ResourceFile(file,rel_path,base_path+rel_path,null) );
      }
    }
  }

  public void jniExitProgram()
  {
    releaseWakeLock();
    if (release_wake_timer != null) release_wake_timer.cancel();
    this.finish();
  }

  public String find_file( int type, String filename )
  {
    // Type: res_general, res_data, res_image, or res_sound

    // Try SD card resources
    if (resources != null)
    {
      for (int i=0; i<resources.size(); ++i)
      {
        ResourceFile resource = resources.get(i);
        switch (type)
        {
          case res_data:  if (!resource.originalFileName.startsWith("data"))  continue; break;
          case res_image: if (!resource.originalFileName.startsWith("images")) continue; break;
          case res_sound: if (!resource.originalFileName.startsWith("sounds")) continue; break;
        }

        if (partialNameMatch(filename,resource.originalFileName))
        {
          return Environment.getExternalStorageDirectory().getAbsolutePath()
            + "/plasmacore/" + project_id + "/" + resource.newFileName;
        }
      }
    }

    File file = new File(filename);
    if (file.exists()) return file.getAbsolutePath();
    return null;
  }

  public String find_asset( int type, String filename )
  {
    for (int i=0; i<assets.size(); ++i)
    {
      String asset = assets.get(i);

      if (partialNameMatch(filename,asset))
      {
        return asset;
      }
    }
    return null;
  }

  public AssetFileDescriptor open_asset_fd( String filename )
  {
    try
    {
      return getAssets().openFd(filename);
    }
    catch (IOException err)
    {
    }

    return null;
  }

  public byte[] jniLoadResource( int type, String filename )
  {
    try
    {
      // Try loading from embedded assets
      for (int i=0; i<assets.size(); ++i)
      {
        String asset = assets.get(i);

        if (partialNameMatch(filename,asset))
        {
          return loadBytes( getAssets().open(asset) );
        }
      }

      // Try loading from SD card resources
      for (int i=0; i<resources.size(); ++i)
      {
        ResourceFile resource = resources.get(i);
        switch (type)
        {
          case res_data:  if (!resource.originalFileName.startsWith("data"))  continue; break;
          case res_image: if (!resource.originalFileName.startsWith("images")) continue; break;
          case res_sound: if (!resource.originalFileName.startsWith("sounds")) continue; break;
        }

        if (partialNameMatch(filename,resource.originalFileName))
        {
          return loadBytes( new FileInputStream(resource.fileHandle) );
        }
      }

      // Try loading the filename as it is
      return loadBytes( new FileInputStream(filename) );
    }
    catch (Exception err)
    {
      return null;
    }
  }

  public byte[] loadBytes( InputStream infile )
  {
    try
    {
      byte[] bytes = new byte[infile.available()];
      infile.read( bytes );
      infile.close();
      return bytes;
    }
    catch (Exception err)
    {
      return null;
    }
  }

  int bytes_loaded;

  int[] jniDecodeBitmapData( byte[] data )
  {
    Bitmap bmp = null;
    bmp = BitmapFactory.decodeByteArray( data, 0, data.length );
    if (bmp == null) return null;

    int w = bmp.getWidth();
    int h = bmp.getHeight();

    int[] pixels = new int[ w * h + 1 ];

    bmp.getPixels( pixels, 0, w, 0, 0, w, h );
    bmp.recycle();
    bmp = null;
    bytes_loaded += data.length;
    if (bytes_loaded >= 100000)
    {
      System.gc();
      bytes_loaded = 0;
    }

    pixels[w*h] = w;
    return pixels;
  }

  byte[] jniEncodeBitmapData( int w, int h, int[] data, int encoding, int quality )
  {
    // encoding: 1=png, 2=jpg
    Bitmap bmp = Bitmap.createBitmap( data, w, h, Bitmap.Config.ARGB_8888 );

    ByteArrayOutputStream buffer = new ByteArrayOutputStream();
    Bitmap.CompressFormat format;

    if (encoding == 1) format = Bitmap.CompressFormat.PNG;
    else format = Bitmap.CompressFormat.JPEG;

    if (bmp.compress( format, quality, buffer )) return buffer.toByteArray();
    else return null;
  }

  public String jniGetDeviceID()
  {
    String device_id = ((TelephonyManager)getSystemService(TELEPHONY_SERVICE)).getDeviceId();
    if (device_id != null) return device_id;

    return Installation.id( AndroidCore.instance );
  }

  public String jniGetCountryName()
  {
    return Locale.getDefault().getDisplayCountry();
  }

  public void jniOpenURL( String url )
  {
    try
    {
      startActivity( new Intent( Intent.ACTION_VIEW, Uri.parse(url) ) );
    }
    catch (RuntimeException err)
    {
      System.err.println(err);
    }
  }

  public boolean jniIsDirectory( String filename )
  {
    return new File(filename).isDirectory();
  }

  public String[] jniDirectoryListing( String path )
  {
    return new File(path).list();
  }

  public String jniAbsoluteFilePath( String path )
  {
    return new File(path).getAbsolutePath();
  }

  public void jniFileCopy( String old_name, String new_name )
  {
    try
    {
      FileInputStream infile = new FileInputStream(old_name);
      FileOutputStream outfile = new FileOutputStream(new_name);
      for (int ch = infile.read(); ch != -1; ch = infile.read())
      {
        outfile.write(ch);
      }
      infile.close();
      outfile.close();
    }
    catch (IOException err)
    {
    }
  }

  public boolean jniFileExists( String filename )
  {
    return (new File(filename)).exists();
  }

  public void jniFileRename( String old_name, String new_name )
  {
    new File(old_name).renameTo(new File(new_name));
  }

  public void jniFileDelete( String filename )
  {
    new File(filename).delete();
  }

  public long jniFileTimestamp( String filename )
  {
    return new File(filename).lastModified();
  }

  public void jniFileTouch( String filename )
  {
    new File(filename).setLastModified( System.currentTimeMillis() );
  }

  public void jniFileMkdir( String filename )
  {
    new File(filename).mkdirs();
  }

  public int jniFileReaderOpen( String filename )
  {
    try
    {
      return infile_bank.add( new FileInputStream(filename) );
    }
    catch (IOException err)
    {
      return 0;
    }
  }

  public void jniFileReaderClose( int index )
  {
    try
    {
      FileInputStream infile = infile_bank.get(index);
      if (infile != null)
      {
         infile.close();
         infile_bank.release(index);
      }
    }
    catch (IOException err)
    {
    }
  }

  public byte[] jniFileReaderReadBytes( int index, int count )
  {
    FileInputStream infile = infile_bank.get(index);
    if (infile == null) return null;

    if (io_buffer == null || io_buffer.length < count)
    {
      io_buffer = new byte[count];
    }

    try
    {
      infile.read(io_buffer);
      return io_buffer;
    }
    catch (IOException err)
    {
      return null;
    }
  }

  public int jniFileReaderAvailable( int index )
  {
    FileInputStream infile = infile_bank.get(index);
    if (infile == null) return 0;

    try
    {
      return infile.available();
    }
    catch (IOException err)
    {
      return 0;
    }
  }

  public byte[] jniGetIOBuffer( int required_size )
  {
    if (io_buffer == null || io_buffer.length < required_size)
    {
      io_buffer = new byte[required_size];
    }
    return io_buffer;
  }

  public int jniFileWriterOpen( String filename, boolean append )
  {
    try
    {
      return outfile_bank.add( new FileOutputStream(filename,append) );
    }
    catch (IOException err)
    {
      return 0;
    }
  }

  public void jniFileWriterClose( int index )
  {
    try
    {
      FileOutputStream outfile = outfile_bank.get(index);
      if (outfile != null)
      {
        outfile.close();
        outfile_bank.release(index);
      }
    }
    catch (IOException err)
    {
    }
  }

  public void jniFileWriterWriteBytes( int index, byte[] data, int count )
  {
    FileOutputStream outfile = outfile_bank.get(index);
    if (outfile == null) return;

    try
    {
      outfile.write( data, 0, count );
    }
    catch (IOException err)
    {
    }
  }

  public int jniSoundLoad( String filename )
  {
    String filepath = find_file(res_sound,filename);
    if (filepath == null) filename = find_asset(res_sound,filename);
    if (filename == null) return 0;

    boolean bg_music;
    String ext = filename.substring( filename.lastIndexOf('.')+1 );
    if(ext.compareToIgnoreCase("wav")==0 || ext.compareToIgnoreCase("caf")==0
        || ext.compareToIgnoreCase("aif")==0 || ext.compareToIgnoreCase("aiff")==0)
    {
      bg_music = false;
    }
    else 
    {
      bg_music = true;
    }

    AndroidSound sound;
    if (filepath != null) sound = new AndroidSound(filepath, bg_music);
    else sound = new AndroidSound( open_asset_fd(filename), filename, bg_music );

    if(sound.error()) return 0;

    return sound_bank.add( sound );
  }

  public int jniSoundDuplicate(int sound_id) 
  {
    AndroidSound sound = sound_bank.get(sound_id);
    if(sound == null) return 0;

    AndroidSound new_sound;
    if (find_file(res_sound,sound.path) != null)
    {
      new_sound = new AndroidSound(sound.path, sound.is_bg_music);
    }
    else
    {
      new_sound = new AndroidSound( open_asset_fd(sound.path), sound.path, sound.is_bg_music );
    }

    if(new_sound.error()) return 0;

    return sound_bank.add( new_sound );
  }

  public void jniSoundPlay(int sound_id)
  {
    AndroidSound sound = sound_bank.get(sound_id);
    if(sound != null) sound.play();
  }

  public void jniSoundPause(int sound_id) 
  {
    AndroidSound sound = sound_bank.get(sound_id);
    if(sound != null) sound.pause();
  }

  public boolean jniSoundIsPlaying(int sound_id)
  {
    AndroidSound sound = sound_bank.get(sound_id);
    if(sound != null) return sound.is_playing();

    return false;
  }

  public void jniSoundSetVolume(int sound_id, double volume)
  {
    AndroidSound sound = sound_bank.get(sound_id);
    if(sound != null) sound.set_volume((float)volume);
  }

  public void jniSoundSetRepeats(int sound_id, boolean repeats)
  {
    AndroidSound sound = sound_bank.get(sound_id);
    if(sound != null) sound.set_repeats(repeats);
  }

  public double jniSoundGetCurrentTime(int sound_id)
  {
    AndroidSound sound = sound_bank.get(sound_id);
    if(sound != null) return sound.get_current_time() / 1000.0;

    return 0.0;
  }

  public void jniSoundSetCurrentTime(int sound_id, double current_time)
  {
    AndroidSound sound = sound_bank.get(sound_id);
    if(sound != null) sound.set_current_time((int)(current_time*1000));
  }

  public double jniSoundDuration(int sound_id)
  {
    AndroidSound sound = sound_bank.get(sound_id);
    if(sound != null) return sound.get_duration() / 1000.0;

    return 0.0;
  }

  public void jniSoundRelease(int sound_id) 
  {
    AndroidSound sound = sound_bank.release(sound_id);
    if(sound != null) 
    {
      sound.release();
    }
  }

  public void jniShowKeyboard( boolean visible )
  {
    if (visible)
    {
      showing_keyboard = true;
      InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
      imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, InputMethodManager.HIDE_IMPLICIT_ONLY);
    }
    else
    {
      showing_keyboard = false;
      InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
      imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
    }
  }

  public boolean jniKeyboardVisible()
  {
    return showing_keyboard;
  }

  public byte[] jniLoadGamestate( String filename )
  {
    try 
    {
      for (String cur_filename : fileList()) 
      {
        if (partialNameMatch(filename,cur_filename))
        {
          // found our match
          FileInputStream fileReader = openFileInput(cur_filename);

          int offset = 0;
          int count = 0;
          byte data[] = new byte[ (int) fileReader.available() ];

          int read = 0;
          int numRead = 0;
          while (read < data.length && (numRead=fileReader.read(data, read, data.length-read)) >= 0) 
          {
            read = read + numRead;
          }

          return data;
        }
      }
    }
    catch (IOException err)
    {
    }

    return null;
  } 

  public boolean jniSaveGamestate( String filename, String content )
  {
    try
    {
      FileOutputStream fileWriter = openFileOutput( filename, MODE_PRIVATE );

      for (int i=0; i<content.length(); i++)
      {
        fileWriter.write( content.charAt(i) );
      }
      fileWriter.flush();
      fileWriter.close();
      return true;
    }
    catch(IOException e) 
    {
    }

    return false;
  }

  public boolean jniDeleteGamestate( String filename )
  {
    return deleteFile(filename);
  }

  public int jniVideoPlay( String filename )
  {
    Intent intent = new Intent(AndroidCore.this, VideoPlayer.class);
    intent.putExtra("video_filename", filename);
    startActivity(intent);

    return 1;
  }

  public boolean jniVideoUpdate( int player_id )
  {
    return false;
  }

  public void jniVideoStop( int player_id )
  {
  }

  public int jniWebViewGet( int index )
  {
    synchronized (web_view_bank)
    {
      if (index > 0) return index;

      index = web_view_bank.add(null);
      new_web_view_handler.obtainMessage(index,this).sendToTarget();
      return index;
    }
  }

  public void jniWebViewURL( int index, String url )
  {
    synchronized (web_view_bank)
    {
      web_view_url_handler.obtainMessage(index,url).sendToTarget();
    }
  }

  public void jniWebViewHTML( int index, String html )
  {
    synchronized (web_view_bank)
    {
      web_view_html_handler.obtainMessage(index,html).sendToTarget();
    }
  }

  public void jniWebViewClose( int index )
  {
    synchronized (web_view_bank)
    {
      web_view_close_handler.obtainMessage(index).sendToTarget();
    }
  }

  public void jniWebViewSetBounds( int index, int x, int y, int w, int h )
  {
    synchronized (web_view_bank)
    {
      web_view_set_bounds_handler.obtainMessage(index,(x<<16)|y,(w<<16)|h).sendToTarget();
    }
  }

  public void jniWebViewSetVisible( int index, boolean setting )
  {
    synchronized (web_view_bank)
    {
      web_view_set_visible_handler.obtainMessage(index,setting?1:0,0).sendToTarget();
    }
  }

  public boolean jniWebViewGetVisible( int index )
  {
    synchronized (web_view_bank)
    {
      PCWebView view = web_view_bank.get(index);
      if (view == null) return false;
      return view.visible;
    }
  }

  public boolean jniWebViewGetLoaded( int index )
  {
    synchronized (web_view_bank)
    {
      PCWebView view = web_view_bank.get(index);
      if (view == null) return false;
      return view.loaded;
    }
  }

  public boolean jniWebViewGetFailed( int index )
  {
    synchronized (web_view_bank)
    {
      PCWebView view = web_view_bank.get(index);
      if (view == null) return false;
      return view.failed;
    }
  }

  class PCWebView extends WebView
  {
    boolean loaded, failed, visible;
    int x, y, width, height;
    boolean suspended;
    boolean closed;

    public PCWebView( Context context )
    {
      super(context);
    }

    public void set_visible( boolean setting )
    {
      if (visible == setting) return;

      visible = setting;
      if (visible)
      {
        if ( !closed )
        {
          layout.addView( this, new AbsoluteLayout.LayoutParams(width,height,x,y) );
        }
      }
      else
      {
        layout.removeView( this );
      }
    }

    public void system_pause()
    {
      if (visible)
      {
        set_visible(false);
        suspended = true;
      }
    }
    
    public void system_resume()
    {
      if (suspended)
      {
        web_view_resume_handler.obtainMessage(0,this).sendToTarget();
        suspended = false;
      }
    }
  }
}

class IntList
{
  int[] data;
  int   size;

  IntList( int capacity )
  {
    data = new int[capacity];
  }

  public void ensure_capacity( int capacity )
  {
    if (data.length < capacity)
    {
      int[] new_data = new int[capacity];
      for (int i=0; i<data.length; ++i) new_data[i] = data[i];
      data = new_data;
    }
  }

  public void add( int value )
  {
    if (size == data.length) ensure_capacity(data.length*2);
    data[size++] = value;
  }

  public int get( int index )
  {
    return data[index];
  }

  public void set( int index, int value )
  {
    data[index] = value;
  }

  public boolean remove_value( int value )
  {
    if (size == 0) return false;

    for (int i=0; i<size; ++i)
    {
      if (data[i] == value)
      {
        data[i] = data[--size];
        return true;
      }
    }

    return false;
  }

  public int remove_last()
  {
    return data[--size];
  }
}

class TouchInfo
{
  boolean active;
  double x,y;

  TouchInfo() { active = false; }
}

class TouchManager
{
  final static public int MAX_TOUCHES = 4;

  TouchInfo[] touches = new TouchInfo[MAX_TOUCHES];
  int         num_active_touches = 0;

  TouchManager() 
  { 
    for (int i=0; i<MAX_TOUCHES; ++i) touches[i] = new TouchInfo();
  }

  // Each method returns the index (1+) that TouchManager
  // is using to track the event.
  int begin_touch( double x, double y )
  {
    if (num_active_touches < MAX_TOUCHES)
    {
      for (int i=0; i<MAX_TOUCHES; ++i)
      {
        if ( !touches[i].active )
        {
          touches[i].active = true;
          touches[i].x = x;
          touches[i].y = y;
          ++num_active_touches;
          return i+1;
        }
      }
    }
    return 0;
  }

  int update_touch( double x, double y )
  {
    int best_i = -1;
    double dx,dy;
    double best_r2 = 0;
    for (int i=0; i<MAX_TOUCHES; i++)
    {
      if (touches[i].active)
      {
        dx = touches[i].x - x;
        dy = touches[i].y - y;
        double r2 = dx*dx + dy*dy;
        if (best_i == -1 || r2 < best_r2)
        {
          best_r2 = r2;
          best_i = i;
        }
      }
    }

    if (best_i == -1)
    {
      //System.out.println( "ERROR: update_touch with no active touches!\n" );
      //  Commented out warning since this happens on Android WebView all the time
      return 0;
    }

    touches[best_i].x = x;
    touches[best_i].y = y;
    return best_i+1;
  }

  int end_touch( double x, double y )
  {
    int i = update_touch(x,y);
    touches[i-1].active = false;
    --num_active_touches;
    return i;
  }
}

//=============================================================================
//  AndroidSound
//=============================================================================
class AndroidSound implements MediaPlayer.OnErrorListener
{
  static SoundPool sound_pool;

  String path;
  AssetFileDescriptor fd;
  boolean error;

  int sound_pool_id;  // for sound effects
  MediaPlayer media_player; // for music

  int stream_id;

  boolean system_sound_repeats;
  boolean is_bg_music;
  boolean system_paused;


  AndroidSound( String path, boolean is_bg_music )
  {
    this.path = path;
    this.is_bg_music = is_bg_music;
    initialize();
  }

  AndroidSound( AssetFileDescriptor fd, String path, boolean is_bg_music )
  {
    this.fd = fd;
    this.path = path;
    this.is_bg_music = is_bg_music;
    initialize();
  }


  void initialize()
  {
    for (int i=0; i<3; ++i)
    {
      try
      {
        sound_pool_id = -1;
        if (media_player != null) media_player.release();
        media_player = null;
        system_sound_repeats = false;

        if (is_bg_music)
        {
          media_player = new MediaPlayer();
          media_player.setOnErrorListener(this);
          try 
          {
            media_player.reset();
            if (fd != null)
            {
              media_player.setDataSource(
                  fd.getFileDescriptor(),
                  fd.getStartOffset(),
                  fd.getLength()
                );
            }
            else
            {
              media_player.setDataSource(path);
            }
            media_player.prepare();
            media_player.setAudioStreamType(AudioManager.STREAM_MUSIC); //Added by Ty so the volume control has an effect on this.
          }
          catch(Exception e) 
          {
            System.out.println( "Error initializing media_player: " + e.toString() );
          }
        }
        else
        {
          if (fd != null) sound_pool_id = sound_pool.load(fd, 1);
          else            sound_pool_id = sound_pool.load(path, 1);

          long timeout = System.currentTimeMillis() + 1000;
          int stream;
          while((stream = sound_pool.play(sound_pool_id, 0.0f, 0.0f, 1, 0, 1.0f)) == 0)
          {
            // Briefly play the stream at zero volume to get it decoded and prepped.
            try
            {
              Thread.sleep(8);
            }
            catch(InterruptedException e) {}

            if(System.currentTimeMillis() >= timeout) break;
          }

          sound_pool.stop(stream);
        }
        error = false;
        return;
      }
      catch (RuntimeException this_sometimes_happens)
      {
        try
        {
          Thread.sleep(2000);
        }
        catch (InterruptedException err)
        {
        }
      }
    }

    System.out.println(
        "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *");
    System.out.println( "Failed to load sound " + path );
    System.out.println(
        "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *");

    sound_pool_id = -1;
    media_player = null;
  }

  public boolean onError( MediaPlayer player, int what, int extra )
  {
    error = true;
    return false;
  }

  void release()
  {
    if(is_bg_music) 
    {
      if (media_player != null) media_player.release();
    }
    else if (sound_pool_id >= 0)
    {
      sound_pool.unload(sound_pool_id);
    }
  }

  boolean error() 
  { 
    return media_player == null && sound_pool_id == -1; 
  }

  void play()
  {
    if (media_player != null)
    {
      media_player.start();
      if ( !media_player.isPlaying() )
      {
        error = true;
        initialize();
        if (media_player != null) media_player.start();
      }
    }
    else if (sound_pool_id >= 0)
    {
      if(system_sound_repeats) stream_id = sound_pool.play(sound_pool_id, 1.0f, 1.0f, 0, -1, 1.0f);
      else stream_id = sound_pool.play(sound_pool_id, 1.0f, 1.0f, 0, 0, 1.0f);
    }
  }

  void system_pause()
  {
    try
    {
      if (error) initialize();
      if (media_player != null && media_player.isPlaying())
      {
        media_player.pause();
        system_paused = true;
      }
    }
    catch (IllegalStateException err)
    {
      // bleh
    }
  }

  void system_resume()
  {
    if (media_player != null && system_paused)
    {
      if (error) initialize();
      media_player.start();
      system_paused = false;
    }
  }

  void pause()
  {
    if (media_player != null ) 
    {
      if(media_player.isPlaying()) 
      {
        media_player.pause();
      }
    }
    else 
    {
      sound_pool.pause(stream_id);
    }
  }

  boolean is_playing()
  {
    if (media_player != null) 
    {
      return media_player.isPlaying();
    }
    return false;
  }

  void set_volume( float new_volume )
  {
    if (media_player != null) 
    {
      media_player.setVolume( new_volume, new_volume );
    }
    //if (audio_player) audio_player.volume = new_volume;
  }

  void set_repeats( boolean setting )
  {
    if (media_player != null)
    {
      media_player.setLooping(setting);
    }
    else
    {
      system_sound_repeats = setting;
    }
  }

  double get_current_time()
  {
    if (media_player != null) 
    {
      return media_player.getCurrentPosition();
    }
    return 0.0;
  }

  void set_current_time( int new_time )
  {
    if (media_player != null) 
    {
      media_player.seekTo(new_time);
    }
    else if (sound_pool_id >= 0)
    {
      sound_pool.pause(stream_id);
    }
  }

  double get_duration()
  {
    if (media_player != null) 
    {
      return media_player.getDuration();
    }
    return 1.0;
  }
}

//=============================================================================
//  ResourceBank
//=============================================================================
class ResourceBank<ResType>
{
  IntList available = new IntList(10);
  ArrayList<ResType> list = new ArrayList<ResType>();

  public ResourceBank()
  {
    list.add(null);  // use up spot #0
  }

  public ResType get( int index )
  {
    if (index <= 0 || index >= list.size()) return null;
    return list.get(index);
  }

  public void set( int index, ResType res )
  {
    // requires 'index' to already be reserved via add(null).
    list.set( index, res );
  }

  public int add( ResType res )
  {
    if (available.size > 0)
    {
      int index = available.remove_last();
      list.set( index, res );
      return index;
    }
    else
    {
      int index = list.size();
      list.add(res);
      return index;
    }
  }

  public ResType release( int index )
  {
    ResType res = get(index);
    if (res != null)
    {
      available.add(index);
      list.set(index,null);
    }
    return res;
  }
}


class Installation 
{
    private static String sID = null;
    private static final String INSTALLATION = "INSTALLATION";

    public synchronized static String id(Context context) 
    {
      if (sID == null)
      {
          File installation = new File(context.getFilesDir(), INSTALLATION);
          try 
          {
            if (!installation.exists())
            {
              writeInstallationFile(installation);
            }
            sID = readInstallationFile(installation);
          } 
          catch (Exception e) 
          {
            throw new RuntimeException(e);
          }
      }
      return sID;
    }

    private static String readInstallationFile(File installation) throws IOException 
    {
        RandomAccessFile f = new RandomAccessFile(installation, "r");
        byte[] bytes = new byte[(int) f.length()];
        f.readFully(bytes);
        f.close();
        return new String(bytes);
    }

    private static void writeInstallationFile(File installation) throws IOException 
    {
        FileOutputStream out = new FileOutputStream(installation);
        String id = UUID.randomUUID().toString();
        out.write(id.getBytes());
        out.close();
    }
}