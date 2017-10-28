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

#ifndef AHL_POLICY_UTILS_INCLUDE
#define AHL_POLICY_UTILS_INCLUDE

#include "ahl-binding.h"

void Add_Endpoint_Property_Double( EndpointInfoT * io_pEndpointInfo, char * in_pPropertyName, double in_dPropertyValue);
void Add_Endpoint_Property_Int( EndpointInfoT * io_pEndpointInfo, char * in_pPropertyName, int in_iPropertyValue);
void Add_Endpoint_Property_String( EndpointInfoT * io_pEndpointInfo, char * in_pPropertyName, const char * in_pPropertyValue);

int PolicyCtxStructToJSON(AHLPolicyCtxT * pPolicyCtx, json_object **ppPolicyCxtJ);
int PolicyCtxJSONToStruct(json_object *CtxJ, int *pNbAudioRole,GPtrArray *pHALList);
int PolicyEndpointStructToJSON(EndpointInfoT * pPolicyEndpoint, json_object **ppPolicyEndpointJ);
int PolicyCtxJSONToEndpoint(json_object *pEndpointJ, EndpointInfoT * pPolicyStream);
int PolicyStreamStructToJSON(StreamInfoT * pPolicyStream, json_object **ppPolicyStreamJ);
int PolicyCtxJSONToStream(json_object *pStreamJ, StreamInfoT * pPolicyStream);

#endif // AHL_POLICY_UTILS_INCLUDE
