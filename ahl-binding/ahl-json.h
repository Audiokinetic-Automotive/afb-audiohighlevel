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

#ifndef AHL_POLICY_JSON_INCLUDE
#define AHL_POLICY_JSON_INCLUDE

int EndpointInfoToJSON(EndpointInfoT * pEndpointInfo, json_object **ppEndpointInfoJ);
int StreamInfoToJSON(StreamInfoT * pStreamInfo, json_object **ppStreamInfoJ);
int UpdateEndpointInfo(EndpointInfoT * pEndpoint,json_object * pEndpointInfoJ);
void JSONPublicPackageEndpoint(EndpointInfoT * pEndpointInfo,json_object **endpointInfoJ);
void JSONPublicPackageStream(StreamInfoT * pStreamInfo,json_object **streamInfoJ);

#endif // AHL_POLICY_JSON_INCLUDE
