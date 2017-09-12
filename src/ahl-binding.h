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

/////////////// Binding private information //////////////////

#define AUDIOHL_MAX_DEVICE_URI_LENGTH 256
#define AUDIOHL_MAX_DEVICE_NAME_LENGTH 256
#define AUDIOHL_MAX_AUDIOROLE_LENGTH 128
#define AUDIOHL_MAX_HALAPINAME_LENGTH 64
#define AUDIOHL_POLICY_ACCEPT 1
#define AUDIOHL_POLICY_REJECT 0

typedef struct EndpointInfo
{
    endpointID_t    endpointID;     // Unique endpoint ID (per type)
    EndpointTypeT   type;           // Source or sink device
    char            deviceName[AUDIOHL_MAX_DEVICE_NAME_LENGTH];   // Device name for applications to display
    char            deviceURI[AUDIOHL_MAX_DEVICE_URI_LENGTH];     // Associated URI 
    DeviceURITypeT  deviceURIType;  // Device URI type (includes audio domain information)
    char            audioRole[AUDIOHL_MAX_AUDIOROLE_LENGTH];     // Audio role that registered this endpoint -> private
    char            halAPIName[AUDIOHL_MAX_AUDIOROLE_LENGTH];    // HAL associated with the device (for volume control) -> private
    int             cardNum;                                     // HW card number -> private
    int             deviceNum;                                   // HW device number -> private                                   
    int             subDeviceNum;                                // HW sub device number -> private
    // Cached endpoint properties
    GHashTable *    pStatesHT;                                   // Keep all known states in key value pairs
} EndpointInfoT;

typedef struct StreamInfo {
    streamID_t      streamID;
    EndpointInfoT   endpointInfo;
} StreamInfoT;

// Parts of the context that are visible to the policy (for state based decisions)
typedef struct AHLPolicyCtx {
    GPtrArray *     pSourceEndpoints; // Array of source end points for each audio role (GArray*)
    GPtrArray *     pSinkEndpoints;   // Array of sink end points for each audio role (GArray*)
    GArray *        pRolePriority;    // List of role priorities (int).  TODO: Should be hash table with role name as key
    GArray *        pAudioRoles;      // List of audio roles (GString)
    GArray *        pActiveStreams;   // List of active streams (StreamInfoT)
    int             iNumberRoles;     // Number of audio roles from configuration   
    // TODO: Global properties   -> exposed to policy
} AHLPolicyCtxT;

// Global binding context
typedef struct AHLCtx {
    AHLPolicyCtxT   policyCtx;
    endpointID_t    nextSourceEndpointID;       // Counter to assign new ID
    endpointID_t    nextSinkEndpointID;         // Counter to assign new ID
    endpointID_t    nextStreamID;               // Counter to assign new ID
    GArray *        pHALList;                   // List of HAL dependencies
    GHashTable *    pDefaultStatesHT;           // List of states and default values known to configuration
} AHLCtxT;

PUBLIC int AhlBindingInit();
// ahl-deviceenum.c
PUBLIC int  EnumerateSources(json_object * in_jSourceArray, int in_iRoleIndex, char * in_pRoleName);
PUBLIC int  EnumerateSinks(json_object * in_jSinkArray, int in_iRoleIndex, char * in_pRoleName);
// ahl-config.c
PUBLIC int  ParseHLBConfig();
// ahl-policy.c
PUBLIC int  Policy_OpenStream(char *pAudioRole, EndpointTypeT endpointType, endpointID_t endpointID);
PUBLIC int  Policy_SetVolume(EndpointTypeT endpointType, endpointID_t endpointID, char *volumeStr, int rampTimeMS);
PUBLIC int  Policy_SetProperty(EndpointTypeT endpointType, endpointID_t endpointID, char *propertyName, char *propValueStr, int rampTimeMS);
PUBLIC int  Policy_SetState(EndpointTypeT endpointType, endpointID_t endpointID, char *pStateName, char *pStateValue);
PUBLIC int  Policy_PostSoundEvent(char *eventName, char *audioRole, char *mediaName, void *audioContext);
PUBLIC int  Policy_AudioDeviceChange();

#define AUDIOHL_MAXHALCONTROLNAME_LENGTH 128

#endif // AHL_BINDING_INCLUDE
