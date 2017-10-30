/*
 * Copyright (C) 2017 "Audiokinetic Inc"
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

#ifndef AHL_POLICY_INCLUDE
#define AHL_POLICY_INCLUDE
#include "ahl-policy-utils.h"

#ifndef AHL_DISCONNECT_POLICY

#define MAX_ACTIVE_STREAM_POLICY 30
#define POLICY_FAIL     1
#define POLICY_SUCCESS  0

#define AHL_POLICY_UNDEFINED_HALNAME "UNDEFINED"
#define AHL_POLICY_UNDEFINED_DISPLAYNAME "DeviceNotFound"

typedef enum SystemState {
    SYSTEM_STARTUP = 0,     // Startup
    SYSTEM_SHUTDOWN,        // ShutDown
    SYSTEM_NORMAL,          // Normal
    SYSTEM_LOW_POWER,       // Low Power, save mode
    SYSTEM_MAXVALUE         // Enum count, keep at the end
} SystemStateT;


typedef struct HalInfo {
    char *pDevID;
    char *pAPIName;
    char *pDisplayName;
} HalInfoT;

typedef struct StreamConfig {
    int iNbMaxStream;
    int iVolumeInit;
    int iVolumeDuckValue;
} StreamConfigT;

// Global Policy Local context
typedef struct PolicyLocalCtx {
    GArray *     pSourceEndpoints; // List of Source Endpoint with playing stream or interrupted stream
    GArray *     pSinkEndpoints;   // List of Sink Endpoint with playing stream or interrupted stream
    GPtrArray *  pHALList;
    SystemStateT systemState;
} PolicyLocalCtxT;

int  Policy_Endpoint_Init(json_object *pPolicyEndpointJ);
int  Policy_OpenStream(json_object *pPolicyStreamJ);
int  Policy_CloseStream(json_object *pPolicyStreamJ);
int  Policy_SetStreamState(json_object *pPolicyStreamJ);
int  Policy_SetStreamMute(json_object *pPolicyStreamJ);
int  Policy_PostAction(json_object *pPolicyActionJ);
int  Policy_SetVolume(json_object *pPolicyEndpointJ);
int  Policy_SetProperty(json_object *pPolicyEndpointJ);
int  Policy_Init();
void Policy_Term(); 
void Policy_OnEvent(const char *evtname, json_object *eventJ);

extern void audiohlapi_raise_event(json_object * pEventDataJ);
#endif // AHL_DISCONNECT_POLICY
#endif // AHL_POLICY_INCLUDE