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

#define AFB_BINDING_VERSION 2
#include <afb/afb-binding.h>

#include "ahl-policy-utils.h"
#include "wrap-json.h"
#include <json-c/json.h>

void Add_Endpoint_Property_Double( json_object * io_pPropertyArray, char * in_pPropertyName, double in_dPropertyValue)
{
    json_object * pPropertyJ = NULL;
    wrap_json_pack(&pPropertyJ, "{s:s,s:o}",
                        "property_name", in_pPropertyName, 
                        "property_value", json_object_new_double(in_dPropertyValue)                  
                        );
    json_object_array_add(io_pPropertyArray, pPropertyJ);
}

void Add_Endpoint_Property_Int( json_object * io_pPropertyArray, char * in_pPropertyName, int in_iPropertyValue)
{
    json_object * pPropertyJ = NULL;
    wrap_json_pack(&pPropertyJ, "{s:s,s:o}",
                        "property_name", in_pPropertyName, 
                        "property_value", json_object_new_int(in_iPropertyValue)                  
                        );
    json_object_array_add(io_pPropertyArray, pPropertyJ);
}

void Add_Endpoint_Property_String( json_object * io_pPropertyArray, char * in_pPropertyName, const char * in_pPropertyValue)
{
    json_object * pPropertyJ = NULL;
    wrap_json_pack(&pPropertyJ, "{s:s,s:o}",
                        "property_name", in_pPropertyName, 
                        "property_value", json_object_new_string(in_pPropertyValue)                 
                        );
    json_object_array_add(io_pPropertyArray, pPropertyJ);
}

int EndpointToJSON(EndPointInterfaceInfoT * pEndpoint, json_object **ppEndpointJ)
{
    if(ppEndpointJ == NULL || pEndpoint == NULL)
    {
        AFB_ERROR("Invalid EndpointToJSON arguments");
        return AHL_POLICY_UTIL_FAIL;
    } 
    
    // Create json object for Endpoint
    int err = wrap_json_pack(ppEndpointJ, "{s:i,s:i,s:s,s:s,s:s,s:s,s:s,s:i,s:s,s:i,s:i,s:i,s:i,s:i,s:i,s:i,s?o}",
                    "endpoint_id", pEndpoint->endpointID, 
                    "endpoint_type", pEndpoint->type,
                    "device_name", pEndpoint->gsDeviceName, 
                    "display_name", pEndpoint->gsDisplayName, 
                    "device_uri",  pEndpoint->gsDeviceURI,
                    "device_domain", pEndpoint->gsDeviceDomain,
                    "audio_role",pEndpoint->pRoleName,
                    "device_uri_type", pEndpoint->deviceURIType,
                    "hal_api_name", pEndpoint->gsHALAPIName,
                    "alsa_cardNum", pEndpoint->alsaInfo.cardNum, 
                    "alsa_deviceNum", pEndpoint->alsaInfo.deviceNum, 
                    "alsa_subDeviceNum", pEndpoint->alsaInfo.subDeviceNum,
                    "format_samplerate", pEndpoint->format.sampleRate,
                    "format_numchannels", pEndpoint->format.numChannels,
                    "format_sampletype",pEndpoint->format.sampleType,
                    "volume", pEndpoint->iVolume,
                    "property_table", pEndpoint->pPropTableJ
                    );
    if (err) {
        AFB_ERROR("Unable to pack JSON endpoint, =%s", wrap_json_get_error_string(err));
        return AHL_POLICY_UTIL_FAIL;
    }    
    AFB_DEBUG("JSON endpoint information=%s", json_object_get_string(*ppEndpointJ));
    return AHL_POLICY_UTIL_SUCCESS;
}

int StreamToJSON(StreamInterfaceInfoT * pStream, json_object **ppStreamJ)
{
    if(pStream == NULL)
    {
        AFB_ERROR("Invalid arguments to StreamToJSON, stream structure is NULL");
        return AHL_POLICY_UTIL_FAIL;
    }

    json_object *EndpointJ = NULL;
    int err = EndpointToJSON(&pStream->endpoint, &EndpointJ);
    if (err) {
        AFB_ERROR("Unable to pack JSON endpoint, =%s", wrap_json_get_error_string(err));
        return AHL_POLICY_UTIL_FAIL;
    }

    // Create json object for stream
    err = wrap_json_pack(ppStreamJ, "{s:i,s:i,s:i,s:s,s:i,s:i,s:o}",
                    "stream_id", pStream->streamID, 
                    "stream_state", pStream->streamState,
                    "stream_mute", pStream->streamMute, 
                    "role_name", pStream->pRoleName,
                    "priority", pStream->iPriority,
                    "interrupt_behavior", pStream->eInterruptBehavior,
                    "endpoint_info", EndpointJ
                    );
    if (err) {
        AFB_ERROR("Unable to pack JSON Stream, =%s", wrap_json_get_error_string(err));
        return AHL_POLICY_UTIL_FAIL;
    }

    AFB_DEBUG("JSON stream information=%s", json_object_get_string(*ppStreamJ));

    return AHL_POLICY_UTIL_SUCCESS;
}

//pEndpointInterfaceInfo must be pre-allocated by the caller
int JSONToEndpoint(json_object *pEndpointJ, EndPointInterfaceInfoT *pEndpoint)
{
    if(pEndpointJ == NULL || pEndpoint == NULL)
    {
        AFB_ERROR("Invalid arguments for JSONToEndpoint");
        return AHL_POLICY_UTIL_FAIL;
    }

    //Unpack Endpoint
    int err = wrap_json_unpack(pEndpointJ, "{s:i,s:i,s:s,s:s,s:s,s:s,s:s,s:i,s:s,s:i,s:i,s:i,s:i,s:i,s:i,s:i,s?o}",
                    "endpoint_id", &pEndpoint->endpointID, 
                    "endpoint_type", &pEndpoint->type,
                    "device_name", &pEndpoint->gsDeviceName, 
                    "display_name", &pEndpoint->gsDisplayName, 
                    "device_uri",  &pEndpoint->gsDeviceURI,
                    "device_domain", &pEndpoint->gsDeviceDomain,
                    "audio_role", &pEndpoint->pRoleName,
                    "device_uri_type", &pEndpoint->deviceURIType,
                    "hal_api_name", &pEndpoint->gsHALAPIName,
                    "alsa_cardNum", &pEndpoint->alsaInfo.cardNum, 
                    "alsa_deviceNum", &pEndpoint->alsaInfo.deviceNum, 
                    "alsa_subDeviceNum", &pEndpoint->alsaInfo.subDeviceNum,
                    "format_samplerate", &pEndpoint->format.sampleRate,
                    "format_numchannels", &pEndpoint->format.numChannels,
                    "format_sampletype",&pEndpoint->format.sampleType,
                    "volume", &pEndpoint->iVolume,                    
                    "property_table", &pEndpoint->pPropTableJ                    
                    );
    if (err) {
        AFB_ERROR("Unable to unpack JSON endpoint, =%s", wrap_json_get_error_string(err));
        return AHL_POLICY_UTIL_FAIL;
    }
    return AHL_POLICY_UTIL_SUCCESS;
}

int JSONToStream(json_object *pStreamJ, StreamInterfaceInfoT * pStream)
{
    if(pStreamJ == NULL || pStream == NULL)
    {
        AFB_ERROR("Invalid arguments for InterfaceCtxJSONToStream");
        return AHL_POLICY_UTIL_FAIL;
    }

    //Unpack StreamInfo
    json_object *pEndpointJ = NULL;
    AFB_WARNING("json object query=%s", json_object_get_string(pStreamJ));
    int err = wrap_json_unpack(pStreamJ, "{s:i,s:i,s:i,s:s,s:i,s:i,s:o}",
                    "stream_id", &pStream->streamID, 
                    "stream_state", &pStream->streamState,
                    "stream_mute", &pStream->streamMute, 
                    "role_name", &pStream->pRoleName,
                    "priority", &pStream->iPriority,
                    "interrupt_behavior", &pStream->eInterruptBehavior,
                    "endpoint_info", &pEndpointJ
                    );

    if (err) {
        AFB_ERROR("Unable to parse JSON stream information=%s", json_object_get_string(pStreamJ));
        return AHL_POLICY_UTIL_FAIL;
    }

    err = JSONToEndpoint(pEndpointJ,&pStream->endpoint);
    if (err) {
        return AHL_POLICY_UTIL_FAIL;
    }
    return AHL_POLICY_UTIL_SUCCESS;
}
 