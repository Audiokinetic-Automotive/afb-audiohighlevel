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

#define UNDEFINED_ID -1

typedef int endpointID_t;
typedef int streamID_t;

typedef enum EndpointType {
    ENDPOINTTYPE_SOURCE = 0,        // source devices
    ENDPOINTTYPE_SINK,              // sink devices
    ENDPOINTTYPE_MAXVALUE           // Enum count, keep at the end
} EndpointTypeT;

// Standardized name for common audio roles (not enforced in any way, just helps system being more compatible)
#define AUDIOROLE_WARNING "warning"             // Safety-relevant or critical alerts/alarms
#define AUDIOROLE_GUIDANCE "guidance"           // Important user information where user action is expected (e.g. navigation instruction)
#define AUDIOROLE_NOTIFICATION "notification"   // HMI or else notifications (e.g. touchscreen events, speech recognition on/off,...)
#define AUDIOROLE_COMMUNICATION "communications" // Voice communications (e.g. handsfree, speech recognition)
#define AUDIOROLE_ENTERTAINMENT "entertainment"  // Multimedia content (e.g. tuner, media player, etc.)
#define AUDIOROLE_SYSTEM "system"               // System level content or development
#define AUDIOROLE_STARTUP "startup"             // Early (startup) sound
#define AUDIOROLE_SHUTDOWN "shutdown"           // Late (shutdown) sound
#define AUDIOROLE_NONE "none"                   // Non-assigned / legacy applications

typedef enum DeviceURIType {
    DEVICEURITYPE_ALSA_HW = 0,  // Alsa hardware device URI
    DEVICEURITYPE_ALSA_DMIX,    // Alsa Dmix device URI (only for playback devices)
    DEVICEURITYPE_ALSA_DSNOOP,  // Alsa DSnoop device URI (only for capture devices)
    DEVICEURITYPE_ALSA_SOFTVOL, // Alsa softvol device URI
    DEVICEURITYPE_ALSA_OTHER,   // Alsa domain URI device of unspecified type
    DEVICEURITYPE_PULSE,        // Pulse device URI
    DEVICEURITYPE_GSTREAMER,    // GStreamer device URI
    DEVICEURITYPE_EXTERNAL,     // Device URI for external ECU device
    DEVICEURITYPE_MAXVALUE      // Enum count, keep at the end
} DeviceURITypeT;

// Standardized list of properties (string used for extensibility)
#define AUDIOHL_PROPERTY_BALANCE "balance"
#define AUDIOHL_PROPERTY_FADE "fade"
#define AUDIOHL_PROPERTY_EQ_LOW "eq_low" 
#define AUDIOHL_PROPERTY_EQ_MID "eq_mid" 
#define AUDIOHL_PROPERTY_EQ_HIGH "eq_high"

// Standardized list of state names/values (string used for extensibility)
#define AUDIOHL_STATE_NAME_ACTIVE "active"
#define AUDIOHL_STATE_NAME_MUTE "mute" 
#define AUDIOHL_STATE_VALUE_ON "on"
#define AUDIOHL_STATE_VALUE_OFF "off" 

// Known audio domain string definitions (for configuration file format)
#define AUDIOHL_DOMAIN_ALSA "Alsa"
#define AUDIOHL_DOMAIN_PULSE "Pulse"
#define AUDIOHL_DOMAIN_GSTREAMER "GStreamer"
#define AUDIOHL_DOMAIN_EXTERNAL "External"

#endif // AHL_INTERFACE_INCLUDE
