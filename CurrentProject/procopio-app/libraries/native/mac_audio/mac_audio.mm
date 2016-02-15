//=============================================================================
// mac_audio.mm
//
// v1.2 2011.03.19
//
// http://plasmaworks.com/plasmacore
//
// Audio player for Mac and iPhone.
//
// HISTORY
//   v1.1 / 2011.02.11 / Abe Pralle
//   - Added 1/4 second timeout to stop sounds that occasionally finish and
//     never have the final callback called.
//
//   v1.2 / 2011.02.11 / Abe Pralle
//   - Increased timeout to 5 seconds since the timeout value is set before
//     the buffer necessarily finishes playing.
//
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
#include "mac_audio.h"
#include "slag.h"

#define SND_STOPPED      0
#define SND_PLAYING      1
#define SND_PAUSED       2
#define SND_REQUEST_STOP 3
#define SND_STOPPING     4
#define SND_FINISHED     5

NSRecursiveLock* sound_lock = [[NSRecursiveLock alloc] init];

SoundPlayer* sounds_requiring_update = 0;

static void lock() { [sound_lock lock]; }
static void unlock() { [sound_lock unlock]; }

int sound_tick_count = 0;

void update_sounds()
{
  lock();
  ++sound_tick_count;
  SoundPlayer* cur = sounds_requiring_update;
  sounds_requiring_update = NULL;
  unlock();

  while (cur)
  {
    cur->update();
    cur = cur->next_sound_requiring_update;
  }
}


//--------------------------------------------------------------------
//  RawAudioData
//--------------------------------------------------------------------
struct RawAudioData
{
  char* data;
  int   data_size;

  RawAudioData( char* data, int data_size )
  {
    // We don't make a copy of the data because we expect to read in 
    // the entire file before returning from SoundPlayer().
    this->data = data;
    this->data_size = data_size;
  }
};


OSStatus RawAudioData_read_proc( void* user_data, SInt64 pos, 
    UInt32 count, void* dest, UInt32* bytes_read )
{
  RawAudioData* sound = (RawAudioData*) user_data;
  if (pos + (int) count > sound->data_size) count = sound->data_size - pos;
  *bytes_read = count;
  if (count) memcpy( dest, sound->data+pos, count );
  return 0;
}

SInt64 RawAudioData_get_size_proc( void* user_data )
{
  return ((RawAudioData*) user_data)->data_size;
}

//--------------------------------------------------------------------
//  SoundPlayer
//--------------------------------------------------------------------
SoundPlayer::SoundPlayer( char* data, int data_size ) :
    queue(0),
    num_packets(0),
    packet_descriptions(NULL),
    volume(1.0),
    start_time(0.0),
    state(SND_STOPPED),
    repeats(false),
    stop_order_tick_count(0),
    next_sound_requiring_update(NULL),
    shutting_down(false),
    sound_buffer(NULL)
{
  AudioFileID         audio_file = 0;
  RawAudioData bytes( data, data_size );
  if (0 != AudioFileOpenWithCallbacks( &bytes, RawAudioData_read_proc, 0, 
      RawAudioData_get_size_proc, 0, 0, &audio_file )) return;

  UInt64 packets_in_file;
  UInt32 size = sizeof(packets_in_file);
  AudioFileGetProperty( audio_file, 
     kAudioFilePropertyAudioDataPacketCount, &size, &packets_in_file );
  num_packets = packets_in_file;

  size = sizeof(duration);
  AudioFileGetProperty( audio_file, 
     kAudioFilePropertyEstimatedDuration, &size, &duration );

  size = sizeof(data_format);
  AudioFileGetProperty( audio_file,
      kAudioFilePropertyDataFormat, &size, &data_format );

  AudioQueueNewOutput(&data_format, SoundPlayer::on_buffer_finished, this,
        //CFRunLoopGetCurrent(),
        NULL,
        kCFRunLoopCommonModes, 0, &queue);
  if ( !queue ) { shut_down(); return; }

  bool is_vbr = (data_format.mBytesPerPacket == 0 
      || data_format.mFramesPerPacket == 0);
  if (is_vbr)
  {
    AudioQueueAllocateBuffer( queue, data_size, &sound_buffer );
    if ( !sound_buffer ) { shut_down(); return; }
    packet_descriptions = new AudioStreamPacketDescription[packets_in_file];
  }
  else
  {
    AudioQueueAllocateBuffer( queue, data_size, &sound_buffer );
    if ( !sound_buffer ) { shut_down(); return; }
  }

  UInt32 bytes_read;
  AudioFileReadPackets( audio_file, false, &bytes_read, 
      packet_descriptions, 0, &num_packets, 
      sound_buffer->mAudioData );
  sound_buffer->mAudioDataByteSize = bytes_read;

  if ( !is_vbr ) 
  {
    num_packets = 0;
  }

  AudioFileClose( audio_file );

  prepare_to_play();

  AudioQueueAddPropertyListener( queue, kAudioQueueProperty_IsRunning, on_playstate_change, this );
}

SoundPlayer::~SoundPlayer()
{
  shut_down();
}

void SoundPlayer::shut_down()
{
  if (shutting_down) return;
  shutting_down = true;

  if (queue)
  {
    lock();
    while (sounds_requiring_update == this)
    {
      sounds_requiring_update = sounds_requiring_update->next_sound_requiring_update;
    }

    SoundPlayer* prev = sounds_requiring_update;
    while (prev)
    {
      while (prev->next_sound_requiring_update == this)
      {
        prev->next_sound_requiring_update = 
            prev->next_sound_requiring_update->next_sound_requiring_update;
      }
      prev = prev->next_sound_requiring_update;
    }
    unlock();

    AudioQueueDispose( queue, true );  // also frees the buffer
    queue = NULL;
  }

  if (packet_descriptions)
  {
    delete packet_descriptions;
    packet_descriptions = NULL;
  }
}

void SoundPlayer::update()
{
  if (error()) return;

  lock();
  switch (state)
  {
    case SND_REQUEST_STOP:
      state = SND_STOPPING;
      unlock();
      {
        stop_order_tick_count = sound_tick_count;
        AudioQueueStop( queue, false );
      }
      return;

    case SND_FINISHED:
      state = SND_STOPPED;
      unlock();
      prepare_to_play();
      return;

    default:
      unlock();
  }
}

void SoundPlayer::prepare_to_play()
{
  if (error() || shutting_down) return;

  AudioQueueEnqueueBufferWithParameters( queue, sound_buffer, 
      num_packets, packet_descriptions, 
      data_format.mSampleRate * start_time, 
      0, 0, NULL, NULL, NULL );
  start_time = 0.0;

  AudioQueuePrime( queue, 16, NULL );
}

void SoundPlayer::play()
{
  if (error()) return;

  // Kludge to fix apparent bug where the playstate change listener won't
  // always be called after AudioQueueStop().
  if (state == SND_STOPPING)
  {
    if (sound_tick_count - stop_order_tick_count >= 300)
    {
      state = SND_STOPPED;
      prepare_to_play();
    }
  }

  if (state == SND_PAUSED || state == SND_STOPPED)
  {
    state = SND_PLAYING;
    AudioQueueStart( queue, NULL );
  }
}

void SoundPlayer::pause()
{
  if (error()) return;
  lock();
  if (is_playing())
  {
    state = SND_PAUSED;
    unlock();
    AudioQueuePause(queue);
  }
  else
  {
    unlock();
  }
}

void SoundPlayer::stop()
{
  if (error()) return;
  lock();
  if ((is_playing() || state == SND_PAUSED) && !shutting_down)
  {
    state = SND_STOPPED;
    unlock();
    stop_order_tick_count = sound_tick_count;
    AudioQueueStop( queue, true );
    prepare_to_play();
  }
  else
  {
    unlock();
  }
}

bool SoundPlayer::is_playing()
{
  if (state == SND_STOPPING)
  {
    // Kludge to fix apparent bug where the playstate change listener won't
    // always be called after AudioQueueStop().
    if (sound_tick_count - stop_order_tick_count >= 300)
    {
      state = SND_STOPPED;
      prepare_to_play();
    }
  }

  return !(state == SND_STOPPED || state == SND_PAUSED);
}

bool SoundPlayer::is_paused()
{
  return state == SND_PAUSED;
}

void SoundPlayer::set_volume( double p )
{
  if (error()) return;
  volume = p;
  AudioQueueSetParameter( queue, kAudioQueueParam_Volume, volume );
}

double SoundPlayer::get_current_time()
{
  if (is_paused() || is_playing())
  {
    AudioTimeStamp timestamp;
    AudioQueueGetCurrentTime( queue, NULL, &timestamp, NULL );
    double t = timestamp.mSampleTime;
    t /= data_format.mSampleRate;
    if (t < 0.0) t = 0.0;
    else if (t > duration) 
    {
      // mod with duration
      t -= floor(t/duration)*duration;
    }
    return t;
  }
  else
  {
    return start_time;
  }
}

void SoundPlayer::set_current_time( double seconds )
{
  if (error()) return;
  if (seconds < 0.0) seconds = 0.0;
  else if (seconds > duration) seconds = duration;

  start_time = seconds;

  if (is_paused())
  {
    stop();
  }
  else if (is_playing())
  {
    stop();
    play();
  }
}

void SoundPlayer::on_buffer_finished( void* user_data, AudioQueueRef queue, 
    AudioQueueBufferRef buffer )
{
  lock();

  SoundPlayer *THIS = (SoundPlayer *)user_data;

  if ( THIS->is_playing() && !(THIS->error() || THIS->shutting_down) )
  {
    if (THIS->repeats)
    {
      AudioQueueEnqueueBuffer( THIS->queue, buffer, THIS->num_packets, THIS->packet_descriptions );
    }
    else
    {
      THIS->state = SND_REQUEST_STOP;
      THIS->next_sound_requiring_update = sounds_requiring_update;
      sounds_requiring_update = THIS;
    }
  }

  unlock();
}

void SoundPlayer::on_playstate_change( void* user_data, AudioQueueRef queue, AudioQueuePropertyID property_id )
{
  lock();

  SoundPlayer *THIS = (SoundPlayer *)user_data;
  if ( !(THIS->error() || THIS->shutting_down) )
  {
    UInt32 p;
    UInt32 sizeof_p = sizeof(p);
    AudioQueueGetProperty( queue, property_id, &p, &sizeof_p );

    if ( !p ) // don't care about play starting, only stopping
    {
      if (THIS->state == SND_STOPPING)
      {
        THIS->state = SND_FINISHED;
        THIS->next_sound_requiring_update = sounds_requiring_update;
        sounds_requiring_update = THIS;
      }
    }
  }

  unlock();
}


