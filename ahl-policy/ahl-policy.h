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