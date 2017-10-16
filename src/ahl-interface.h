/*
 * Copyright (C) 2017 "Audiokinetic Inc"
 * Author Francois Thibault <fthibault@audiokinetic.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AHL_INTERFACE_INCLUDE
#define AHL_INTERFACE_INCLUDE

typedef enum EndpointType {
    ENDPOINTTYPE_SOURCE = 0,        // source devices
    ENDPOINTTYPE_SINK,              // sink devices
    ENDPOINTTYPE_MAXVALUE           // Enum count, keep at the end
} EndpointTypeT;

typedef enum DeviceURIType {
    DEVICEURITYPE_ALSA_HW = 0,  // Alsa hardware device URI
    DEVICEURITYPE_ALSA_DMIX,    // Alsa Dmix device URI (only for playback devices)
    DEVICEURITYPE_ALSA_DSNOOP,  // Alsa DSnoop device URI (only for capture devices)
    DEVICEURITYPE_ALSA_SOFTVOL, // Alsa softvol device URI
    DEVICEURITYPE_ALSA_PLUG,    // Alsa plug device URI
    DEVICEURITYPE_ALSA_OTHER,   // Alsa domain URI device of unspecified type
    DEVICEURITYPE_PULSE,        // Pulse device URI
    DEVICEURITYPE_GSTREAMER,    // GStreamer device URI
    DEVICEURITYPE_EXTERNAL,     // Device URI for external ECU device
    DEVICEURITYPE_MAXVALUE      // Enum count, keep at the end
} DeviceURITypeT;

typedef enum StreamState {
    STREAM_STATE_IDLE  = 0,    // Stream is inactive
    STREAM_STATE_RUNNING,      // Stream is active and running    
    STREAM_STATE_PAUSED,       // Stream is active but paused
    STREAM_STATE_MAXVALUE      // Enum count, keep at the end
} StreamStateT;

typedef enum StreamMute {
    STREAM_UNMUTED = 0,         // Stream is not muted
    STREAM_MUTED,               // Stream is muted
    STREAM_MUTE_MAXVALUE,       // Enum count, keep at the end
} StreamMuteT;

typedef enum StreamEvent {
    STREAM_EVENT_START  = 0,   // Stream is inactive
    STREAM_EVENT_STOP,         // Stream is running    
    STREAM_EVENT_PAUSE,        // Audio stream paused
    STREAM_EVENT_RESUME,       // Audio stream resumed
    STREAM_EVENT_MUTED,        // Audio stream muted
    STREAM_EVENT_UNMUTED,      // Audio stream unmuted
    STREAM_STATUS_MAXVALUE     // Enum count, keep at the end
} StreamEventT;

// Define default behavior of audio role when interrupted by higher priority sources (in configuration)
#define AHL_INTERRUPTEDBEHAVIOR_CONTINUE_STR "continue" // Continue to play when interrupted (e.g. media may be ducked)
#define AHL_INTERRUPTEDBEHAVIOR_CANCEL_STR "cancel"     // Abort playback when interrupted (e.g. non-important HMI feedback that does not make sense later)
#define AHL_INTERRUPTEDBEHAVIOR_PAUSE_STR "pause"       // Pause source when interrupted, to be resumed afterwards (e.g. non-temporal guidance)

#define AHL_ENDPOINT_PROPERTY_EVENT "ahl_endpoint_property_event"
#define AHL_ENDPOINT_VOLUME_EVENT "ahl_endpoint_volume_event"
#define AHL_POST_EVENT "ahl_post_event"

// CPU endianness assumed in all formats
typedef enum SampleType {
  AHL_FORMAT_UNKNOWN = -1,              // Unknown
  AHL_FORMAT_U8 = 0,                    // Unsigned 8 bit
  AHL_FORMAT_S16,                       // Signed 16 bit Little Endian
  AHL_FORMAT_S24,                       // Signed 24 bit Little Endian using low three bytes in 32-bit word
  AHL_FORMAT_S32,                       // Signed 32 bit Little Endian
  AHL_FORMAT_FLOAT,                     // Float 32 bit Little Endian, Range -1.0 to 1.0
  AHL_FORMAT_FLOAT64,                   // Float 64 bit Little Endian, Range -1.0 to 1.0
  AHL_FORMAT_IEC958,                    // IEC-958 Little Endian (SPDIF)
  AHL_FORMAT_MU_LAW,                    // Mu-Law
  AHL_FORMAT_A_LAW,                     // A-Law
  AHL_FORMAT_IMA_ADPCM,                 // Ima-ADPCM
  AHL_FORMAT_MPEG,                      // MPEG
  AHL_FORMAT_GSM,                       // GSM
  AHL_FORMAT_G723,                      // G723
  AHL_FORMAT_DSD,                       // Direct stream digital
  AHL_FORMAT_MAXVALUE,                  // Enum count, keep at the end
} SampleTypeT;

// Known audio domain string definitions (for configuration file format and device URI interpretation)
#define AHL_DOMAIN_ALSA "alsa"
#define AHL_DOMAIN_PULSE "pulse"
#define AHL_DOMAIN_GSTREAMER "gstreamer"
#define AHL_DOMAIN_EXTERNAL "external"

// Standardized name for common audio roles (not enforced in any way, just helps compatibility)
#define AHL_ROLE_WARNING "warning"             // Safety-relevant or critical alerts/alarms
#define AHL_ROLE_GUIDANCE "guidance"           // Important user information where user action is expected (e.g. navigation instruction)
#define AHL_ROLE_NOTIFICATION "notification"   // HMI or else notifications (e.g. touchscreen events, speech recognition on/off,...)
#define AHL_ROLE_COMMUNICATION "communication" // Voice communications (e.g. handsfree, speech recognition)
#define AHL_ROLE_ENTERTAINMENT "entertainment"  // Multimedia content (e.g. tuner, media player, etc.)
#define AHL_ROLE_SYSTEM "system"               // System level content or development
#define AHL_ROLE_STARTUP "startup"             // Early (startup) sound
#define AHL_ROLE_SHUTDOWN "shutdown"           // Late (shutdown) sound
#define AHL_ROLE_NONE "none"                   // Non-assigned / legacy applications

// Standardized list of properties (not enforced in any way, just helps compatibility)
#define AHL_PROPERTY_BALANCE "balance"
#define AHL_PROPERTY_FADE "fade"
#define AHL_PROPERTY_EQ_LOW "eq_bass" 
#define AHL_PROPERTY_EQ_MID "eq_mid" 
#define AHL_PROPERTY_EQ_HIGH "eq_treble"

// Standardized list of events (not enforced in any way, just helps compatibility)
#define AHL_EVENTS_PLAYSOUND "play_sound"
#define AHL_EVENTS_ECHOCANCEL_ENABLE "echocancel_enable"
#define AHL_EVENTS_ECHOCANCEL_DISABLE "echocancel_disable"


#endif // AHL_INTERFACE_INCLUDE
