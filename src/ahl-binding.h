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


#ifndef AHL_BINDING_INCLUDE
#define AHL_BINDING_INCLUDE

#define AFB_BINDING_VERSION 2
#include <afb/afb-binding.h>
#include <json-c/json.h>

#ifndef PUBLIC
  #define PUBLIC
#endif

#define UNDEFINED_ID -1

typedef int endpointID_t;
typedef int streamID_t;
typedef int routingID_t;

typedef enum EndpointType {
    ENDPOINTTYPE_SOURCE = 0,    // source devices
    ENDPOINTTYPE_SINK,          // sink devices
    ENDPOINTTYPE_MAXVALUE       // Enum count, keep at the end
} EndpointTypeT;

typedef enum AudioRole {
    AUDIOROLE_WARNING = 0,      // Safety-relevant or critical alerts/alarms
    AUDIOROLE_GUIDANCE,         // Important user information where user action is expected (e.g. navigation instruction) 
    AUDIOROLE_NOTIFICATION,     // HMI or else notifications (e.g. touchscreen events, speech recognition on/off,...)
    AUDIOROLE_COMMUNICATIONS,   // Voice communications (e.g. handsfree, speech recognition)
    AUDIOROLE_ENTERTAINMENT,    // Multimedia content (e.g. tuner, media player, etc.)
    AUDIOROLE_SYSTEM,           // System level content
    AUDIOROLE_DEFAULT,          // No specific audio role (legacy applications)
    AUDIOROLE_MAXVALUE          // Enum count, keep at the end
} AudioRoleT;

typedef enum AudioDeviceClass {
    AUDIODEVICE_SPEAKERMAIN = 0,
    AUDIODEVICE_SPEAKERHEADREST,
    AUDIODEVICE_HEADSET,
    AUDIODEVICE_HEADPHONE,
    AUDIODEVICE_LINEOUT,
    AUDIODEVICE_LINEIN,
    AUDIODEVICE_BLUETOOTH,
    AUDIODEVICE_HANDSET,
    AUDIODEVICE_HDMI,
    AUDIODEVICE_USB,
    AUDIODEVICE_TONES,
    AUDIODEVICE_VOICE,
    AUDIODEVICE_PHONELINK,
    AUDIODEVICE_DEFAULT,
    AUDIODEVICE_MAXVALUE // Enum count, keep at the end
} AudioDeviceClassT;

typedef struct EndpointInfo
{
    endpointID_t    endpoint_id;
    EndpointTypeT   type;
    char *          name;
    // TODO: Consider adding associated device class
} EndpointInfoT;

typedef struct StreamInfo {
    streamID_t      stream_id;
    char *          pcm_name;
    EndpointInfoT   endpoint_info;
} StreamInfoT;

typedef struct RoutingInfo {
    routingID_t     routing_id;
    endpointID_t    source_id;
    endpointID_t    sink_id;
} RoutingInfoT;

PUBLIC int AhlBindingInit();
// ahl-deviceenum.c
PUBLIC int  EnumerateSources();
PUBLIC int  EnumerateSinks();

#endif
