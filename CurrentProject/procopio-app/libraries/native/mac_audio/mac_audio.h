//=============================================================================
// mac_audio.h
//
// v1.2 2011.03.19
//
// http://plasmaworks.com/plasmacore
//
// Audio player for Mac and iPhone.
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
//=============================================================================
#ifndef MAC_AUDIO_H
#define MAC_AUDIO_H

#import <QuartzCore/QuartzCore.h>

#include <AudioToolbox/AudioQueue.h>
#include <AudioToolbox/AudioFile.h>

void update_sounds();

struct SoundPlayer
{
  // PROPERTIES
  AudioQueueRef       queue;
  AudioQueueBufferRef sound_buffer;
  AudioStreamPacketDescription* packet_descriptions;
  UInt32 num_packets;
  AudioStreamBasicDescription data_format;
  bool    repeats;
  double  volume;   // must use set_volume() to change
  double  start_time;
  double  duration; // in seconds
  //int num_enqueued_buffers;
  bool shutting_down;

  int state;
  int stop_order_tick_count;
  SoundPlayer* next_sound_requiring_update;

  // METHODS
  SoundPlayer( char* data, int data_size );  
    // Takes raw data from audio file.  You can discard data afterwards.
 
  ~SoundPlayer();
  void shut_down();

  bool error() { return !queue; }

  void update();

  void prepare_to_play();

  void play();  // if paused or stopped
  void pause();
  void stop();

  bool is_playing();
  bool is_paused();

  void   set_volume( double p );  // 0.0 to 1.0

  double get_current_time();  // in seconds
  void   set_current_time( double seconds );

  static void on_buffer_finished( void* user_data, AudioQueueRef queue, 
      AudioQueueBufferRef buffer );

  static void on_playstate_change( void* user_data, AudioQueueRef queue, AudioQueuePropertyID property_id );
};

#endif // MAC_AUDIO_H

