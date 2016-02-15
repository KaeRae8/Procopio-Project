//=============================================================================
// PlasmacoreView.java
//
// $(PLASMACORE_VERSION) $(DATE)
//
// Used instead of GLSurfaceView so we can tell when the rendering context has
// been lost.
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
import java.util.*;
import java.util.concurrent.*;
import android.app.*;
import android.content.*;
import android.os.*;
import android.opengl.*;
import android.util.*;
import android.view.*;
import javax.microedition.khronos.egl.*;
import javax.microedition.khronos.opengles.*;

/**
 * An implementation of SurfaceView that uses the dedicated surface for
 * displaying an OpenGL animation.  This allows the animation to run in a
 * separate thread, without requiring that it be driven by the update mechanism
 * of the view hierarchy.
 *
 * The application-specific rendering code is delegated to a PlasmacoreView.Renderer
 * instance.
 */
class PlasmacoreView extends SurfaceView implements SurfaceHolder.Callback
{
  private static final Semaphore lock = new Semaphore(1);

  SurfaceHolder holder;

  private DriverThread thread;
  private Renderer renderer;
  volatile private boolean active, ready, paused;
  private boolean size_changed = true;
  private int width;
  private int height;

  PlasmacoreView(Context context)
  {
    super(context);
    init();
  }

  public PlasmacoreView(Context context, AttributeSet attrs)
  {
    super(context, attrs);
    init();
  }

  public void onPause()
  {
    paused = true;
  }

  public void onResume()
  {
    paused = false;
  }

  public void onDestroy()
  {
    active = false;
    try
    {
      thread.join();
    }
    catch (InterruptedException err)
    {
    }
  }

  private void init()
  {
    // Install a SurfaceHolder.Callback so we get notified when the
    // underlying surface is created and destroyed
    holder = getHolder();
    holder.addCallback(this);
    holder.setType(SurfaceHolder.SURFACE_TYPE_GPU);
  }

  public void setRenderer(Renderer renderer)
  {
    this.renderer = renderer;
  }

  public void surfaceCreated(SurfaceHolder holder)
  {
    // The Surface has been created, start our drawing thread.
    System.out.println( "surfaceCreated" );
    ready = true;
    if (thread == null)
    {
      thread = new DriverThread(renderer);
      thread.start();
    }
  }

  public void surfaceDestroyed(SurfaceHolder holder)
  {
    // Surface will be destroyed when we return
    System.out.println( "Surface Destroyed" );
    ready = false;
  }

  public void surfaceChanged(SurfaceHolder holder, int format, int w, int h)
  {
    // Surface size or format has changed. This should not happen in this
    // example.
    thread.onWindowResize(w, h);
  }

  // ----------------------------------------------------------------------

  /**
   * A generic GL Thread. Takes care of initializing EGL and GL. Delegates
   * to a Renderer instance to do the actual drawing.
   *
   */

  class DriverThread extends Thread
  {
    DriverThread(Renderer renderer)
    {
      super();
      width = 0;
      height = 0;
      PlasmacoreView.this.renderer = renderer;
    }

    @Override
      public void run()
      {
        /*
         * When the android framework launches a second instance of
         * an activity, the new instance's onCreate() method may be
         * called before the first instance returns from onDestroy().
         *
         * This semaphore ensures that only one instance at a time
         * accesses EGL.
         */
        active = true;
        try
        {
          try
          {
            lock.acquire();
          }
          catch (InterruptedException e)
          {
            return;
          }
          guardedRun();
        }
        finally
        {
          lock.release();
        }
      }

    private void sleep( int ms )
    {
      try
      {
        Thread.sleep(ms);
      }
      catch (InterruptedException err)
      {
      }
    }

    private void guardedRun()
    {
      renderer.init();

      while ( active )
      {
        if (paused || !ready)
        {
          sleep(1000);
          continue;
        }

        System.out.println( "Creating new EGL Context" );

        // Get an EGL instance
        EGL10 egl = (EGL10) EGLContext.getEGL();

        // Get to the default display.
        EGLDisplay dpy = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);

        // We can now initialize EGL for that display
        int[] version = new int[2];
        egl.eglInitialize(dpy, version);

        // Specify a configuration for our opengl session
        // and grab the first configuration that matches is
        int[] configSpec = renderer.getConfigSpec();

        EGLConfig[] configs = new EGLConfig[1];
        int[] num_config = new int[1];
        egl.eglChooseConfig(dpy, configSpec, configs, 1, num_config);
        EGLConfig config = configs[0];

        // Create an OpenGL ES context. This must be done only once, an
        // OpenGL context is a somewhat heavy object.
        EGLContext context = egl.eglCreateContext(dpy, config,
            EGL10.EGL_NO_CONTEXT, null);

        EGLSurface surface = null;
        GL10 gl = null;

        // This is our main activity thread's loop, we go until
        // asked to quit.
        while (active && ready)
        {
          if (paused)
          {
            sleep(250);
            continue;
          }

          // Update the asynchronous state (window size, key events)
          int w, h;
          boolean changed;
          synchronized (this)
          {
            changed = size_changed;
            w = width;
            h = height;
            size_changed = false;
          }

          if (changed)
          {
            //  The window size has changed, so we need to create a new
            //  surface.
            if (surface != null)
            {

              // Unbind and destroy the old EGL surface, if
              // there is one.
              egl.eglMakeCurrent(dpy, EGL10.EGL_NO_SURFACE,
                  EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);
              egl.eglDestroySurface(dpy, surface);
            }

            // Create an EGL surface we can render into.
            surface = egl.eglCreateWindowSurface(dpy, config, holder,
                null);

            // Before we can issue GL commands, we need to make sure
            // the context is current and bound to a surface.
            egl.eglMakeCurrent(dpy, surface, surface, context);

            // Get to the appropriate GL interface.
            // This is simply done by casting the GL context to either
            // GL10 or GL11.
            gl = (GL10) context.getGL();

            if (renderer != null)
            {
              renderer.sizeChanged(gl, w, h);
            }
          }

          if (renderer != null)
          {
            if (renderer.drawFrame(gl))
            {
              // Once we're done with GL, we need to call swapBuffers()
              // to instruct the system to display the rendered frame
              egl.eglSwapBuffers(dpy, surface);
            }
          }

          // Always check for EGL_CONTEXT_LOST, which means the context
          // and all associated data were lost (For instance because
          // the device went to sleep). We need to quit immediately.
          if (egl.eglGetError() == EGL11.EGL_CONTEXT_LOST) 
          {
            // we lost the gpu
            System.out.println("CONTEXT LOST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" );
            Context c = getContext();
            if (c instanceof Activity) 
            {
              ((Activity) c).finish();
            }
            active = false;
          }
        }
        System.out.println( "*** Cleaning up ***" );

        // Clean up
        egl.eglMakeCurrent(dpy, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE,
            EGL10.EGL_NO_CONTEXT);
        egl.eglDestroySurface(dpy, surface);
        egl.eglDestroyContext(dpy, context);
        egl.eglTerminate(dpy);
      }
    }

    public void onWindowResize(int w, int h)
    {
      synchronized (this)
      {
        width = w;
        height = h;
        size_changed = true;
      }
    }
  }

  /**
   * A generic renderer interface.
   */

  public interface Renderer
  {
    public void init();
    public int[] getConfigSpec();
    public void sizeChanged(GL10 gl, int width, int height);
    public boolean drawFrame(GL10 gl);
  }
}

