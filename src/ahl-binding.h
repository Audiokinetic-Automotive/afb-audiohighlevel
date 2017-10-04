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
#include <glib.h>

#include "ahl-interface.h"

#ifndef PUBLIC
  #define PUBLIC
#endif

#define AHL_POLICY_ACCEPT 1
#define AHL_POLICY_REJECT 0

#define AHL_UNDEFINED -1

typedef int endpointID_t;
typedef int streamID_t;

typedef enum EndpointSelectionMode {
    AHL_ENDPOINTSELMODE_AUTO = 0,          // Automatic endpoint selection based on config priority
    AHL_ENDPOINTSELMODE_MANUAL,             // Explicit endpoint selection
    AHL_ENDPOINTSELMODEMAXVALUE,            // Enum count, keep at the end
} EndpointSelectionModeT;

typedef struct AudioFormat {
    int         sampleRate;     // Sample rate
    int         numChannels;    // Number of channels
    SampleTypeT sampleType;     // Sample type
    // TODO: Interleaving?
    // TODO: Sample sub format?
} AudioFormatT;

typedef struct AlsaDeviceInfo {
    int             cardNum;            // HW card number
    int             deviceNum;          // HW device number                                 
    int             subDeviceNum;       // HW sub device number
} AlsaDeviceInfoT;

typedef struct EndpointInfo
{
    endpointID_t    endpointID;         // Unique endpoint ID (per type)
    EndpointTypeT   type;               // Source or sink device
    GString *       gsDeviceName;       // Device name for applications to display
    GString *       gsDeviceURI;        // Associated URI 
    DeviceURITypeT  deviceURIType;      // Device URI type (includes audio domain information)
    GString *       gsAudioRole;        // Audio role that registered this endpoint
    GString *       gsHALAPIName;       // HAL associated with the device (for volume control)
    AlsaDeviceInfoT alsaInfo;           // ALSA specific device information
    AudioFormatT    format;             // Preferred audio format supported (later could be array of supported formats)
    int             iVolume;            // Storage for current endpoint volume (policy effected). Target volume during ramping?
    GHashTable *    pPropTable;         // Storage for array of properties (policy effected)         
} EndpointInfoT;

typedef struct StreamInfo {
    streamID_t      streamID;           // Stream unique ID
    EndpointInfoT * pEndpointInfo;      // Associated endpoint information
    StreamStateT   streamState;         // Stream activity state
    StreamMuteT     streamMute;         // Stream mute state
    struct afb_event streamStateEvent;  // Stream specific event for stream state changes
    EndpointSelectionModeT endpointSelMode; // Automatic (priority based) or manual endpoint selection
} StreamInfoT;

// Parts of the context that are visible to the policy (for state based decisions)
typedef struct AHLPolicyCtx {
    GPtrArray *     pSourceEndpoints; // Array of source end points for each audio role (GArray*)
    GPtrArray *     pSinkEndpoints;   // Array of sink end points for each audio role (GArray*)
    GPtrArray *     pEventList;       // Event list per audio roles (GArray*)
    GHashTable *    pRolePriority;    // List of role priorities (int). 
    GArray *        pInterruptBehavior;    // List of interrupt behavior per audio role (int/enum). 
    GArray *        pAudioRoles;      // List of audio roles (GString)
    GArray *        pActiveStreams;   // List of active streams (StreamInfoT)
    int             iNumberRoles;     // Number of audio roles from configuration
    struct afb_event propertyEvent;   // AGL event used when property changes
    struct afb_event volumeEvent;     // AGL event used when volume changes
    struct afb_event postEvent;       // AGL event used on post event call
} AHLPolicyCtxT;

// Global binding context
typedef struct AHLCtx {
    AHLPolicyCtxT   policyCtx;
    endpointID_t    nextSourceEndpointID;       // Counter to assign new ID
    endpointID_t    nextSinkEndpointID;         // Counter to assign new ID
    endpointID_t    nextStreamID;               // Counter to assign new ID
    GArray *        pHALList;                   // List of HAL dependencies
} AHLCtxT;

// ahl-binding.c
PUBLIC int AhlBindingInit();
PUBLIC void AhlOnEvent(const char *evtname, json_object *eventJ);

// ahl-deviceenum.c
int  EnumerateSources(json_object * in_jSourceArray, int in_iRoleIndex, char * in_pRoleName);
int  EnumerateSinks(json_object * in_jSinkArray, int in_iRoleIndex, char * in_pRoleName);
void TermEndpoints();
// ahl-config.c
int  ParseHLBConfig();
// ahl-policy.c
int  Policy_Endpoint_Property_Init(EndpointInfoT * io_pEndpointInfo);
int  Policy_Init();
void Policy_Term(); 
int  Policy_OpenStream(char *pAudioRole, EndpointTypeT endpointType, endpointID_t endpointID);
int  Policy_CloseStream(streamID_t streamID);
int  Policy_SetStreamState(streamID_t streamID, StreamStateT streamState );
int  Policy_SetStreamMute(streamID_t streamID, StreamMuteT streamMute);
int  Policy_SetVolume(EndpointTypeT endpointType, endpointID_t endpointID, char *volumeStr);
int  Policy_SetProperty(EndpointTypeT endpointType, endpointID_t endpointID, char *propertyName, char *propValueStr);
int  Policy_PostEvent(char *eventName, char *audioRole, char *mediaName, void *audioContext);
int  Policy_AudioDeviceChange();



#endif // AHL_BINDING_INCLUDE
